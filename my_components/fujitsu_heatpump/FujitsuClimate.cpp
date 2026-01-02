#include "FujitsuClimate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace fujitsu_uart {

// --- UART pin override (inyectado desde climate.py con cg.add_define) ---
#ifndef FUJI_UART_RX_PIN
#define FUJI_UART_RX_PIN 1  // GPIO1 (RX)
#endif

#ifndef FUJI_UART_TX_PIN
#define FUJI_UART_TX_PIN 0  // GPIO0 (TX)
#endif

// ESP32-C3 es unicore -> core 0
static inline int fuji_task_core() {
#if defined(SOC_CPU_CORES_NUM) && (SOC_CPU_CORES_NUM > 1)
  return 1;
#else
  return 0;
#endif
}

static void serialTask(void *pvParameters) {
  auto *climate = static_cast<FujitsuClimate *>(pvParameters);

  ESP_LOGD("fuji", "serialTask started on core %d", xPortGetCoreID());

  for (;;) {
    if (climate->heatPump.waitForFrame()) {
      delay(60);
      climate->heatPump.sendPendingFrame();
      climate->pendingUpdate = false;
    }

    if (xSemaphoreTake(climate->lock, (TickType_t) 200) == pdTRUE) {
      memcpy(&(climate->sharedState), climate->heatPump.getCurrentState(), sizeof(FujiFrame));
      xSemaphoreGive(climate->lock);
    }
  }
}

void FujitsuClimate::setup() {
  ESP_LOGD("fuji", "Fuji initialized");

  this->lock = xSemaphoreCreateBinary();
  xSemaphoreGive(this->lock);

  this->pendingUpdate = false;
  memcpy(&(this->sharedState), this->heatPump.getCurrentState(), sizeof(FujiFrame));

  // CLAVE: ESP32-C3 -> Serial1. Serial2 no existe en C3.
  // RX/TX vienen por defines desde YAML: rx_pin / tx_pin.
  this->heatPump.connect(&Serial1, true, FUJI_UART_RX_PIN, FUJI_UART_TX_PIN);

  ESP_LOGD("fuji", "starting task (core=%d)", fuji_task_core());
  xTaskCreatePinnedToCore(
      serialTask,
      "FujiTask",
      10000,
      (void *) this,
      configMAX_PRIORITIES - 1,
      &(this->taskHandle),
      fuji_task_core());
}

// ---- El resto de tu fichero lo puedes dejar igual (control/traits/etc) ----
// Pega aquí tu implementación existente de:
//  - fujiToEspMode / espToFujiMode
//  - fujiToEspFanMode / espToFujiFanMode
//  - updateState / loop / control / traits
// sin cambiar nada funcional.

}  // namespace fujitsu_uart
}  // namespace esphome
