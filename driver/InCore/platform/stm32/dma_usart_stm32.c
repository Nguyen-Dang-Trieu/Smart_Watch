#include "dma_usart.h"
#include "stm32f1xx.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_rcc.h"
#include <string.h>

// Only a Queue Tx DMA in system
static Queue_Packet_t Singleton = {0};

/* Track DMA busy */
static volatile bool dma_busy = false;

/* Forward declarations */
static void start_DMA_transfer_from_queue(void);
static bool queue_push(Packet_t packet_in);
static bool queue_pop(Packet_t *packet_out);

/* -------- Initialization -------- */

void dma_usart_init(void)
{
    /* 1) Enable clocks */
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

    /* 2) Configure GPIO PA9 = TX (AF PP), PA10 = RX (floating input) */
    LL_GPIO_InitTypeDef gpio_init = {0};
		// GPIO PA9
    gpio_init.Pin 			 = LL_GPIO_PIN_9;
    gpio_init.Mode 			 = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed 		 = LL_GPIO_SPEED_FREQ_HIGH;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOA, &gpio_init);
		
		// ---- GPIO PA10 ----
    gpio_init.Pin 			 = LL_GPIO_PIN_10;
    gpio_init.Mode 			 = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(GPIOA, &gpio_init);

    /* 3) Configure USART1 (example 115200 8N1) */
    LL_USART_InitTypeDef usart_init = {0};
    usart_init.BaudRate 					 = 115200;
    usart_init.DataWidth 					 = LL_USART_DATAWIDTH_8B;
    usart_init.StopBits 					 = LL_USART_STOPBITS_1;
    usart_init.Parity 					   = LL_USART_PARITY_NONE;
    usart_init.TransferDirection   = LL_USART_DIRECTION_TX_RX;
    usart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    usart_init.OverSampling 			 = LL_USART_OVERSAMPLING_16;

    LL_USART_Init(USARTx, &usart_init);
    LL_USART_Enable(USARTx);

    /* 4) Configure DMA channel for USART1_TX */
    LL_DMA_DisableChannel(DMAx, DMA_CHANNEL_TX);
    while (LL_DMA_IsEnabledChannel(DMAx, DMA_CHANNEL_TX)) {} // wait off

    LL_DMA_ConfigTransfer(DMAx, DMA_CHANNEL_TX,
                          LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                          LL_DMA_PRIORITY_HIGH |
                          LL_DMA_MODE_NORMAL |
                          LL_DMA_PERIPH_NOINCREMENT |
                          LL_DMA_MEMORY_INCREMENT |
                          LL_DMA_PDATAALIGN_BYTE |
                          LL_DMA_MDATAALIGN_BYTE);

    /* Periph address = USART_DR */
    LL_DMA_SetPeriphAddress(DMAx, DMA_CHANNEL_TX, (uint32_t)&USARTx->DR);

    /* Enable USART DMA request for TX */
    LL_USART_EnableDMAReq_TX(USARTx);

    /* 5) Enable DMA IRQ (transfer complete) */
    NVIC_SetPriority(DMA1_Channel4_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);

    /* Clear queue state */
    Singleton.q_head = Singleton.q_tail = Singleton.q_count = 0;
    dma_busy = false;
}

/* -------- Queue API -------- */
static bool queue_push(Packet_t packet_in)
{
	  /* Critical section */
    __disable_irq();
    if (Singleton.q_count >= DMA_USART_TX_QUEUE_DEPTH) {
        __enable_irq();
        return false;
    }
		// Push packet into queue Tx
		Singleton.q_tx_buffer[Singleton.q_tail] = packet_in;
    Singleton.q_tail = (Singleton.q_tail + 1) & (DMA_USART_TX_QUEUE_DEPTH - 1);
    Singleton.q_count++;
		
    __enable_irq();
    return true;
}

static bool queue_pop(Packet_t *packet_out)
{
	  /* Critical section */
    __disable_irq();
    if (Singleton.q_count == 0) {
        __enable_irq();
        return false;
    }
		*packet_out = Singleton.q_tx_buffer[Singleton.q_head]; 
    Singleton.q_head = (Singleton.q_head + 1) & (DMA_USART_TX_QUEUE_DEPTH - 1);
    Singleton.q_count--;
    __enable_irq();
		
    return true;
}

/* -------- Start DMA from queue (internal) -------- */
static void start_dma_transfer_from_queue(void)
{
	  // DMA dang trong qua trinh truyen data
    if (dma_busy) return; // already running

	  /* Get packet in queue */
		Packet_t TxBuffer;
    if (!queue_pop(&TxBuffer)) {
        return; // nothing to send
    }

		/* Xu li packet truoc khi qua trinh truyen DMA*/
		uint8_t BUFFER_SIZE = TxBuffer.preamble[2];
		if(BUFFER_SIZE > MAX_BUFFER_SIZE) return;
		
		static uint8_t temp[PREAMBLE_SIZE + MAX_BUFFER_SIZE + POSTAMBLE_SIZE] = {0};
		
		memcpy(temp, TxBuffer.preamble, PREAMBLE_SIZE);
		memcpy(temp + PREAMBLE_SIZE, TxBuffer.buffer, BUFFER_SIZE);
		memcpy(temp + PREAMBLE_SIZE + BUFFER_SIZE, TxBuffer.postamble, POSTAMBLE_SIZE);
	
    /* Ensure channel disabled before config */
    LL_DMA_DisableChannel(DMAx, DMA_CHANNEL_TX);
    while (LL_DMA_IsEnabledChannel(DMAx, DMA_CHANNEL_TX)) {}

    /* Set memory address and length */
    LL_DMA_SetMemoryAddress(DMAx, DMA_CHANNEL_TX, (uint32_t)temp);
    LL_DMA_SetDataLength(DMAx, DMA_CHANNEL_TX, PREAMBLE_SIZE + BUFFER_SIZE + POSTAMBLE_SIZE);

    /* Clear flags for the channel */
    LL_DMA_ClearFlag_TC4(DMAx); // channel 4 TC
    LL_DMA_ClearFlag_GI4(DMAx);

    /* Mark busy then enable channel */
    dma_busy = true;
    LL_DMA_EnableChannel(DMAx, DMA_CHANNEL_TX); // Start DMA tranmission
}

/* -------- Public send API -------- */
bool dma_usart_send(Packet_t packetToSend)
{
    bool ok = queue_push(packetToSend);
    if (!ok) return false;

    /* If DMA idle, start transfer immediately */
    if (!dma_busy) {
        start_dma_transfer_from_queue();
    }
    return true;
}

/* -------- DMA IRQ Handler (TX complete) -------- */
void DMA1_Channel4_IRQHandler(void)
{
    /* Transfer complete for channel 4 */
    if (LL_DMA_IsActiveFlag_TC4(DMAx)) {
        LL_DMA_ClearFlag_TC4(DMAx);
        /* disable channel (already complete) */
        LL_DMA_DisableChannel(DMAx, DMA_CHANNEL_TX);
        dma_busy = false;

        /* Start next transfer if exists */
        start_dma_transfer_from_queue();
    }

    /* (Optional) handle transfer error */
    if (LL_DMA_IsActiveFlag_TE4(DMAx)) {
        LL_DMA_ClearFlag_TE4(DMAx);
        LL_DMA_DisableChannel(DMAx, DMA_CHANNEL_TX);
        dma_busy = false;
        // handle error: you may log or attempt restart
    }
}
