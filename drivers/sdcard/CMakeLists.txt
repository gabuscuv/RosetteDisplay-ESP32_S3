idf_component_register(
    SRCS
        "SD_MMC.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        spi_flash
        driver
        fatfs
)