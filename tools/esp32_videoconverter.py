#!/usr/bin/env python3

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile


FFMPEG = "ffmpeg"
FFPROBE = "ffprobe"

# ------------------------------------------------------------
# Helpers
# ------------------------------------------------------------

def run(cmd):
    print("  $", " ".join(map(str, cmd)))
    subprocess.run(cmd, check=True)


def ffprobe_json(path):
    out = subprocess.check_output([
        FFPROBE, "-v", "error",
        "-print_format", "json",
        "-show_format",
        "-show_streams",
        path
    ])
    return json.loads(out)


def get_duration(path):
    return float(ffprobe_json(path)["format"]["duration"])


def ffprobe_size(path):
    info = ffprobe_json(path)
    for s in info["streams"]:
        if s["codec_type"] == "video":
            return s["width"], s["height"]
    raise RuntimeError("No video stream found")

# ------------------------------------------------------------
# Preprocessing
# ------------------------------------------------------------

def normalize_fps(input_path, fps):
    """Convert video to CFR at given FPS."""
    tmp = tempfile.NamedTemporaryFile(delete=False, suffix=".mov")
    tmp_path = tmp.name
    tmp.close()

    print("\n→ Normalizing to CFR…")

    run([
        FFMPEG, "-y",
        "-i", input_path,
        "-r", str(fps),
        "-fps_mode", "cfr",
        "-pix_fmt", "argb",
        "-c:v", "qtrle",
        "-an",
        tmp_path
    ])

    dur = get_duration(tmp_path)
    print(f"  Duration after CFR: {dur:.3f}s")
    return tmp_path, dur


def extract_first_frame(input_path, fps):
    """Extract only the first frame and wrap it as a 1-frame video."""
    tmp = tempfile.NamedTemporaryFile(delete=False, suffix=".mov")
    tmp_path = tmp.name
    tmp.close()

    print("\n→ Extracting first frame…")

    run([
        FFMPEG, "-y",
        "-i", input_path,
        "-vf", "select=eq(n\\,0),setpts=0",
        "-vframes", "1",
        "-r", str(fps),
        "-pix_fmt", "argb",
        "-c:v", "qtrle",
        "-an",
        tmp_path
    ])

    return tmp_path, 1.0 / fps

# ------------------------------------------------------------
# Filter graph
# ------------------------------------------------------------

def build_filter(iw, ih, dur, args):
    """Scale video into a circle-aware canvas and apply circular alpha mask."""

    target = args.size / args.ratio
    scale_factor = min(target / iw, target / ih)
    sw = int(iw * scale_factor)
    sh = int(ih * scale_factor)
    # 3) Filter graph
    vf = (
        f"[0:v]scale={sw}:{sh},format=rgba[scaled];"
        f"nullsrc=size={args.size}x{args.size}:rate={args.fps}:duration={dur},format=rgba[canvas];"
        f"[canvas][scaled]overlay=(W-w)/2:(H-h)/2:shortest=1[ov];"
        f"[ov]geq="
        f"r='r(X,Y)':"
        f"g='g(X,Y)':"
        f"b='b(X,Y)':"
        f"a='if(lte(hypot(X-W/2,Y-H/2),W/2),255,0)'"
    )


    return vf

# ------------------------------------------------------------
# Postprocessing
# ------------------------------------------------------------

def process_filter(clean_video, vf, args):
    """Apply circular mask filter."""
    tmp = tempfile.NamedTemporaryFile(delete=False, suffix=".mov")
    tmp_path = tmp.name
    tmp.close()

    print("\n→ Applying filter graph...")

    run([
        FFMPEG,
        "-y",
        "-i", clean_video,
        "-filter_complex", vf,
        "-c:v", "qtrle",
        "-pix_fmt", "argb",
        "-an",
        tmp_path
    ])

    return tmp_path

def flatten_on_black(input_video, fps, size):
    """Remove alpha channel by compositing onto black."""
    
    tmp = tempfile.NamedTemporaryFile(
        delete=False,
        suffix=".mp4"
    )
    tmp_path = tmp.name
    tmp.close()

    print("\n→ Flattening alpha onto black...")

    vf = (
        f"color=c=black:s={size}x{size}:r={fps}[bg];"
        f"[bg][0:v]overlay=(W-w):(H-h):shortest=1:format=auto"
    )

    run([
        FFMPEG,
        "-y",
        "-i", input_video,
        "-filter_complex", vf,
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        "-an",
        tmp_path
    ])

    return tmp_path

