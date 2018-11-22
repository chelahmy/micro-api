// session.h
// ESP8266 Session Management
// By Abdullah Daud, chelahmy@gmail.com
// 3 November 2018

#ifndef __SESSION_H__
#define __SESSION_H__

#include "osapi.h"

#define SES_EXPIRY 300

struct _session {
	uint32 id;
	uint32 expiry;
	void *payload;
	struct _session *next;
};

extern struct _session *ses_all;

struct _session *ses_new(uint32 id, uint32 expiry);
void ses_delete(uint32 id);
void ses_delete_all();
void ses_expire(uint32 now);
struct _session *ses_find(long id);

/**
 * Get the number of sessions. 
 */
int ses_count();

/**
 * Register a new session.
 * The expiry param is the validity duration of the
 * session from now. The unit for expiry is defined
 * by specific application. It could be in seconds or
 * any unit of time.
 * Return a non-zero session id.
 */
uint32 ses_reg(uint32 expiry);

/**
 * Check if the session id exists.
 */
int ses_has(uint32 id);

/**
 * Renew the session expiry.
 * See ses_reg() for details on expiry.
 */
int ses_renew(uint32 id, uint32 expiry);

/**
 * Set payload.
 * The payload must be allocated with os_malloc().
 * The session will free the payload. If there is 
 * already a payload then it will be freed before
 * the new payload is set. Set the payload to NULL
 * to free the existing payload.
 * Return:
 *  1: The payload is set.
 *  0: The payload is not set because the session id
 *     does not exist. The caller must free the payload. 
 */
int ses_set_payload(uint32 id, void *payload);

/**
 * Get payload.
 * Return the payload, or NULL if the session id does
 * not exist or the payload has not been set.
 * Do not free the payload.
 */
void *ses_get_payload(uint32 id);

#endif

