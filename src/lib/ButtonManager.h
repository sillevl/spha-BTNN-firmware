# pragma once

#include "mbed.h"

using namespace std::chrono;

class ButtonManager {
    public:
        enum ButtonState {  RELEASED = 0, PRESSED = 1 };

        ButtonManager(EventQueue* queue, const PinName* pinnames, uint8_t numberOfButtons);
        void onStateChange(mbed::Callback<void(uint8_t buttonIndex, ButtonState state, uint32_t time)> cb);

    private:
        uint32_t currentTimeStamp();

        InterruptIn* buttons[6];
        EventQueue* queue;
        mbed::Callback<void(uint8_t buttonIndex, ButtonState state, uint32_t time)> stateChanged;

        Timer timer;
        uint32_t lastPressEventTimeStamp[6]= { 0 };
        uint32_t lastReleaseEventTimeStamp[6]= { 0 };

        void released(uint8_t index);
        void pressed(uint8_t index);
};