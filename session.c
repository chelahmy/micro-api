// session.c
// ESP8266 Session Management
// By Abdullah Daud, chelahmy@gmail.com
// 3 November 2018

#include <mem.h>
#include "session.h"

struct _session *ses_all = NULL;
uint32 ses_xnow = 0;

struct _session ICACHE_FLASH_ATTR *ses_new(uint32 id, uint32 expiry) {
	struct _session *ses;
	ses = (struct _session *)os_malloc(sizeof(struct _session));
	if (ses == NULL)
		return NULL;
	ses->id = id;
	ses->expiry = expiry;
	ses->payload = NULL;
	ses->next = ses_all;
	ses_all = ses;
	return ses;
}

void ICACHE_FLASH_ATTR ses_delete(uint32 id) {
	struct _session *xptr = NULL, *ptr = ses_all;
	while (ptr) {
		if (ptr->id == id) {
			if (xptr != NULL)
				xptr->next = ptr->next;
			else
				ses_all = ptr->next;
			if (ptr->payload)
				os_free(ptr->payload);
			os_free(ptr);	
			return;
		}
		xptr = ptr;
		ptr = ptr->next;
	}
}

void ICACHE_FLASH_ATTR ses_delete_all() {
	struct _session *n, *ptr = ses_all;
	while (ptr) {
		n = ptr->next;
		if (ptr->payload)
			os_free(ptr->payload);
		os_free(ptr);	
		ptr = n;
	}
	ses_all = NULL;
}

void ICACHE_FLASH_ATTR ses_expire(uint32 now) {
	struct _session *xptr = NULL, *ptr = ses_all;
	if (now < ses_xnow) { // clock overflowed, restart all sessions
		ses_xnow = now;
		ses_delete_all();
		return;
	}
	ses_xnow = now;
	while (ptr) {
		if (ptr->expiry < now) {
			if (xptr != NULL)
				xptr->next = ptr->next;
			else
				ses_all = ptr->next;
			if (ptr->payload)
				os_free(ptr->payload);
			os_free(ptr);	
			return;
		}
		xptr = ptr;
		ptr = ptr->next;
	}
}

struct _session ICACHE_FLASH_ATTR *ses_find(long id) {
	struct _session *ptr = ses_all;
	while (ptr) {
		if (ptr->id == id)
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}

int ICACHE_FLASH_ATTR ses_count() {
	int cnt = 0;
	struct _session *ptr = ses_all;
	while (ptr) {
		++cnt;
		ptr = ptr->next;
	}
	return cnt;
}

uint32 ICACHE_FLASH_ATTR ses_reg(uint32 expiry) {
	uint8 i = 0;
	uint32 id;
	struct _session *ptr;
	id = (uint32)os_random();
	ptr = ses_find(id);
	while ((id == 0 || ptr != NULL) && i < 10) { // try to get a unique id
		id = (uint32)os_random();
		ptr = ses_find(id);
		++i;
	}
	if (id == 0 || ptr != NULL) // fails if the id is not unique
		return 0;
	ptr = ses_new(id, expiry);
	if (ptr == NULL)
		return 0;
	return id;
}

int ICACHE_FLASH_ATTR ses_has(uint32 id) {
	struct _session *ptr = ses_all;
	while (ptr) {
		if (ptr->id == id) {
			return 1;
		}
		ptr = ptr->next;
	}
	return 0;
}

int ICACHE_FLASH_ATTR ses_renew(uint32 id, uint32 expiry) {
	struct _session *ptr = ses_all;
	while (ptr) {
		if (ptr->id == id) {
			ptr->expiry = expiry;
			return 1;
		}
		ptr = ptr->next;
	}
	if (ses_new(id, expiry) != NULL)
		return 1;
	return 0;
}

int ICACHE_FLASH_ATTR ses_set_payload(uint32 id, void *payload) {
	struct _session *ptr = ses_all;
	while (ptr) {
		if (ptr->id == id) {
			if (ptr->payload)
				os_free(ptr->payload);
			ptr->payload = payload;
			return 1;
		}
		ptr = ptr->next;
	}
	return 0;
}

void ICACHE_FLASH_ATTR *ses_get_payload(uint32 id) {
	struct _session *ptr = ses_all;
	while (ptr) {
		if (ptr->id == id) {
			return ptr->payload;
		}
		ptr = ptr->next;
	}
	return NULL;
}

