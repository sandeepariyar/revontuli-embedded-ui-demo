#include "lvgl.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include "src/drivers/windows/lv_windows_display.h"
#include "src/drivers/windows/lv_windows_input.h"
#include <windows.h>
#endif
#include "../core/controller.hpp"
#include <cstdio>
#include <cstring>
#include <utility>

namespace {
revontuli::Simulator device;
revontuli::Controller control(device);
int seat = 0, frame = 0, effect = 0;
int saved[2] = {-1, -1};
lv_obj_t *chair, *overlay, *caption, *slider, *value, *massage, *status;
lv_obj_t *seatButtons[2], *heat[4], *cool[4];

constexpr unsigned ink = 0xedf1e7, green = 0x3ddc97, panel = 0x151d1e;
void fill(lv_obj_t *object, unsigned color) {
    lv_obj_set_style_bg_color(object, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
}
lv_obj_t *box(lv_obj_t *parent, int x, int y, int w, int h, unsigned color, int radius = 14) {
    auto *object = lv_obj_create(parent);
    lv_obj_remove_style_all(object);
    lv_obj_remove_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(object, x, y);
    lv_obj_set_size(object, w, h);
    lv_obj_set_style_radius(object, radius, 0);
    fill(object, color);
    return object;
}
lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y, unsigned color = ink) {
    auto *object = lv_label_create(parent);
    lv_label_set_text(object, text);
    lv_obj_set_style_text_color(object, lv_color_hex(color), 0);
    lv_obj_set_pos(object, x, y);
    return object;
}
void select(lv_obj_t *button, bool selected, unsigned accent = green) {
    fill(button, selected ? accent : 0x20292a);
    lv_obj_set_style_text_color(lv_obj_get_child(button, 0), lv_color_hex(selected ? 0x092419 : ink), 0);
}
enum Action { None, Seat, Heat, Cool, Massage, Save, Recall, Reset };
struct Command { Action action; int value; };
void click(lv_event_t *event);
lv_obj_t *button(lv_obj_t *parent, const char *text, int x, int y, int width,
                 Action action = None, int number = 0) {
    auto *object = box(parent, x, y, width, 44, 0x20292a);
    lv_obj_center(label(object, text, 0, 0));
    if (action != None) {
        // Buttons live for the duration of the simulator.
        lv_obj_add_flag(object, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(object, click, LV_EVENT_CLICKED, new Command{action, number});
    }
    return object;
}
void paintEffect() {
    const auto &s = control.state().seats[seat];
    if (!effect) { lv_obj_add_flag(overlay, LV_OBJ_FLAG_HIDDEN); return; }
    lv_obj_remove_flag(overlay, LV_OBJ_FLAG_HIDDEN);
    char source[64];
    const char *kind = effect == 1 ? "heat" : effect == 2 ? "cool" : "massage";
    const int level = effect == 1 ? s.heat : effect == 2 ? s.cool : 1;
    std::snprintf(source, sizeof(source), "A:/%sfx/%s-%d-%d.png", seat ? "right/" : "", kind, level, frame);
    lv_image_set_src(overlay, source);
}
void render(int preferred = 0) {
    const auto &s = control.state().seats[seat];
    effect = s.cool ? 2 : s.heat ? 1 : s.massage ? 3 : 0;
    if (preferred == 3 && s.massage) effect = 3;
    if (preferred == 4) effect = 0;
    char source[64];
    std::snprintf(source, sizeof(source), "A:/%srecline/seat_%02d.jpg", seat ? "right/" : "", (s.recline * 24 + 50) / 100);
    lv_image_set_src(chair, source);
    lv_obj_set_x(overlay, seat ? 157 : 233);
    lv_label_set_text(caption, seat ? "Comfort · Rear right" : "Comfort · Rear left");
    for (int i = 0; i < 2; ++i) select(seatButtons[i], seat == i);
    for (int i = 0; i < 4; ++i) {
        select(heat[i], s.heat == i, 0xffb249);
        select(cool[i], s.cool == i, 0x80d4ff);
    }
    lv_label_set_text(lv_obj_get_child(massage, 0), s.massage ? "On" : "Off");
    select(massage, s.massage);
    lv_slider_set_value(slider, s.recline, LV_ANIM_OFF);
    lv_label_set_text_fmt(value, "%d%%", s.recline);
    paintEffect();
}
void apply(const char *key, int n, int preferred = 0) {
    if (control.dispatch(seat, key, n) != revontuli::Result::Applied) return;
    const auto &s = control.state().seats[seat];
    lv_label_set_text_fmt(status, "C++ confirmed · %s %d · heat %d / cool %d", key, n, s.heat, s.cool);
    render(preferred);
}
void click(lv_event_t *event) {
    const auto [action, number] = *static_cast<Command *>(lv_event_get_user_data(event));
    switch (action) {
    case Seat: seat = number; render(); break;
    case Heat: apply("heat", number); break;
    case Cool: apply("cool", number); break;
    case Massage: apply("massage", !control.state().seats[seat].massage, 3); break;
    case Save: saved[seat] = control.state().seats[seat].recline; lv_label_set_text(status, "Seat position saved for this session"); break;
    case Recall: if (saved[seat] >= 0) apply("recline", saved[seat], 4); else lv_label_set_text(status, "No position saved for this seat"); break;
    case Reset: apply("resetSeat", 1); break;
    default: break;
    }
}
void buildUi() {
    auto *root = lv_screen_active();
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    fill(root, 0x0b1112);
    label(root, "19:04", 18, 12, 0xa6b9b2);
    lv_obj_set_style_text_font(label(root, "Comfort", 18, 42), &lv_font_montserrat_26, 0);
    button(root, "Climate", 18, 88, 218);
    select(button(root, "Seat", 244, 88, 218), true);
    auto *scene = box(root, 0, 143, 480, 280, 0x020404, 0);
    chair = lv_image_create(scene);
    lv_obj_set_pos(chair, 120, 12);
    overlay = lv_image_create(scene);
    lv_obj_set_pos(overlay, 233, 91);
    seatButtons[0] = button(scene, "Rear left", 304, 10, 80, Seat, 0);
    seatButtons[1] = button(scene, "Rear right", 389, 10, 80, Seat, 1);
    caption = label(scene, "", 18, 254, 0xbacac3);
    const char *levels[] = {"Off", "1", "2", "3"};
    for (int row = 0; row < 2; ++row) {
        auto *card = box(root, 18, 433 + row * 59, 444, 51, panel);
        label(card, row ? "Seat cooling" : "Seat heat", 15, 17);
        for (int i = 0; i < 4; ++i)
            (row ? cool : heat)[i] = button(card, levels[i], 226 + i * 54, 4, 48, row ? Cool : Heat, i);
    }
    auto *card = box(root, 18, 551, 444, 51, panel);
    label(card, "Massage", 15, 17);
    massage = button(card, "Off", 356, 4, 72, Massage);
    card = box(root, 18, 610, 444, 60, panel);
    label(card, "Recline", 15, 6);
    value = label(card, "", 398, 6);
    slider = lv_slider_create(card);
    lv_obj_set_pos(slider, 19, 36);
    lv_obj_set_size(slider, 406, 7);
    lv_obj_set_style_bg_color(slider, lv_color_hex(green), LV_PART_INDICATOR);
    lv_slider_set_range(slider, 0, 100);
    lv_obj_add_event_cb(slider, [](lv_event_t *) { apply("recline", lv_slider_get_value(slider), 4); }, LV_EVENT_VALUE_CHANGED, nullptr);
    button(root, "Save", 18, 678, 142, Save);
    button(root, "Recall", 168, 678, 142, Recall);
    button(root, "Reset", 318, 678, 144, Reset);
    status = label(root, "LVGL simulator · C++ ready", 18, 727, 0xa6b9b2);
    auto *nav = box(root, 0, 749, 480, 51, 0x101718);
    const char *tabs[] = {"Home", "Comfort", "Atmosphere", "Media", "Cabin"};
    for (int i = 0; i < 5; ++i) select(button(nav, tabs[i], 10 + i * 92, 3, 87), i == 1);
    lv_timer_create([](lv_timer_t *) { frame = (frame + 1) % 4; if (effect) paintEffect(); }, 150, nullptr);
    render();
}
}
#ifdef __EMSCRIPTEN__
EM_JS(void, draw, (int x, int y, int w, int h, const unsigned char *pixels), {
    const context = document.querySelector("canvas").getContext("2d", {alpha:false});
    const image = context.createImageData(w,h);
    for (let i=0;i<w*h*4;i+=4) {
        image.data[i]=HEAPU8[pixels+i+2];
        image.data[i+1]=HEAPU8[pixels+i+1];
        image.data[i+2]=HEAPU8[pixels+i];
        image.data[i+3]=255;
    }
    context.putImageData(image,x,y);
});
EM_JS(int, pointer, (int field), { return window.seatPointer[field]; });
int main() {
    lv_init();
    lv_tick_set_cb([]() -> uint32_t { return static_cast<uint32_t>(emscripten_get_now()); });
    auto *display = lv_display_create(480,800);
    static uint32_t buffer[480*800];
    lv_display_set_color_format(display, LV_COLOR_FORMAT_XRGB8888);
    lv_display_set_buffers(display, buffer, nullptr, sizeof(buffer), LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display, [](lv_display_t *d, const lv_area_t *a, uint8_t *pixels) {
        draw(a->x1,a->y1,lv_area_get_width(a),lv_area_get_height(a),pixels);
        lv_display_flush_ready(d);
    });
    auto *input = lv_indev_create();
    lv_indev_set_type(input, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(input, [](lv_indev_t *, lv_indev_data_t *data) {
        data->point.x=pointer(0);
        data->point.y=pointer(1);
        data->state=pointer(2) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    });
    buildUi();
    emscripten_set_main_loop([] { lv_timer_handler(); },0,true);
}
#else
int main(int argc, char **argv) {
    wchar_t executable[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH) return 1;
    if (auto *separator = wcsrchr(executable, L'\\')) {
        *separator = 0;
        if (!SetCurrentDirectoryW(executable)) return 1;
    }
    lv_init();
    if (argc == 2 && std::strcmp(argv[1], "--verify") == 0) {
        lv_display_create(480, 800);
        buildUi();
        lv_obj_send_event(cool[2], LV_EVENT_CLICKED, nullptr);
        const auto &left = control.state().seats[0];
        if (left.cool != 2 || left.heat != 0 || control.state().seats[1].heat != 2 || effect != 2)
            return 2;
        lv_obj_send_event(massage, LV_EVENT_CLICKED, nullptr);
        lv_obj_send_event(massage, LV_EVENT_CLICKED, nullptr);
        if (effect != 2) return 3;
        for (const char *source : {"A:/recline/seat_00.jpg", "A:/fx/cool-2-0.png", "A:/right/recline/seat_00.jpg", "A:/right/fx/cool-2-0.png"}) {
            lv_image_header_t header{};
            if (lv_image_decoder_get_info(source, &header) != LV_RESULT_OK) return 4;
        }
        std::puts("LVGL events, shared C++ seat state and image headers verified.");
        return 0;
    }
    auto *display = lv_windows_create_display(L"Revontuli | LVGL simulator", 480, 800, 100, false, false);
    if (!display) return 1;
    lv_windows_acquire_pointer_indev(display);
    buildUi();
    const HWND window = lv_windows_get_display_window_handle(display);
    while (IsWindow(window)) {
        lv_timer_handler();
        Sleep(5);
    }
}
#endif
