/**
 * @file    octopus_uart_hal.c
 * @brief   UART HAL (Hardware Abstraction Layer) implementation for Octopus platform.
 *          This file provides UART initialization, data transmission, and event handling
 *          for both CST OSAL RTOS and ITE OPEN RTOS platforms.
 * @date    2020
 * @copyright Copyright (c) 2020, CHIPSEA Co., Ltd.
 */

/*********************************************************************
 * INCLUDES
 * Includes necessary headers for UART functionality and platform-specific configurations.
 */
#include "octopus_platform.h"
#include "octopus_cfifo.h"

#include "octopus_log.h"
#include "octopus_uart_hal.h"
#include "octopus_uart_ptl.h"

/*******************************************************************************
 * DEBUG SWITCH MACROS
 * Enable debug logging for specific events during UART operations.
 */
//#define TEST_LOG_DEBUG_UART_RX_DATA 

/*******************************************************************************
 * MACROS
 * Define commonly used constants and values for UART operations.
 */
#define PI_FLOAT (3.14159f)  // Example constant for mathematical computations.

/*******************************************************************************
 * GLOBAL VARIABLES
 * Variables that are accessible throughout the file or program.
 */
uint8_t Hal_TaskID;              // Task ID for UART event handling.
//#ifdef PLATFORM_CST_OSAL_RTOS
com_uart_data_buff_t com_uart_data_buff;           // Buffer structure for received UART data.
//#endif

#ifdef PLATFORM_ITE_OPEN_RTOS
static bool UartIsInit = false;  // Tracks whether UART is initialized.
static sem_t UartSemIntr;        // Semaphore for UART interrupt handling.
uint32_t com_uart_parity_error = 0;         // Placeholder for parity error tracking.
#endif

static cFifo_t* usart_rx_fifo = NULL;
static uint8_t uart_rx_fifo_buff[cFifo_ObjSize(256 + 64)];

/*******************************************************************************
 * LOCAL FUNCTIONS DECLARATION
 * Declare static functions used only within this file.
 */
#ifdef PLATFORM_CST_OSAL_RTOS
static void hal_com_uart_receive_callback(uart_Evt_t* pev);
#elif defined(PLATFORM_ITE_OPEN_RTOS)
static void hal_com_uart_receive_callback(void* arg1, uint32_t arg2);
#endif

/*******************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */

#ifdef PLATFORM_CST_OSAL_RTOS

/**
 * @brief   Initializes the UART hardware for CST OSAL RTOS platform.
 * @param   task_id  Task identifier for UART event handling.
 * @return  None.
 */
static void hal_uart_init(void) {
    // Configure UART settings.
    uart_Cfg_t cfg = {
        .tx_pin = P26,
        .rx_pin = P27,
        .baudrate = 115200,
        .use_fifo = TRUE,
        .hw_fwctrl = FALSE,
        .use_tx_buf = FALSE,
        .parity = FALSE,
        .evt_handler = hal_com_uart_receive_callback,
    };
    // Initialize UART1 with the specified configuration.
    HalUartInit(UART1, cfg);
    cFifo_Init(&usart_rx_fifo, uart_rx_fifo_buff, sizeof(uart_rx_fifo_buff));
    //LOG_("\r\n"); 
}

void hal_com_uart_init(uint8 task_id)
{
	Hal_TaskID = task_id;
	if(task_id == 0)
	{
    hal_uart_init();
	LOG_LEVEL("hal uart1 init for protocol\r\n");
	}
}
/**
 * @brief   Callback function triggered when UART events occur.
 * @param   pev  Pointer to the UART event structure.
 * @return  None.
 */
