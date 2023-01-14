#ifndef _REQUEST_HANDLER_
#define _REQUEST_HANDLER_

#include <stdio.h>

#include "server_util.h"


void handle_request(int fd, struct header* header, char* buf);


void rpc_new(char* buf, char* response_body, struct header* rsp_header);
void rpc_lookup(char* buf, char* response_body, struct header* rsp_header);
void rpc_directory(char* buf, char* response_body, struct header* rsp_header);
void rpc_howsDoing(char* buf, char* response_body, struct header* rsp_header);
void rpc_readEntry(char* buf, char* response_body, struct header* rsp_header);
void rpc_giveTreats(char* buf, char* response_body, struct header* rsp_header);
void rpc_collect(char* buf, char* response_body, struct header* rsp_header);




#endif