#include "mbed.h"
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

static const uint16_t FirmwareVersion = 0x0010;
static const uint16_t DeviceType = 0x0011;

static const PinName BUTTONS[6] = { PA_5, PA_6, PA_7, PA_8, PA_9, PA_10 };

I2C i2c(MBED_CONF_APP_SDA_PIN, MBED_CONF_APP_SCL_PIN);
CAN can(MBED_CONF_SIXPACK_CAN_TXD_PIN, MBED_CONF_SIXPACK_CAN_RXD_PIN, MBED_CONF_SIXPACK_CAN_BITRATE);

static const int I2C_ADDRESS = 0x80;
TLC59108 leds(&i2c,I2C_ADDRESS);
BME280 bme280(i2c);

// TODO: ButtonManager needs own queue???
EventQueue queue(32 * EVENTS_EVENT_SIZE);
ButtonManager buttons(&queue, BUTTONS, 4);

SixPack sixPack(&can, &queue);

FileHandle *mbed_override_console(int fd)
{
    static BufferedSerial console(MBED_CONF_APP_UART_TX, MBED_CONF_APP_UART_RX, MBED_CONF_PLATFORM_STDIO_BAUD_RATE);
    return &console;
}

void initLeds() {
    leds.enable();
    for(int i = 0; i < 6; i++) {
        leds.setChannel(i, 0);
    }
}

int main() {
    mbed_trace_init();
    mbed_trace_config_set(TRACE_MODE_COLOR | TRACE_ACTIVE_LEVEL_ALL);

    tr_info("*** SPHA Button Universal ***");

    initLeds();

    sixPack.setDeviceType(DeviceType);
    sixPack.setFirmwareVersion(FirmwareVersion);

    ActivityLed commsLed(MBED_CONF_APP_STATUS_LED_PIN);
    BusVoltage busVoltage(MBED_CONF_APP_VBUS_ADC);

    FeedbackLed feedbackLeds;
    feedbackLeds.onMessage([leds](uint8_t index, uint8_t brightness){
        leds.setChannel(index, brightness / 255.0f);
    });

    sixPack.registerComponent(&commsLed);
    sixPack.registerComponent(&busVoltage);
    sixPack.registerComponent(&feedbackLeds);


    buttons.onStateChange([] (uint8_t index, ButtonManager::ButtonState state, uint32_t time) {
        tr_info("Button %d: %d, (%d)", index, state, time);
        sixPack.send(Events::Button(index, state, time));
    });

    queue.dispatch_forever();
}