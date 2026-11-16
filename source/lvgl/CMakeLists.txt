cmake_minimum_required(VERSION 3.21)
project(RevontuliLvgl LANGUAGES C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(LVGL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build/lvgl-9.2.2" CACHE PATH "LVGL 9.2.2 source")
set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON)
set(LV_CONF_BUILD_DISABLE_DEMOS ON)
set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL ON)
add_subdirectory("${LVGL_SOURCE_DIR}" lvgl)
add_executable(revontuli-lvgl main.cpp)
target_link_libraries(revontuli-lvgl PRIVATE lvgl)
if(EMSCRIPTEN)
    file(GLOB_RECURSE demo_assets CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/assets/*"
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")
    set_property(TARGET revontuli-lvgl APPEND PROPERTY LINK_DEPENDS "${demo_assets}")
    target_link_options(revontuli-lvgl PRIVATE
        "-sALLOW_MEMORY_GROWTH=1" "-sFORCE_FILESYSTEM=1"
        "SHELL:--preload-file \"${CMAKE_CURRENT_SOURCE_DIR}/../mcu/assets@/assets\""
        "SHELL:--preload-file \"${CMAKE_CURRENT_SOURCE_DIR}/assets/right@/assets/right\""
        "SHELL:--preload-file \"${CMAKE_CURRENT_SOURCE_DIR}/assets/skins@/assets/skins\"")
else()
    target_link_libraries(revontuli-lvgl PRIVATE user32 gdi32 imm32)
endif()
if(MINGW)
    target_link_options(revontuli-lvgl PRIVATE -static)
endif()
add_custom_command(TARGET revontuli-lvgl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/assets" "$<TARGET_FILE_DIR:revontuli-lvgl>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:revontuli-lvgl>/assets" VERBATIM)
