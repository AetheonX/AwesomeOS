/**
 * @brief: message.h  message management header file
 * @author Valeriy Chibisov
 */

#ifndef _MESSAGE_H
#define _MESSAGE_H

typedef struct MessageEnvelope {
	uint32_t sender_id;
	uint32_t receiver_id;
	int message_type;
	void *message;
} MessageEnvelope;

int k_send_message (uint32_t pid, void *message);
void* k_receive_message (uint32_t* sender_ID);
    
#endif // ! _MESSAGE_H