static void hal_com_uart_receive_callback(uart_Evt_t* pev) {
    switch (pev->type) {
        case UART_EVT_TYPE_RX_DATA:       // Data received event.
        case UART_EVT_TYPE_RX_DATA_TO:   // Timeout on data reception.
        {
            uint16_t i;
            for (i = 0; i < pev->len; i++) {
                com_uart_data_buff.data[(com_uart_data_buff.wr + i) & (UART_BUFF_MAX_SIZE - 1)] = pev->data[i];
            }
            com_uart_data_buff.wr += pev->len;

            // Start a timer to handle received data.
            osal_start_timerEx(Hal_TaskID, UART_RECEIVE_DATA_EVENT, 5);
        }
        break;

        case UART_EVT_TYPE_TX_COMPLETED:  // Transmission complete event.
            LOG_LEVEL("tx completed\r\n");
        break;
    }
}

/**
 * @brief   Handles UART events for CST OSAL RTOS platform.
 * @param   task_id  Task identifier for UART event handling.
 * @param   events   Event flags indicating the specific events to handle.
 * @return  Remaining unhandled events.
 */
uint16_t hal_com_uart_event_handler(uint8_t task_id, uint16 events) {
    if (task_id != Hal_TaskID) return 0;

    if (events & UART_RECEIVE_DATA_EVENT) {
        #ifdef TEST_LOG_DEBUG_UART_RX_DATA
        LOG_("Hal receive data:\r\n");
        #endif

        // Process received data from UART buffer.
        while (com_uart_data_buff.rd != com_uart_data_buff.wr) {
            #ifdef TEST_LOG_DEBUG_UART_RX_DATA
            LOG_("%02x ", com_uart_data_buff.data[com_uart_data_buff.rd & (UART_BUFF_MAX_SIZE - 1)]);
            #endif
            ///ptl_com_uart_receive_handler(com_uart_data_buff.data[com_uart_data_buff.rd & (UART_BUFF_MAX_SIZE - 1)]);
            cFifo_Push(usartRxFifo,         com_uart_data_buff.data[com_uart_data_buff.rd & (UART_BUFF_MAX_SIZE - 1)]);
            com_uart_data_buff.rd++;
        }

        #ifdef TEST_LOG_DEBUG_UART_RX_DATA
        LOG_("\r\n");
        #endif

        return (events ^ UART_RECEIVE_DATA_EVENT);
    }

    return 0;
}

#elif defined(PLATFORM_ITE_OPEN_RTOS)
/**
 * @brief   Configures the USART peripheral for ITE OPEN RTOS platform.
 * @return  None.
 */
void hal_uart_init(void) {
    UART_OBJ* pUartInfo = (UART_OBJ*)malloc(sizeof(UART_OBJ));
    pUartInfo->port = PROTOCOL_UART_ITH_PORT;
    pUartInfo->parity = PROTOCOL_UART_PARITY;
    pUartInfo->txPin = PROTOCOL_UART_GPIO_TX;
    pUartInfo->rxPin = PROTOCOL_UART_GPIO_RX;
    pUartInfo->baud = PROTOCOL_UART_BAUDRATE;
    pUartInfo->timeout = 0;
    pUartInfo->mode = UART_INTR_MODE;
    pUartInfo->forDbg = false;

    // Register and initialize the UART device.
    itpRegisterDevice(PROTOCOL_UART_PORT, &PROTOCOL_UART_DEVICE);
    ioctl(PROTOCOL_UART_PORT, ITP_IOCTL_INIT, (void*)pUartInfo);
    ioctl(PROTOCOL_UART_PORT, ITP_IOCTL_REG_UART_CB, (void*)hal_com_uart_receive_callback);
    sem_init(&UartSemIntr, 0, 0);
    cFifo_Init(&usart_rx_fifo, uart_rx_fifo_buff, sizeof(uart_rx_fifo_buff));
    UartIsInit = true;
}

void hal_com_uart_init(uint8_t task_id)
{
    pthread_t task_receive;
    pthread_attr_t attr_receive;
    pthread_attr_init(&attr_receive);
    hal_uart_init();
    LOG_LEVEL("hal uart2 init for protocol\r\n");
    pthread_create(&task_receive, &attr_receive, hal_com_uart_event_handler, NULL);
}

/**
 * @brief   Callback function for UART interrupt handling on ITE platform.
 * @param   arg1  Argument 1 (unused).
 * @param   arg2  Event-specific data.
 * @return  None.
 */
