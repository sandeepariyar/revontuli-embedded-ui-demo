# Revontuli · Comfort → Seat

Open the Windows LVGL simulator from the build output. It uses LVGL widgets and the same C++ controller as the Qt Quick Ultralite source. Select Cooling 2, then turn massage on and off. The cooling effect returns, and the other seat retains its own settings. Only Comfort → Seat is implemented in this slice. Saved positions last for the current session.

## Build and verify

Use LVGL 9.2.2 source, CMake, Ninja and a C/C++ compiler. From the lab directory:

```powershell
cmake -S lvgl -B build/lvgl-demo -G Ninja -DCMAKE_BUILD_TYPE=Release -DLVGL_SOURCE_DIR=C:/path/to/lvgl-9.2.2
cmake --build build/lvgl-demo
./build/lvgl-demo/revontuli-lvgl.exe --verify
./build/lvgl-demo/revontuli-lvgl.exe
```

The build copies the artwork and pre-baked mirrored files for the right seat. The local verification checks LVGL button callbacks, confirmed C++ state, and JPEG/PNG image headers. The Qt Quick Ultralite source is in the adjacent mcu directory. Its SDK build is pending.

## Example memory budget

| Resource | Size |
| --- | ---: |
| Original chair and effect files, on disk | 168,201 bytes |
| One 480 × 800 RGB565 framebuffer | 768,000 bytes |
| One 480 × 40 RGB565 partial draw buffer | 38,400 bytes |
| One decoded 240 × 240 RGB565 chair | 115,200 bytes |
| One decoded 90 × 128 RGBA overlay | 46,080 bytes |

The framebuffer and partial buffer represent different buffering choices. Decoded image estimates exclude decoder scratch space, caches, alignment and widget memory. The desktop demo uses a Windows display driver and 32-bit colour. The example budget describes a possible embedded configuration.
