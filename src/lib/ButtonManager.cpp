#include "ButtonManager.h"

template <uint8_t N>
ButtonManager<N>::ButtonManager(EventQueue* queue, const std::array<PinName, N> pinnames) {
    this->queue = queue;
    for(uint8_t i = 0; i<N; i++) {
        InterruptIn* button = new InterruptIn(pinnames[i], PullUp);
        button->rise(this->queue->event(Callback<void(uint8_t)>(this, &ButtonManager<N>::released), i));
        button->fall(this->queue->event(Callback<void(uint8_t)>(this, &ButtonManager<N>::pressed), i));
        buttons[i] = button;
    };
    timer.start();
}

template <uint8_t N>
void ButtonManager<N>::onStateChange(mbed::Callback<void(uint8_t buttonIndex, ButtonState state, uint32_t time)> callback){
    stateChanged = callback;
}

template <uint8_t N>
void ButtonManager<N>::pressed(uint8_t button) {
    uint32_t now = currentTimeStamp();
    stateChanged(button, PRESSED, now - lastReleaseEventTimeStamp[button]);
    lastPressEventTimeStamp[button] = now;
}

template <uint8_t N>
void ButtonManager<N>::released(uint8_t button) {
    uint32_t now = currentTimeStamp();
    stateChanged(button, RELEASED, now - lastPressEventTimeStamp[button]);
    lastReleaseEventTimeStamp[button] = now;
}

template <uint8_t N>
uint32_t ButtonManager<N>::currentTimeStamp() {
    return duration_cast<milliseconds>(timer.elapsed_time()).count();
}

template class ButtonManager<4>;
template class ButtonManager<6>;