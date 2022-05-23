#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#define F_CPU 2666666L
#endif

#define BAUD(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

uint8_t buffer[UART_BUFFER_SIZE];
uint16_t read_ix = 0;
uint16_t write_ix = 0;

void uart_init(void)
{
  USART0.BAUD = (uint16_t)BAUD(UART_BAUD_RATE);
  USART0.CTRLA = USART_RXCIE_bm;
  USART0.CTRLB = 0;
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_SBMODE_1BIT_gc | USART_CHSIZE_8BIT_gc;

  // Enable Receiver
  PORTB.DIRCLR = PIN3_bm;
  USART0.CTRLB |= USART_RXEN_bm;

  // Enable Transmitter
  PORTB.DIRSET = PIN2_bm;
  PORTB.OUTSET = PIN2_bm;
  USART0.CTRLB |= USART_TXEN_bm;

  // Enable interrupts
  sei();
}

uint16_t uart_available(void)
{
  if (read_ix <= write_ix)
  {
    return write_ix - read_ix;
  }
  return UART_BUFFER_SIZE - read_ix + write_ix;
}

uint16_t uart_read(uint8_t *data, uint16_t len)
{
  uint16_t available = uart_available();
  if (len > available)
  {
    len = available;
  }

  for (uint8_t i = 0; i < len; i++)
  {
    data[i] = buffer[read_ix];
    read_ix = (read_ix + 1) % UART_BUFFER_SIZE;
  }

  return len;
}

void uart_write(uint8_t *data, uint16_t len)
{
  for (uint16_t i = 0; i < len; i++)
  {
    uart_putc(data[i]);
  }
}

void uart_putc(uint8_t data)
{
  // Wait for empty transmit buffer
  while (!(USART0.STATUS & USART_DREIF_bm))
    ;
  // Put data into buffer, sends the data
  USART0.TXDATAL = data;
  // Wait for data to be transmitted
  while (!(USART0.STATUS & USART_TXCIF_bm))
    ;
}

ISR(USART0_RXC_vect)
{
  uint8_t data = USART0.RXDATAL;
  buffer[write_ix] = data;
  write_ix = (write_ix + 1) % UART_BUFFER_SIZE;

  // If - after increment write_ix - it is equal to read_ix,
  // then the buffer is full and we will start dropping data.
  if (write_ix == read_ix)
  {
    read_ix = (read_ix + 1) % UART_BUFFER_SIZE;
  }
}