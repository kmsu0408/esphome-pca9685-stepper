#pragma once

#include "esphome.h"
#include <esp_system.h> // esp_random() 사용을 위한 헤더

namespace esphome {

class Pca9685Stepper : public Component, public stepper::Stepper {
 public:
  output::FloatOutput *pin_a;
  output::FloatOutput *pin_b;
  output::FloatOutput *pin_c;
  output::FloatOutput *pin_d;

  uint32_t last_step_time{0};
  uint8_t step_state{0};

  // I2C 쓰기 빈도를 줄이기 위한 상태 캐싱 변수들
  float last_a{-1.0f};
  float last_b{-1.0f};
  float last_c{-1.0f};
  float last_d{-1.0f};

  Pca9685Stepper(output::FloatOutput *a, output::FloatOutput *b,
                 output::FloatOutput *c, output::FloatOutput *d)
      : pin_a(a), pin_b(b), pin_c(c), pin_d(d) {}

  // 중복 I2C 쓰기를 필터링하는 핀 제어 헬퍼 함수
  void set_pins(float a, float b, float c, float d) {
    if (a != this->last_a) { this->pin_a->set_level(a); this->last_a = a; }
    if (b != this->last_b) { this->pin_b->set_level(b); this->last_b = b; }
    if (c != this->last_c) { this->pin_c->set_level(c); this->last_c = c; }
    if (d != this->last_d) { this->pin_d->set_level(d); this->last_d = d; }
  }

  void setup() override {
    // 초기화: 모든 핀을 비활성화하여 기동 초기 모터 과열 예방
    this->set_pins(0.0f, 0.0f, 0.0f, 0.0f);
    // ESP32 하드웨어 RNG 기반 난수 시드 초기화로 매번 고유한 무작위 패턴 생성
    srand(esp_random());
  }

  void dump_config() override {
    ESP_LOGCONFIG("pca9685_stepper", "PCA9685 Stepper Motor Driver");
  }

  void loop() override {
    if (this->current_position == this->target_position) {
      // 목표 위치 도달 시, 모터 과열 및 전원 소모 방지를 위해 모든 코일 전류 차단 (Freewheeling)
      this->set_pins(0.0f, 0.0f, 0.0f, 0.0f);
      return;
    }

    uint32_t now = millis();
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

      // 4상 스텝 모터의 2상 여자(2-phase excitation) 출력 시퀀스 설정
      switch (this->step_state) {
        case 0:
          this->set_pins(1.0f, 0.0f, 1.0f, 0.0f);
          break;
        case 1:
          this->set_pins(0.0f, 1.0f, 1.0f, 0.0f);
          break;
        case 2:
          this->set_pins(0.0f, 1.0f, 0.0f, 1.0f);
          break;
        case 3:
          this->set_pins(1.0f, 0.0f, 0.0f, 1.0f);
          break;
      }
    }
  }
};

// YAML 람다식에서 간편하게 static 캐스팅 없이 제어할 수 있도록 전역 인스턴스 포인터 선언
inline Pca9685Stepper *global_pca9685_stepper = nullptr;

} // namespace esphome
