#include "board.h"
#include <avr/interrupt.h>
#include <mcu/twi.h>
#include <mcu/util.h>
#include <os/os.h>
#include <stdint.h>

#include "sensirion_uart.h"
#include "sps30.h"

#define TWI_CMD_PERFORM 0x10
#define TWI_CMD_READ 0x11

#define MEDIAN_COUNT 29

// The last median of X measured PM2.5 ug/cm3 values
volatile uint16_t last_pm25 = 0x00;

uint16_t median_buf[MEDIAN_COUNT] = {0};
uint8_t median_buf_ix = 0;
void insert_sort(uint8_t value) {
  if (median_buf_ix == MEDIAN_COUNT)
    return;

  for (int i = 0; i < median_buf_ix; i++) {
    if (median_buf[i] > value) {
      // Shift array
      for (int j = median_buf_ix; j > i; j--) {
        median_buf[j] = median_buf[j - 1];
      }
      // Insert value
      median_buf[i] = value;
      median_buf_ix++;
      return;
    }
  }

  median_buf[median_buf_ix] = value;
  median_buf_ix++;
}

int main(void) {
  sei(); // Enable interrupts
  delay_init();

  // Enable power to SPS30
  SENSOR_PWR_PORT.DIRSET = 1 << SENSOR_PWR_PIN;
  SENSOR_PWR_PORT.OUTSET = 1 << SENSOR_PWR_PIN;
  delay_ms(50);
  sensirion_uart_open();
  delay_ms(50);
  sps30_start_manual_fan_cleaning();
  delay_ms(10000);
  sps30_start_measurement();
  delay_ms(15000);

  // Make ourself available to MFM Core on I2C bus
  twi_init(0x36, 1);

  struct sps30_measurement mbuf;
  int16_t error;
  for (;;) {
    // Fetch PM2.5 ug/cm3 value
    error = sps30_read_measurement(&mbuf);
    if (error < 0) {
      delay_ms(1000);
      continue;
    }

    insert_sort(mbuf.mc_2p5);

    // If the median buffer is full, store the median value and reset the buffer
    if (median_buf_ix == MEDIAN_COUNT) {
      last_pm25 = median_buf[MEDIAN_COUNT / 2];
      median_buf_ix = 0;
    }

    // Give SPS30 time to gather a new measurement
    delay_ms(1000);
  }
}

void twi_perform(uint8_t *buf, uint8_t length) {}

void twi_read(uint8_t *buf, uint8_t length) {
  buf[0] = 0x02;
  buf[1] = last_pm25 >> 8;
  buf[2] = last_pm25;
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
