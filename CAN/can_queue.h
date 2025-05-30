/*
 * can_msg_queue.h
 *
 * Created on: July 14, 2020
 *     Author: xiaolb
 *
 * Description:
 * This header file defines the data structures and function interfaces for a circular
 * message queue used to temporarily store CAN (Controller Area Network) messages.
 * This queue is primarily used for decoupling CAN message reception (e.g., from interrupts)
 * from their processing in the main loop or another thread/task.
 */

#ifndef __CAN_CANQUEUE_H_
#define __CAN_CANQUEUE_H_

#include <stdint.h>

#define CAN_MSG_QUEUE_SIZE (128U)                    // Must be a power of 2
#define CAN_MSG_QUEUE_SIZE_MASK (CAN_MSG_QUEUE_SIZE - 1)  // Used for efficient modulo operation

/**
 * @brief Structure representing a single CAN message in the queue.
 */
typedef struct {
    uint8_t channel;       // CAN channel index (e.g., 0 = CAN1, 1 = CAN2)
    uint32_t id;           // CAN message identifier (standard or extended)
    uint8_t data_len;      // Length of CAN data (0 ~ 8)
    uint8_t data[8];       // CAN data payload
} CanQueueMsg_t;

/**
 * @brief Structure representing the circular CAN message queue.
 */
typedef struct {
    uint16_t head;                         // Index of the next message to be popped
    uint16_t tail;                         // Index of the next free slot for pushing
    CanQueueMsg_t msg[CAN_MSG_QUEUE_SIZE]; // Internal buffer to store CAN messages
} CanQueue_t;

/**
 * @brief Initializes the CAN message queue by resetting head and tail.
 * 
 * @param queue Pointer to the CAN queue instance to be initialized.
 */
void Can_Queue_Init(CanQueue_t *queue);

/**
 * @brief Returns the current number of messages stored in the queue.
 * 
 * @param queue Pointer to the CAN queue instance.
 * @return Number of messages currently in the queue.
 */
uint16_t CanQueue_Length(CanQueue_t *queue);

/**
 * @brief Attempts to push a CAN message into the queue.
 * 
 * @param queue Pointer to the CAN queue instance.
 * @param channel CAN channel index from which the message is received.
 * @param id CAN message ID.
 * @param data Pointer to the CAN data payload (up to 8 bytes).
 * @param data_len Length of the data payload.
 * @return 0 if push was successful, 1 if the queue is full.
 */
uint8_t CanQueue_Push(CanQueue_t *queue, uint8_t channel, uint32_t id, const uint8_t *data, uint8_t data_len);

/**
 * @brief Attempts to pop the oldest CAN message from the queue.
 * 
 * @param queue Pointer to the CAN queue instance.
 * @param msg Output pointer to receive the popped CAN message.
 * @return 1 if a message was successfully popped, 0 if the queue was empty.
 */
uint8_t CanQueue_Pop(CanQueue_t *queue, CanQueueMsg_t *msg);

#endif /* __CAN_CANQUEUE_H_ */
