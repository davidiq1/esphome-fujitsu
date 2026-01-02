import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_ID

CONF_RX_PIN = "rx_pin"
CONF_TX_PIN = "tx_pin"

# Debe coincidir con: namespace esphome { namespace fujitsu_uart { ... } }
fujitsu_uart_ns = cg.esphome_ns.namespace("fujitsu_uart")

FujitsuClimate = fujitsu_uart_ns.class_(
    "FujitsuClimate",
    climate.Climate,
    cg.Component,
)

# En ESPHome >= 2025.11, CLIMATE_SCHEMA fue reemplazado por climate_schema()
CONFIG_SCHEMA = climate.climate_schema(FujitsuClimate).extend(
    {
        cv.GenerateID(): cv.declare_id(FujitsuClimate),

        # Pines para Serial1.begin(..., rx, tx) vía defines
        cv.Optional(CONF_RX_PIN, default=1): cv.int_range(min=0, max=48),
        cv.Optional(CONF_TX_PIN, default=0): cv.int_range(min=0, max=48),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    # Inyecta defines de compilación (equivalente a -D...)
    cg.add_define("FUJI_UART_RX_PIN", str(config[CONF_RX_PIN]))
    cg.add_define("FUJI_UART_TX_PIN", str(config[CONF_TX_PIN]))
