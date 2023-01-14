/**
 * @file	request_handler.c
 * @brief	Handles the requests for the hamster server
 *
 */


#include "request_handler.h"
#include "hamsterprotocol.h"


/**
 * @brief Reads the payload from a request and gives a corresponding answer
 * 
 * @param fd File descriptor for active connection
 * @param header Header 
 * @param buf char buffer with payload
 */
void handle_request(int fd, struct header* header, char* buf)
{
    char response[HAMSTER_RPC_PROT_HEADER_SIZE + MAX_BODY_SIZE];
    char* response_body = response + HAMSTER_RPC_PROT_HEADER_SIZE;  //Abstand fuer den Header lassen

    struct header rsp_header;

    //Header auffuellen
    rsp_header.version = HAMSTER_RPC_PROT_VERSION;
    rsp_header.flags = HAMSTER_RPC_MASK_FLAG_RESPONSE;
    rsp_header.rpc_call_id = header->rpc_call_id;
    rsp_header.msg_id = header->msg_id;
    rsp_header.payload_length = 0;


    switch (header->rpc_call_id)
    {
    case HAMSTER_RPC_FUNCID_NEW:
        rpc_new(buf, response_body, &rsp_header);
        break;
    
    case HAMSTER_RPC_FUNCID_LOOKUP:
        rpc_lookup(buf, response_body, &rsp_header);
        break;

    case HAMSTER_RPC_FUNCID_DIRECTORY:
        rpc_directory(buf, response_body, &rsp_header);
        break;
    
    case HAMSTER_RPC_FUNCID_HOWSDOING:
        rpc_howsDoing(buf, response_body, &rsp_header);
        break;

    case HAMSTER_RPC_FUNCID_READENTRY:
        rpc_readEntry(buf, response_body, &rsp_header);
        break;

    case HAMSTER_RPC_FUNCID_GIVETREATS:
        rpc_giveTreats(buf, response_body, &rsp_header);
        break;

    case HAMSTER_RPC_FUNCID_COLLECT:
        rpc_collect(buf, response_body, &rsp_header);
        break;
    default:
        rsp_header.flags |= HAMSTER_RPC_MASK_FLAG_ERROR;
        break;
    }

    write_header(response, &rsp_header);
    write(fd, response, HAMSTER_RPC_PROT_HEADER_SIZE + rsp_header.payload_length);

    printf("Sending. Payload:\n");
    dump((unsigned char*) response + HAMSTER_RPC_PROT_HEADER_SIZE, rsp_header.payload_length);
}

void rpc_new(char* buf, char* response_body, struct header* rsp_header)
{
    char name[STRING_LENGTH];
    char hamster_name[STRING_LENGTH];
    uint16_t treats;
    
    buf += decode_string(buf, name);
    buf += decode_string(buf, hamster_name);
    buf += decode_uint16(buf, &treats);
    
    int32_t id = hmstr_new(name, hamster_name, treats);

    response_body += insert_int32(response_body, id);
    rsp_header->payload_length += 4;

    if (id < 0) {
        rsp_header->flags |= HAMSTER_RPC_MASK_FLAG_ERROR;
    }
}

/**
 * @brief rpc for hmstr_lookup
 * 
 * @param buf request payload
 * @param response_body response payload
 * @param rsp_header response header
 */
void rpc_lookup(char* buf, char* response_body, struct header* rsp_header) 
{
    char name[STRING_LENGTH];
    char hamster_name[STRING_LENGTH];
    buf += decode_string(buf, name);
    buf += decode_string(buf, hamster_name);
    
    int32_t id = hmstr_lookup(name, hamster_name);

    insert_int32(response_body, id);
    response_body += 4;

    rsp_header->payload_length += 4;

    if (id < 0) {
        rsp_header->flags |= HAMSTER_RPC_MASK_FLAG_ERROR;
    }
}

/**
 * @brief rpc for hmstr_directory
 * 
 * @param buf request payload
 * @param response_body response payload
 * @param rsp_header response header
 */
