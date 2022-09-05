#ifndef GPIO_H_
#define GPIO_H_

void pwm_control(int intensity_signal);
void turn_resistance_off();
void turn_resistance_on(int new_resistor_value);
void fanOff();
void fanOn(int fan_value);

#endif /* PID_H_ */