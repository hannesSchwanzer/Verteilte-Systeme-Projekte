/**
 * @file	server_util.c
 * @brief	Usefull functions for reading and handling requests
 *
 */


#include "server_util.h"
#include <stdio.h>
#include <arpa/inet.h>

/**
 * @brief read 8 bit integer from the buffer, decodes and writes it into value
 * 
 * @param buf buffer to read from
 * @param value 8 bit integer pointer to write into
 * @return int how many bytes were read
 */
int decode_octet(char* buf, uint8_t* value)
{
	*value = *buf;
	return sizeof(uint8_t);
}

/**
 * @brief read 16 bit integer from the buffer, decodes and writes it into value
 * 
 * @param buf buffer to read from
 * @param value 16 bit integer pointer to write into
 * @return int how many bytes were read
 */
int decode_uint16(char* buf, uint16_t* value)
{
	*value = ntohs(*((uint16_t*) buf));
	return sizeof(uint16_t);
}

/**
 * @brief read 32 bit integer from the buffer, decodes and writes it into value
 * 
 * @param buf buffer to read from
 * @param value 32 bit integer pointer to write into
 * @return int how many bytes were read
 */
int decode_uint32(char* buf, uint32_t* value)
{
	*value = ntohl(*((uint32_t*) buf));
	return sizeof(uint32_t);
}

/**
 * @brief writes 32 bit from buf in str_buf
 * 
 * @param buf buffer to read from
 * @param str_buf buffer to write into
 * @return int how many bytes were read
 */
int decode_string(char* buf, char* str_buf)
{
    strncpy(str_buf , buf, STRING_LENGTH);
    return STRING_LENGTH;
}




/**
 * @brief Inserts an 8 bit Integer in a char buffer
 * 
 * @param buf char buffer
 * @param n integer
 * @return int how many bytes were written
 */
int insert_int8(char* buf, int8_t n)
{
    *((uint8_t *) buf) = n;
    return sizeof(uint8_t);
}

/**
 * @brief Inserts an 16 bit Integer in a char buffer
 * 
 * @param buf char buffer
 * @param n integer
 * @return int how many bytes were written
 */
int insert_int16(char* buf, int16_t n)
{
    *((uint16_t *) buf) = htons(n);
    return sizeof(uint16_t);
}

/**
 * @brief Inserts an 32 bit Integer in a char buffer
 * 
 * @param buf char buffer
 * @param n integer
 * @return int how many bytes were written
 */
int insert_int32(char* buf, int32_t n)
{
    *((uint32_t *) buf) = htonl(n);
    return sizeof(uint32_t);
}

/**
 * @brief Writes 32 bits from str_buf to buf
 * 
 * @param buf buffer to write into
 * @param str_buf buffer to write from
 * @return int how many bytes were written
 */
int insert_string(char* buf, char* str_buf)
{
    strncpy(buf, str_buf, STRING_LENGTH);
    return STRING_LENGTH;
}


/**
 * @brief reads from socket and writes into buffer
 * 
 * @param socket socket file descriptor
 * @param buf buffer to write into
 * @param N Max Elements to read
 * @return int Returns the amount of bytes who were read
 */
int readFromSocket(int socket, char* buf, ssize_t N)
{
	unsigned int bytesread = 0;
	unsigned int remaining;
	int n;
	for(remaining = N; remaining > 0; remaining -= n) {
		n = read(socket, buf, remaining);
		if (n <= 0)
			return 0;
        
		buf += n;  //Pointer um n verschieben (damit die zuvorgelesenen Sachen nicht ueberschrieben werden)
        bytesread += n;
	}

    return bytesread;
}


/**
 * @brief reads the information from an active connection, then decodes and writes it in the header
 * 
 * @param fd file descriptor for an active connection
 * @param header empty header struct
 * @return int 
 */
int read_header(int fd, struct header* header)
{
    char buf[HAMSTER_RPC_PROT_HEADER_SIZE];
    if (readFromSocket(fd, buf, 8) != 8) return -1;


    char* bufPointer = buf;

    bufPointer += decode_octet(bufPointer, (uint8_t *) &(header->version));
    bufPointer += decode_octet(bufPointer, (uint8_t* ) &(header->flags));
    bufPointer += decode_uint16(bufPointer, &(header->msg_id));
    bufPointer += decode_uint16(bufPointer, &(header->payload_length));
    bufPointer += decode_uint16(bufPointer, &(header->rpc_call_id));


    return 0;
}

/**
 * @brief writes the information from the header struct in a char buffer
 * 
 * @param buf char buffer
 * @param header header
 */
void write_header(char* buf, struct header* header)
{
    insert_int8(buf + HAMSTER_RPC_PROT_HEADER_VERSION_OFFSET, header->version);
    insert_int8(buf + HAMSTER_RPC_PROT_HEADER_FLAGS_OFFSET, header->flags);
    insert_int16(buf + HAMSTER_RPC_PROT_HEADER_MSGID_OFFSET, header->msg_id);
    insert_int16(buf + HAMSTER_RPC_PROT_HEADER_CALLID_OFFSET, header->rpc_call_id);
    insert_int16(buf + HAMSTER_RPC_PROT_HEADER_PAYLOADLENGTH_OFFSET, header->payload_length);
}

/**
 * @brief prints the information from the header struct to the console
 * 
 * @param header header
 */
void print_header(struct header* header)
{
    printf("Version: %x\t\t\n", header->version & 0xff);
    printf("Flags: %x\t\t\n", header->flags);
    printf("Message ID: %04x\t\n", header->msg_id);
    printf("Payload Length: %d\n", header->payload_length);
    printf("RPC call ID: %d\t\n", header->rpc_call_id);
}


/**
 * @brief Hexdumps n elements from the char array p
 * 
 * @param p char array
 * @param n elements to dump
 */
void dump(unsigned char *p, unsigned int n)
{
    int j, k;
    int i = 0;

    while (i < n)
    {
        printf("%08x:   ", i);                          //start of the line
        j = (i+16 > n) ? n-i : 16;                      //length

        for (k = 0; k < j; k++)                         //hex dump
            printf("%02x ", *(p-k));
        for (k = k; k < 16; k++)                        //fill remainder if line was < 16
            printf("    ");

        for (k = 0; k < j; k++)                         //ascii dump
            if (*(p + k) < ' ' || *(p + k) > '~')
                putchar('.');                           //print controls etc. as dot!
            else
                printf("%c", *(p + k) );
        
        p += j;                                         //prepare for the next line
        i += j;
        putchar('\n');
    }
}