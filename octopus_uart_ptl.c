
/**
 * ****************************************************************************
 * @copyright Copyright (c) XXX
 * All rights reserved.
 *
 * C file for the Octopus Task Manager module.
 * This file defines macros, includes necessary libraries, and declares functions
 * for UART communication, task management, and frame processing for the Octopus protocol.
 * Function definitions for UART communication, frame analysis, error handling,
 * and message processing within the Octopus Task Manager module.
 */

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_platform.h"
#include "octopus_log.h" 
#include "octopus_uart_ptl.h"      // Include UART protocol header
#include "octopus_uart_hal.h"      // Include UART hardware abstraction layer header
#include "octopus_timer.h"         // Include timer utility functions
#include "octopus_msgqueue.h"      // Include message queue header for task communication
#include "octopus_task_manager.h"  // Include task manager header for task state management

/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
#define TEST_LOG_DEBUG_PTL_RX_FRAME  // Enable debugging for receiving frames
//#define TEST_LOG_DEBUG_PTL_TX_FRAME  // Enable debugging for transmitting frames

/*******************************************************************************
 * MACROS
 */
#define PTL_RUNNING_NONE            0   // Define no task running state
#define PTL_TX_BUF_NUM              4   // Define number of transmission buffers
#define PTL_RX_BUF_NUM              16  // Define number of reception buffers

#define PTL_TX_TIMEOUT              50  // Define timeout for TX operations (in ms)

#define PTL_FRAME_TYPE_REQ          1   // Define request frame type
#define PTL_FRAME_TYPE_ACK          0   // Define acknowledgment frame type

#define PTL_MODULE_MIN              1   // Define minimum module number
#define PTL_MODULE_MAX              126 // Define maximum module number
#define PTL_DATA_MAX_LEN            248 // Define maximum data length
#define PTL_RETRY_CNT_MAX           2   // Define maximum retry count for transmission

#define PTL_MODULE_SUPPORT_CNT      16  // Define number of supported modules

#define PTL_DEM_EID                 DEM_EID_INT_COM_ERROR  // Define error event ID for communication errors

/*******************************************************************************
 * TYPEDEFS
 */

// Define a structure for module information, including frame type and send/receive handlers
typedef struct
{
    ptl_frame_type_t           frame_type;      // Frame type (e.g., request or acknowledgment)
    module_send_handler_t      send_handler;    // Send handler function for the module
    module_receive_handler_t   receive_handler; // Receive handler function for the module
} module_info_t;

/*******************************************************************************
 * LOCAL FUNCTIONS DECLARATION
 */

// Declare function prototypes for various tasks and processing functions
void ptl_init(void);                             // Initialize the Octopus protocol
void ptl_hal_tx(uint8_t *data, uint16_t len);    // Hardware abstraction layer for transmitting data
///void ptl_hal_rx(ptl_proc_buff_t *buf, uint8_t data);  // Hardware abstraction layer for receiving data
void ptl_remove_none_header_data(ptl_proc_buff_t *buf); // Remove data that is not part of the header
void ptl_find_valid_frame(ptl_proc_buff_t *buf); // Find a valid frame from the received data
void ptl_proc_valid_frame(uint8_t *data, uint16_t len); // Process the valid frame
void ptl_error_detect(void);                   // Detect communication errors

void ptl_tx_event_message_handler(void);      // Handle the transmission event message
void ptl_rx_event_message_handler(uint8_t data);

bool ptl_is_sleep_enable(void);               // Check if sleep mode is enabled
module_info_t* ptl_get_module(ptl_frame_type_t frame_type); // Get module information by frame type

/*******************************************************************************
 * STATIC VARIABLES
 */

// Define static variables for internal management
static uint32_t l_u32_running_req_mask = 0; // Mask indicating running tasks (initialized to none)
static ptl_proc_buff_t l_t_tx_proc_buf;     // Transmission buffer
static ptl_proc_buff_t l_t_rx_proc_buf;     // Reception buffer
static uint32_t l_t_ptl_rx_main_timer;      // Timer for RX main task
static uint32_t l_t_ptl_tx_main_timer;      // Timer for TX main task
static uint32_t l_t_ptl_error_detect_timer; // Timer for error detection
static bool lb_com_error = false;           // Flag indicating communication error
static bool lb_opposite_running = false;    // Flag indicating opposite task running state
static module_info_t l_t_module_info[PTL_MODULE_SUPPORT_CNT]; // Array holding module information
static uint8_t l_u8_next_empty_module = 0;  // Index for the next empty module slot

