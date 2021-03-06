#ifndef _MSG_H_
#define _MSG_H_

#include "stdint.h"

/**
 * Message passing
 **/

#define MSG_MESSAGE_SIZE    1024

typedef struct {
    uint32_t source;        // source tid, filled by api
    uint32_t target;        // target tid
    uint32_t number:31,     // message number, optional
             has_more:1;    // if more messages are coming, optional
} MessageHeader;


typedef struct {
    MessageHeader header;
    char body[MSG_MESSAGE_SIZE - sizeof(MessageHeader)];        // the message body, up to 1024 bytes
} Message;

#define MESSAGE(m)          ((Message*)m)

/**
 * Sends a message to a given process, waits for
 * answer, which is stored on the same message 
 * structure
 **/
int msg_send_sync(Message* message);

/**
 * Receives a message if any, returning 0,
 * otherwise exits returning 1
 **/
int msg_recv(Message* message);

/**
 * Sends a message with no wait for answer
 **/
int msg_send(Message* message);

/**
 * Waits for a message
 **/
int msg_recv_wait(Message* message);

#endif
