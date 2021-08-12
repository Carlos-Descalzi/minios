#ifndef _TCPIPD_H_
#define _TCPIPD_H_

#include "msg.h"

#define MSG_OPEN                1
#define MSG_CLOSE               2
#define MSG_ACCEPT              3
#define MSG_RECV                4
#define MSG_SEND                5
#define MSG_RESPONSE            6

#define SOCKET_TYPE_UDP     0x01
#define SOCKET_TYPE_TCP     0x02
#define SOCKET_TYPE_SERVER  0x10

typedef struct {
    uint8_t socket_type;
    uint16_t port;
} OpenRequest;

typedef struct {
    uint8_t socket_type;
    uint16_t port;
} CloseRequest;

typedef struct {
} ListenRequest;

typedef struct {
    uint8_t socket_type;
    uint16_t port;
} AcceptRequest;

typedef struct {
    uint32_t result;
    uint8_t socket_type;
    uint16_t port;
    uint32_t remote_address;
    uint16_t remote_port;
} AcceptResponse;

typedef struct {
    uint8_t socket_type;
    uint16_t port;
} RecvRequest;

typedef struct {
    uint16_t size;
    uint8_t payload[];
} RecvResponse;

typedef struct {
    uint32_t remote_address;
    uint16_t remote_port;
    uint16_t size;
    uint8_t payload[];
} RecvHeaderResponse;

#define RECV_HEADER_RESPONSE_SIZE   (\
        MSG_MESSAGE_SIZE - sizeof(RecvHeaderResponse) - 1 - sizeof(MessageHeader))

#define RECV_RESPONSE_SIZE  (\
        MSG_MESSAGE_SIZE - sizeof(RecvResponse) - 1 - sizeof(MessageHeader))

typedef struct {
    uint8_t socket_type;
    uint16_t port;
    uint16_t size;
    uint8_t payload[];
} SendRequest;

typedef struct {
    uint8_t socket_type;
    uint16_t port;
    uint32_t remote_address;
    uint16_t remote_port;
    uint16_t size;
    uint8_t payload[];
} SendHeaderRequest;

#define SEND_HEADER_REQUEST_SIZE   (\
        MSG_MESSAGE_SIZE - sizeof(SendHeaderRequest) - 1 - sizeof(MessageHeader))

#define SEND_REQUEST_SIZE   (\
        MSG_MESSAGE_SIZE - sizeof(SendRequest) - 1 - sizeof(MessageHeader))

typedef struct {
    MessageHeader header;
    uint8_t type;
    union {
        OpenRequest open_request;
        CloseRequest close_request;
        ListenRequest listen_request;
        AcceptRequest accept_request;
        AcceptResponse accept_response;
        RecvRequest recv_request;
        RecvHeaderResponse recv_header_response;
        RecvResponse recv_response;
        SendHeaderRequest send_header_request;
        SendRequest send_request;
        int32_t int_response;
    };
} UserMessage;

#endif
