#include "lvgl.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include "src/drivers/windows/lv_windows_display.h"
#include "src/drivers/windows/lv_windows_input.h"
#include <windows.h>
#endif
#include "../core/controller.hpp"
#include <algorithm>
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
lv_obj_t *heatDetail, *coolDetail;
lv_obj_t *seatPage, *climatePage, *climateTab, *seatTab, *airflow, *temperature, *fanSlider;
lv_obj_t *airButtons[3], *climateSeats[2], *acButton, *autoButton;
lv_obj_t *fanValue;
lv_obj_t *extraPage, *heading, *navigation[5], *extraImage, *extraValue;
int activePage = 1;
int track = 0, progress = 84, windowTarget = -1, windowSeat = 0;
int lightSpeed = 1, lightZones = 7;
bool repeatTrack = false, shuffleTrack = false, beat = false, privacy = false;
bool climate = false;
int airflowFrame = 0;
uint32_t nextAirflowFrame = 0;

constexpr unsigned ink = 0xedf1e7, green = 0x3ddc97, panel = 0x151d1e;
void icon(lv_obj_t *parent, const char *name, int x, int y) {
    char path[80];
    std::snprintf(path, sizeof(path), "A:/skins/%s.png", name);
    auto *object = lv_image_create(parent);
    lv_image_set_src(object, path);
    lv_obj_set_pos(object, x, y);
    lv_obj_remove_flag(object, LV_OBJ_FLAG_CLICKABLE);
}
// Image icons use the exported artwork from the web design.
void fill(lv_obj_t *object, unsigned color) {
    lv_obj_set_style_bg_color(object, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
}
void skin(lv_obj_t *object, const char *name, int width, int height) {
    char path[80];
    std::snprintf(path, sizeof(path), "A:/skins/%s-%dx%d.png", name, width, height);
    // The image widget owns a copy of the file path. Keep it behind the text.
    auto *image = lv_obj_get_child(object, 0);
    if (!image || !lv_obj_check_type(image, &lv_image_class)) {
        image = lv_image_create(object);
        lv_obj_move_to_index(image, 0);
        lv_obj_remove_flag(image, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_center(image);
    }
    lv_image_set_src(image, path);
    lv_obj_set_style_bg_opa(object, LV_OPA_TRANSP, 0);
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
    const int width = lv_obj_get_style_width(button, LV_PART_MAIN);
    const int height = lv_obj_get_style_height(button, LV_PART_MAIN);
    if (width != 87) {
        const char *name = "button";
        if (selected) name = accent == 0xffb249 ? "selected-heat" : accent == 0x80d4ff ? "selected-cool" : "selected";
        skin(button, name, width, height);
    } else {
        lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, 0);
        if (lv_obj_get_child_count(button) > 1) {
            auto *glyph = lv_obj_get_child(button, 1);
            lv_obj_set_style_image_recolor(glyph, lv_color_hex(selected ? green : 0xa4b1ac), 0);
            lv_obj_set_style_image_recolor_opa(glyph, LV_OPA_COVER, 0);
        }
    }
    lv_obj_set_style_text_color(lv_obj_get_child(button, width == 87 ? 0 : -1),
        lv_color_hex(width == 87 ? (selected ? green : 0xa4b1ac) : selected ? 0x092419 : ink), 0);
}
enum Action { None, Seat, Heat, Cool, Massage, Save, Recall, Reset, Mode,
              Temp, Direction, Ac, Auto, Page, HomeTemp, HomeHeat, HomeWindow,
              Lighting, Preset, Beat, LightSpeed, Zone, Source, Output, Focus,
              Play, Track, Repeat, Shuffle, Window, Roof, Reading, Privacy,
              Fresh, CloseBoth, Mute };
struct Command { Action action; int value; };
void click(lv_event_t *event);
void showPage(int page);
void refreshExtra();
lv_obj_t *button(lv_obj_t *parent, const char *text, int x, int y, int width,
                 Action action = None, int number = 0, int height = 44) {
    auto *object = box(parent, x, y, width, height, 0x20292a);
    auto *textLabel = label(object, text, 0, 0);
    lv_obj_set_style_text_font(textLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(textLabel);
    if (width != 87) skin(object, "button", width, height);
    if (action != None) {
        lv_obj_add_flag(object, LV_OBJ_FLAG_CLICKABLE);
        auto *command = new Command{action, number};
        lv_obj_add_event_cb(object, click, LV_EVENT_CLICKED, command);
        lv_obj_add_event_cb(object, [](lv_event_t *event) {
            delete static_cast<Command *>(lv_event_get_user_data(event));
        }, LV_EVENT_DELETE, command);
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
void paintAirflow() {
    const auto &state = control.state();
    const auto &s = state.seats[seat];
    const char *directions[] = {"face", "body", "feet"};
    const char *tone = s.temperature > 22.25 ? "warm" : state.ac && s.temperature < 21.75 ? "cool" : "vent";
    char path[80];
    if (s.fan) std::snprintf(path, sizeof(path), "A:/skins/climate/%s-%s-%d.jpg", tone,
        directions[s.air], airflowFrame);
    else std::snprintf(path, sizeof(path), "A:/skins/climate/static.jpg");
    lv_image_set_src(airflow, path);
}
void renderClimate() {
    const auto &state = control.state();
    const auto &s = state.seats[seat];
    lv_label_set_text_fmt(temperature, "%d.%d C", int(s.temperature), int(s.temperature * 10) % 10);
    lv_obj_set_style_text_color(temperature, lv_color_hex(
        s.temperature > 22.25 ? 0xffc779 : state.ac && s.temperature < 21.75 ? 0x7dceff : ink), 0);
    lv_slider_set_value(fanSlider, s.fan, LV_ANIM_OFF);
    lv_label_set_text_fmt(fanValue, "%d", s.fan);
    for (int i = 0; i < 3; ++i) select(airButtons[i], s.air == i);
    for (int i = 0; i < 2; ++i) select(climateSeats[i], seat == i);
    select(acButton, state.ac);
    select(autoButton, state.automatic);
    paintAirflow();
}
void setMode(bool showClimate) {
    climate = showClimate;
    if (climate) {
        airflowFrame = 0;
        nextAirflowFrame = lv_tick_get();
        lv_obj_add_flag(seatPage, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(climatePage, LV_OBJ_FLAG_HIDDEN);
        renderClimate();
    } else {
        lv_obj_add_flag(climatePage, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(seatPage, LV_OBJ_FLAG_HIDDEN);
    }
    select(climateTab, climate);
    select(seatTab, !climate);
}
void render(int preferred = 0) {
    const auto &s = control.state().seats[seat];
    effect = s.cool ? 2 : s.heat ? 1 : s.massage ? 3 : 0;
    if (preferred == 3 && s.massage) effect = 3;
    if (preferred == 4) effect = 0;
    char source[64];
    std::snprintf(source, sizeof(source), "A:/skins/chair/%s/seat_%02d.png", seat ? "right" : "left", (s.recline * 24 + 50) / 100);
    lv_image_set_src(chair, source);
    lv_obj_set_x(overlay, seat ? 150 : 171);
    lv_label_set_text(caption, seat ? "Comfort - Rear right" : "Comfort - Rear left");
    for (int i = 0; i < 2; ++i) select(seatButtons[i], seat == i);
    for (int i = 0; i < 4; ++i) {
        select(heat[i], s.heat == i && i != 0, 0xffb249);
        select(cool[i], s.cool == i && i != 0, 0x80d4ff);
        lv_obj_set_style_border_width(heat[i], s.heat == 0 && i == 0 ? 1 : 0, 0);
        lv_obj_set_style_border_width(cool[i], s.cool == 0 && i == 0 ? 1 : 0, 0);
        lv_obj_set_style_border_color(heat[i], lv_color_hex(0x7c8d85), 0);
        lv_obj_set_style_border_color(cool[i], lv_color_hex(0x7c8d85), 0);
    }
    const char *levels[] = {"Off", "Low", "Medium", "High"};
    lv_label_set_text(heatDetail, levels[s.heat]);
    lv_label_set_text(coolDetail, levels[s.cool]);
    select(massage, s.massage);
    lv_obj_set_x(lv_obj_get_child(massage, -1), s.massage ? 27 : 4);
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
    const auto &state = control.state();
    const auto &selected = state.seats[seat];
    switch (action) {
    case Seat: seat = number; render(); if (activePage == 1 && climate) renderClimate();
        else if (activePage != 1) refreshExtra(); break;
    case Page: showPage(number); break;
    case Mode: setMode(number == 0); break;
    case Temp: {
        const auto target = control.state().seats[seat].temperature + number * 0.5;
        control.dispatch(seat, "temperature", target);
        renderClimate();
        break;
    }
    case Direction: control.dispatch(seat, "air", number); renderClimate(); break;
    case Ac: control.dispatch(seat, "ac", !control.state().ac); renderClimate(); break;
    case Auto: control.dispatch(seat, "automatic", !control.state().automatic); renderClimate(); break;
    case Heat: apply("heat", number); break;
    case Cool: apply("cool", number); break;
    case Massage: apply("massage", !control.state().seats[seat].massage, 3); break;
    case Save: saved[seat] = control.state().seats[seat].recline; lv_label_set_text(status, "Seat position saved for this session"); break;
    case Recall: if (saved[seat] >= 0) apply("recline", saved[seat], 4); else lv_label_set_text(status, "No position saved for this seat"); break;
    case Reset: apply("resetSeat", 1); break;
    case HomeTemp: control.dispatch(seat, "temperature", selected.temperature + number * 0.5); refreshExtra(); break;
    case HomeHeat: control.dispatch(seat, "heat", (selected.heat + 1) % 4); refreshExtra(); break;
    case HomeWindow: windowSeat = seat; windowTarget = selected.window ? 0 : 100; break;
    case Lighting: control.dispatch(seat, "light", !state.light); refreshExtra(); break;
    case Preset: control.dispatch(seat, "scene", number); control.dispatch(seat, "light", 1); refreshExtra(); break;
    case Beat: beat = !beat; refreshExtra(); break;
    case LightSpeed: lightSpeed = number; refreshExtra(); break;
    case Zone: lightZones ^= 1 << number; refreshExtra(); break;
    case Source: control.dispatch(seat, "source", number); refreshExtra(); break;
    case Output: control.dispatch(seat, "output", number); refreshExtra(); break;
    case Focus: if (!state.output) control.dispatch(seat, "focus", number); refreshExtra(); break;
    case Play: control.dispatch(seat, "playing", !state.playing); refreshExtra(); break;
    case Track: track = (track + number + 3) % 3; progress = 0; refreshExtra(); break;
    case Repeat: repeatTrack = !repeatTrack; refreshExtra(); break;
    case Shuffle: shuffleTrack = !shuffleTrack; refreshExtra(); break;
    case Mute: control.dispatch(seat, "volume", state.volume ? 0 : 45); refreshExtra(); break;
    case Window: windowSeat = seat; windowTarget = number; refreshExtra(); break;
    case Roof: control.dispatch(seat, "roof", number); refreshExtra(); break;
    case Reading: control.dispatch(seat, "reading", !state.reading); refreshExtra(); break;
    case Privacy: privacy = !privacy; control.dispatch(seat, "shade", privacy ? 100 : 0); refreshExtra(); break;
    case Fresh: control.dispatch(seat, "fresh", !state.fresh); refreshExtra(); break;
    case CloseBoth:
        windowTarget = -1;
        control.dispatch(0, "window", 0); control.dispatch(1, "window", 0); refreshExtra(); break;
    default: break;
    }
}
void buildClimate(lv_obj_t *root) {
    climatePage = box(root, 0, 128, 480, 444, 0x080e10, 0);
    auto *scene = box(climatePage, 0, 0, 480, 240, 0x000000, 0);
    airflow = lv_image_create(scene);
    lv_obj_center(airflow);
    climateSeats[0] = button(scene, "Rear left", 313, 8, 70, Seat, 0);
    climateSeats[1] = button(scene, "Rear right", 391, 8, 70, Seat, 1);
    acButton = button(scene, "A/C", 18, 188, 70, Ac);
    autoButton = button(scene, "Auto", 392, 188, 70, Auto);
    label(scene, "Rear console vent", 18, 168, 0xbacac3);
    auto *card = box(climatePage, 18, 249, 444, 51, panel);
    skin(card, "panel", 444, 51);
    label(card, "Set temperature", 14, 16);
    button(card, "-", 240, 4, 38, Temp, -1);
    temperature = label(card, "", 290, 10);
    lv_obj_set_style_text_font(temperature, &lv_font_montserrat_26, 0);
    button(card, "+", 392, 4, 38, Temp, 1);
    card = box(climatePage, 18, 308, 444, 60, panel);
    skin(card, "panel", 444, 60);
    label(card, "Fan speed", 14, 6);
    fanValue = label(card, "", 416, 6);
    fanSlider = lv_slider_create(card);
    lv_obj_set_pos(fanSlider, 19, 37);
    lv_obj_set_size(fanSlider, 406, 7);
    lv_slider_set_range(fanSlider, 0, 5);
    lv_obj_set_style_bg_color(fanSlider, lv_color_hex(green), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(fanSlider, lv_color_hex(0xedf1e7), LV_PART_KNOB);
    lv_obj_add_event_cb(fanSlider, [](lv_event_t *) {
        control.dispatch(seat, "fan", lv_slider_get_value(fanSlider));
        airflowFrame = 0;
        nextAirflowFrame = lv_tick_get();
        renderClimate();
    }, LV_EVENT_VALUE_CHANGED, nullptr);
    const char *directions[] = {"Face", "Body", "Feet"};
    for (int i = 0; i < 3; ++i)
        airButtons[i] = button(climatePage, directions[i], 18 + i * 150, 377, 142, Direction, i, 44);
}
const char *presetNames[] = {"Aurora", "Lounge", "Relax", "Club", "Cinema"};
const char *presetFiles[] = {"aurora", "lounge", "relax", "club", "cinema"};
const char *trackNames[] = {"Chromatic Waves", "Northern Lights", "Still Water"};
lv_obj_t *extraSlider = nullptr;

void updateExtraImage() {
    if (!extraImage) return;
    const auto &state = control.state();
    const auto &s = state.seats[seat];
    char path[96];
    if (activePage == 3) {
        std::snprintf(path, sizeof(path), "A:/skins/cabin/album.jpg");
    } else if (activePage == 4 && (s.window || windowTarget >= 0)) {
        std::snprintf(path, sizeof(path), "A:/skins/cabin/window-%d.jpg", (s.window * 9 + 50) / 100);
    } else if (activePage == 4 && s.shade) {
        std::snprintf(path, sizeof(path), "A:/skins/cabin/shade-%d.jpg", (s.shade * 9 + 50) / 100);
    } else if (activePage == 4 && state.roof) {
        std::snprintf(path, sizeof(path), "A:/skins/cabin/roof-%d.jpg", state.roof == 1 ? 3 : 9);
    } else if (state.light && lightZones) {
        int level = (state.brightness * 9 + 50) / 100;
        if (activePage == 2 && beat) level = std::max(0, level - (3 - frame));
        std::snprintf(path, sizeof(path), "A:/skins/cabin/light-%s-%d.jpg", presetFiles[state.scene], level);
    } else {
        std::snprintf(path, sizeof(path), "A:/skins/cabin/cabin.jpg");
    }
    lv_image_set_src(extraImage, path);
}

lv_obj_t *extraCard(int x, int y, const char *name, int width = 444) {
    auto *card = box(extraPage, x, y, width, 51, panel);
    if (width == 444) skin(card, "panel", 444, 51);
    label(card, name, 13, 7);
    return card;
}
lv_obj_t *extraRange(const char *name, const char *key, int y, int val, int max = 100) {
    const bool timeline = std::string_view(key) == "progress";
    auto *card = timeline ? box(extraPage, 18, y, 444, 51, 0x080e10, 0) : extraCard(18, y, name);
    auto *number = label(card, "", 390, 7, 0xbacac3);
    if (timeline) {
        lv_obj_set_pos(number, 0, 36);
        lv_label_set_text_fmt(number, "%d:%02d", val / 60, val % 60);
        label(card, "3:58", 410, 36, 0xbacac3);
    } else lv_label_set_text_fmt(number, "%d%%", val);
    auto *range = lv_slider_create(card);
    lv_obj_set_pos(range, 19, timeline ? 19 : 34);
    lv_obj_set_size(range, 406, 6);
    lv_slider_set_range(range, 0, max);
    lv_slider_set_value(range, val, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(range, lv_color_hex(green), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(range, lv_color_hex(0xedf1e7), LV_PART_KNOB);
    if (activePage == 4) lv_obj_set_style_bg_color(range, lv_color_hex(0x354140), LV_PART_MAIN);
    lv_obj_set_user_data(range, number);
    lv_obj_add_event_cb(range, [](lv_event_t *event) {
        auto *slider = lv_event_get_target_obj(event);
        const auto *key = static_cast<const char *>(lv_event_get_user_data(event));
        const int value = lv_slider_get_value(slider);
        if (std::string_view(key) == "progress") progress = value;
        else {
            if (std::string_view(key) == "window") windowTarget = -1;
            control.dispatch(seat, key, value);
        }
        auto *number = static_cast<lv_obj_t *>(lv_obj_get_user_data(slider));
        if (std::string_view(key) == "progress") lv_label_set_text_fmt(number, "%d:%02d", value / 60, value % 60);
        else lv_label_set_text_fmt(number, "%d%%", value);
        updateExtraImage();
    }, LV_EVENT_VALUE_CHANGED, const_cast<char *>(key));
    return range;
}
void extraHero(int y) {
    extraImage = lv_image_create(extraPage);
    lv_obj_set_pos(extraImage, 0, y);
    updateExtraImage();
}
void extraSeats(int y) {
    select(button(extraPage, "Rear left", 312, y, 70, Seat, 0), seat == 0);
    select(button(extraPage, "Rear right", 392, y, 70, Seat, 1), seat == 1);
}
void presets(int y, int count) {
    const auto &state = control.state();
    for (int i = 0; i < count; ++i) {
        const int width = count == 3 ? 142 : 88;
        auto *tile = button(extraPage, presetNames[i], 18 + i * (width + (count == 3 ? 8 : 2)), y, width, Preset, i);
        lv_obj_clean(tile);
        lv_obj_set_height(tile, 54);
        lv_obj_set_style_bg_opa(tile, LV_OPA_TRANSP, 0);
        auto *photo = lv_image_create(tile);
        char path[64];
        std::snprintf(path, sizeof(path), "A:/skins/cabin/%s-%d.png", presetFiles[i], width);
        lv_image_set_src(photo, path);
        lv_obj_remove_flag(photo, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_text_font(label(tile, presetNames[i], 7, 32), &lv_font_montserrat_12, 0);
        if (state.light && state.scene == i) {
            lv_obj_set_style_outline_width(tile, 2, 0);
            lv_obj_set_style_outline_color(tile, lv_color_hex(green), 0);
        }
    }
}
lv_obj_t *glyphButton(lv_obj_t *parent, const char *glyph, int x, int y, Action action,
                      int number = 0, bool selected = false, bool bare = false, bool circle = false) {
    auto *item = button(parent, "", x, y, 44, action, number);
    lv_obj_clean(item);
    if (!bare && !circle) skin(item, selected ? "selected" : "button", 44, 44);
    else if (circle) {
        lv_obj_set_style_radius(item, LV_RADIUS_CIRCLE, 0);
        fill(item, green);
    } else lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
    icon(item, glyph, 10, 10);
    if ((selected && !bare) || circle) {
        auto *image = lv_obj_get_child(item, -1);
        lv_obj_set_style_image_recolor(image, lv_color_hex(0x05251b), 0);
        lv_obj_set_style_image_recolor_opa(image, LV_OPA_COVER, 0);
    } else if (selected) {
        auto *image = lv_obj_get_child(item, -1);
        lv_obj_set_style_image_recolor(image, lv_color_hex(green), 0);
        lv_obj_set_style_image_recolor_opa(image, LV_OPA_COVER, 0);
    }
    return item;
}
void miniPlayer(int y) {
    const auto &state = control.state();
    auto *card = box(extraPage, 18, y, 444, 51, panel);
    skin(card, "panel", 444, 51);
    auto *album = lv_image_create(card);
    lv_image_set_src(album, "A:/skins/cabin/album-mini.jpg");
    lv_obj_set_pos(album, 10, 7);
    lv_obj_set_style_text_font(label(card, trackNames[track], 57, 8), &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_font(label(card, "Kaamos", 57, 28, 0xa4b1ac), &lv_font_montserrat_12, 0);
    auto *play = glyphButton(card, state.playing ? "pause" : "play", 354, 4, Play, 0, true, false, true);
    lv_obj_set_size(play, 38, 38);
    lv_obj_set_pos(lv_obj_get_child(play, -1), 7, 7);
    glyphButton(card, "next", 399, 4, Track, 1, false, true);
}
void extraSwitch(const char *name, int x, int y, Action action, bool enabled) {
    label(extraPage, name, x, y + 5, 0xbacac3);
    auto *toggle = button(extraPage, "", x + 66, y, 52, action, 0, 26);
    lv_obj_clean(toggle);
    skin(toggle, enabled ? "selected" : "button", 52, 26);
    auto *knob = box(toggle, enabled ? 28 : 4, 3, 20, 20, 0xedf1e7, 10);
    lv_obj_remove_flag(knob, LV_OBJ_FLAG_CLICKABLE);
}
void refreshExtra() {
    lv_obj_clean(extraPage);
    extraImage = extraValue = extraSlider = nullptr;
    const auto &state = control.state();
    const auto &s = state.seats[seat];
    if (activePage == 0) {
        extraHero(0);
        icon(extraPage, "snow", 18, 130);
        label(extraPage, "Oulu  -4 C  Light snow", 48, 134);
        label(extraPage, "Preview connected", 18, 172, 0xbacac3);
        extraSeats(156);
        label(extraPage, "Your comfort", 18, 222);
        auto *card = extraCard(18, 252, "", 216);
        icon(card, "temp", 11, 14);
        label(card, "Temperature", 44, 7);
        lv_label_set_text_fmt(label(card, "", 44, 27), "%d.%d C", int(s.temperature), int(s.temperature * 10) % 10);
        button(card, "-", 125, 4, 38, HomeTemp, -1);
        button(card, "+", 171, 4, 38, HomeTemp, 1);
        card = extraCard(246, 252, "", 216);
        icon(card, "window", 11, 14);
        label(card, "Window", 44, 7);
        extraValue = label(card, s.window ? "Open" : "Closed", 44, 27);
        select(button(card, s.window ? "Close" : "Open", 131, 4, 70, HomeWindow), true);
        card = extraCard(18, 312, "", 216);
        icon(card, "heat", 11, 14);
        label(card, "Seat heat", 44, 7);
        lv_label_set_text_fmt(label(card, "", 44, 27), "Level %d", s.heat);
        lv_obj_add_event_cb(card, [](lv_event_t *) {
            const int heat = control.state().seats[seat].heat;
            control.dispatch(seat, "heat", (heat + 1) % 4); refreshExtra();
        }, LV_EVENT_CLICKED, nullptr);
        card = extraCard(246, 312, "", 216);
        icon(card, "sun", 11, 14);
        label(card, "Lighting", 44, 7);
        label(card, state.light ? presetNames[state.scene] : "Off", 44, 27);
        lv_obj_add_event_cb(card, [](lv_event_t *) {showPage(2);}, LV_EVENT_CLICKED, nullptr);
        label(extraPage, "Experiences", 18, 376);
        presets(400, 3);
        miniPlayer(458);
    } else if (activePage == 2) {
        extraHero(0);
        extraSwitch("Effect", 340, 8, Lighting, state.light);
        extraSwitch("Beat sync", 18, 162, Beat, beat);
        const char *speeds[] = {"Slow", "Flow", "Fast"};
        for (int i = 0; i < 3; ++i)
            select(button(extraPage, speeds[i], 282 + i * 60, 157, 56, LightSpeed, i, 40), lightSpeed == i);
        presets(222, 5);
        extraRange("Intensity", "brightness", 278, state.brightness);
        label(extraPage, "Light zones", 18, 348);
        const char *zones[] = {"Doors", "Roof", "Footwell"};
        for (int i = 0; i < 3; ++i)
            select(button(extraPage, zones[i], 18 + i * 150, 374, 142, Zone, i), lightZones & (1 << i));
        miniPlayer(451);
    } else if (activePage == 3) {
        const char *sources[] = {"Spotify", "Radio", "Bluetooth"};
        for (int i = 0; i < 3; ++i)
            glyphButton(extraPage, sources[i], 153 + i * 66, 3, Source, i, state.source == i);
        extraImage = lv_image_create(extraPage);
        lv_obj_set_pos(extraImage, 145, 61);
        updateExtraImage();
        glyphButton(extraPage, "volume", 416, 140, Output, 0, state.output == 0);
        glyphButton(extraPage, "headphones", 416, 192, Output, 1, state.output == 1);
        const char *focus[] = {"All", "Front", "Rear"};
        for (int i = 0; i < 3; ++i)
            select(button(extraPage, focus[i], 18, 115 + i * 50, 70, Focus, i), !state.output && state.focus == i);
        lv_obj_align(label(extraPage, trackNames[track], 0, 0), LV_ALIGN_TOP_MID, 0, 255);
        lv_obj_align(label(extraPage, "Kaamos", 0, 0, 0xbacac3), LV_ALIGN_TOP_MID, 0, 278);
        extraSlider = extraRange("Playback position", "progress", 305, progress, 238);
        glyphButton(extraPage, "repeat", 90, 368, Repeat, 0, repeatTrack, true);
        glyphButton(extraPage, "prev", 154, 368, Track, -1, false, true);
        glyphButton(extraPage, state.playing ? "pause" : "play", 218, 368, Play, 0, true, false, true);
        glyphButton(extraPage, "next", 282, 368, Track, 1, false, true);
        glyphButton(extraPage, "shuffle", 346, 368, Shuffle, 0, shuffleTrack, true);
        extraRange("Volume", "volume", 429, state.volume);
        glyphButton(extraPage, "volume", 294, 430, Mute, 0, state.volume != 0, true);
        label(extraPage, "Audio preview - simulated", 18, 493, 0xbacac3);
    } else if (activePage == 4) {
        auto *hero = box(extraPage, 0, 0, 480, 160, 0x080e10, 0);
        extraImage = lv_image_create(hero);
        lv_obj_set_pos(extraImage, 0, -33);
        updateExtraImage();
        label(extraPage, "19:04", 18, 10, 0xbacac3);
        lv_obj_set_style_text_font(label(extraPage, "Cabin", 18, 42), &lv_font_montserrat_26, 0);
        lv_obj_set_style_text_font(label(extraPage, seat == 0 ? "Rear left - Live preview" : "Rear right - Live preview",
            20, 139, 0xbacac3), &lv_font_montserrat_12, 0);
        extraSeats(40);
        const char *windowTitle = windowTarget >= 0 ? (windowTarget ? "Window - opening" : "Window - closing") :
            s.window == 0 ? "Window - closed" : s.window == 100 ? "Window - open" : "Window - partly open";
        extraSlider = extraRange(windowTitle, "window", 168, s.window);
        select(button(extraPage, "Open", 18, 227, 142, Window, 100, 36), windowTarget == 100 || (windowTarget < 0 && s.window == 100));
        select(button(extraPage, "Stop", 168, 227, 142, Window, -1, 36), windowTarget < 0 && s.window > 0 && s.window < 100);
        select(button(extraPage, "Close", 318, 227, 144, Window, 0, 36), windowTarget == 0 || (windowTarget < 0 && s.window == 0));
        extraRange(s.shade == 0 ? "Sunshade - closed" : s.shade == 100 ? "Sunshade - open" : "Sunshade - partly open",
            "shade", 275, s.shade);
        auto *roofCard = box(extraPage, 18, 334, 444, 51, panel);
        skin(roofCard, "panel", 444, 51);
        label(roofCard, "Sunroof", 13, 16);
        const char *roof[] = {"Closed", "Tilt", "Open"};
        for (int i = 0; i < 3; ++i)
            select(button(roofCard, roof[i], 274 + i * 55, 6, 52, Roof, i, 38), state.roof == i);
        const char *names[] = {"Reading light", "Privacy", "Fresh air"};
        const char *glyphs[] = {"bulb", "lock", "wave"};
        const bool enabled[] = {state.reading, privacy, state.fresh};
        const Action actions[] = {Reading, Privacy, Fresh};
        for (int i = 0; i < 3; ++i) {
            const int width = i == 2 ? 144 : 142;
            auto *tile = button(extraPage, names[i], 18 + i * 150, 393, width, actions[i], 0, 80);
            lv_obj_clean(tile);
            skin(tile, "panel", width, 80);
            lv_obj_align(label(tile, names[i], 0, 0), LV_ALIGN_TOP_MID, 0, 36);
            lv_obj_align(label(tile, enabled[i] ? "On" : "Off", 0, 0, 0xbacac3), LV_ALIGN_TOP_MID, 0, 56);
            icon(tile, glyphs[i], (width - 24) / 2, 8);
            if (enabled[i]) lv_obj_set_style_image_recolor(lv_obj_get_child(tile, -1), lv_color_hex(green), 0);
            lv_obj_set_style_image_recolor_opa(lv_obj_get_child(tile, -1), enabled[i] ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        }
        button(extraPage, "Close rear windows", 18, 485, 444, CloseBoth, 0, 38);
    }
}
void showPage(int page) {
    activePage = page;
    const char *names[] = {"Revontuli", "Comfort", "Atmosphere", "Media", "Cabin"};
    lv_label_set_text(heading, names[page]);
    for (int i = 0; i < 5; ++i) select(navigation[i], i == page);
    if (page == 1) {
        lv_obj_add_flag(extraPage, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(climateTab, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(seatTab, LV_OBJ_FLAG_HIDDEN);
        setMode(climate);
        render(4);
    } else {
        lv_obj_add_flag(seatPage, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(climatePage, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(climateTab, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(seatTab, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(extraPage, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_y(extraPage, page == 4 ? 0 : 77);
        lv_obj_set_height(extraPage, page == 4 ? 572 : 495);
        refreshExtra();
    }
}
void buildUi() {
    auto *root = lv_screen_active();
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    fill(root, 0x080e10);
    label(root, "19:04", 18, 12, 0xa6b9b2);
    heading = label(root, "Comfort", 18, 42);
    lv_obj_set_style_text_font(heading, &lv_font_montserrat_26, 0);
    climateTab = button(root, "Climate", 18, 84, 218, Mode, 0, 36);
    seatTab = button(root, "Seat", 244, 84, 218, Mode, 1, 36);
    seatPage = box(root, 0, 0, 480, 572, 0x080e10, 0);
    lv_obj_set_style_bg_opa(seatPage, LV_OPA_TRANSP, 0);
    lv_obj_remove_flag(seatPage, LV_OBJ_FLAG_CLICKABLE);
    auto *scene = box(seatPage, 0, 128, 480, 170, 0x000000, 0);
    chair = lv_image_create(scene);
    lv_obj_set_pos(chair, 118, -3);
    overlay = lv_image_create(scene);
    lv_image_set_scale(overlay, 188);
    lv_obj_set_pos(overlay, 196, 42);
    seatButtons[0] = button(scene, "Rear left", 392, 37, 70, Seat, 0);
    seatButtons[1] = button(scene, "Rear right", 392, 89, 70, Seat, 1);
    caption = label(scene, "", 22, 148, 0xbacac3);
    lv_obj_set_style_text_font(caption, &lv_font_montserrat_12, 0);
    const char *levels[] = {"Off", "1", "2", "3"};
    for (int row = 0; row < 2; ++row) {
        auto *card = box(seatPage, 18, 300 + row * 57, 444, 51, panel);
        skin(card, "panel", 444, 51);
        icon(card, row ? "snow" : "heat", 13, 13);
        lv_obj_set_style_text_font(label(card, row ? "Seat cooling" : "Seat heat", 42, 9), &lv_font_montserrat_12, 0);
        auto *detail = label(card, "", 42, 25, 0xa4b1ac);
        lv_obj_set_style_text_font(detail, &lv_font_montserrat_12, 0);
        (row ? coolDetail : heatDetail) = detail;
        for (int i = 0; i < 4; ++i)
            (row ? cool : heat)[i] = button(card, levels[i], 270 + i * 42, 4, 38, row ? Cool : Heat, i);
    }
    auto *card = box(seatPage, 18, 414, 444, 51, panel);
    skin(card, "panel", 444, 51);
    icon(card, "wave", 13, 13);
    label(card, "Massage", 47, 17);
    massage = box(card, 376, 13, 52, 26, 0x20292a);
    auto *knob = box(massage, 4, 3, 20, 20, 0xedf1e7, 10);
    lv_obj_remove_flag(knob, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(massage, click, LV_EVENT_CLICKED, new Command{Massage, 0});
    card = box(seatPage, 18, 470, 444, 60, panel);
    skin(card, "panel", 444, 60);
    label(card, "Recline", 15, 6);
    value = label(card, "", 398, 6);
    slider = lv_slider_create(card);
    lv_obj_set_pos(slider, 19, 36);
    lv_obj_set_size(slider, 406, 7);
    lv_obj_set_style_bg_color(slider, lv_color_hex(green), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x354140), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xedf1e7), LV_PART_KNOB);
    lv_slider_set_range(slider, 0, 100);
    lv_obj_add_event_cb(slider, [](lv_event_t *) { apply("recline", lv_slider_get_value(slider), 4); }, LV_EVENT_VALUE_CHANGED, nullptr);
    button(seatPage, "Save", 18, 536, 142, Save, 0, 32);
    button(seatPage, "Recall", 168, 536, 142, Recall, 0, 32);
    button(seatPage, "Reset", 318, 536, 144, Reset, 0, 32);
    buildClimate(root);
    extraPage = box(root, 0, 77, 480, 535, 0x080e10, 0);
    lv_obj_add_flag(extraPage, LV_OBJ_FLAG_HIDDEN);
    status = label(root, "", 18, 690, 0xa6b9b2);
    lv_obj_add_flag(status, LV_OBJ_FLAG_HIDDEN);
    auto *nav = box(root, 0, 572, 480, 66, 0x101718);
    skin(nav, "dock", 480, 66);
    const char *tabs[] = {"Home", "Comfort", "Atmosphere", "Media", "Cabin"};
    for (int i = 0; i < 5; ++i) {
        auto *item = button(nav, tabs[i], 10 + i * 92, 9, 87, Page, i);
        navigation[i] = item;
        lv_obj_align(lv_obj_get_child(item, 0), LV_ALIGN_BOTTOM_MID, 0, 2);
        icon(item, tabs[i], 31, 0);
        select(item, i == 1);
        lv_obj_add_flag(item, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    }
    lv_timer_create([](lv_timer_t *) {
        frame = (frame + 1) % 4;
        if (activePage == 1 && !climate && effect) paintEffect();
    }, 150, nullptr);
    lv_timer_create([](lv_timer_t *) {
        if (activePage != 1 || !climate) return;
        const int speed = control.state().seats[seat].fan;
        const uint32_t now = lv_tick_get();
        if (!speed || static_cast<int32_t>(now - nextAirflowFrame) < 0) return;
        constexpr uint32_t intervals[] = {0, 320, 240, 175, 125, 90};
        airflowFrame = (airflowFrame + 1) % 4;
        nextAirflowFrame = now + intervals[speed];
        paintAirflow();
    }, 15, nullptr);
    lv_timer_create([](lv_timer_t *) {
        if (windowTarget < 0) return;
        const int from = control.state().seats[windowSeat].window;
        const int next = from + std::clamp(windowTarget - from, -5, 5);
        control.dispatch(windowSeat, "window", next);
        const bool finished = next == windowTarget;
        if (finished) windowTarget = -1;
        if ((activePage == 0 || activePage == 4) && seat == windowSeat) {
            if (finished) refreshExtra();
            else if (activePage == 4) {
                lv_slider_set_value(extraSlider, next, LV_ANIM_OFF);
                lv_label_set_text_fmt(static_cast<lv_obj_t *>(lv_obj_get_user_data(extraSlider)), "%d%%", next);
                updateExtraImage();
            } else if (extraValue) lv_label_set_text_fmt(extraValue, "%d%%", next);
        }
    }, 80, nullptr);
    lv_timer_create([](lv_timer_t *) {
        if (!control.state().playing) return;
        if (++progress > 238) {
            progress = 0;
            if (!repeatTrack) track = (track + (shuffleTrack ? 2 : 1)) % 3;
            if (activePage == 3) refreshExtra();
        } else if (activePage == 3) {
            lv_slider_set_value(extraSlider, progress, LV_ANIM_OFF);
            lv_label_set_text_fmt(static_cast<lv_obj_t *>(lv_obj_get_user_data(extraSlider)), "%d:%02d", progress / 60, progress % 60);
        }
    }, 1000, nullptr);
    lv_timer_create([](lv_timer_t *timer) {
        if (activePage == 2 && beat) updateExtraImage();
        const uint32_t rates[] = {360, 210, 100};
        lv_timer_set_period(timer, rates[lightSpeed]);
    }, 210, nullptr);
    setMode(false);
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
    auto *display = lv_display_create(480,638);
    static uint32_t buffer[480*638];
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
        lv_display_create(480, 638);
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
    auto *display = lv_windows_create_display(L"Revontuli | LVGL simulator", 480, 638, 100, false, false);
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
