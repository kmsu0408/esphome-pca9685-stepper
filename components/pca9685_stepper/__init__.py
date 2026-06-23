import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID
from esphome.components import output

# ESPHome 글로벌 네임스페이스 매핑
ns = cg.esphome_ns
Pca9685Stepper = ns.class_("Pca9685Stepper", cg.Component)

# 필수 설정용 핀 파라미터 상수 정의
CONF_PIN_A = "pin_a"
CONF_PIN_B = "pin_b"
CONF_PIN_C = "pin_c"
CONF_PIN_D = "pin_d"

# YAML 설정 스키마 정의
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Pca9685Stepper),
    cv.Required(CONF_PIN_A): cv.use_id(output.FloatOutput),
    cv.Required(CONF_PIN_B): cv.use_id(output.FloatOutput),
    cv.Required(CONF_PIN_C): cv.use_id(output.FloatOutput),
    cv.Required(CONF_PIN_D): cv.use_id(output.FloatOutput),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    # 생성된 C++ 파일 상단에 헤더 파일을 강제 인클루드 하도록 지시 (ESPHome 컴포넌트 풀 패스 사용)
    cg.add_global(cg.RawStatement('#include "esphome/components/pca9685_stepper/pca9685_stepper.h"'))

    # 각 핀(PCA9685 FloatOutput 채널)의 C++ 변수 포인터 획득
    pin_a_var = await cg.get_variable(config[CONF_PIN_A])
    pin_b_var = await cg.get_variable(config[CONF_PIN_B])
    pin_c_var = await cg.get_variable(config[CONF_PIN_C])
    pin_d_var = await cg.get_variable(config[CONF_PIN_D])

    # C++ 생성자를 호출하여 인스턴스를 동적으로 생성
    var = cg.new_Pvariable(config[CONF_ID], pin_a_var, pin_b_var, pin_c_var, pin_d_var)

    # ESPHome 프레임워크에 컴포넌트 및 라이프사이클(setup/loop 등) 등록
    await cg.register_component(var, config)
