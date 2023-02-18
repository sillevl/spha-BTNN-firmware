#include "mbed.h"
#include "./version.h"
#include "sixpack/version.h"
#include "lib/ButtonManager.h"
#include "mbed_trace.h"

#include "SixPack.h"
#include "Messages/Events.h"
#include "StatusLed.h"
#include "ActivityLed.h"
#include "BusVoltage.h"
#include "FeedbackLed.h"
#include "TLC59108.h"

using namespace SixPackLib;

#define TRACE_GROUP "APP"

// SixPack
EventQueue queue(32 * EVENTS_EVENT_SIZE);
SixPack sixPack(&queue, FIRMWARE_VERSION_NUMBER);

// Board drivers
I2C i2c(MBED_CONF_APP_SDA_PIN, MBED_CONF_APP_SCL_PIN);
TLC59108 leds(&i2c,MBED_CONF_APP_TLC59116_I2C_ADDRESS);
ButtonManager buttons(
    &queue,
    (const PinName[]) { MBED_CONF_APP_BUTTON_1, MBED_CONF_APP_BUTTON_2, MBED_CONF_APP_BUTTON_3, MBED_CONF_APP_BUTTON_4 },
    MBED_CONF_APP_NUMBER_OF_BUTTONS
);

// Override stdio
FileHandle *mbed_override_console(int fd) {
    static BufferedSerial console(MBED_CONF_APP_UART_TX, MBED_CONF_APP_UART_RX, MBED_CONF_PLATFORM_STDIO_BAUD_RATE);
    return &console;
}

// Initialize LEDs
void initLeds() {
    leds.enable();
    leds.setBrightness(1.0f);
    for(int i = 0; i < 6; i++) {
        leds.setChannel(i, 1.0f);
    }
}

int main() {
    // Enable mbed trace
    mbed_trace_init();
    mbed_trace_config_set(TRACE_MODE_COLOR | TRACE_ACTIVE_LEVEL_ALL);

    // show hardware and firmware info
    tr_info("\n\n*** SPHA Button Universal ***");
    tr_info("Firmware version: %s", FIRMWARE_VERSION);
    tr_info("Sixpack version: %s", SIXPACK_FIRMWARE_VERSION);

    // Initialize LEDs
    initLeds();

    // Initialize Sixpack modules
    ActivityLed commsLed(MBED_CONF_APP_STATUS_LED_PIN);
    BusVoltage busVoltage(MBED_CONF_APP_VBUS_ADC);
    FeedbackLed feedbackLeds;
    feedbackLeds.onMessage([leds](uint8_t index, uint8_t brightness){
        leds.setChannel(index, brightness / 255.0f);
    });

    // Handle button events
    buttons.onStateChange([] (uint8_t index, ButtonManager::ButtonState state, uint32_t time) {
        tr_info("Button %d: %d, (%d)", index, state, time);
        sixPack.send(Events::Button(index, state, time));
    });

    // Register modules
    sixPack.registerComponent(&commsLed);
    sixPack.registerComponent(&busVoltage);
    sixPack.registerComponent(&feedbackLeds);

    // Handle evens
    queue.dispatch_forever();
}