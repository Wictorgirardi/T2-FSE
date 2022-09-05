#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>

#include "../lib/bme280.h"
#include "../lib/display.h"
#include "../lib/gpio.h"
#include "../lib/pid.h"
#include "../lib/thermometer.h"
#include "../lib/uart.h"

#define true 1
#define false 0

struct bme280_dev bme_connection;
int uart_filesystem, key_gpio = 1;
int timer_trigger = 0;
float ref_temp = 0;

void timer_interrupt(int sig_num);

void shutdown_program()
{
  printf("Programa finalizado\n");
  turn_resistance_off();
  fanOff();
  close_uart(uart_filesystem);
  exit(0);
}

void pid_routine(int key)
{
  float TI, TE;
  int value_to_send = 0;
  pid_setup(30, 0.2, 400);

  do
  {
    // Read internal temp
    write_uart_get(uart_filesystem, GET_INTERNAL_TEMP);
    TI = read_uart(uart_filesystem, GET_INTERNAL_TEMP).float_value;

    // Calculate resistance duty cycle
    double value_to_send = pid_control(TI);
    pwm_control(value_to_send);

    // Read reference temp
    write_uart_get(uart_filesystem, GET_REFERENCE_TEMP);
    ref_temp = read_uart(uart_filesystem, GET_REFERENCE_TEMP).float_value;

    // Register signal to be sent in timer_trigger seconds
    if (TI - ref_temp < 1)
    {
      ualarm(timer_trigger * 1000000, 0);
    }

    // Get external temp
    pid_reference(ref_temp);
    TE = get_current_temperature(&bme_connection);

    // Updates display 16x2
    printf("\tTI: %.2f⁰C - TR: %.2f⁰C - TE: %.2f⁰C\n", TI, ref_temp, TE);
    print_display(TI, ref_temp, TE);

    // Read commands from keyboard
    write_uart_get(uart_filesystem, GET_KEY_VALUE);
    key_gpio = read_uart(uart_filesystem, GET_KEY_VALUE).int_value;
    execute_keyboard(key_gpio);

    write_uart_send(uart_filesystem, value_to_send);
  } while (key_gpio == key);
}

void terminal_routine(float TR, int key)
{
  float TI, TE;
  int value_to_send = 0;
  pid_setup(30, 0.2, 400);
  do
  {
    // Read internal temp
    write_uart_get(uart_filesystem, GET_INTERNAL_TEMP);
    TI = read_uart(uart_filesystem, GET_INTERNAL_TEMP).float_value;

    // Calculate resistance duty cycle
    double value_to_send = pid_control(TI);
    pwm_control(value_to_send);

    pid_reference(TR);

    // Register signal to be sent in timer_trigger seconds
    if (TI - TR < 1)
    {
      ualarm(timer_trigger * 1000000, 0);
    }

    // Get external temp
    TE = get_current_temperature(&bme_connection);

    // Updates display 16x2
    printf("\tTERMINAL TI: %.2f⁰C - TR: %.2f⁰C - TE: %.2f⁰C\n", TI, TR, TE);
    print_display(TI, TR, TE);

    write_uart_send(uart_filesystem, TR);
  } while (key_gpio == key);
}

void execute_keyboard(int gpio_key)
{
  int sig, value;
  switch (gpio_key)
  {
  case 0x01: // Turn on oven
    turn_resistance_on(sig);
    fanOn(value);
    break;

  case 0x02: // Turn off oven
    turn_resistance_off(sig);
    fanOff();
    break;

  case 0x03: // Increase temperature
    ref_temp += 5;
    break;

  case 0x04: // Decrease temperature
    ref_temp -= 5;
    break;

  case 0x05: // Timer increase
    timer_trigger += 1;
    break;

  case 0x06: // Timer decrease
    timer_trigger -= 1;
    break;

    // case 0x07: // Pre-defined cooking

    // break;
  }
}

void init()
{
  wiringPiSetup();
  turn_resistance_off();
  fanOff();
  connect_display();
  signal(SIGALRM, timer_interrupt);

  bme_connection = connect_bme();
  uart_filesystem = connect_uart();
  system("clear");
}

void menu()
{
  int option;
  float tr;
  printf("Deseja controlar o programa por:\n\t1) Terminal\n\t2) Teclado\n");
  scanf("%d", &option);
  switch (option)
  {
  case 1:
    system("clear");
    printf("Insira o valor da Temperatura Referêncial(TR): \n");
    scanf("%f", &tr);
    printf("\n================== Iniciada rotina Terminal ==================\n");
    terminal_routine(tr, 0);
    break;
  case 2:
    printf("\n================== Iniciada rotina PID ==================\n");
    pid_routine(0);
    break;
  case 3:
    break;
  default:
    system("clear");
    printf("Opção invalida\n");
    menu();
    break;
  }
}

void timer_interrupt(int sig_num)
{
  if (sig_num == SIGALRM)
  {
    turn_resistance_off();
    fanOff();
  }
}

int main()
{
  init();
  signal(SIGINT, shutdown_program);
  menu();
  return 0;
}