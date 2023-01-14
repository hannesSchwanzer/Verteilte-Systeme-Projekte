#ifndef __SERVER_UTIL__
#define __SERVER_UTIL__

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "hamsterprotocol.h"
#include "hamsterlib.h"

#define MAX_BODY_SIZE 65536
#define STRING_LENGTH 32


struct header {
    char version;
    char flags;
    uint16_t msg_id;
    uint16_t payload_length;
    uint16_t rpc_call_id;
};



void dump(unsigned char *p, unsigned int n);

int decode_octet(char* buf, uint8_t* value);
int decode_uint16(char* buf, uint16_t* value);
int decode_uint32(char* buf, uint32_t* value);
int decode_string(char* buf, char* str_buf);

int insert_int8(char* buf, int8_t n);
int insert_int16(char* buf, int16_t n);
int insert_int32(char* buf, int32_t n);
int insert_string(char* buf, char* str_buf);

int readFromSocket(int socket, char* buf, ssize_t N);

int read_header(int fd, struct header* header);
void print_header(struct header* header);

void write_header(char* buf, struct header* header);

void request_error(int fd, struct header* header);

void handle_request(int fd, struct header* header, char* buf);

void read_string(char* buf, char* str);


#endif