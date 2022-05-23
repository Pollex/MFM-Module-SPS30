#if !defined(_UART_H_)
#define _UART_H_

#include <stdint.h>

#define UART_BUFFER_SIZE 1024
#define UART_BAUD_RATE 115200

#ifdef __cplusplus
extern "C"
{
#endif

  void uart_init(void);
  uint16_t uart_available(void);
  uint16_t uart_read(uint8_t *data, uint16_t len);
  void uart_write(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // _UART_H_