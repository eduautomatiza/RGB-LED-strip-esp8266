#pragma once

#include <Arduino.h>
#include <inttypes.h>

#include "rgb_t.h"

class stripLedRgb {
 private:
#define DEFAULT_PWM_RGB_FREQUENCY 1000
#define DEFAULT_PWM_RGB_RANGE 1020

  uint8_t _pin_red, _pin_green, _pin_blue;
  float _hue, _saturation, _lightness;
  float _red_factor, _green_factor, _blue_factor;
  rgb_t _rgb;
  rgb_t _last_rgb;
  float _duty_cycle;
  uint32_t _time_step;
  float _angle_step;

  uint32_t _end_of_active_time;
  uint32_t _end_of_inactive_time;

  /* data */
 public:
  stripLedRgb();
  stripLedRgb(uint8_t pin_red, uint8_t pin_green, uint8_t pin_blue);
  ~stripLedRgb();

  void factor(float red, float green, float blue);

  void hsl(float hue, float saturation, float lightness);
  void hsl(float hue);

  void step(uint32_t time_step, float angle_step, float duty_cycle );

  void time_step(uint32_t time_step);
  void angle_step(float angle_step);
  void duty_cycle(float duty_cycle);

  float hue(void) { return _hue; }
  float saturation(void) { return _saturation; }
  float lightness(void) { return _lightness; }

  uint32_t time_step(void) { return _time_step; }
  float angle_step(void) { return _angle_step; }
  float duty_cycle(void) { return _duty_cycle; }

  void begin();
  void begin(uint32_t frequency_hz);
  void begin(uint8_t pin_red, uint8_t pin_green, uint8_t pin_blue);
  void begin(uint8_t pin_red, uint8_t pin_green, uint8_t pin_blue,
             uint32_t frequency_hz);

  void handle();
};
