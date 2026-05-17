idf_component_register(
    INCLUDE_DIRS "include"
)

target_compile_definitions(${COMPONENT_LIB} INTERFACE
    LV_CONF_INCLUDE_SIMPLE
)