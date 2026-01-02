#pragma once
#ifndef ESPHOME_FUJITSU_UART_FUJI_HEATPUMP_H
#define ESPHOME_FUJITSU_UART_FUJI_HEATPUMP_H

#include <Arduino.h>
#include <HardwareSerial.h>

namespace esphome {
namespace fujitsu_uart {

// --- Protocolo / offsets ---
static const byte kModeIndex = 3;
static const byte kModeMask = 0b00001110;
static const byte kModeOffset = 1;

static const byte kFanIndex = 3;
static const byte kFanMask = 0b01110000;
static const byte kFanOffset = 4;

static const byte kEnabledIndex = 3;
static const byte kEnabledMask = 0b00000001;
static const byte kEnabledOffset = 0;

static const byte kErrorIndex = 3;
static const byte kErrorMask = 0b10000000;
static const byte kErrorOffset = 7;

static const byte kEconomyIndex = 4;
static const byte kEconomyMask = 0b10000000;
static const byte kEconomyOffset = 7;

static const byte kTemperatureIndex = 4;
static const byte kTemperatureMask = 0b01111111;
static const byte kTemperatureOffset = 0;

static const byte kUpdateMagicIndex = 5;
static const byte kUpdateMagicMask = 0b11110000;
static const byte kUpdateMagicOffset = 4;

static const byte kSwingIndex = 5;
static const byte kSwingMask = 0b00000100;
static const byte kSwingOffset = 2;

static const byte kSwingStepIndex = 5;
static const byte kSwingStepMask = 0b00000010;
static const byte kSwingStepOffset = 1;

static const byte kControllerPresentIndex = 6;
static const byte kControllerPresentMask = 0b00000001;
static const byte kControllerPresentOffset = 0;

static const byte kControllerTempIndex = 6;
static const byte kControllerTempMask = 0b01111110;
static const byte kControllerTempOffset = 1;

// --- Estructura de estado ---
typedef struct FujiFrames {
  byte onOff = 0;
  byte temperature = 16;
  byte acMode = 0;
  byte fanMode = 0;
  byte acError = 0;
  byte economyMode = 0;
  byte swingMode = 0;
  byte swingStep = 0;
  byte controllerPresent = 0;
  byte updateMagic = 0;
  byte controllerTemp = 16;

  bool writeBit = false;
  bool loginBit = false;
  bool unknownBit = false;

  byte messageType = 0;
  byte messageSource = 0;
  byte messageDest = 0;
} FujiFrame;

// --- Enums ---
enum class FujiMode : byte { UNKNOWN = 0, FAN = 1, DRY = 2, COOL = 3, HEAT = 4, AUTO = 5 };
enum class FujiMessageType : byte { STATUS = 0, ERROR = 1, LOGIN = 2, UNKNOWN = 3 };
enum class FujiAddress : byte { START = 0, UNIT = 1, PRIMARY = 32, SECONDARY = 33 };
enum class FujiFanMode : byte { FAN_AUTO = 0, FAN_QUIET = 1, FAN_LOW = 2, FAN_MEDIUM = 3, FAN_HIGH = 4 };

// --- Máscaras de “update” ---
static const byte kOnOffUpdateMask = 0b10000000;
static const byte kTempUpdateMask = 0b01000000;
static const byte kModeUpdateMask = 0b00100000;
static const byte kFanModeUpdateMask = 0b00010000;
static const byte kEconomyModeUpdateMask = 0b00001000;
static const byte kSwingModeUpdateMask = 0b00000100;
static const byte kSwingStepUpdateMask = 0b00000010;

// --- Clase principal ---
class FujiHeatPump {
 public:
  void connect(HardwareSerial *serial, bool secondary);
  void connect(HardwareSerial *serial, bool secondary, int rxPin, int txPin);

  bool waitForFrame();
  void sendPendingFrame();
  bool isBound();
  bool updatePending();

  void setOnOff(bool o);
  void setTemp(byte t);
  void setMode(byte m);
  void setFanMode(byte fm);
  void setEconomyMode(byte em);
  void setSwingMode(byte sm);
  void setSwingStep(byte ss);
  void setState(FujiFrame *state);

  bool getOnOff();
  byte getTemp();
  byte getMode();
  byte getFanMode();
  byte getEconomyMode();
  byte getSwingMode();
  byte getSwingStep();
  byte getControllerTemp();

  FujiFrame *getCurrentState();
  FujiFrame *getUpdateState();
  byte getUpdateFields();

 private:
  HardwareSerial *_serial = nullptr;

  byte readBuf[8]{};
  byte writeBuf[8]{};

  byte controllerAddress = 0;
  bool controllerIsPrimary = true;
  bool seenSecondaryController = false;
  unsigned long lastFrameReceived = 0;

  byte updateFields = 0;
  FujiFrame updateState{};
  FujiFrame currentState{};

  bool pendingFrame = false;

  FujiFrame decodeFrame();
  void encodeFrame(FujiFrame ff);
  void printFrame(byte buf[8], FujiFrame ff);
};

}  // namespace fujitsu_uart
}  // namespace esphome

#endif  // ESPHOME_FUJITSU_UART_FUJI_HEATPUMP_H
