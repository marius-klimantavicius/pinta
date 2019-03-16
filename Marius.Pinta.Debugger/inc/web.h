/*
web.h - BSD LICENSE - Andreas Fredriksson

ABOUT:
This is a web server intended for debugging tools inside a
program with a continously running main loop. It's intended to be used when
all you need is something tiny and performance isn't a key concern.
NOTE: this is a single header port of Andreas Fredriksson
Webby(https://github.com/deplinenoise/webby).

Features
- No dynamic memory allocations -- server memory is completely fixed
- No threading, all I/O and serving happens on the calling thread
- Supports socket keep-alives
- Supports the 100-Continue scheme for file uploading
- Basic support for WebSockets is available.

Because request/response I/O is synchronous on the calling thread, performance
will suffer when you are serving data. For the use-cases wby is intended for,
this is fine. You can still run wby in a background thread at your
discretion if this is a problem.

LICENSE: (BSD)
Copyright (c) 2016, Andreas Fredriksson, Micha Mettke
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

CONTRIBUTORS:
Andreas Fredriksson (implementation)
Micha Mettke (single header conversion)

USAGE:
Request handling
When you configure the server, you give it a function pointer to your
dispatcher. The dispatcher is called by wby when a request has been fully
read into memory and is ready for processing. The socket the request came in on
has then been switched to blocking mode, and you're free to read any request
data using `pinta_web_context_request_read()` (if present, check `content_length`) and then write
your response.
There are two ways to generate a response; explicit size or chunked.

When you know the size of the data
When you know in advance how big the response is going to be, you should pass
that size in bytes to `pinta_web_context_response_begin()` (it will be sent as the
Content-Length header). You then call `pinta_web_context_response_write()` to push that data out, and
finally `pinta_web_context_response_end()` to finalize the response and prepare the socket
for a new request.

When the response size is dynamic
Sometimes you want to generate an arbitrary amount of text in the response, and
you don't know how much that will be. Rather than buffering everything in RAM,
you can use chunked encoding. First call `pinta_web_context_response_begin()` as normal, but
pass it -1 for the content length. This triggers sending the
`Transfer-Encoding: chunked` header. You then call `pinta_web_context_response_write()` as desired
until the response is complete. When you're done, call `pinta_web_context_response_end()` to finish up.
*/

#ifndef PINTA_WEB_H
#define PINTA_WEB_H

#include "pinta.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    typedef uintptr_t PintaWebSocket;
#else
    typedef i32 PintaWebSocket;
#endif

#define PINTA_WEB_OK (0)

