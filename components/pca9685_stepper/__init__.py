import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID
from esphome.components import output

# stepper 컴포넌트 라이브러리 강제 오토로드 설정
AUTO_LOAD = ["stepper"]

# ESPHome 글로벌 네임스페이스 매핑
ns = cg.esphome_ns
Pca9685Stepper = ns.class_("Pca9685Stepper", cg.Component)

# TB6612 H-브릿지 제어용 6개 핀 파라미터 상수 정의
CONF_M3_PWM = "m3_pwm"
CONF_M3_IN1 = "m3_in1"
CONF_M3_IN2 = "m3_in2"
CONF_M4_PWM = "m4_pwm"
CONF_M4_IN1 = "m4_in1"
CONF_M4_IN2 = "m4_in2"

# YAML 설정 스키마 정의
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Pca9685Stepper),
    cv.Required(CONF_M3_PWM): cv.use_id(output.FloatOutput),
    cv.Required(CONF_M3_IN1): cv.use_id(output.FloatOutput),
    cv.Required(CONF_M3_IN2): cv.use_id(output.FloatOutput),
    cv.Required(CONF_M4_PWM): cv.use_id(output.FloatOutput),
    cv.Required(CONF_M4_IN1): cv.use_id(output.FloatOutput),
    cv.Required(CONF_M4_IN2): cv.use_id(output.FloatOutput),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    # 생성된 C++ 파일 상단에 헤더 파일을 강제 인클루드 하도록 지시 (ESPHome 컴포넌트 풀 패스 사용)
    cg.add_global(cg.RawStatement('#include "esphome/components/pca9685_stepper/pca9685_stepper.h"'))

    # 각 핀(PCA9685 H-브릿지 채널)의 C++ 변수 포인터 획득
    m3_pwm_var = await cg.get_variable(config[CONF_M3_PWM])
    m3_in1_var = await cg.get_variable(config[CONF_M3_IN1])
    m3_in2_var = await cg.get_variable(config[CONF_M3_IN2])
    m4_pwm_var = await cg.get_variable(config[CONF_M4_PWM])
    m4_in1_var = await cg.get_variable(config[CONF_M4_IN1])
    m4_in2_var = await cg.get_variable(config[CONF_M4_IN2])

    # C++ 생성자를 호출하여 인스턴스를 동적으로 생성 (6개 핀 주입)
    var = cg.new_Pvariable(config[CONF_ID], 
                           m3_pwm_var, m3_in1_var, m3_in2_var,
                           m4_pwm_var, m4_in1_var, m4_in2_var)

    # ESPHome 프레임워크에 컴포넌트 및 라이프사이클(setup/loop 등) 등록
    await cg.register_component(var, config)
