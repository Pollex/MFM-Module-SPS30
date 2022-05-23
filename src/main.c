#include <avr/interrupt.h>
#include <os/os.h>
#include <mcu/twi.h>
#include <mcu/util.h>
#include "board.h"

#include "sensirion_uart.h"
#include "sps30.h"

#define TWI_CMD_PERFORM 0x10
#define TWI_CMD_READ 0x11

void measurement(void);
os_task measurement_task = {
    .func = &measurement,
    .priority = 1,
};

int main(void)
{
  sei(); // Enable interrupts
  delay_init();
  SENSOR_PWR_PORT.DIRSET = 1 << SENSOR_PWR_PIN;

  // Perform manual clean
  SENSOR_PWR_PORT.OUTSET = 1 << SENSOR_PWR_PIN;
  delay_ms(10);
  sensirion_uart_open();
  sps30_start_measurement();
  delay_ms(10);
  sps30_start_manual_fan_cleaning();
  delay_ms(10000);
  sps30_stop_measurement();
  SENSOR_PWR_PORT.OUTCLR = 1 << SENSOR_PWR_PIN;

  os_init();
  twi_init(0x36, 1);

  for (;;)
  {
    measurement();
    delay_ms(3000);
  }

  for (;;)
  {
    os_processTasks();
    os_sleep();
  }
}

union
{
  float fval;
  uint8_t bval[4];
} float_m;

struct sps30_measurement m;
void measurement(void)
{
  SENSOR_PWR_PORT.OUTSET = 1 << SENSOR_PWR_PIN;
  sensirion_uart_open();
  delay_ms(100);

  // Perform measurement and store results
  sps30_start_measurement();
  delay_ms(5000);

  for (uint8_t i = 0; i < 3; i++)
  {
    if (sps30_read_measurement(&m) == 0)
    {
      break;
    }
    delay_ms(1000);
  }

  sps30_stop_measurement();
  delay_ms(100);
  SENSOR_PWR_PORT.OUTCLR = 1 << SENSOR_PWR_PIN;
}

void twi_perform(uint8_t *buf, uint8_t length)
{
  // os_pushTask(&measurement_task);
}

void twi_read(uint8_t *buf, uint8_t length)
{
  uint16_t p25 = (uint16_t)m.mc_2p5;
  buf[0] = 0x02;
  buf[1] = p25 >> 8;
  buf[2] = p25;
}

twi_cmd_t twi_cmds[] = {
    {
        .cmd = TWI_CMD_PERFORM,
        .handler = &twi_perform,
    },
    {
        .cmd = TWI_CMD_READ,
        .handler = &twi_read,
    },
};

void os_presleep()
{
}

void os_postsleep()
{
}