idf_component_register(
    SRCS
        "tjpgd/src/tjpgd.c"

    INCLUDE_DIRS
        "tjpgd/src/"

    REQUIRES
        esp_common
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    JD_FORMAT=1
)