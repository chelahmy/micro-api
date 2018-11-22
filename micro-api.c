// micro-api.c
// ESP8266 Micro Web API Framework
// By Abdullah Daud, chelahmy@gmail.com
// 14 November 2018

/**
 * A framework to implement a micro web API in ESP8266.
 * - The tiny ESP8266 microcontroller is designed to host a micro
 *   web server which is served through the built-in WiFi.
 * - This framework is based on esp-open-sdk and libesphttpd.
 *   It makes use of cookies to maintain the sessions.
 * - The sample APIs are implemented in the cgiApi() function.
 * - And the sample APIs respond with JSON.
 *
 * esp-open-sdk - https://github.com/pfalcon/esp-open-sdk
 * libesphttpd  - https://github.com/Spritetm/libesphttpd
 *
 */
 
/**
 * Theory of Operations
 * - A client makes a GET request.
 * - The server responds with a status code and the
 *   requested data in JSON format.
 * - The server may want to authenticate the client
 *   before serving it with any data.
 * - Once authenticated the client will be assigned
 *   with a session ID which will be passed to the client
 *   as a cookie. The client must enable cookies.
 * - The server controls the session expiry.
 * - How the server authenticate the client is not
 *   part of this framework since there are many ways
 *   to do it.
 * - The server may assign any anonymous client with a
 *   session id to identify the client in a multi-step
 *   oriented process. The states of the process can be
 *   be kept in the session payload.
 * - A session can be extended with a payload which
 *   can be any form of data. I.e. authentication data
 *   can be kept in a session payload.
 * - The server may need to define its own session payload.
 *
 */
 
// esp-open-sdk
#include "user_interface.h"
#include "espconn.h"

// libesphttpd
#include "platform.h"
#include "httpd.h"
#include "captdns.h"

// our implementations
#include "session.h"
#include "api.h"


// ESP-12 LED pin
static const int LED = 2;

static os_timer_t ms50_timer;
uint32 app_sub_sec = 0, app_sec = 0; // application second counter

// URL-to-handler assignments
static HttpdBuiltInUrl builtInUrls[] = {
	{"/api.cgi", cgiApi, NULL},
	{NULL, NULL, NULL}
};

// The httpdConnSendFinish() function in libesphttpd calls the strcasecmp()
// function. We don't want to include a bigger library so we implement and
// wrap it here.
// Makefile LDLIBS += -Wl,--wrap,strcasecmp
int ICACHE_FLASH_ATTR __wrap_strcasecmp(const char *f1, const char *f2) {
	char c1, c2;
	while (*f1 != '\0' && *f2 != '\0') {
		c1 = *f1;
		if (c1 >= 'A' && c1 <= 'Z') // if uppercase then covert to lowercase
			c1 = c1 - 'A' + 'a';
		c2 = *f2;
		if (c2 >= 'A' && c2 <= 'Z') // if uppercase then covert to lowercase
			c2 = c2 - 'A' + 'a';
		if (c1 != c2)
			return c1 > c2 ? 1 : -1;
		++f1;
		++f2;
	}
	return 0;
}

// Blink the LED so that we know the module is working
void ICACHE_FLASH_ATTR blink(void) {
	if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << LED))
		gpio_output_set(0, (1 << LED), 0, 0);
	else
		gpio_output_set((1 << LED), 0, 0, 0);
}

// The 50ms timer interrupt routine.
// We use the timer to blink the LED, to implement the second counter,
// and to update the session expiry.
void ms50_timerfunc(void *arg) {

	blink();

	if (++app_sub_sec >= 20) { // 1 sec interval on 50ms timer 
		app_sub_sec = 0;
		++app_sec;

		ses_expire(app_sec);
	}
}

// MAIN
// In ESP8266 SDK, the user_init() function is equivalent to the main() in C.
void ICACHE_FLASH_ATTR user_init()
{
	struct softap_config sc;

	//http://kacangbawang.com/esp8266-sdk-os_printf-prints-garbage/
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	os_printf("\n\nSDK version:%s\n", system_get_sdk_version());

	// init gpio subsytem
	gpio_init();
	// gpio_output_set(set_mask, clear_mask, enable_mask, disable_mask)
	gpio_output_set(0, 0, (1 << LED), 0);
	
	// By default the WiFi is set to Soft-AP.
	wifi_softap_get_config_default(&sc);
	os_memcpy(sc.ssid, "Micro API", 9); // Set our SSID
	sc.ssid_len = 9;
	wifi_softap_set_config_current(&sc);
	
	// Initialize the http server
	captdnsInit(); // Enable the captive DNS server so that all domains will be redirected to this http server
	httpdInit(builtInUrls, 80);

	// setup timer (50ms, repeating)
	os_timer_setfn(&ms50_timer, (os_timer_func_t *)ms50_timerfunc, NULL);
	os_timer_arm(&ms50_timer, 50, 1);
}
