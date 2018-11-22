// api.c
// ESP8266 Micro Web API
// By Abdullah Daud, chelahmy@gmail.com
// 13 November 2018

#include <mem.h>
#include "session.h"
#include "api.h"

extern uint32 app_sec;
static char *app_key = "micro-api"; // Our API key

// Get a cookie from the header
int ICACHE_FLASH_ATTR getCookie(HttpdConnData *conn, char *name, char *val, int vlen) {
	char *ptr, *cookies;
	int len = strlen(name);
	os_memset(val, 0, vlen);
	cookies = (char *)os_malloc(128);
	if (cookies == NULL)
		return 0;
	os_memset(cookies, 0, 128);
	if (!httpdGetHeader(conn, "Cookie", cookies, 127)) {
		os_free(cookies);
		return 0;
	}
	ptr = cookies;
	while (*ptr != '\0') { // find 'name'
		if (strncmp(ptr, name, len) == 0) {
			ptr += len;
			break;
		}
		++ptr;
	}
	if (*ptr == '\0') {
		os_free(cookies);
		return 0;
	}
	while (*ptr != '\0') { // find '='
		if (*ptr == '=') {
			++ptr;
			break;
		}
		++ptr;
	}	
	while (*ptr != '\0') { // skip whitespaces
		if (*ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r')
			break;
		++ptr;
	}	
	while (*ptr != '\0') { // copy value
		if (*ptr == ';' || *ptr == '\n' || *ptr == '\r')
			break;
		*val = *ptr;
		++val;
		++ptr;
		if (--vlen <= 0)
			break;
	}
	os_free(cookies);
	return 1;
}

// Get the session id from the cookie
uint32 ICACHE_FLASH_ATTR getSessionID(HttpdConnData *conn) {
	char val[32];
	if (!getCookie(conn, "sessid", val, sizeof(val) - 1))
		return 0;
	return (uint32)atoi(val);
}

// A simplified version of a key validation 
int ICACHE_FLASH_ATTR isValidKey(char *key) {
	return strcmp(app_key, key) == 0 ? 1 : 0;
}

// The API main function
int ICACHE_FLASH_ATTR cgiApi(HttpdConnData *connData) {
	int len, stt = APISTT_INVALID; 
	char q[16], p0[32];
	char buff[128], data[128];
	uint32 sessid = 0;
	
	if (connData->conn == NULL) {
		return HTTPD_CGI_DONE;
	}

	// We only accept the GET method
	if (connData->requestType != HTTPD_METHOD_GET) {
		httpdStartResponse(connData, 406);  // http error code 'unacceptable'
		httpdEndHeaders(connData);
		return HTTPD_CGI_DONE;
	}

	data[0] = '\0';
	len = httpdFindArg(connData->getArgs, "q", q, sizeof(q)); // get query

	if (len != -1) {
		if (strcmp(q, "login") == 0) { // login
			len = httpdFindArg(connData->getArgs, "key", p0, sizeof(p0));
			if (len != -1) {
				if (isValidKey(p0)) {
					sessid = getSessionID(connData);
					if (sessid > 0)									// on last session:
						ses_renew(sessid, app_sec + SES_EXPIRY);	// renew the session
					else
						sessid = ses_reg(app_sec + SES_EXPIRY);		// register a new session
					if (sessid > 0)
						stt = APISTT_OK;
				}
			}
		}
		else if (strcmp(q, "logout") == 0) { // logout
			sessid = getSessionID(connData);
			if (sessid > 0) {
				ses_delete(sessid);
				sessid = 0;
				stt = APISTT_OK;
			}
		}
		else if (strcmp(q, "getsec") == 0) { // get application seconds
			sessid = getSessionID(connData);
			if (sessid > 0 && ses_has(sessid)) {
				ses_renew(sessid, app_sec + SES_EXPIRY); // renew the session to delay the expiry time
				sprintf(data, "\"sec\":%d", app_sec);
				stt = APISTT_OK;
			}
			else
				stt = APISTT_ACCESS_DENIED;
		}
		else
			stt = APISTT_UNKNOWN_QUERY;
	}
	else
		stt = APISTT_NO_QUERY;

	httpdStartResponse(connData, 200); 
	httpdHeader(connData, "Content-Type", "application/json");

	if (sessid > 0) {
		sprintf(buff, "sessid=%u", sessid);
		httpdHeader(connData, "Set-Cookie", buff);
	}

	httpdEndHeaders(connData);
	
	if (strlen(data) > 0)
		len = sprintf(buff, "{\"status\":%d,\"data\":{%s}}", stt, data);
	else
		len = sprintf(buff, "{\"status\":%d}", stt);

	httpdSend(connData, buff, len);

	return HTTPD_CGI_DONE;	
}





