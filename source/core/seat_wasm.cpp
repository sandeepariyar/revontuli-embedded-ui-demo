#include "controller.hpp"
#include <emscripten/emscripten.h>

namespace {
revontuli::Simulator device;
revontuli::Controller controller(device);
}

extern "C" {
EMSCRIPTEN_KEEPALIVE int seat_command(int seat, int command, int value) {
    const char *keys[] = {"heat", "cool", "massage", "recline"};
    if (command < 0 || command >= 4) return -1;
    if (controller.dispatch(seat, keys[command], value) != revontuli::Result::Applied)
        return -1;
    const auto &state = controller.state().seats[seat];
    return state.heat | (state.cool << 2) | (int(state.massage) << 4) | (state.recline << 5);
}
}