/*******************************************************************************
 * GLOBAL FUNCTIONS IMPLEMENTATION
 */

// Initialize UART communication for the task
void com_uart_init_running(void)
{
    LOG_LEVEL("app_comuart_init\r\n");
    OTMS(TASK_ID_PTL, OTMS_S_INVALID);
}

// Start the UART communication for the task
void com_uart_start_running(void)
{
    LOG_LEVEL("app_comuart_start\r\n");
    OTMS(TASK_ID_PTL, OTMS_S_ASSERT_RUN);
}

// Assert that UART communication is running
void com_uart_assert_running(void)
{
    if (PTL_RUNNING_NONE != l_u32_running_req_mask)
    {
        OTMS(TASK_ID_PTL, OTMS_S_RUNNING);
        StartTimer(&l_t_ptl_rx_main_timer);
        StartTimer(&l_t_ptl_tx_main_timer);
        StartTimer(&l_t_ptl_error_detect_timer);
        lb_com_error = false;
        lb_opposite_running = false;

        ptl_init();
    }
}

// Main running function for UART communication
void com_uart_running(void)
{
    if (true == ptl_is_sleep_enable())
    {
        OTMS(TASK_ID_PTL, OTMS_S_POST_RUN);
    }
    else
    {
        ptl_rx_event_message_handler(0);
        ptl_frame_analysis_handler();
        
        ptl_tx_event_message_handler();
        ptl_error_detect();
    }
}

// Post-running function for UART communication
void com_uart_post_running(void)
{
    if (true == ptl_is_sleep_enable())
    {
    }
    else
    {
        OTMS(TASK_ID_PTL, OTMS_S_RUNNING);
    }
}

// Stop the UART communication task
void com_uart_stop_running(void)
{
    OTMS(TASK_ID_PTL, OTMS_S_INVALID);
}

// Request the UART task to start running (source indicates who requested)
bool com_uart_reqest_running(uint8_t source)
{
    if (source >= 32)
    {
        return false;
    }
    else
    {
        l_u32_running_req_mask |= (1 << source);
        return true;
    }
}

// Release the UART task from running (source indicates who is releasing)
bool com_uart_release_running(uint8_t source)
{
    if (source >= 32)
    {
        return false;
    }
    else
    {
        l_u32_running_req_mask &= ~(1 << source);
        return true;
    }
}

// Set the opposite task's running state (for mutual task coordination)
void com_uart_set_opposite_running(bool running)
{
    lb_opposite_running = running;
    if (true == lb_opposite_running)
    {
        StartTimer(&l_t_ptl_error_detect_timer);
    }
}

// Check if there is a communication error
bool com_uart_is_com_error(void)
{
    return lb_com_error;
}

// Register a new module for communication (with its send and receive handlers)
void ptl_com_uart_register_module(ptl_frame_type_t frame_type, module_send_handler_t send_handler, module_receive_handler_t receive_handler)
{
    if (l_u8_next_empty_module < PTL_MODULE_SUPPORT_CNT)
    {
        l_t_module_info[l_u8_next_empty_module].frame_type = frame_type;
        l_t_module_info[l_u8_next_empty_module].send_handler = send_handler;
        l_t_module_info[l_u8_next_empty_module].receive_handler = receive_handler;
        l_u8_next_empty_module++;
    }
}

// Build a communication frame with the given data and command
void ptl_com_uart_build_frame(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint8_t *data, uint8_t datelen, ptl_proc_buff_t *framebuff)
{
    assert(data);
    assert(framebuff);
    assert((datelen + PTL_FRAME_HEADER_SIZE + 1) < PTL_FRAME_MAX_SIZE);
    ptl_com_uart_build_frame_header(frame_type, cmd, datelen, framebuff);
    for (int i = 0; i < datelen; i++)
    {
        framebuff->buff[PTL_FRAME_HEADER_SIZE + i] = data[i];
    }
    framebuff->buff[PTL_FRAME_HEADER_SIZE + datelen] = ptl_com_uart_get_checksum(&(framebuff->buff[PTL_FRAME_HEADER_SIZE - 1]), (datelen + 1));

    framebuff->size = PTL_FRAME_HEADER_SIZE + datelen + 1;
}