void rpc_directory(char* buf, char* response_body, struct header* rsp_header)
{
    int32_t fdptr;
    char name[STRING_LENGTH];
    char hamster_name[STRING_LENGTH];

    buf += decode_uint32(buf, (uint32_t*) &fdptr);
    buf += decode_string(buf, name);
    buf += decode_string(buf, hamster_name);

    int32_t ret = hmstr_directory(&fdptr, strlen(name) ? name: NULL, strlen(hamster_name) ? hamster_name : NULL);
    
    response_body += insert_int32(response_body, ret);
    rsp_header->payload_length = 4;

    if (ret < 0) {
        rsp_header->flags |= HAMSTER_RPC_MASK_FLAG_ERROR;
    } else {
        response_body += insert_int32(response_body, fdptr);
        rsp_header->payload_length += 4;
    }
}

/**
 * @brief rpc for hmstr_howsdoing
 * 
 * @param buf request payload
 * @param response_body response payload
 * @param rsp_header response header
 */
void rpc_howsDoing(char* buf, char* response_body, struct header* rsp_header)
{
    uint32_t id;
    buf += decode_uint32(buf, &id);

    struct hmstr_state state;
    int32_t ret = hmstr_howsdoing(id, &state);

    response_body += insert_int32(response_body, ret);
    rsp_header->payload_length = 4;

    if (ret < 0) {
        rsp_header->flags |= HAMSTER_RPC_MASK_FLAG_ERROR;
    } else {
        response_body += insert_int16(response_body, state.treats_left);
        response_body += insert_int32(response_body, state.rounds);
        response_body += insert_int16(response_body, state.cost);

        rsp_header->payload_length += 8;
    }
}

/**
 * @brief rpc for hmstr_readentry
 * 
 * @param buf request payload
 * @param response_body response payload
 * @param rsp_header response header
 */
void rpc_readEntry(char* buf, char* response_body, struct header* rsp_header)
{
    uint32_t id;
    decode_uint32(buf, &id);

    char name[STRING_LENGTH];
    char hamster[STRING_LENGTH];
    int16_t price;

    int32_t ret = hmstr_readentry(id, name, hamster, &price);
    
    response_body += insert_int32(response_body, ret);
    rsp_header->payload_length = 4;

    if (ret < 0) {
        rsp_header->flags |= HAMSTER_RPC_MASK_FLAG_ERROR;
    } else {
        response_body += insert_string(response_body, name);
        response_body += insert_string(response_body, hamster);
        response_body += insert_int16(response_body, price);

        rsp_header->payload_length += 2*STRING_LENGTH + 2;
    }
}

/**
 * @brief rpc for hmstr_givetreats
 * 
 * @param buf request payload
 * @param response_body response payload
 * @param rsp_header response header
 */
void rpc_giveTreats(char* buf, char* response_body, struct header* rsp_header)
{
    uint32_t id;
    uint16_t treats;

    buf += decode_uint32(buf, &id);
    buf += decode_uint16(buf, &treats);

    int32_t trLeft = hmstr_givetreats(id, treats);
    
    response_body += insert_int32(response_body, trLeft);
    rsp_header->payload_length = 4;

    if (trLeft < 0) {
        rsp_header->flags |= HAMSTER_RPC_MASK_FLAG_ERROR;
    }
}

/**
 * @brief rpc for hmstr_collect
 * 
 * @param buf request payload
 * @param response_body response payload
 * @param rsp_header response header
 */
void rpc_collect(char* buf, char* response_body, struct header* rsp_header)
{
    char name[STRING_LENGTH];

    buf += decode_string(buf, name);

    int32_t sum = hmstr_collect(name);

    response_body += insert_int32(response_body, sum);
    rsp_header->payload_length = 4;

    if (sum < 0) {
        rsp_header->flags |= HAMSTER_RPC_MASK_FLAG_ERROR;
    }
}