// api.h
// ESP8266 Web API
// By Abdullah Daud, chelahmy@gmail.com
// 13 November 2018

#ifndef __API_H__
#define __API_H__

// libesphttpd
// https://github.com/Spritetm/libesphttpd
#include "platform.h"
#include "httpd.h"

// API Status
#define APISTT_OK				0
#define APISTT_NO_QUERY			-1
#define APISTT_UNKNOWN_QUERY	-2
#define APISTT_INVALID			-3
#define APISTT_ACCESS_DENIED	-4
#define APISTT_SYSTEM_ERR		-5

int getCookie(HttpdConnData *conn, char *name, char *val, int vlen);
uint32 getSessionID(HttpdConnData *conn);
int isValidKey(char *key);
int cgiApi(HttpdConnData *connData);

#endif

