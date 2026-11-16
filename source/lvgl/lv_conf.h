#pragma once
#define LV_CONF_H
#ifndef __EMSCRIPTEN__
#define LV_USE_OS LV_OS_WINDOWS
#define LV_USE_WINDOWS 1
#endif
#define LV_COLOR_DEPTH 32
#define LV_USE_FS_STDIO 1
#define LV_FS_STDIO_LETTER 'A'
#define LV_FS_STDIO_PATH "assets/"
#define LV_FS_STDIO_CACHE_SIZE 0
#define LV_USE_LODEPNG 1
#define LV_USE_TJPGD 1
#define LV_USE_LOG 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_MEM_SIZE (1024U * 1024U)
