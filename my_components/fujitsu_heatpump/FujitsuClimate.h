#pragma once
#ifndef ESPHOME_FUJITSU_UART_FUJITSU_CLIMATE_H
#define ESPHOME_FUJITSU_UART_FUJITSU_CLIMATE_H

#include "FujiHeatPump.h"

#include "esphome/core/component.h"
#include "esphome/core/optional.h"
#include "esphome/components/climate/climate.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

namespace esphome {
namespace fujitsu_uart {

class FujitsuClimate : public climate::Climate, public Component {
 public:
  void setup() override;
  void loop() override;
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  TaskHandle_t taskHandle{};
  FujiHeatPump heatPump;
  FujiFrame sharedState{};
  SemaphoreHandle_t lock{};
  bool pendingUpdate{false};

 protected:
  void updateState();

  optional<climate::ClimateMode> fujiToEspMode(FujiMode fujiMode);
  optional<FujiMode> espToFujiMode(climate::ClimateMode espMode);

  optional<climate::ClimateFanMode> fujiToEspFanMode(FujiFanMode fujiFanMode);
  optional<FujiFanMode> espToFujiFanMode(climate::ClimateFanMode espFanMode);
};

}  // namespace fujitsu_uart
}  // namespace esphome

#endif  // ESPHOME_FUJITSU_UART_FUJITSU_CLIMATE_H
