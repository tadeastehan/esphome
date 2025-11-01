from esphome import automation
import esphome.codegen as cg
from esphome.components import i2c, time
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@tadeastehan"]

DEPENDENCIES = ["i2c"]


mcp7940n_ns = cg.esphome_ns.namespace("mcp7940n")
MCP7940NComponent = mcp7940n_ns.class_(
    "MCP7940NComponent", time.RealTimeClock, i2c.I2CDevice
)
WriteAction = mcp7940n_ns.class_("WriteAction", automation.Action)
ReadAction = mcp7940n_ns.class_("ReadAction", automation.Action)


CONFIG_SCHEMA = time.TIME_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MCP7940NComponent),
    }
).extend(i2c.i2c_device_schema(0x6F))


@automation.register_action(
    "mcp7940n.write_time",
    WriteAction,
    automation.maybe_simple_id(
        {
            cv.GenerateID(): cv.use_id(MCP7940NComponent),
        }
    ),
)
async def mcp7940n_write_time_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "mcp7940n.read_time",
    ReadAction,
    automation.maybe_simple_id(
        {
            cv.GenerateID(): cv.use_id(MCP7940NComponent),
        }
    ),
)
async def mcp7940n_read_time_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    await time.register_time(var, config)
