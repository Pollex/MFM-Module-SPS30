#include "uart.h"
#include "mcu/util.h"

/**
 * sensirion_uart_select_port() - select the UART port index to use
 *                                THE IMPLEMENTATION IS OPTIONAL ON SINGLE-PORT
 *                                SETUPS (only one SPS30)
 *
 * Return:      0 on success, an error code otherwise
 */
int16_t sensirion_uart_select_port(uint8_t port)
{
  return 0;
}

/**
 * sensirion_uart_open() - initialize UART
 *
 * Return:      0 on success, an error code otherwise
 */
int16_t sensirion_uart_open()
{
  uart_init();
  return 0;
}

/**
 * sensirion_uart_close() - release UART resources
 *
 * Return:      0 on success, an error code otherwise
 */
int16_t sensirion_uart_close()
{
  return 0;
}

/**
 * sensirion_uart_tx() - transmit data over UART
 *
 * @data_len:   number of bytes to send
 * @data:       data to send
 * Return:      Number of bytes sent or a negative error code
 */
int16_t sensirion_uart_tx(uint16_t data_len, const uint8_t *data)
{
  uart_write(data, data_len);
  return data_len;
}

/**
 * sensirion_uart_rx() - receive data over UART
 *
 * @data_len:   max number of bytes to receive
 * @data:       Memory where received data is stored
 * Return:      Number of bytes received or a negative error code
 */
int16_t sensirion_uart_rx(uint16_t max_data_len, uint8_t *data)
{
  return uart_read(data, max_data_len);
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * Despite the unit, a <10 millisecond precision is sufficient.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_sleep_usec(uint32_t useconds)
{
  delay_us(useconds);
}