// Build the header of the communication frame
void ptl_com_uart_build_frame_header(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint8_t datalen, ptl_proc_buff_t *proc_buff)
{
    assert(proc_buff);
    if (frame_type >= A2M_MOD_START)
    {
        proc_buff->buff[0] = A2M_PTL_HEADER;
    }
    else
    {
        proc_buff->buff[0] = M2A_PTL_HEADER;
    }

    proc_buff->buff[1] = frame_type;
    proc_buff->buff[2] = cmd;
    proc_buff->buff[3] = datalen;
    proc_buff->buff[4] = ptl_com_uart_get_checksum(proc_buff->buff, PTL_FRAME_HEADER_SIZE - 1);
}

// Calculate the checksum for the data
uint8_t ptl_com_uart_get_checksum(uint8_t *data, uint8_t length)
{
    assert(data);
    uint8_t sum = 0;
    for (int i = 0; i < length; i++)
    {
        sum += data[i];
    }
    return (uint8_t)(~sum + 1);
}

/**
 * Check if sleep mode is enabled based on UART pending state.
 * Returns true if no task is currently running.
 */
bool ptl_is_sleep_enable(void)
{
    // Check if there are no tasks running
    if (l_u32_running_req_mask == PTL_RUNNING_NONE)
    {
        return true;  // Sleep mode is enabled
    }
    else
    {
        return false; // Sleep mode is not enabled, tasks are running
    }
}

void ptl_com_uart_receive_handler(uint8_t data)
{
    // Check if the platform is CST OSAL RTOS.
    #ifdef PLATFORM_CST_OSAL_RTOS
    // Ensure that the received data does not exceed the maximum buffer size.
    if (l_t_rx_proc_buf.size < PTL_FRAME_MAX_SIZE)
    {
        // Store the received byte of data in the buffer at the current position.
        l_t_rx_proc_buf.buff[l_t_rx_proc_buf.size] = data;

        // Increment the size of the buffer to account for the new byte.
        l_t_rx_proc_buf.size++;
    }
    #endif
}


/**
 * Handles the event message for transmitting data via UART.
 * Checks if enough time has passed since the last transmission and sends the data if necessary.
 */
void ptl_tx_event_message_handler(void)
{
    module_info_t *p_module = NULL;
    
    // If the time since the last TX event is less than 10 ms, do nothing
    if (GetTimer(&l_t_ptl_tx_main_timer) < 10)
    {
        return;
    }
    
    // Restart the timer for the next transmission
    StartTimer(&l_t_ptl_tx_main_timer);

    // Retrieve the message from the UART task message queue
    Msg_t* msg = get_message(TASK_ID_PTL);
    if(msg->id != NO_MSG)
    {        
        ptl_frame_type_t frame_type = (ptl_frame_type_t)msg->id;
        uint16_t param1 = msg->param1;
        uint16_t param2 = msg->param2;
        
        // Get the module corresponding to the frame type
        p_module = ptl_get_module(frame_type);
        
        // If module and send handler are available, call the send handler
        if((NULL != p_module) && (NULL != p_module->send_handler))
        {
            bool res = p_module->send_handler(frame_type, (ptl_frame_cmd_t)param1, param2, &l_t_tx_proc_buf);
            if(res)
            {
            // Send the processed data over UART
            #ifdef TEST_LOG_DEBUG_PTL_TX_FRAME
            LOG_LEVEL("msg_id=%02x cmd=%02x data[]=", (uint8_t)msg->id, (uint8_t)msg->param1);
            //DBG("[ ] msg_id:%02x cmd:%02x\r\n", (uint8_t)msg->id, (uint8_t)msg->param1);
            LOG_BUFF(l_t_tx_proc_buf.buff,l_t_tx_proc_buf.size);
            //LOG_("\r\n");
            #endif
            ptl_hal_tx(l_t_tx_proc_buf.buff, l_t_tx_proc_buf.size);
            }
        }
    }
    else
    {
        // If no message, do nothing (idle state)
        ;
    }
}

/**
 * Handler for receiving UART data byte-by-byte.
 * Calls the UART reception handler to store the received data.
 */
