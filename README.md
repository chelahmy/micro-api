# micro-api
ESP8288 based micro web API framework

The tiny [ESP8266](https://en.wikipedia.org/wiki/ESP8266) is a microcontroller with the capability to host a micro web server which is serving through the built-in WiFi. However, the ESP8266 resources are very limited. It cannot host a full blown web server. Thus, it is best to off load the processes as much as possible to the client. And keep the web server to the minimal.

This is a framework to implement a micro web API in ESP8266 to serve external clients. This framework is based on [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk) and [libesphttpd](https://github.com/Spritetm/libesphttpd).

## Theory of Operations
- A client makes a GET request.
- The server responds with a status code and the requested data in JSON format.
- The server may want to authenticate the client before serving it with any data.
- Once authenticated the client will be assigned with a session ID which will be passed to the client as a cookie. The client must enable cookies.
- The server controls the session expiry.
- The server may control access to certain data according to the client's type or role.
- A session can be extended with a payload which can be any form of data. I.e. authentication data can be kept in a session payload.
- The server may assign any anonymous client with a session id to identify the client in a multi-step oriented process. The states of the process can also be kept in the session payload.
- The server may need to define its own session payload data's structure according to individual application requirement.

Authentication is not part of this framework since there are many ways to do it. Hence, access control is also not part of this framework because it depends on the authentication method.

This framework focuses on the web application progrmming interface (API) and session control. It uses the ESP8266 system randomizer to generate 32-bit session id. And it depends on cookies to maintain the session. Hence, high security is also not part of this framework.

This is a framework which expects more structures to be built on top of it.
