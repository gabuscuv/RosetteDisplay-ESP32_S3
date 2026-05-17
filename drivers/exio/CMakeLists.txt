idf_component_register(
    SRCS
        "TCA9554PWR.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        driver
        i2c
)