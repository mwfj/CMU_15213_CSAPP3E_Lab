#pragma once
#ifndef __PROXY_H__
#define __PROXY_H__

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAXLINE 8192

#define ERROR_CODE_BAD_REQUEST          "400"
#define ERROR_CODE_NOT_IMPLEMENT        "501"

#define NOT_IMPLEMENT_SHORT_MSG         "Not Implemented"
#define NOT_IMPLEMENT_METHOD_LONG_MSG   "Oops, looks like proxy does not support this method"
#define WRONG_VERSION_FORMAT            "Wrong Version Format"
#define BAD_REQUEST                     "Bad Request"

#define HTTP_PREFIX                     "http://"
#define SLASH		                    "/"
#define COLON		                    ":"

void process_request(int);
void reply_client_error(int , char *, char *, char *, char *);
void modify_http_version(int , char *, char *);
void parse_uri(char* , char *, char *, char *);
int  change_request(rio_t *, char *, char *, char *, char *);

#endif /* __PROXY_H__ */