void ptl_rx_event_message_handler(uint8_t data)
{
    // Call the HAL function to process received byte
    #ifdef PLATFORM_ITE_OPEN_RTOS
    //uint8_t data = 0;
    while(1)
    {
        if (l_t_rx_proc_buf.size < PTL_FRAME_MAX_SIZE)
        {
            if (0 != hal_com_uart_get_fifo_data(&data, 1))
            {
                l_t_rx_proc_buf.buff[l_t_rx_proc_buf.size] = data;
                l_t_rx_proc_buf.size ++;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
   #endif
}

/**
 * Analyzes the received frame by first removing any non-header data, 
 * and then processing any valid frames.
 */
void ptl_frame_analysis_handler(void)
{
    // Remove data that is not part of the header
    ptl_remove_none_header_data(&l_t_rx_proc_buf);
    
    // Process the valid frame found in the buffer
    ptl_find_valid_frame(&l_t_rx_proc_buf);
}

/**
 * Removes any data from the buffer that is not part of the header.
 * Only keeps the data after the A2M_PTL_HEADER byte.
 */
void ptl_remove_none_header_data(ptl_proc_buff_t *proc_buff)
{
    // If the first byte is A2M_PTL_HEADER, no action needed
    #ifdef TASK_MANAGER_STATE_MACHINE_MCU
    if (proc_buff->buff[0] == A2M_PTL_HEADER)
        return;   
    // Search for A2M_PTL_HEADER and remove data before it
    for (uint16_t i = 0; i < proc_buff->size; i++)
    {
        if (proc_buff->buff[i] == A2M_PTL_HEADER)
        {
            // Shift data to remove the position before the header
            for (uint16_t j = i; j < proc_buff->size; j++)
            {
                proc_buff->buff[j - i] = proc_buff->buff[j];
            }
            proc_buff->size -= i;
            break;
        }
    }
    #endif
    #ifdef TASK_MANAGER_STATE_MACHINE_SOC
    if (proc_buff->buff[0] == M2A_PTL_HEADER)
           return;
    for (uint16_t i = 0; i < proc_buff->size; i ++)
    {
           if (proc_buff->buff[i] == M2A_PTL_HEADER)
           {
               // remove data before header
               for (uint16_t j = i; j < proc_buff->size; j++)
               {
                   proc_buff->buff[j-i] = proc_buff->buff[j];
               }
               proc_buff->size -= i;
               break;
           }
     }

    #endif
    // If _PTL_HEADER is not found, clear the buffer
    proc_buff->size = 0;
}

/**
 * Finds a valid frame in the received buffer.
 * Validates the header and frame checksums.
 */
void ptl_find_valid_frame(ptl_proc_buff_t *proc_buff)
{
    uint16_t offset = 0;            // Header offset
    uint8_t datalen = 0;            // Data length
    uint8_t framelen = 0;           // Frame length
    uint8_t crc;                    // Header checksum
    uint8_t crc_read = 0;           // Read checksum value
    bool head_crc_ok = false;       // Flag for header checksum validity
    bool frame_crc_ok = false;      // Flag for frame checksum validity
    bool find = false;              // Flag to indicate if a valid frame is found
    uint16_t next_valid_offset = 0; // Next valid frame offset
    bool header_invalid = false;    // Flag for invalid header detection

    // Iterate through the received buffer to find a valid frame
    for (uint16_t i = 0; i < proc_buff->size; i++)
    {
        #ifdef TASK_MANAGER_STATE_MACHINE_MCU
        if (proc_buff->buff[i] == A2M_PTL_HEADER)
        {
        #else
        if (proc_buff->buff[i] == M2A_PTL_HEADER)
        {
        #endif
            offset = i;
            datalen = proc_buff->buff[i + 3];
            framelen = datalen + PTL_FRAME_HEADER_SIZE + 1;
            
            // Ensure minimum frame size is met
            if ((framelen < PTL_FRAME_MIN_SIZE) && (proc_buff->size >= PTL_FRAME_MIN_SIZE) && (offset == 0))
            {
                next_valid_offset = PTL_FRAME_MIN_SIZE;
            }
            else if ((framelen >= PTL_FRAME_MIN_SIZE) && (framelen <= (proc_buff->size - offset)))
            {
                // Validate header checksum
                crc = ptl_com_uart_get_checksum(&proc_buff->buff[offset], PTL_FRAME_HEADER_SIZE - 1);
                crc_read = proc_buff->buff[offset + PTL_FRAME_HEADER_SIZE - 1];
                head_crc_ok = (crc == crc_read);

                if (head_crc_ok)
                {
                    // Validate frame checksum
                    crc = ptl_com_uart_get_checksum(&(proc_buff->buff[offset + PTL_FRAME_HEADER_SIZE - 1]), (datalen + 1));
                    crc_read = proc_buff->buff[offset + PTL_FRAME_HEADER_SIZE + datalen];
                    frame_crc_ok = (crc == crc_read);
                }

                // If both header and frame checksums are valid, process the frame
                if (head_crc_ok && frame_crc_ok)
                {
                    next_valid_offset = offset + framelen;
                    find = true;
                    break;
                }
                else
                {
                    // If the first header is invalid, mark it and skip to next frame
                    if ((offset == 0) || (header_invalid == true))
                    {
                        header_invalid = true;
                        next_valid_offset = framelen + offset;
                    }
                }
            }
            else
            {
                // No valid frame found yet, continue searching
            }
        }
    }

    // If a valid frame is found, process it
    if (find == true)
    {
        ptl_proc_valid_frame(proc_buff->buff + offset, framelen);
    }

    // Shift buffer data if a valid frame is found
    if (next_valid_offset != 0)
    {
        for (uint16_t i = next_valid_offset; i < proc_buff->size; i++)
        {
            proc_buff->buff[i - next_valid_offset] = proc_buff->buff[i];
        }
        proc_buff->size = proc_buff->size - next_valid_offset;
    }
}

/**
 * Processes the valid frame after it has been extracted from the buffer.
 * The payload is passed to the appropriate module handler.
 */
void ptl_proc_valid_frame(uint8_t *data, uint16_t length)
{
    ptl_frame_payload_t payload;
    module_info_t *module_ = NULL;
    ptl_proc_buff_t *tx_frame = &l_t_tx_proc_buf;

    // Set up the frame payload from the received data
    payload.frame_type = (ptl_frame_type_t)data[1];
    payload.cmd = (ptl_frame_cmd_t)data[2];
    payload.data_len = data[3];
    payload.data = &data[PTL_FRAME_DATA_START];

    // Get the module associated with the frame type
    module_ = ptl_get_module(payload.frame_type);

    #ifdef TEST_LOG_DEBUG_PTL_RX_FRAME
    LOG_LEVEL("frame_type=%02x cmd=%02x,length=%d data[]=", payload.frame_type, payload.cmd, payload.data_len);
    LOG_BUFF(data,length);
    #endif
    
    // If module handler exists, call the receive handler
    if (NULL != module_)
    {
        bool res = module_->receive_handler(&payload, tx_frame);
        if (res)
        {
            // If the handler succeeds, clear error flag and restart timer
            lb_com_error = false;
            StartTimer(&l_t_ptl_error_detect_timer);
            ptl_hal_tx(tx_frame->buff, tx_frame->size);  // Send response via UART
        }
    }
}

/**
 * Detects communication errors based on the error detection timer.
 * If the opposite side is running and the timeout expires, mark an error.
 */
void ptl_error_detect(void)
{
    if (GetTimer(&l_t_ptl_error_detect_timer) > 5000)
    {
        StartTimer(&l_t_ptl_error_detect_timer);
        if (lb_opposite_running)
        {
            lb_com_error = true;
        }
    }
}

/**
 * Retrieves the module corresponding to the specified frame type.
 */
module_info_t * ptl_get_module(ptl_frame_type_t frame_type)
{
    module_info_t *module_info = NULL;

    for (uint8_t i = 0; i < l_u8_next_empty_module; i++)
    {
        if (l_t_module_info[i].frame_type == (frame_type & 0x7F))
        {
            module_info = &l_t_module_info[i];
            break;
        }
    }

    return module_info;
}

/**
 * Initializes the Octopus protocol (no specific actions for now).
 */
void ptl_init(void)
{
    // No initialization needed
}

/**
 * Sends data over UART using the hardware abstraction layer.
 */
void ptl_hal_tx(uint8_t *data, uint16_t length)
{
    hal_com_uart_send_buffer(data, length);
}



