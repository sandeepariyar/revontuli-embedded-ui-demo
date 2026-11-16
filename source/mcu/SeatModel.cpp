#include "SeatModel.h"
#include "../core/controller.hpp"

namespace {
revontuli::Simulator device;
revontuli::Controller control(device);
int positions[] = {-1, -1};

void sync(SeatModel& model, int preferred = 0) {
    const int index = model.selected.value();
    const auto& seat = control.state().seats[index];
    model.heat.setValue(seat.heat);
    model.cool.setValue(seat.cool);
    model.massage.setValue(seat.massage);
    model.recline.setValue(seat.recline);
    model.saved.setValue(positions[index] >= 0);
    int effect = seat.cool ? 2 : seat.heat ? 1 : seat.massage ? 3 : 0;
    if (preferred == 3 && seat.massage) effect = 3;
    if (preferred == 4) effect = 0;
    model.preview.setValue(effect);
}
void apply(SeatModel& model, const char* key, int value, int preview = 0) {
    if (control.dispatch(model.selected.value(), key, value) == revontuli::Result::Applied)
        sync(model, preview);
}
}
SeatModel::SeatModel() { select(0); }
void SeatModel::select(int index) {
    if (index < 0 || index > 1) return;
    selected.setValue(index);
    sync(*this);
}
void SeatModel::setHeat(int level) { apply(*this, "heat", level); }
void SeatModel::setCool(int level) { apply(*this, "cool", level); }
void SeatModel::toggleMassage() { apply(*this, "massage", !massage.value(), 3); }
void SeatModel::setRecline(int position) { apply(*this, "recline", position, 4); }
void SeatModel::save() {
    positions[selected.value()] = recline.value();
    sync(*this, 4);
}
void SeatModel::recall() {
    if (saved.value()) setRecline(positions[selected.value()]);
}
void SeatModel::reset() { apply(*this, "resetSeat", 1); }
