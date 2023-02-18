# pragma once

#include "mbed.h"

using namespace std::chrono;

template <uint8_t N>
class ButtonManager {
    public:
        enum ButtonState {  RELEASED = 0, PRESSED = 1 };

        ButtonManager(EventQueue* queue, const std::array<PinName, N> pinnames);
        void onStateChange(mbed::Callback<void(uint8_t buttonIndex, ButtonState state, uint32_t time)> cb);

    private:
        uint32_t currentTimeStamp();

        InterruptIn* buttons[N];
        EventQueue* queue;
        mbed::Callback<void(uint8_t buttonIndex, ButtonState state, uint32_t time)> stateChanged;

        Timer timer;
        uint32_t lastPressEventTimeStamp[N]= { 0 };
        uint32_t lastReleaseEventTimeStamp[N]= { 0 };

        void released(uint8_t index);
        void pressed(uint8_t index);
};