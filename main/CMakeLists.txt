string(TIMESTAMP WATCH_BUILD_EPOCH "%s" UTC)
idf_component_register(
    SRCS "watch_main.c" "watch_clock.c"
    INCLUDE_DIRS "."
    REQUIRES bsp esp_timer
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE WATCH_BUILD_EPOCH=${WATCH_BUILD_EPOCH}LL)
