#pragma once

#include <netinet/in.h>

#define DIE(assertion, call_description)  \
    do {                                  \
        if (assertion) {                  \
            fprintf(stderr, "(%s, %d): ", \
                    __FILE__, __LINE__);  \
            perror(call_description);     \
            exit(EXIT_FAILURE);           \
        }                                 \
    } while (0)

typedef struct Integer
{
    uint32_t val;
    char sign;
} Integer;

typedef struct ShortReal
{
    uint16_t val;
} ShortReal;

typedef struct Float
{
    uint32_t val;
    char sign;
    uint8_t pos;
} Float;

typedef struct String
{
    char txt[1501];
} String;

typedef struct TCPMessage {
    char msg_type;
    char client_id[11];
    in_addr_t src_addr;
    in_port_t port;
    char topic[51];
    bool sf;
    char data_type;
    union { Integer i; ShortReal sr; Float f; String str;} content;
} TCPMessage;