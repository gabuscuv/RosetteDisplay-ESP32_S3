idf_component_register(
    SRCS
        "app_init.c"
        "app_state.c"
        "app_display.c"

    INCLUDE_DIRS
        "include"
    
    REQUIRES ui
)