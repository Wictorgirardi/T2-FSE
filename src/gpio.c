#include "../lib/gpio.h"
#include <softPwm.h>
#include <wiringPi.h>

#define RESISTOR_PIN 4
#define AF_PIN 5

void turn_resistance_on(int new_resistor_value) {
  pinMode(RESISTOR_PIN, OUTPUT);
  softPwmCreate(RESISTOR_PIN, 0, 100);
  softPwmWrite(RESISTOR_PIN, new_resistor_value);
}

void turn_resistance_off() {
  pinMode(RESISTOR_PIN, OUTPUT);
  softPwmCreate(RESISTOR_PIN, 0, 100);
  softPwmWrite(RESISTOR_PIN, 0);
}

void fanOn(int new_af_value) {
  pinMode(AF_PIN, OUTPUT);
  softPwmCreate(AF_PIN, 0, 100);
  softPwmWrite(AF_PIN, new_af_value);
}

void fanOff() {
  pinMode(AF_PIN, OUTPUT);
  softPwmCreate(AF_PIN, 0, 100);
  softPwmWrite(AF_PIN, 0);
}

void pwm_control(int intensity_signal) {
  if (intensity_signal > 0) {
    turn_resistance_on(intensity_signal);
    fanOff();
  } else {
    if (intensity_signal <= -40)
      fanOn(intensity_signal * -1);
    else
      fanOff();
    turn_resistance_off();
  }
}