#ifndef PINTA_WEB_MAX_HEADERS
#define PINTA_WEB_MAX_HEADERS 32
#endif

    struct PintaWebHeader
    {
        const char *name;
        const char *value;
    };

    /* A HTTP request. */
    struct PintaWebRequest
    {
        /* The method of the request, e.g. "GET", "POST" and so on */
        const char *method;
        /* The URI that was used. */
        const char *uri;
        /* The used HTTP version */
        const char *http_version;
        /* The query parameters passed in the URL, or NULL if none were passed. */
        const char *query_params;
        /* The number of bytes of request body that are available via WebbyRead() */
        i32 content_length;
        /* The number of headers */
        i32 header_count;
        /* Request headers */
        struct PintaWebHeader headers[PINTA_WEB_MAX_HEADERS];
    };

    /* Connection context, as published to the serving callback. */
    struct PintaWebContext
    {
        /* The request being served. Read-only. */
        struct PintaWebRequest request;
        /* User data. Read-write. */
        void *user_data;
    };

    struct PintaWebFrame
    {
        u8 flags;
        u8 opcode;
        u8 header_length;
        u8 padding_;
        u8 mask_key[4];
        i32 payload_length;
    };

    enum PintaWebWebsocketFlags
    {
        PINTA_WEB_WSF_FIN = (1 << 0),
        PINTA_WEB_WSF_MASKED = (1 << 1)
    };

    enum PintaWebWebsocketOperation
    {
        PINTA_WEB_WSOP_CONTINUATION = 0,
        PINTA_WEB_WSOP_TEXT_FRAME = 1,
        PINTA_WEB_WSOP_BINARY_FRAME = 2,
        PINTA_WEB_WSOP_CLOSE = 8,
        PINTA_WEB_WSOP_PING = 9,
        PINTA_WEB_WSOP_PONG = 10
    };

    /* Configuration data required for starting a server. */
    typedef void(*PintaWebLog)(const char *message);
    struct PintaWebConfiguration
    {
        /* userdata which will be passed */
        void *user_data;
        /* The bind address. Must be a textual IP address. */
        const char *address;
        /* The port to listen to. */
        u16 port;
        /* Maximum number of simultaneous connections. */
        u32 connection_max;
        /* The size of the request buffer. This must be big enough to contain all
        * headers and the request line sent by the client. 2-4k is a good size for
        * this buffer. */
        u32 request_buffer_length;
        /* The size of the I/O buffer, used when writing the reponse. 4k is a good
        * choice for this buffer.*/
        u32 io_buffer_length;
        /* Optional callback function that receives debug log text (without newlines). */
        PintaWebLog log;
        /* Request dispatcher function. This function is called when the request
        * structure is ready.
        * If you decide to handle the request, call pinta_web_context_response_begin(),
        * pinta_web_context_response_write() and pinta_web_context_response_end() and then return 0. Otherwise, return a
        * non-zero value to send back a 404 response. */
        i32(*on_request)(struct PintaWebContext *connection, void *user_data);
        /*WebSocket connection dispatcher. Called when an incoming request wants to
        * update to a WebSocket connection.
        * Return 0 to allow the connection.
        * Return 1 to ignore the connection.*/
        i32(*on_ws_connect)(struct PintaWebContext*, void *user_data);
        /* Called when a WebSocket connection has been established.*/
        void(*on_ws_connected)(struct PintaWebContext*, void *user_data);
        /*Called when a WebSocket connection has been closed.*/
        void(*on_ws_closed)(struct PintaWebContext*, void *user_data);
        /*Called when a WebSocket data frame is incoming.
        * Call pinta_web_context_request_read() to read the payload data.
        * Return non-zero to close the connection.*/
        i32(*on_ws_frame)(struct PintaWebContext*, const struct PintaWebFrame *frame, void *user_data);
    };

    struct PintaWebConnection;
    struct PintaWebServer
    {
        /* server configuration */
        struct PintaWebConfiguration configuration;
        /* minimum required memory */
        u32 memory_length;
        /* server socket */
        PintaWebSocket socket;
        /* number of active connection */
        u32 connection_count;
        /* connections */
        struct PintaWebConnection *connections;
#ifdef _WIN32
        /* whether WSAStartup had to be called on Windows */
        i32 windows_socket_initialized;
#endif
    };

    /*  this function clears the server and calculates the needed memory to run
    Input:
    -   filled server configuration data to calculate the needed memory
    Output:
    -   needed memory for the server to run
    */
    void pinta_web_server_init(struct PintaWebServer* server, const struct PintaWebConfiguration* server_configuration, u32 *needed_memory);
    
    /*  this function starts running the server in the specificed memory space. Size
    *  must be at least big enough as determined in the wby_server_init().
    Input:
    -   allocated memory space to create the server into
    */
    i32 pinta_web_server_start(struct PintaWebServer* server, void *memory);
    
    /* updates the server by being called frequenctly (at least once a frame) */
    void pinta_web_server_update(struct PintaWebServer* server);
    
    /* stops and shutdown the server */
    void pinta_web_server_stop(struct PintaWebServer* server);
    
    /*  this function begins a response
    Input:
    -   HTTP status code to send. (Normally 200).
    -   size in bytes you intend to write, or -1 for chunked encoding
    -   array of HTTP headers to transmit (can be NULL of header_count == 0)
    -   number of headers in the array
    Output:
    -   returns 0 on success, non-zero on error.
    */
    i32 pinta_web_context_response_begin(struct PintaWebContext* context, i32 status_code, i32 content_length, const struct PintaWebHeader *headers, i32 header_count);
    
    /*  this function finishes a response. When you're done wirting the response
    *  body, call this function. this makes sure chunked encoding is terminated
    *  correctly and that the connection is setup for reuse. */
    void pinta_web_context_response_end(struct PintaWebContext* context);
    
    /*  this function reads data from the request body. Only read what the client
    *  has priovided (via content_length) parameter, or you will end up blocking
    *  forever.
    Input:
    - pointer to a memory block that will be filled
    - size of the memory block to fill
    */
    i32 pinta_web_context_request_read(struct PintaWebContext* context, void *output_buffer, u32 output_length);
    
    /*  this function writes a response data to the connection. If you're not using
    *  chunked encoding, be careful not to send more than the specified content
    *  length. You can call this function multiple times as long as the total
    *  number of bytes matches up with the content length.
    Input:
    - pointer to a memory block that will be send
    - size of the memory block to send
    */
    i32 pinta_web_context_response_write(struct PintaWebContext* context, const void *output_buffer, u32 output_length);
    
    /*  this function begins an outgoing websocket frame */
    i32 pinta_web_context_frame_begin(struct PintaWebContext* context, i32 opcode);
    
    /*  this function finishes an outgoing websocket frame */
    i32 pinta_web_context_frame_end(struct PintaWebContext* context);
    
    /*  this function is a helper function to lookup a query parameter given a URL
    *  encoded string. Returns the size of the returned data, or -1 if the query
    *  var wasn't found. */
    i32 pinta_web_context_find_query_var(const char *buffer, const char *name, char *destination, u32 destination_length);
    
    /*  this convenience function to find a header in a request. Returns the value
    *  of the specified header, or NULL if its was not present. */
    const char* pinta_web_context_find_header(struct PintaWebContext* context, const char *name);

#ifdef __cplusplus
}
#endif
#endif /* PINTA_WEB_H */