static void hal_com_uart_receive_callback(void* arg1, uint32_t arg2) {
    sem_post(&UartSemIntr);
    if (arg2 == 1) {
        LOG_LEVEL("arg2=1 error\n");
        com_uart_parity_error = arg2;
    }
}

/**
 * @brief   UART event handler for ITE OPEN RTOS platform.
 * @param   arg  Pointer to handler argument (unused).
 * @return  None.
 */
void* hal_com_uart_event_handler(void* arg) {
    uint8_t length = 0;
    while (1) {
        sem_wait(&UartSemIntr);
        length = 0;

        if (UartIsInit)
                length = read(PROTOCOL_UART_PORT, com_uart_data_buff.data, UART_BUFF_MAX_SIZE);
        if(length > 0)
        {
              for (int i = 0; i < length; i++) {
                #ifdef TEST_LOG_DEBUG_UART_RX_DATA
                LOG_("%02x ",com_uart_data_buff.data[i]);
                #endif
                cFifo_Push(usart_rx_fifo, com_uart_data_buff.data[i]);
              }
        }
    }
}

#else
void hal_com_com_uart_init(uint8_t task_id)
{
	
}

#endif // PLATFORM_ITE_OPEN_RTOS
/**
 * Reads data from the UART RX FIFO buffer into the provided buffer.
 * It will attempt to read up to `length` bytes from the FIFO.
 * 
 * @param buffer Pointer to the buffer where the received data will be stored.
 * @param length Maximum number of bytes to read from the FIFO.
 * 
 * @return The number of bytes actually received and copied into the buffer.
 */
uint8_t hal_com_uart_get_fifo_data(uint8_t* buffer, uint16_t length)   
{
    uint8_t data = 0;   // Variable to hold each byte read from the FIFO
    uint8_t index = 0;  // Index to track how many bytes have been stored in the buffer

    // Get the current size of the FIFO (number of available bytes to read)
    uint8_t datasize = cFifo_DataSize(usart_rx_fifo);

    // Loop to read data from FIFO until we either fill the buffer or run out of data
    while (1)
    {
        // If we haven't reached the desired length and there's still data in the FIFO
        if (index < length)
        {
            // Try to pop a byte from the FIFO
            if (true == cFifo_Pop(usart_rx_fifo, &data))
            {
                // Store the byte in the provided buffer
                buffer[index] = data;
                index++;  // Increment the index to store the next byte
            }
            else
            {
                // If no more data is available in the FIFO, exit the loop
                break;
            }
        }
        else
        {
            // If we've already read the desired number of bytes, exit the loop
            break;
        }
    }

    // Return the number of bytes actually read from the FIFO
    return index;
}

/**
 * @brief   Sends a string over UART.
 * @param   str     Pointer to the string.
 * @param   length  Length of the string.
 * @return  Number of bytes sent.
 */
uint8_t hal_com_uart_send_string(const char* str, uint8_t length) {
	  uint8_t ret_code = length;
    #ifdef PLATFORM_CST_OSAL_RTOS
    osal_memcpy(com_uart_data_buff.data, str, strlen(str));
    ret_code = HalUartSendBuf(UART1, com_uart_data_buff.data, strlen(str));
    #endif

    #ifdef PLATFORM_ITE_OPEN_RTOS
    ret_code = write(PROTOCOL_UART_PORT, str, length);
    #endif
	  return ret_code;
}

/**
 * @brief   Sends a buffer of data over UART.
 * @param   buff    Pointer to the buffer.
 * @param   length  Length of the buffer.
 * @return  Number of bytes sent.
 */
uint8_t hal_com_uart_send_buffer(uint8_t* buff, uint16_t length) {
	  uint8_t ret_code = length;
    #ifdef PLATFORM_CST_OSAL_RTOS
    ret_code = HalUartSendBuf(UART1, buff, length);
    #endif

    #ifdef PLATFORM_ITE_OPEN_RTOS
    ret_code = write(PROTOCOL_UART_PORT, buff, length);
    #endif
	  return ret_code;
}



