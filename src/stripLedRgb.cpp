#include "stripLedRgb.h"

stripLedRgb::stripLedRgb() {
  _pin_red = -1;
  _pin_green = -1;
  _pin_blue = -1;
  _duty_cycle = 1;

  _red_factor = 4;
  _green_factor = 2;
  _blue_factor = 1.2;

  _invert = false;
}

stripLedRgb::stripLedRgb(bool invert) {
  _pin_red = -1;
  _pin_green = -1;
  _pin_blue = -1;
  _duty_cycle = 1;

  _red_factor = 4;
  _green_factor = 2;
  _blue_factor = 1.2;

  _invert = invert;
}

stripLedRgb::stripLedRgb(uint8_t pin_red, uint8_t pin_green, uint8_t pin_blue, bool invert) {
  _pin_red = pin_red;
  _pin_green = pin_green;
  _pin_blue = pin_blue;
  _duty_cycle = 1;

  _red_factor = 4;
  _green_factor = 2;
  _blue_factor = 1.2;

  _invert = invert;
}

stripLedRgb::~stripLedRgb() {}

void stripLedRgb::hsl(float hue, float saturation, float lightness) {
  _hue = hue;
  _saturation = saturation;
  _lightness = lightness;
  _rgb = hsl_to_rgb(hue, saturation / 100, lightness / 100);
}

void stripLedRgb::hsl(float hue) { hsl(hue, _saturation, _lightness); }

void stripLedRgb::step(uint32_t time_step, float angle_step, float duty_cycle) {
  _time_step = time_step;
  _duty_cycle = duty_cycle;
  _angle_step = angle_step;
}

void stripLedRgb::factor(float red, float green, float blue) {
  _red_factor = red;
  _green_factor = green;
  _blue_factor = blue;
}

void stripLedRgb::time_step(uint32_t time_step) { _time_step = time_step; }

void stripLedRgb::duty_cycle(float duty_cycle) { _duty_cycle = duty_cycle; }

void stripLedRgb::angle_step(float angle_step) { _angle_step = angle_step; }

void stripLedRgb::begin() {
  begin(_pin_red, _pin_green, _pin_blue, DEFAULT_PWM_RGB_FREQUENCY, _invert);
}

void stripLedRgb::begin(uint32_t frequency_hz) {
  begin(_pin_red, _pin_green, _pin_blue, frequency_hz, _invert);
}

void stripLedRgb::begin(uint8_t pin_red, uint8_t pin_green, uint8_t pin_blue, bool invert) {
  begin(pin_red, pin_green, pin_blue, DEFAULT_PWM_RGB_FREQUENCY, invert);
}

void stripLedRgb::begin(uint8_t pin_red, uint8_t pin_green, uint8_t pin_blue,
                        uint32_t frequency_hz, bool invert) {
  _pin_red = pin_red;
  _pin_green = pin_green;
  _pin_blue = pin_blue;

  _invert = invert;

  if (_invert) {
    digitalWrite(_pin_red, 1);
    digitalWrite(_pin_green, 1);
    digitalWrite(_pin_blue, 1);
  } else {
    digitalWrite(_pin_red, 0);
    digitalWrite(_pin_green, 0);
    digitalWrite(_pin_blue, 0);
  }
  pinMode(_pin_red, OUTPUT);
  pinMode(_pin_green, OUTPUT);
  pinMode(_pin_blue, OUTPUT);

  analogWriteFreq(frequency_hz);
  analogWriteRange(DEFAULT_PWM_RGB_RANGE);

  step(0, 0, 1);
  hsl(0, 100, 0);
  handle();
}

void stripLedRgb::handle() {
  bool active_cycle = true;

  if (!_duty_cycle) {
    active_cycle = false;
  } else if (_time_step) {
    unsigned long now = millis();
    if (now > _end_of_active_time) {
      if (now > _end_of_inactive_time) {
        _end_of_active_time = now + (_time_step * _duty_cycle);
        _end_of_inactive_time = now + _time_step;
        hsl(fmod((_hue + _angle_step), 360));
      } else {
        active_cycle = false;
      }
    }
  }

  if (active_cycle == false) {
    if (_last_rgb.packed != 0) {
      _last_rgb = 0;
      analogWrite(_pin_red, 0);
      analogWrite(_pin_green, 0);
      analogWrite(_pin_blue, 0);
      if (_invert) {
        digitalWrite(_pin_red, 1);
        digitalWrite(_pin_green, 1);
        digitalWrite(_pin_blue, 1);
      }      
    }
    return;
  }

  if (_last_rgb == _rgb) {
    return;
  }

  _last_rgb = _rgb;
  if (_invert) {
    analogWrite(_pin_red, DEFAULT_PWM_RGB_RANGE - _rgb.r * _red_factor);
    analogWrite(_pin_green, DEFAULT_PWM_RGB_RANGE - _rgb.g * _green_factor);
    analogWrite(_pin_blue, DEFAULT_PWM_RGB_RANGE - _rgb.b * _blue_factor);
  } else {
    analogWrite(_pin_red, _rgb.r * _red_factor);
    analogWrite(_pin_green, _rgb.g * _green_factor);
    analogWrite(_pin_blue, _rgb.b * _blue_factor);
  }
}
