#ifndef _DMA_USART_H
#define _DMA_USART_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "packet.h"

/* -------- Configuration -------- */
// UART and DMA selection for STM32F103C8T6:
// USART1 TX uses DMA1_Channel4
#define USARTx               USART1
#define DMAx                 DMA1
#define DMA_CHANNEL_TX       LL_DMA_CHANNEL_4

// Queue for TX BUFFER DMA
#ifndef DMA_USART_TX_QUEUE_DEPTH
#define DMA_USART_TX_QUEUE_DEPTH 8
#endif

#ifndef DMA_USART_PACKET_MAX_LEN
#define DMA_USART_PACKET_MAX_LEN 256
#endif
/* -------------------------------- */

/* Add #include "packet.h" above to use Packet_t definition 
#define PREAMBLE_SIZE   3	 	    // Start byte + Packet ID + payload length byte
#define POSTAMBLE_SIZE  2	      // 8-bit CRC + Stop byte
#define MAX_BUFFER_SIZE 0xFE    // Maximum allowed payload bytes per packet

typedef struct {
	 uint8_t preamble[PREAMBLE_SIZE];
   uint8_t buffer[MAX_BUFFER_SIZE]; 
   uint8_t postamble[POSTAMBLE_SIZE]; 
} Packet_t;
*/

/* Internal queue structures */
typedef struct {
					 Packet_t q_tx_buffer[DMA_USART_TX_QUEUE_DEPTH];
	volatile uint8_t  q_head; // index of next to pop
	volatile uint8_t  q_tail; // index of next to push
	volatile uint8_t  q_count; 
} Queue_Packet_t;


/**
  * @brief Peripheral initialization
	* @Note
  *  - Initialize USART1
  *  - Initialize DMA1
  * @param None
  * @retval None
  */
void dma_usart_init(void);


/**
 * @brief Send packet to Queue
 * @note
 * - Non-blocking api.
 * - If DMA idle, start transfer immediately
 * @param packToSend packet to be sent
 * @retval True if successful.
 */
bool dma_usart_send(Packet_t packetToSend);

#endif // _DMA_USART_H