import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, uart
from esphome.const import CONF_ID

# Definimos las dependencias
DEPENDENCIES = ['uart']

# Creamos el espacio de nombres (namespace) fujitsu
# Esto debe coincidir con el "namespace fujitsu" que suele estar en los .h/.cpp
fujitsu_ns = cg.esphome_ns.namespace('fujitsu')
FujitsuClimate = fujitsu_ns.class_('FujitsuClimate', climate.Climate, cg.Component)

# Esquema de configuración para el YAML
CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(FujitsuClimate),
}).extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    # Esta función le dice a ESPHome qué archivos compilar
    var = cg.new_Pvariable(config[CONF_ID])
    
    # Registramos los archivos .cpp para que el compilador los incluya
    cg.add_sources("FujiHeatPump.cpp", "FujitsuClimate.cpp")
    
    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)
    yield uart.register_uart_device(var, config)
