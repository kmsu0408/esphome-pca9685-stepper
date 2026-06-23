#pragma once

#include "esphome.h"
#include "esphome/components/stepper/stepper.h" // 부모 클래스 stepper::Stepper 인식을 위한 헤더
#include <esp_system.h> // esp_random() 사용을 위한 헤더

namespace esphome {

class Pca9685Stepper : public Component, public stepper::Stepper {
 public:
  output::FloatOutput *m3_pwm;
  output::FloatOutput *m3_in1;
  output::FloatOutput *m3_in2;
  output::FloatOutput *m4_pwm;
  output::FloatOutput *m4_in1;
  output::FloatOutput *m4_in2;

  uint32_t last_step_time{0};
  uint8_t step_state{0};

  // I2C 쓰기 빈도를 줄이기 위한 상태 캐싱 변수들 (6핀 확장)
  float last_m3_pwm{-1.0f}, last_m3_in1{-1.0f}, last_m3_in2{-1.0f};
  float last_m4_pwm{-1.0f}, last_m4_in1{-1.0f}, last_m4_in2{-1.0f};

  Pca9685Stepper(output::FloatOutput *m3_pwm_pin, output::FloatOutput *m3_in1_pin, output::FloatOutput *m3_in2_pin,
                 output::FloatOutput *m4_pwm_pin, output::FloatOutput *m4_in1_pin, output::FloatOutput *m4_in2_pin)
      : m3_pwm(m3_pwm_pin), m3_in1(m3_in1_pin), m3_in2(m3_in2_pin),
        m4_pwm(m4_pwm_pin), m4_in1(m4_in1_pin), m4_in2(m4_in2_pin) {}

  // 중복 I2C 쓰기를 필터링하는 H-브릿지 핀 제어 헬퍼 함수
  void set_pins(float m3_p, float m3_1, float m3_2, float m4_p, float m4_1, float m4_2) {
    if (m3_p != this->last_m3_pwm) { this->m3_pwm->set_level(m3_p); this->last_m3_pwm = m3_p; }
    if (m3_1 != this->last_m3_in1) { this->m3_in1->set_level(m3_1); this->last_m3_in1 = m3_1; }
    if (m3_2 != this->last_m3_in2) { this->m3_in2->set_level(m3_2); this->last_m3_in2 = m3_2; }
    if (m4_p != this->last_m4_pwm) { this->m4_pwm->set_level(m4_p); this->last_m4_pwm = m4_p; }
    if (m4_1 != this->last_m4_in1) { this->m4_in1->set_level(m4_1); this->last_m4_in1 = m4_1; }
    if (m4_2 != this->last_m4_in2) { this->m4_in2->set_level(m4_2); this->last_m4_in2 = m4_2; }
  }

  void setup() override {
    // 초기화: 모든 핀을 비활성화하여 기동 초기 모터 과열 예방
    this->set_pins(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    // ESP32 하드웨어 RNG 기반 난수 시드 초기화로 매번 고유한 무작위 패턴 생성
    srand(esp_random());
  }

  void dump_config() override {
    ESP_LOGCONFIG("pca9685_stepper", "PCA9685 H-Bridge TB6612 Stepper Driver");
  }

  void loop() override {
    if (this->current_position == this->target_position) {
      // 목표 위치 도달 시, 모터 과열 및 전원 소모 방지를 위해 H-브릿지 모든 출력 전류 차단 (Freewheeling)
      this->set_pins(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
      return;
    }

    uint32_t now = esphome::millis(); // ESP-IDF 호환을 위한 네임스페이스 명시 millis() 호출
    // 스텝 간 간격 계산 (밀리초)
    float speed = std::abs(this->max_speed);
    if (speed < 1.0f) speed = 1.0f; // 최저 속도 제한
    uint32_t interval = static_cast<uint32_t>(1000.0f / speed);

    if (now - this->last_step_time >= interval) {
      this->last_step_time = now;

      // 방향 판정 및 스텝 카운트 변경
      if (this->target_position > this->current_position) {
        this->step_state = (this->step_state + 1) % 4;
        this->current_position++;
      } else {
        this->step_state = (this->step_state - 1 + 4) % 4;
        this->current_position--;
      }

      // TB6612 H-브릿지 2상 여자(2-phase excitation) 출력 제어 시퀀스
      switch (this->step_state) {
        case 0: // 코일 1 (+), 코일 2 (+)
          this->set_pins(1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f);
          break;
        case 1: // 코일 1 (-), 코일 2 (+)
          this->set_pins(1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f);
          break;
        case 2: // 코일 1 (-), 코일 2 (-)
          this->set_pins(1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
          break;
        case 3: // 코일 1 (+), 코일 2 (-)
          this->set_pins(1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
          break;
      }
    }
  }
};

// YAML 람다식에서 간편하게 static 캐스팅 없이 제어할 수 있도록 전역 인스턴스 포인터 선언
inline Pca9685Stepper *global_pca9685_stepper = nullptr;

} // namespace esphome