def build_mjpeg(clean_video, output_file, args):
    with tempfile.TemporaryDirectory() as frames_dir:

        print("\n→ Exporting JPEG frames...")

        run([
            FFMPEG,
            "-y",
            "-i", clean_video,
            "-q:v", str(args.quality),
            os.path.join(frames_dir, "frame_%05d.jpg")
        ])

        oversized = []
        warned = []
        written = 0

        with open(output_file, "wb") as out:

            for fname in sorted(os.listdir(frames_dir)):

                if not fname.endswith(".jpg"):
                    continue

                path = os.path.join(frames_dir, fname)
                size = os.path.getsize(path)

                if args.warn_size < size < args.max_size:
                    warned.append((fname, size))

                if size >= args.max_size:
                    oversized.append((fname, size))
                    continue

                with open(path, "rb") as f:
                    shutil.copyfileobj(f, out)

                written += 1

        print(f"\nFrames written: {written}")

        if warned:
            print("\n⚠ Large frames detected:")
            for fname, size in warned:
                print(
                    f"  {fname}: {size/1024:.1f} KB"
                )

        if oversized:
            print("\n❌ Frames exceeding limit:")
            for fname, size in oversized:
                print(
                    f"  {fname}: {size/1024:.1f} KB SKIPPED"
                )

            print(
                "\nTry increasing JPEG compression "
                "(--quality 10 or 12)"
            )

        print(f"\n✔ MJPEG created: {output_file}")

# ------------------------------------------------------------
# Main pipeline
# ------------------------------------------------------------

def process_video(path, args):
    print("\n==============================")
    print(f"Processing: {path}")
    print("==============================")

    temp_files = []

    def track(f):
        temp_files.append(f)
        return f

    try:
        # Preprocess
        if args.first_frame:
            clean_video, duration = extract_first_frame(args.input, args.fps)
        else:
            clean_video, duration = normalize_fps(args.input, args.fps)
        track(clean_video)

        iw, ih = ffprobe_size(clean_video)
        print(f"  FPS:      {args.fps}")
        print(f"  Duration: {duration:.3f}s")

        # Build filter
        vf = build_filter(iw, ih, duration, args)

        # Apply mask
        clean_video = track(process_filter(clean_video, vf, args))

        # Remove alpha
        clean_video = track(flatten_on_black(clean_video, args.fps, args.size))

        # Output
        if args.preview:
            try:
                subprocess.run(["mpv", clean_video])
                print("→ Preview opened in mpv")
            except FileNotFoundError:
                print("⚠ mpv not found")

        if args.output:
            out_path = args.output
        else:
            name = os.path.splitext(os.path.basename(path))[0]
            out_path = f"output/{name}_circle.mjpeg"
            os.makedirs(os.path.dirname(out_path), exist_ok=True)

        build_mjpeg(clean_video, out_path, args)

    finally:
        for f in temp_files:
            if os.path.exists(f):
                os.unlink(f)

def main():
    parser = argparse.ArgumentParser(
        description="Circular video → raw MJPEG stream for ESP32 players"
    )

    parser.add_argument("input", help="Input video file")
    parser.add_argument("-o", "--output", help="Output MJPEG file")

    parser.add_argument("--ratio", type=float, default=0.56, help="Zoom ratio")
    parser.add_argument("--fps", type=int, default=15, help="Output FPS")
    parser.add_argument("--size", type=int, default=320, help="Output canvas size")

    parser.add_argument("--quality", "-q", type=int, default=8, help="JPEG quality")
    parser.add_argument("--max-size", type=int, default=200 * 1024, help="Max JPEG size")
    parser.add_argument("--warn-size", type=int, default=150 * 1024, help="Warn JPEG size")

    parser.add_argument("--first-frame", action="store_true", help="Use only first frame")
    parser.add_argument("--preview", action="store_true", help="Preview output in mpv")

    args = parser.parse_args()
    process_video(args.input, args)


if __name__ == "__main__":
    main()

