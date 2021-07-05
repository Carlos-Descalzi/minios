#ifndef _MSG_H_
#define _MSG_H_

#include "stdint.h"

/**
 * Message passing
 **/

typedef struct {
    uint32_t source;
    uint32_t target;
    uint32_t number:31,
             has_more:1;
    char body[1024];
} Message;

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
