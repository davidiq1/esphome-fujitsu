/* This file is based on unreality's FujiHeatPump project */

// #define DEBUG_FUJI
#include "FujiHeatPump.h"
#include "esphome/core/log.h"

// Workaround for ESPHome log macro namespace quirk
using esphome::esp_log_printf_;

namespace esphome {
namespace fujitsu_uart {

FujiFrame FujiHeatPump::decodeFrame()
{
    FujiFrame ff;

    ff.messageSource = readBuf[0];
    ff.messageDest = readBuf[1] & 0b01111111;
    ff.messageType = (readBuf[2] & 0b00110000) >> 4;

    ff.acError = (readBuf[kErrorIndex] & kErrorMask) >> kErrorOffset;
    ff.temperature =
        (readBuf[kTemperatureIndex] & kTemperatureMask) >> kTemperatureOffset;
    ff.acMode = (readBuf[kModeIndex] & kModeMask) >> kModeOffset;
    ff.fanMode = (readBuf[kFanIndex] & kFanMask) >> kFanOffset;
    ff.economyMode = (readBuf[kEconomyIndex] & kEconomyMask) >> kEconomyOffset;
    ff.swingMode = (readBuf[kSwingIndex] & kSwingMask) >> kSwingOffset;
    ff.swingStep =
        (readBuf[kSwingStepIndex] & kSwingStepMask) >> kSwingStepOffset;
    ff.controllerPresent =
        (readBuf[kControllerPresentIndex] & kControllerPresentMask) >>
        kControllerPresentOffset;
    ff.updateMagic =
        (readBuf[kUpdateMagicIndex] & kUpdateMagicMask) >> kUpdateMagicOffset;
    ff.onOff = (readBuf[kEnabledIndex] & kEnabledMask) >> kEnabledOffset;
    ff.controllerTemp = (readBuf[kControllerTempIndex] & kControllerTempMask) >> kControllerTempOffset;

    ff.writeBit = (readBuf[2] & 0b00001000) != 0;
    ff.loginBit = (readBuf[1] & 0b00100000) != 0;
    ff.unknownBit = (readBuf[1] & 0b10000000) > 0;

    return ff;
}

void FujiHeatPump::encodeFrame(FujiFrame ff)
{
    memset(writeBuf, 0, 8);

    writeBuf[0] = ff.messageSource;

    writeBuf[1] &= 0b10000000;
    writeBuf[1] |= ff.messageDest & 0b01111111;

    writeBuf[2] &= 0b11001111;
    writeBuf[2] |= ff.messageType << 4;

    if (ff.writeBit)
        writeBuf[2] |= 0b00001000;
    else
        writeBuf[2] &= 0b11110111;

    writeBuf[1] &= 0b01111111;
    if (ff.unknownBit)
        writeBuf[1] |= 0b10000000;

    if (ff.loginBit)
        writeBuf[1] |= 0b00100000;
    else
        writeBuf[1] &= 0b11011111;

    writeBuf[kModeIndex] =
        (writeBuf[kModeIndex] & ~kModeMask) | (ff.acMode << kModeOffset);
    writeBuf[kModeIndex] = (writeBuf[kEnabledIndex] & ~kEnabledMask) |
                           (ff.onOff << kEnabledOffset);
    writeBuf[kFanIndex] =
        (writeBuf[kFanIndex] & ~kFanMask) | (ff.fanMode << kFanOffset);
    writeBuf[kErrorIndex] =
        (writeBuf[kErrorIndex] & ~kErrorMask) | (ff.acError << kErrorOffset);
    writeBuf[kEconomyIndex] = (writeBuf[kEconomyIndex] & ~kEconomyMask) |
                              (ff.economyMode << kEconomyOffset);
    writeBuf[kTemperatureIndex] =
        (writeBuf[kTemperatureIndex] & ~kTemperatureMask) |
        (ff.temperature << kTemperatureOffset);
    writeBuf[kSwingIndex] =
        (writeBuf[kSwingIndex] & ~kSwingMask) | (ff.swingMode << kSwingOffset);
    writeBuf[kSwingStepIndex] = (writeBuf[kSwingStepIndex] & ~kSwingStepMask) |
                                (ff.swingStep << kSwingStepOffset);
    writeBuf[kControllerPresentIndex] =
        (writeBuf[kControllerPresentIndex] & ~kControllerPresentMask) |
        (ff.controllerPresent << kControllerPresentOffset);
    writeBuf[kUpdateMagicIndex] =
        (writeBuf[kUpdateMagicIndex] & ~kUpdateMagicMask) |
        (ff.updateMagic << kUpdateMagicOffset);
    writeBuf[kControllerTempIndex] =
        (writeBuf[kControllerTempIndex] & ~kControllerTempMask) |
        (ff.controllerTemp << kControllerTempOffset);
}

void FujiHeatPump::connect(HardwareSerial *serial, bool secondary)
{
    return this->connect(serial, secondary, -1, -1);
}

void FujiHeatPump::connect(HardwareSerial *serial, bool secondary, int rxPin, int txPin)
{
    _serial = serial;
    if (rxPin != -1 && txPin != -1)
    {
#ifdef ESP32
        _serial->begin(500, SERIAL_8E1, rxPin, txPin);
#else
        ::Serial.print("Setting RX/TX pin unsupported, using defaults.\n");
        _serial->begin(500, SERIAL_8E1);
#endif
    }
    else
    {
        _serial->begin(500, SERIAL_8E1);
    }
    _serial->setTimeout(200);

    if (secondary)
    {
        controllerIsPrimary = false;
        controllerAddress = static_cast<byte>(FujiAddress::SECONDARY);
    }
    else
    {
        controllerIsPrimary = true;
        controllerAddress = static_cast<byte>(FujiAddress::PRIMARY);
    }

    lastFrameReceived = 0;
}

void FujiHeatPump::printFrame(byte buf[8], FujiFrame ff)
{
    ESP_LOGD("fuji", "%X %X %X %X %X %X %X %X  ", buf[0], buf[1], buf[2],
             buf[3], buf[4], buf[5], buf[6], buf[7]);
    ESP_LOGD(
        "fuji",
        " mSrc: %d mDst: %d mType: %d write: %d login: %d unknown: %d onOff: "
        "%d temp: %d, mode: %d cP:%d uM:%d cTemp:%d acError:%d \n",
        ff.messageSource, ff.messageDest, ff.messageType, ff.writeBit,
        ff.loginBit, ff.unknownBit, ff.onOff, ff.temperature, ff.acMode,
        ff.controllerPresent, ff.updateMagic, ff.controllerTemp, ff.acError);
}

// ... (resto EXACTO de tu fichero, sin cambios) ...

byte FujiHeatPump::getUpdateFields() { return updateFields; }

}  // namespace fujitsu_uart
}  // namespace esphome
