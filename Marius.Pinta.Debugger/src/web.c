#include "web.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#define PINTA_SOCKET(s) ((PintaWebSocket)(s))
#define PINTA_LENGTH(a) (sizeof(a)/sizeof((a)[0]))

/* ===============================================================
*                          UTIL
* ===============================================================*/
struct PintaWebBuffer
{
    /* current buffer size */
    u32 used;
    /* buffer capacity */
    u32 capacity;
    /* pointer inside a global buffer */
    u8 *data;
};

static void pinta_web_log(PintaWebLog log, const char *format, ...)
{
    char buffer[1024];
    va_list args;
    if (!log) return;

    va_start(args, format);
    vsnprintf(buffer, sizeof buffer, format, args);
    va_end(args);
    buffer[(sizeof buffer) - 1] = '\0';
    log(buffer);
}

static i32 pinta_web_peek_request_length(const u8 *buffer, i32 buffer_length)
{
    i32 i;
    i32 capacity = buffer_length - 3;
    for (i = 0; i < capacity; ++i)
    {
        if ('\r' != buffer[i + 0]) continue;
        if ('\n' != buffer[i + 1]) continue;
        if ('\r' != buffer[i + 2]) continue;
        if ('\n' != buffer[i + 3]) continue;
        /* OK; we have CRLFCRLF which indicates the end of the header section */
        return i + 4;
    }
    return -1;
}

static char* pinta_web_skip_whitespace(char *p)
{
    for (;;)
    {
        char ch = *p;
        if (' ' == ch || '\t' == ch) ++p;
        else break;
    }
    return p;
}

#define PINTA_WEB_TOKENIZER_SKIP_WHITESPACE (1 << 0)
static i32 pinta_web_tokenize_in_place(char *buffer, const char* separator, char *tokens[], i32 capacity, i32 flags)
{
    char *b = buffer;
    char *e = buffer;
    i32 token_count = 0;
    i32 separator_length = (i32)strlen(separator);
    while (token_count < capacity)
    {
        if (flags & PINTA_WEB_TOKENIZER_SKIP_WHITESPACE)
            b = pinta_web_skip_whitespace(b);
        if (NULL != (e = strstr(b, separator)))
        {
            i32 len = (i32)(e - b);
            if (len > 0)
                tokens[token_count++] = b;
            *e = '\0';
            b = e + separator_length;
        }
        else
        {
            tokens[token_count++] = b;
            break;
        }
    }
    return token_count;
}

static u32 pinta_web_make_websocket_header(u8 buffer[10], u8 opcode, i32 payload_length, i32 is_final)
{
    buffer[0] = (u8)((is_final ? 0x80 : 0x00) | opcode);
    if (payload_length < 126)
    {
        buffer[1] = (u8)(payload_length & 0x7f);
        return 2;
    }
    else if (payload_length < 65536)
    {
        buffer[1] = 126;
        buffer[2] = (u8)(payload_length >> 8);
        buffer[3] = (u8)payload_length;
        return 4;
    }
    else
    {
        buffer[1] = 127;
        /* Ignore high 32-bits. I didn't want to require 64-bit types and typdef hell in the API. */
        buffer[2] = buffer[3] = buffer[4] = buffer[5] = 0;
        buffer[6] = (u8)(payload_length >> 24);
        buffer[7] = (u8)(payload_length >> 16);
        buffer[8] = (u8)(payload_length >> 8);
        buffer[9] = (u8)payload_length;
        return 10;
    }
}

static i32 pinta_web_read_buffered_data(i32 *data_left, struct PintaWebBuffer* buffer, char **output_buffer, u32 *output_length)
{
    i32 offset, read_length;
    i32 left = *data_left;
    i32 len;
    if (left == 0)
        return 0;

    len = (i32)*output_length;
    offset = (i32)buffer->used - left;
    read_length = (len > left) ? left : len;
    memcpy(*output_buffer, buffer->data + offset, (u32)read_length);

    (*output_buffer) += read_length;
    (*output_length) -= (u32)read_length;
    (*data_left) -= read_length;
    return read_length;
}

/* ---------------------------------------------------------------
*                          SOCKET
* ---------------------------------------------------------------*/
#ifdef _WIN32

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

typedef i32 PintaWebSocketLength;
typedef BOOL PintaWebSocketOptions;

#if defined(__GNUC__)
#define PINTA_WEB_ALIGN(x) __attribute__((aligned(x)))
#else
#define PINTA_WEB_ALIGN(x) __declspec(align(x))
#endif

#define PINTA_WEB_INVALID_SOCKET INVALID_SOCKET
#define snprintf _snprintf

static i32 pinta_socket_get_error(void)
{
    return WSAGetLastError();
}

#if !defined(__GNUC__)
static i32 strcasecmp(const char *a, const char *b)
{
    return _stricmp(a, b);
}

static i32 strncasecmp(const char *a, const char *b, u32 len)
{
    return _strnicmp(a, b, len);
}
#endif

static i32 pinta_socket_set_blocking(PintaWebSocket socket, i32 blocking)
{
    u_long val = !blocking;
    return ioctlsocket(socket, FIONBIO, &val);
}

static i32 pinta_socket_is_valid(PintaWebSocket socket)
{
    return (INVALID_SOCKET != socket);
}

static void pinta_socket_close(PintaWebSocket socket)
{
    closesocket(socket);
}

static i32 pinta_socket_is_blocking_error(i32 error)
{
    return WSAEWOULDBLOCK == error;
}

#else /* UNIX */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>

typedef socklen_t PintaWebSocketLength;
typedef i32 PintaWebSocketOptions;

#define PINTA_WEB_ALIGN(x) __attribute__((aligned(x)))
#define PINTA_WEB_INVALID_SOCKET (-1)

static i32 pinta_socket_get_error(void)
{
    return errno;
}

static i32 pinta_socket_is_valid(PintaWebSocket socket)
{
    return (socket > 0);
}

static void pinta_socket_close(PintaWebSocket socket)
{
    close(socket);
}

static i32 pinta_socket_is_blocking_error(i32 error)
{
    return (EAGAIN == error);
}

static i32 pinta_socket_set_blocking(PintaWebSocket socket, i32 blocking)
{
    i32 flags = fcntl(socket, F_GETFL, 0);
    if (flags < 0) return flags;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return fcntl(socket, F_SETFL, flags);
}
#endif

static i32 pinta_socket_config_incoming(PintaWebSocket socket)
{
    PintaWebSocketOptions off = 0;
    i32 err;
    if ((err = pinta_socket_set_blocking(socket, 0)) != PINTA_WEB_OK) return err;
    setsockopt(socket, SOL_SOCKET, SO_LINGER, (const char*)&off, sizeof(i32));
    return 0;
}

static i32 pinta_socket_send(PintaWebSocket socket, const u8 *buffer, i32 buffer_length)
{
    while (buffer_length > 0)
    {
        i32 err = send(socket, (const char*)buffer, (u32)buffer_length, 0);
        if (err <= 0) return 1;
        buffer += err;
        buffer_length -= (i32)err;
    }
    return 0;
}

/* Read as much as possible without blocking while there is buffer space. */
enum { PINTA_WEB_FILL_OK, PINTA_WEB_FILL_ERROR, PINTA_WEB_FILL_FULL };

static i32 pinta_socket_receive(PintaWebSocket socket, struct PintaWebBuffer *buffer, PintaWebLog log)
{
    i32 error;
    i32 buffer_left;
    for (;;)
    {
        buffer_left = (i32)buffer->capacity - (i32)buffer->used;
        pinta_web_log(log, "buffer space left = %d", buffer_left);
        if (buffer_left == 0)
            return PINTA_WEB_FILL_FULL;

        /* Read what we can into the current buffer space. */
        error = recv(socket, (char*)buffer->data + buffer->used, (u32)buffer_left, 0);
        if (error < 0)
        {
            i32 socket_error = pinta_socket_get_error();
            if (pinta_socket_is_blocking_error(socket_error))
            {
                return PINTA_WEB_FILL_OK;
            }
            else
            {
                /* Read error. Give up. */
                pinta_web_log(log, "read error %d - connection dead", socket_error);
                return PINTA_WEB_FILL_ERROR;
            }
        }
        else if (error == 0)
        {
            /* The peer has closed the connection. */
            pinta_web_log(log, "peer has closed the connection");
            return PINTA_WEB_FILL_ERROR;
        }
        else
        {
            buffer->used += (u32)error;
        }
    }
}

static i32 pinta_socket_flush(PintaWebSocket socket, struct PintaWebBuffer *buffer)
{
    if (buffer->used > 0)
    {
        if (pinta_socket_send(socket, buffer->data, (i32)buffer->used) != PINTA_WEB_OK)
            return 1;
    }
    buffer->used = 0;
    return 0;
}

/* ---------------------------------------------------------------
*                          URL
* ---------------------------------------------------------------*/
/* URL-decode input buffer into destination buffer.
* 0-terminate the destination buffer. Return the length of decoded data.
* form-url-encoded data differs from URI encoding in a way that it
* uses '+' as character for space, see RFC 1866 section 8.2.1
* http://ftp.ics.uci.edu/pub/ietf/html/rfc1866.txt
*
* This bit of code was taken from mongoose.
*/
static u32 pinta_web_url_decode(const char *source, u32 source_length, char *destination, u32 destination_length, i32 is_form_url_encoded)
{
    i32 a, b;
    u32 i, j;
#define HEXTOI(x) (isdigit(x) ? x - '0' : x - 'W')
    for (i = j = 0; i < source_length && j < destination_length - 1; i++, j++)
    {
        if (source[i] == '%' &&
            isxdigit(*(const u8*)(source + i + 1)) &&
            isxdigit(*(const u8*)(source + i + 2)))
        {
            a = tolower(*(const u8*)(source + i + 1));
            b = tolower(*(const u8*)(source + i + 2));
            destination[j] = (char)((HEXTOI(a) << 4) | HEXTOI(b));
            i += 2;
        }
        else if (is_form_url_encoded && source[i] == '+')
        {
            destination[j] = ' ';
        }
        else destination[j] = source[i];
    }
#undef HEXTOI
    destination[j] = '\0'; /* Null-terminate the destination */
    return j;
}

/* Pulled from mongoose */
i32 pinta_web_context_find_query_var(const char *buffer, const char *name, char *destination, u32 destination_length)
{
    const char *p, *e, *s;
    u32 name_length;
    i32 len;
    u32 buffer_length = (u32)strlen(buffer);

    name_length = (u32)strlen(name);
    e = buffer + buffer_length;
    len = -1;
    destination[0] = '\0';

    /* buffer is "var1=val1&var2=val2...". Find variable first */
    for (p = buffer; p != NULL && p + name_length < e; p++)
    {
        if ((p == buffer || p[-1] == '&') && p[name_length] == '=' &&
            strncasecmp(name, p, name_length) == 0)
        {
            /* Point p to variable value */
            p += name_length + 1;
            /* Point s to the end of the value */
            s = (const char *)memchr(p, '&', (u32)(e - p));
            if (s == NULL) s = e;
            pinta_assert(s >= p);
            /* Decode variable into destination buffer */
            if ((u32)(s - p) < destination_length)
                len = (i32)pinta_web_url_decode(p, (u32)(s - p), destination, destination_length, 1);
            break;
        }
    }
    return len;
}

/* ---------------------------------------------------------------
*                          BASE64
* ---------------------------------------------------------------*/
#define PINTA_WEB_BASE64_QUADS_BEFORE_LINEBREAK 19

static u32 pinta_web_base64_get_length(u32 input_length)
{
    u32 triplets = (input_length + 2) / 3;
    u32 base_length = 4 * triplets;
    u32 line_breaks = 2 * (triplets / PINTA_WEB_BASE64_QUADS_BEFORE_LINEBREAK);
    u32 null_termination = 1;
    return base_length + line_breaks + null_termination;
}

static i32 pinta_web_base64_encode(char* output, u32 output_length, const u8 *input, u32 input_length)
{
    u32 i = 0;
    i32 line_out = 0;
    static const char enc[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/=";
    if (output_length < pinta_web_base64_get_length(input_length))
        return 1;

    while (i < input_length)
    {
        u32 idx_0, idx_1, idx_2, idx_3;
        u32 i0;

        i0 = (u32)(input[i]) << 16; i++;
        i0 |= (u32)(i < input_length ? input[i] : 0) << 8; i++;
        i0 |= (i < input_length ? input[i] : 0); i++;

        idx_0 = (i0 & 0xfc0000) >> 18; i0 <<= 6;
        idx_1 = (i0 & 0xfc0000) >> 18; i0 <<= 6;
        idx_2 = (i0 & 0xfc0000) >> 18; i0 <<= 6;
        idx_3 = (i0 & 0xfc0000) >> 18;

        if (i - 1 > input_length) idx_2 = 64;
        if (i > input_length) idx_3 = 64;

        *output++ = enc[idx_0];
        *output++ = enc[idx_1];
        *output++ = enc[idx_2];
        *output++ = enc[idx_3];

        if (++line_out == PINTA_WEB_BASE64_QUADS_BEFORE_LINEBREAK)
        {
            *output++ = '\r';
            *output++ = '\n';
        }
    }
    *output = '\0';
    return 0;
}

/* ---------------------------------------------------------------
*                          SHA1
* ---------------------------------------------------------------*/
struct PintaWebSha1
{
    u32 state[5];
    u32 message_length[2];
    u32 buffer_used;
    u8 buffer[64];
};

static u32 pinta_web_sha1_rol(u32 value, u32 bits)
{
    return ((value) << bits) | (value >> (32 - bits));
}

static void pinta_web_sha1_hash_block(u32 state[5], const u8 *block)
{
    i32 i;
    u32 a, b, c, d, e;
    u32 w[80];

    /* Prepare message schedule */
    for (i = 0; i < 16; ++i)
    {
        w[i] = (((u32)block[(i * 4) + 0]) << 24) |
            (((u32)block[(i * 4) + 1]) << 16) |
            (((u32)block[(i * 4) + 2]) << 8) |
            (((u32)block[(i * 4) + 3]) << 0);
    }

    for (i = 16; i < 80; ++i)
        w[i] = pinta_web_sha1_rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    /* Initialize working variables */
    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4];

    /* This is the core loop for each 20-word span. */
#define SHA1_LOOP(start, end, func, constant) \
        for (i = (start); i < (end); ++i) { \
            u32 t = pinta_web_sha1_rol(a, 5) + (func) + e + (constant) + w[i]; \
            e = d; d = c; c = pinta_web_sha1_rol(b, 30); b = a; a = t;}

    SHA1_LOOP(0, 20, ((b & c) ^ (~b & d)), 0x5a827999)
        SHA1_LOOP(20, 40, (b ^ c ^ d), 0x6ed9eba1)
        SHA1_LOOP(40, 60, ((b & c) ^ (b & d) ^ (c & d)), 0x8f1bbcdc)
        SHA1_LOOP(60, 80, (b ^ c ^ d), 0xca62c1d6)
#undef SHA1_LOOP

        /* Update state */
        state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
}

static void pinta_web_sha1_init(struct PintaWebSha1 *s)
{
    s->state[0] = 0x67452301;
    s->state[1] = 0xefcdab89;
    s->state[2] = 0x98badcfe;
    s->state[3] = 0x10325476;
    s->state[4] = 0xc3d2e1f0;

    s->message_length[0] = 0;
    s->message_length[1] = 0;
    s->buffer_used = 0;
}

static void pinta_web_sha1_update(struct PintaWebSha1 *s, const void *buffer, u32 buffer_length)
{
    u8 *data = (u8*)buffer;
    u32 length_lo;
    u32 length_lo_orig;
    u32 remain = buffer_length;

    while (remain > 0)
    {
        u32 buffer_space = sizeof(s->buffer) - s->buffer_used;
        u32 copy_length = (remain < buffer_space) ? remain : buffer_space;
        memcpy(s->buffer + s->buffer_used, data, copy_length);

        s->buffer_used += copy_length;
        data += copy_length;
        remain -= copy_length;

        if (s->buffer_used == sizeof(s->buffer))
        {
            pinta_web_sha1_hash_block(s->state, s->buffer);
            s->buffer_used = 0;
        }
    }

    length_lo = length_lo_orig = s->message_length[1];
    length_lo += (u32)(buffer_length * 8);

    if (length_lo < length_lo_orig)
        s->message_length[0] += 1;

    s->message_length[1] = length_lo;
}

static void pinta_web_sha1_final(struct PintaWebSha1 *s, u8 digest[20])
{
    u8 zero = 0x00;
    u8 one_bit = 0x80;
    u8 count_data[8];
    i32 i;

    /* Generate size data in bit endian format */
    for (i = 0; i < 8; ++i)
    {
        u32 word = s->message_length[i >> 2];
        count_data[i] = (u8)(word >> ((3 - (i & 3)) * 8));
    }

    /* Set trailing one-bit */
    pinta_web_sha1_update(s, &one_bit, 1);
    /* Emit null padding to to make room for 64 bits of size info in the last 512 bit block */
    while (s->buffer_used != 56)
        pinta_web_sha1_update(s, &zero, 1);

    /* Write size in bits as last 64-bits */
    pinta_web_sha1_update(s, count_data, 8);
    /* Make sure we actually finalized our last block */
    pinta_assert(s->buffer_used == 0);

    /* Generate digest */
    for (i = 0; i < 20; ++i)
    {
        u32 word = s->state[i >> 2];
        u8 byte = (u8)((word >> ((3 - (i & 3)) * 8)) & 0xff);
        digest[i] = byte;
    }
}

/* ---------------------------------------------------------------
*                          CONNECTION
* ---------------------------------------------------------------*/
#define PINTA_WEB_WEBSOCKET_VERSION "13"

static const char pinta_web_continue_header[] = "HTTP/1.1 100 Continue\r\n\r\n";
static const u32 pinta_web_continue_header_length = sizeof(pinta_web_continue_header) - 1;
static const char pinta_web_websocket_guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static const u32 pinta_web_websocket_guid_length = sizeof(pinta_web_websocket_guid) - 1;
static const u8 pinta_web_websocket_pong[] = { 0x80, PINTA_WEB_WSOP_PONG, 0x00 };
static const struct PintaWebHeader pinta_web_plain_text_headers[] = { { "Content-Type", "text/plain" } };

enum PintaWebConnectionFlags
{
    PINTA_WEB_CONNECTION_FLAG_ALIVE = (1 << 0),
    PINTA_WEB_CONNECTION_FLAG_FRESH_CONNECTION = (1 << 1),
    PINTA_WEB_CONNECTION_FLAG_CLOSE_AFTER_RESPONSE = (1 << 2),
    PINTA_WEB_CONNECTION_FLAG_CHUNKED_RESPONSE = (1 << 3),
    PINTA_WEB_CONNECTION_FLAG_WEBSOCKET = (1 << 4)
};

enum PintaWebConnectionState
{
    PINTA_WEB_CONNECTION_STATE_REQUEST,
    PINTA_WEB_CONNECTION_STATE_SEND_CONTINUE,
    PINTA_WEB_CONNECTION_STATE_SERVE,
    PINTA_WEB_CONNECTION_STATE_WEBSOCKET
};

struct PintaWebConnection
{
    struct PintaWebContext context;

    u16 flags;
    u16 state;

    PintaWebSocket socket;
    PintaWebLog log;

    u32 request_buffer_length;
    struct PintaWebBuffer header_buffer;
    struct PintaWebBuffer io_buffer;
    u32 io_buffer_length;

    i32 header_body_left;
    i32 io_data_left;
    i32 continue_data_left;
    i32 body_bytes_read;

    struct PintaWebFrame on_ws_frame;
    u8 ws_opcode;
    u32 blocking_count;
};

static i32 pinta_web_connection_set_blocking(struct PintaWebConnection *connection)
{
    if (connection->blocking_count == 0)
    {
        if (PINTA_SOCKET(connection->socket) == PINTA_WEB_INVALID_SOCKET)
        {
            pinta_web_log(connection->log, "failed to switch connection to blocking: invalid socket");
            connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
            return -1;
        }

        if (pinta_socket_set_blocking(PINTA_SOCKET(connection->socket), 1) != PINTA_WEB_OK)
        {
            pinta_web_log(connection->log, "failed to switch connection to blocking: %d", pinta_socket_get_error());
            connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
            return -1;
        }
    }
    ++connection->blocking_count;
    return 0;
}

static i32 pinta_web_connection_set_nonblocking(struct PintaWebConnection *connection)
{
    u32 count = connection->blocking_count;
    if ((connection->flags & PINTA_WEB_CONNECTION_FLAG_ALIVE) != 0 && count == 1)
    {
        if (pinta_socket_set_blocking(PINTA_SOCKET(connection->socket), 0) != PINTA_WEB_OK)
        {
            pinta_web_log(connection->log, "failed to switch connection to non-blocking");
            connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
            return -1;
        }
    }
    connection->blocking_count = count - 1;
    return 0;
}

static void pinta_web_connection_reset(struct PintaWebConnection *connection, u32 request_buffer_length, u32 io_buffer_length)
{
    connection->header_buffer.used = 0;
    connection->header_buffer.capacity = request_buffer_length;
    connection->header_body_left = 0;
    connection->io_buffer.capacity = io_buffer_length;
    connection->io_buffer.used = 0;
    connection->io_data_left = 0;
    connection->context.user_data = NULL;
    connection->blocking_count = 0;
    connection->continue_data_left = 0;
    connection->state = PINTA_WEB_CONNECTION_STATE_REQUEST;
    connection->body_bytes_read = 0;
}

static void pinta_web_connection_reset_ws(struct PintaWebConnection *connection, u32 request_buffer_length, u32 io_buffer_length)
{
    connection->header_buffer.used = 0;
    connection->header_buffer.capacity = request_buffer_length;
    connection->header_body_left = 0;
    connection->context.user_data = NULL;
    connection->blocking_count = 0;
    connection->continue_data_left = 0;
    connection->state = PINTA_WEB_CONNECTION_STATE_REQUEST;
    connection->io_buffer.capacity = io_buffer_length;

    if (connection->io_data_left == 0)
    {
        connection->io_buffer.used = 0;
    }
    else
    {
        i32 length = connection->on_ws_frame.header_length + connection->on_ws_frame.payload_length;
        memmove(connection->io_buffer.data, connection->io_buffer.data + length, connection->io_data_left);
        connection->io_buffer.used = connection->io_data_left;
    }

    connection->io_data_left = 0;
    connection->body_bytes_read = 0;
}

static void pinta_web_connection_close(struct PintaWebConnection* connection)
{
    if (PINTA_SOCKET(connection->socket) != PINTA_WEB_INVALID_SOCKET)
    {
        pinta_socket_close(PINTA_SOCKET(connection->socket));
        connection->socket = (PintaWebSocket)PINTA_WEB_INVALID_SOCKET;
    }
    connection->flags = 0;
}

static i32 pinta_web_connection_setup_request(struct PintaWebConnection *connection, i32 request_length)
{
    char* lines[PINTA_WEB_MAX_HEADERS + 2];
    i32 line_count;
    char* tokens[16];
    char* query_params;
    i32 token_count;

    i32 i;
    i32 header_count;
    char *buffer = (char*)connection->header_buffer.data;
    struct PintaWebRequest *req = &connection->context.request;

    /* Null-terminate the request envelope by overwriting the last CRLF with 00LF */
    buffer[request_length - 2] = '\0';
    /* Split header into lines */
    line_count = pinta_web_tokenize_in_place(buffer, "\r\n", lines, PINTA_LENGTH(lines), 0);
    header_count = line_count - 2;
    if (line_count < 1 || header_count >(i32) PINTA_LENGTH(req->headers))
        return 1;

    /* Parse request line */
    token_count = pinta_web_tokenize_in_place(lines[0], " ", tokens, PINTA_LENGTH(tokens), 0);
    if (3 != token_count) return 1;

    req->method = tokens[0];
    req->uri = tokens[1];
    req->http_version = tokens[2];
    req->content_length = 0;

    /* See if there are any query parameters */
    if ((query_params = (char*)strchr(req->uri, '?')) != NULL)
    {
        req->query_params = query_params + 1;
        *query_params = '\0';
    }
    else req->query_params = NULL;

    {
        /* Decode the URI in place */
        u32 uri_length = (u32)strlen(req->uri);
        pinta_web_url_decode(req->uri, uri_length, (char*)req->uri, uri_length + 1, 1);
    }

    /* Parse headers */
    for (i = 0; i < header_count; ++i)
    {
        token_count = pinta_web_tokenize_in_place(lines[i + 1], ":", tokens, 2, PINTA_WEB_TOKENIZER_SKIP_WHITESPACE);
        if (token_count != 2) return 1;
        req->headers[i].name = tokens[0];
        req->headers[i].value = tokens[1];

        if (!strcasecmp("content-length", tokens[0]))
        {
            req->content_length = (i32)strtoul(tokens[1], NULL, 10);
            pinta_web_log(connection->log, "request has body; content length is %d", req->content_length);
        }
        else if (!strcasecmp("transfer-encoding", tokens[0]))
        {
            pinta_web_log(connection->log, "cowardly refusing to handle Transfer-Encoding: %s", tokens[1]);
            return 1;
        }
    }
    req->header_count = header_count;
    return 0;
}

static i32 pinta_web_connection_send_websocket_upgrade(struct PintaWebConnection* connection)
{
    const char *header;
    struct PintaWebSha1 sha;
    u8 digest[20];
    char output_digest[64];
    struct PintaWebHeader headers[3];
    struct PintaWebContext *context = &connection->context;

    if ((header = pinta_web_context_find_header(context, "Sec-WebSocket-Version")) == NULL)
    {
        pinta_web_log(connection->log, "Sec-WebSocket-Version header not present");
        return 1;
    }
    if (strcmp(header, PINTA_WEB_WEBSOCKET_VERSION))
    {
        pinta_web_log(connection->log, "WebSocket version %s not supported (we only do %s)", header, PINTA_WEB_WEBSOCKET_VERSION);
        return 1;
    }
    if ((header = pinta_web_context_find_header(context, "Sec-WebSocket-Key")) == NULL)
    {
        pinta_web_log(connection->log, "Sec-WebSocket-Key header not present");
        return 1;
    }

    /* Compute SHA1 hash of Sec-Websocket-Key + the websocket guid as required by
    * the RFC.
    *
    * This handshake is bullshit. It adds zero security. Just forces me to drag
    * in SHA1 and create a base64 encoder.
    */
    pinta_web_sha1_init(&sha);
    pinta_web_sha1_update(&sha, header, (u32)strlen(header));
    pinta_web_sha1_update(&sha, pinta_web_websocket_guid, pinta_web_websocket_guid_length);
    pinta_web_sha1_final(&sha, &digest[0]);
    if (pinta_web_base64_encode(output_digest, sizeof output_digest, &digest[0], sizeof(digest)) != PINTA_WEB_OK)
        return 1;

    headers[0].name = "Upgrade";
    headers[0].value = "websocket";
    headers[1].name = "Connection";
    headers[1].value = "Upgrade";
    headers[2].name = "Sec-WebSocket-Accept";
    headers[2].value = output_digest;
    pinta_web_context_response_begin(&connection->context, 101, 0, headers, PINTA_LENGTH(headers));
    pinta_web_context_response_end(&connection->context);
    return 0;
}

static i32 pinta_web_connection_push(struct PintaWebConnection *connection, const void *buffer, i32 buffer_length)
{
    struct PintaWebBuffer *io_buffer = &connection->io_buffer;
    const u8* data = (const u8*)buffer;

    if (connection->state != PINTA_WEB_CONNECTION_STATE_SERVE)
    {
        pinta_web_log(connection->log, "attempt to write in non-serve state");
        return 1;
    }

    if (buffer_length == 0)
        return pinta_socket_flush(PINTA_SOCKET(connection->socket), io_buffer);

    while (buffer_length > 0)
    {
        i32 buffer_space = (i32)io_buffer->capacity - (i32)io_buffer->used;
        i32 copy_length = buffer_length < buffer_space ? buffer_length : buffer_space;
        memcpy(io_buffer->data + io_buffer->used, data, (u32)copy_length);
        io_buffer->used += (u32)copy_length;

        data += copy_length;
        buffer_length -= copy_length;
        if (io_buffer->used == io_buffer->capacity)
        {
            if (pinta_socket_flush(PINTA_SOCKET(connection->socket), io_buffer) != PINTA_WEB_OK)
                return 1;
            if ((u32)buffer_length >= io_buffer->capacity)
                return pinta_socket_send(PINTA_SOCKET(connection->socket), data, buffer_length);
        }
    }
    return 0;
}

/* ---------------------------------------------------------------
*                          CON/REQUEST
* ---------------------------------------------------------------*/
static i32 pinta_web_connection_discard_incoming_data(struct PintaWebContext* context, i32 count)
{
    while (count > 0)
    {
        char buffer[1024];
        i32 read_length = (i32)(((u32)count > sizeof(buffer)) ? sizeof(buffer) : (u32)count);

        if (pinta_web_context_request_read(context, buffer, (u32)read_length) != PINTA_WEB_OK)
            return -1;
        count -= read_length;
    }
    return 0;
}

const char* pinta_web_context_find_header(struct PintaWebContext *context, const char *name)
{
    i32 i, count;
    for (i = 0, count = context->request.header_count; i < count; ++i)
    {
        if (!strcasecmp(context->request.headers[i].name, name))
            return context->request.headers[i].value;
    }
    return NULL;
}

static i32 pinta_web_connection_is_websocket_request(struct PintaWebContext* context)
{
    const char *header;

    if ((header = pinta_web_context_find_header(context, "Connection")) == NULL) return 0;
    if (strcasecmp(header, "Upgrade"))
    {
        // might be token like: keep-alive, Upgrade
        i32 has_upgrade = 0;
        u32 header_length = (u32)strlen(header);
        u32 start = 0, end = 0;
        u32 index = 0;
        u8 sub[9];

        while (!has_upgrade && start < header_length)
        {
            for (index = start; index < header_length; index++)
            {
                if (header[index] == ',')
                    break;
            }

            end = index;

            // trim start
            for (index = start; index < end; index++)
            {
                if (header[index] != ' ' && header[index] != '\t')
                    break;
            }

            start = index;

            // trim end
            index = end;
            for (; index - 1 >= start; index--)
            {
                if (header[index - 1] != ' ' && header[index - 1] != '\t')
                    break;
            }

            if (index - start == 7)
            {
                strncpy(sub, &header[start], index - start);
                sub[index - start] = 0;

                if (!strcasecmp(sub, "Upgrade"))
                    has_upgrade = 1;
            }

            start = end + 1;
        }

        if (!has_upgrade)
            return 0;
    }
    if ((header = pinta_web_context_find_header(context, "Upgrade")) == NULL) return 0;
    if (strcasecmp(header, "websocket")) return 0;
    return 1;
}

static i32 pinta_web_scan_websocket_frame(struct PintaWebFrame *frame, const struct PintaWebBuffer *buffer)
{
    u8 flags = 0;
    u32 len = 0;
    u32 opcode = 0;
    u8* data = buffer->data;
    u8* data_end = data + buffer->used;

    i32 i;
    i32 len_bytes = 0;
    i32 mask_bytes = 0;
    u8 header0, header1;
    if (buffer->used < 2)
        return -1;

    header0 = *data++;
    header1 = *data++;
    if (header0 & 0x80)
        flags |= PINTA_WEB_WSF_FIN;
    if (header1 & 0x80)
    {
        flags |= PINTA_WEB_WSF_MASKED;
        mask_bytes = 4;
    }

    opcode = header0 & 0xf;
    len = header1 & 0x7f;
    if (len == 126)
        len_bytes = 2;
    else if (len == 127)
        len_bytes = 8;
    if (data + len_bytes + mask_bytes > data_end)
        return -1;

    /* Read big endian length from length bytes (if greater than 125) */
    len = len_bytes == 0 ? len : 0;
    for (i = 0; i < len_bytes; ++i)
    {
        /* This will totally overflow for 64-bit values. I don't care.
        * If you're transmitting more than 4 GB of data using Webby,
        * seek help. */
        len <<= 8;
        len |= *data++;
    }

    /* Read mask word if present */
    for (i = 0; i < mask_bytes; ++i)
        frame->mask_key[i] = *data++;
    frame->header_length = (u8)(data - buffer->data);
    frame->flags = flags;
    frame->opcode = (u8)opcode;
    frame->payload_length = (i32)len;
    return 0;
}

i32 pinta_web_context_frame_begin(struct PintaWebContext *context, i32 opcode)
{
    struct PintaWebConnection *connection = (struct PintaWebConnection*)context;
    connection->ws_opcode = (u8)opcode;
    /* Switch socket to blocking mode */
    return pinta_web_connection_set_blocking(connection);
}

i32 pinta_web_context_frame_end(struct PintaWebContext *context)
{
    u8 header[10];
    u32 header_length;
    struct PintaWebConnection *connection = (struct PintaWebConnection*) context;
    header_length = pinta_web_make_websocket_header(header, connection->ws_opcode, 0, 1);
    if (pinta_socket_send(PINTA_SOCKET(connection->socket), header, (i32)header_length) != PINTA_WEB_OK)
        connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
    /* Switch socket to non-blocking mode */
    return pinta_web_connection_set_nonblocking(connection);
}

i32 pinta_web_context_request_read(struct PintaWebContext *context, void *output_buffer, u32 output_length)
{
    struct PintaWebConnection* connection = (struct PintaWebConnection*)context;
    char *output = (char*)output_buffer;
    i32 count;

    i32 start_pos = connection->body_bytes_read;
    if (connection->header_body_left > 0)
    {
        count = pinta_web_read_buffered_data(&connection->header_body_left, &connection->header_buffer, &output, &output_length);
        connection->body_bytes_read += count;
    }

    /* Read buffered websocket data */
    if (connection->io_data_left > 0)
    {
        count = pinta_web_read_buffered_data(&connection->io_data_left, &connection->io_buffer, &output, &output_length);
        connection->body_bytes_read += count;
    }

    while (output_length > 0)
    {
        i32 err = recv(PINTA_SOCKET(connection->socket), output, (u32)output_length, 0);
        if (err < 0)
        {
            connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
            return (i32)err;
        }
        output_length -= (u32)err;
        output += (u32)err;
        connection->body_bytes_read += (i32)err;
    }

    if ((connection->flags & PINTA_WEB_CONNECTION_FLAG_WEBSOCKET) && (connection->on_ws_frame.flags & PINTA_WEB_WSF_MASKED))
    {
        /* XOR outgoing data with websocket ofuscation key */
        i32 i, end_pos = connection->body_bytes_read;
        const u8 *mask = connection->on_ws_frame.mask_key;
        output = (char*)output_buffer; /* start over */
        for (i = start_pos; i < end_pos; ++i)
        {
            u8 byte = (u8)*output;
            *output++ = (char)(byte ^ mask[i & 3]);
        }
    }
    return 0;
}

i32 pinta_web_context_response_write(struct PintaWebContext *context, const void *data_buffer, u32 data_length)
{
    struct PintaWebConnection *connection = (struct PintaWebConnection*) context;
    if (connection->flags & PINTA_WEB_CONNECTION_FLAG_WEBSOCKET)
    {
        u8 header[10];
        u32 header_length;
        header_length = pinta_web_make_websocket_header(header, connection->ws_opcode, (i32)data_length, 0);

        /* Overwrite opcode to be continuation packages from here on out */
        connection->ws_opcode = PINTA_WEB_WSOP_CONTINUATION;
        if (pinta_socket_send(PINTA_SOCKET(connection->socket), header, (i32)header_length) != PINTA_WEB_OK)
        {
            connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
            return -1;
        }
        if (pinta_socket_send(PINTA_SOCKET(connection->socket), (const u8*)data_buffer, (i32)data_length) != PINTA_WEB_OK)
        {
            connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
            return -1;
        }
        return 0;
    }
    else if (connection->flags & PINTA_WEB_CONNECTION_FLAG_CHUNKED_RESPONSE)
    {
        char chunk_header[128];
        i32 header_length = snprintf(chunk_header, sizeof chunk_header, "%x\r\n", (i32)data_length);
        pinta_web_connection_push(connection, chunk_header, header_length);
        pinta_web_connection_push(connection, data_buffer, (i32)data_length);
        return pinta_web_connection_push(connection, "\r\n", 2);
    }
    else return pinta_web_connection_push(connection, data_buffer, (i32)data_length);
}

static i32 pinta_web_printf(struct PintaWebContext* context, const char* format, ...)
{
    i32 length;
    char buffer[1024];
    va_list args;
    va_start(args, format);
    length = vsnprintf(buffer, sizeof buffer, format, args);
    va_end(args);
    return pinta_web_context_response_write(context, buffer, (u32)length);
}

/* ---------------------------------------------------------------
*                          RESPONSE
* ---------------------------------------------------------------*/
#define PINTA_WEB_STATUS_MAP(STATUS)\
    STATUS(100, "Continue")\
    STATUS(101, "Switching Protocols")\
    STATUS(200, "OK")\
    STATUS(201, "Created")\
    STATUS(202, "Accepted")\
    STATUS(203, "Non-Authoritative Information")\
    STATUS(204, "No Content")\
    STATUS(205, "Reset Content")\
    STATUS(206, "Partial Content")\
    STATUS(300, "Multiple Choices")\
    STATUS(301, "Moved Permanently")\
    STATUS(302, "Found")\
    STATUS(303, "See Other")\
    STATUS(304, "Not Modified")\
    STATUS(305, "Use Proxy")\
    STATUS(307, "Temporary Redirect")\
    STATUS(400, "Bad Request")\
    STATUS(401, "Unauthorized")\
    STATUS(402, "Payment Required")\
    STATUS(403, "Forbidden")\
    STATUS(404, "Not Found")\
    STATUS(405, "Method Not Allowed")\
    STATUS(406, "Not Acceptable")\
    STATUS(407, "Proxy Authentication Required")\
    STATUS(408, "Request Time-out")\
    STATUS(409, "Conflict")\
    STATUS(410, "Gone")\
    STATUS(411, "Length Required")\
    STATUS(412, "Precondition Failed")\
    STATUS(413, "Request Entity Too Large")\
    STATUS(414, "Request-URI Too Large")\
    STATUS(415, "Unsupported Media Type")\
    STATUS(416, "Requested range not satisfiable")\
    STATUS(417, "Expectation Failed")\
    STATUS(500, "Internal Server Error")\
    STATUS(501, "Not Implemented")\
    STATUS(502, "Bad Gateway")\
    STATUS(503, "Service Unavailable")\
    STATUS(504, "Gateway Time-out")\
    STATUS(505, "HTTP Version not supported")

static const short pinta_web_status_code[] = {
#define PINTA_STATUS(id, name) id,
    PINTA_WEB_STATUS_MAP(PINTA_STATUS)
#undef PINTA_STATUS
};
static const char* pinta_web_status_text[] = {
#define PINTA_STATUS(id, name) name,
    PINTA_WEB_STATUS_MAP(PINTA_STATUS)
#undef PINTA_STATUS
};

static const char* pinta_web_get_response_status_text(i32 status_code)
{
    i32 i;
    for (i = 0; i < (i32)PINTA_LENGTH(pinta_web_status_code); ++i)
    {
        if (pinta_web_status_code[i] == status_code)
            return pinta_web_status_text[i];
    }
    return "Unknown";
}

i32 pinta_web_context_response_begin(struct PintaWebContext *context, i32 status_code, i32 content_length, const struct PintaWebHeader *headers, i32 header_count)
{
    i32 i = 0;
    struct PintaWebConnection *connection = (struct PintaWebConnection *)context;
    if (connection->body_bytes_read < (i32)connection->context.request.content_length)
    {
        i32 body_left = connection->context.request.content_length - (i32)connection->body_bytes_read;
        if (pinta_web_connection_discard_incoming_data(context, body_left) != PINTA_WEB_OK)
        {
            connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
            return -1;
        }
    }

    pinta_web_printf(context, "HTTP/1.1 %d %s\r\n", status_code, pinta_web_get_response_status_text(status_code));
    if (content_length >= 0)
        pinta_web_printf(context, "Content-Length: %d\r\n", content_length);
    else pinta_web_printf(context, "Transfer-Encoding: chunked\r\n");
    pinta_web_printf(context, "Server: pinta web\r\n");

    for (i = 0; i < header_count; ++i)
    {
        if (!strcasecmp(headers[i].name, "Connection"))
        {
            if (!strcasecmp(headers[i].value, "close"))
                connection->flags |= PINTA_WEB_CONNECTION_FLAG_CLOSE_AFTER_RESPONSE;
        }
        pinta_web_printf(context, "%s: %s\r\n", headers[i].name, headers[i].value);
    }

    if (!(connection->flags & PINTA_WEB_CONNECTION_FLAG_CLOSE_AFTER_RESPONSE))
    {
        /* See if the client wants us to close the connection. */
        const char* connection_header = pinta_web_context_find_header(context, "Connection");
        if (connection_header && !strcasecmp("close", connection_header))
        {
            connection->flags |= PINTA_WEB_CONNECTION_FLAG_CLOSE_AFTER_RESPONSE;
            pinta_web_printf(context, "Connection: close\r\n");
        }
    }
    pinta_web_printf(context, "\r\n");
    if (content_length < 0)
        connection->flags |= PINTA_WEB_CONNECTION_FLAG_CHUNKED_RESPONSE;
    return 0;
}

void pinta_web_context_response_end(struct PintaWebContext *context)
{
    struct PintaWebConnection *connection = (struct PintaWebConnection*) context;
    if (connection->flags & PINTA_WEB_CONNECTION_FLAG_CHUNKED_RESPONSE)
    {
        /* Write final chunk */
        pinta_web_connection_push(connection, "0\r\n\r\n", 5);
        connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_CHUNKED_RESPONSE;
    }
    /* Flush buffers */
    pinta_web_connection_push(connection, "", 0);

    /* Close connection when Content-Length is zero that maybe HTTP/1.0. */
    if (context->request.content_length == 0 && !pinta_web_connection_is_websocket_request(context))
        pinta_web_connection_close(connection);
}

/* ---------------------------------------------------------------
*                          SERVER
* ---------------------------------------------------------------*/
/* Pointer to Integer type conversion for pointer alignment */
#if defined(__PTRDIFF_TYPE__) /* This case should work for GCC*/
# define PINTA_WEB_UINT_TO_PTR(x) ((void*)(__PTRDIFF_TYPE__)(x))
# define PINTA_WEB_PTR_TO_UINT(x) ((u32)(__PTRDIFF_TYPE__)(x))
#elif !defined(__GNUC__) /* works for compilers other than LLVM */
# define PINTA_WEB_UINT_TO_PTR(x) ((void*)&((char*)0)[x])
# define PINTA_WEB_PTR_TO_UINT(x) ((u32)(((char*)x)-(char*)0))
#else /* generates warning but works */
# define PINTA_WEB_UINT_TO_PTR(x) ((void*)(x))
# define PINTA_WEB_PTR_TO_UINT(x) ((u32)(x))
#endif

/* simple pointer math */
#define PINTA_WEB_PTR_ADD(t, p, i) ((t*)((void*)((u8*)(p) + (i))))
#define PINTA_WEB_ALIGN_PTR(x, mask)\
    (PINTA_WEB_UINT_TO_PTR((PINTA_WEB_PTR_TO_UINT((u8*)(x) + (mask-1)) & ~(mask-1))))

/* pointer alignment  */
#ifdef __cplusplus
template<typename T> struct pinta_web_alignof;
template<typename T, i32 size_diff> struct pinta_web_helper { enum { value = size_diff }; };
template<typename T> struct pinta_web_helper<T, 0> { enum { value = pinta_web_alignof<T>::value }; };
template<typename T> struct pinta_web_alignof
{
    struct Big { T x; char c; }; enum
    {
        diff = sizeof(Big) - sizeof(T), value = pinta_web_helper<Big, diff>::value
    };
};
#define PINTA_WEB_ALIGNOF(t) (pinta_web_alignof<t>::value);
#elif defined(_MSC_VER)
#define PINTA_WEB_ALIGNOF(t) __alignof(t)
#else
#define PINTA_WEB_ALIGNOF(t) ((char*)(&((struct {char c; t _h;}*)0)->_h) - (char*)0)
#endif

void pinta_web_server_init(struct PintaWebServer *server, const struct PintaWebConfiguration *configuration, u32 *needed_memory)
{
    static const u32 pinta_web_connection_align = PINTA_WEB_ALIGNOF(struct PintaWebConnection);
    pinta_assert(server);
    pinta_assert(configuration);
    pinta_assert(needed_memory);

    memset(server, 0, sizeof(*server));
    server->configuration = *configuration;
    pinta_assert(configuration->on_request);

    *needed_memory = 0;
    *needed_memory += configuration->connection_max * sizeof(struct PintaWebConnection);
    *needed_memory += configuration->connection_max * configuration->request_buffer_length;
    *needed_memory += configuration->connection_max * configuration->io_buffer_length;
    *needed_memory += pinta_web_connection_align;
    server->memory_length = *needed_memory;
}

i32 pinta_web_server_start(struct PintaWebServer *server, void *memory)
{
    u32 i;
    PintaWebSocket server_socket;
    PintaWebSocketOptions on = 1;
    u8 *buffer = (u8*)memory;
    struct sockaddr_in bind_address;
    static const u32 pinta_web_connection_align = PINTA_WEB_ALIGNOF(struct PintaWebConnection);

    pinta_assert(server);
    pinta_assert(memory);
    memset(buffer, 0, server->memory_length);

    /* setup sever memory */
    server->socket = (PintaWebSocket)PINTA_WEB_INVALID_SOCKET;
    server->connections = (struct PintaWebConnection*)PINTA_WEB_ALIGN_PTR(buffer, pinta_web_connection_align);

    buffer += ((u8*)server->connections - buffer);
    buffer += server->configuration.connection_max * sizeof(struct PintaWebConnection);

    for (i = 0; i < server->configuration.connection_max; ++i)
    {
        server->connections[i].log = server->configuration.log;
        server->connections[i].header_buffer.data = buffer;
        buffer += server->configuration.request_buffer_length;
        server->connections[i].io_buffer.data = buffer;
        server->connections[i].request_buffer_length = server->configuration.request_buffer_length;
        server->connections[i].io_buffer_length = server->configuration.io_buffer_length;
        buffer += server->configuration.io_buffer_length;
    }
    pinta_assert((u32)(buffer - (u8*)memory) <= server->memory_length);

    /* server socket setup */
    server_socket = (PintaWebSocket)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef _WIN32
    if (server_socket == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED)
    {
        /* Make sure WSAStartup has been called. */
        pinta_web_log(server->configuration.log, "Calling WSAStartup.");
        WSADATA wsaData;
        i32 wsaReturn;
        wsaReturn = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaReturn != 0)
        {
            pinta_web_log(server->configuration.log, "WSAStartup failed, error = %d", wsaReturn);
            goto error;
        }
        server_socket = (PintaWebSocket)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        server->windows_socket_initialized = 1;
    }
#endif
    pinta_web_log(server->configuration.log, "Server socket = %d", (i32)server_socket);
    if (!pinta_socket_is_valid(server_socket))
    {
        pinta_web_log(server->configuration.log, "failed to initialized server socket: %d", pinta_socket_get_error());
        goto error;
    }

    setsockopt(PINTA_SOCKET(server_socket), SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
#ifdef __APPLE__ /* Don't generate SIGPIPE when writing to dead socket, we check all writes. */
    signal(SIGPIPE, SIG_IGN);
#endif
    if (pinta_socket_set_blocking(server_socket, 0) != PINTA_WEB_OK) goto error;

    /* bind server socket */
    pinta_web_log(server->configuration.log, "binding to %s:%d", server->configuration.address, server->configuration.port);
    memset(&bind_address, 0, sizeof(bind_address));
    bind_address.sin_family = AF_INET;
    bind_address.sin_port = htons((u16)server->configuration.port);
    bind_address.sin_addr.s_addr = inet_addr(server->configuration.address);
    if (bind(server_socket, (struct sockaddr*) &bind_address, sizeof(bind_address)) != PINTA_WEB_OK)
    {
        pinta_web_log(server->configuration.log, "bind() failed: %d", pinta_socket_get_error());
        pinta_web_log(server->configuration.log, "bind() failed: %s", strerror(pinta_socket_get_error()));
        goto error;
    }

    /* set server socket to listening */
    if (listen(server_socket, SOMAXCONN) != PINTA_WEB_OK)
    {
        pinta_web_log(server->configuration.log, "listen() failed: %d", pinta_socket_get_error());
        pinta_socket_close(PINTA_SOCKET(server_socket));
        goto error;
    }
    server->socket = (PintaWebSocket)server_socket;
    pinta_web_log(server->configuration.log, "server initialized: %s", strerror(errno));
    return 0;

error:
    if (pinta_socket_is_valid(PINTA_SOCKET(server_socket)))
        pinta_socket_close(PINTA_SOCKET(server_socket));
    return -1;
}

void pinta_web_server_stop(struct PintaWebServer *server)
{
#ifdef _WIN32
    if (server->windows_socket_initialized)
    {
        WSACleanup();
    }
#endif
    u32 i;
    pinta_socket_close(PINTA_SOCKET(server->socket));
    for (i = 0; i < server->connection_count; ++i)
        pinta_socket_close(PINTA_SOCKET(server->connections[i].socket));
}

static i32 pinta_web_server_on_incoming(struct PintaWebServer *server)
{
    u32 connection_index;
    char PINTA_WEB_ALIGN(8) client_addr[64];
    struct PintaWebConnection* connection;
    PintaWebSocketLength client_addr_length = sizeof(client_addr);
    PintaWebSocket client_socket;

    /* Make sure we have space for a new connection */
    connection_index = server->connection_count;
    if (connection_index == server->configuration.connection_max)
    {
        pinta_web_log(server->configuration.log, "out of connection slots");
        return 1;
    }

    /* Accept the incoming connection. */
    client_socket = accept(PINTA_SOCKET(server->socket), (struct sockaddr*)&client_addr[0], &client_addr_length);
    if (!pinta_socket_is_valid(client_socket))
    {
        i32 err = pinta_socket_get_error();
        if (!pinta_socket_is_blocking_error(err))
            pinta_web_log(server->configuration.log, "accept() failed: %d", err);
        return 1;
    }

    connection = &server->connections[connection_index];
    pinta_web_connection_reset(connection, server->configuration.request_buffer_length, server->configuration.io_buffer_length);
    connection->flags = PINTA_WEB_CONNECTION_FLAG_FRESH_CONNECTION;
    server->connection_count = connection_index + 1;

    /* Configure socket */
    if (pinta_socket_config_incoming(client_socket) != PINTA_WEB_OK)
    {
        pinta_socket_close(client_socket);
        return 1;
    }

    /* OK, keep this connection */
    pinta_web_log(server->configuration.log, "tagging connection %d as alive", connection_index);
    connection->flags |= PINTA_WEB_CONNECTION_FLAG_ALIVE;
    connection->socket = (PintaWebSocket)client_socket;
    return 0;
}

static void pinta_web_server_update_connection(struct PintaWebServer *server, struct PintaWebConnection* connection)
{
    /* This is no longer a fresh connection. Only read from it when select() says
    * so in the future. */
    connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_FRESH_CONNECTION;
    for (;;)
    {
        switch (connection->state)
        {
        default: break;
        case PINTA_WEB_CONNECTION_STATE_REQUEST: {
            const char *expect_header;
            i32 request_length;
            i32 result = pinta_socket_receive(PINTA_SOCKET(connection->socket),
                &connection->header_buffer, server->configuration.log);
            if (PINTA_WEB_FILL_ERROR == result)
            {
                connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
                return;
            }

            /* Scan to see if the buffer has a complete HTTP request header package. */
            request_length = pinta_web_peek_request_length(connection->header_buffer.data,
                (i32)connection->header_buffer.used);
            if (request_length < 0)
            {
                /* Nothing yet. */
                if (connection->header_buffer.capacity == connection->header_buffer.used)
                {
                    pinta_web_log(server->configuration.log, "giving up as buffer is full");
                    /* Give up, we can't fit the request in our buffer. */
                    connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
                }
                return;
            }
            pinta_web_log(server->configuration.log, "peek request size: %d", request_length);


            /* Set up request data. */
            if (pinta_web_connection_setup_request(connection, request_length) != PINTA_WEB_OK)
            {
                pinta_web_log(server->configuration.log, "failed to set up request");
                connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
                return;
            }

            /* Remember how much of the remaining buffer is body data. */
            connection->header_body_left = (i32)connection->header_buffer.used - request_length;
            /* If the client expects a 100 Continue, send one now. */
            if (NULL != (expect_header = pinta_web_context_find_header(&connection->context, "Expect")))
            {
                if (!strcasecmp(expect_header, "100-continue"))
                {
                    pinta_web_log(server->configuration.log, "connection expects a 100 Continue header.. making him happy");
                    connection->continue_data_left = (i32)pinta_web_continue_header_length;
                    connection->state = PINTA_WEB_CONNECTION_STATE_SEND_CONTINUE;
                }
                else
                {
                    pinta_web_log(server->configuration.log, "unrecognized Expected header %s", expect_header);
                    connection->state = PINTA_WEB_CONNECTION_STATE_SERVE;
                }
            }
            else connection->state = PINTA_WEB_CONNECTION_STATE_SERVE;
        } break; /* PINTA_WEB_REQUEST */

        case PINTA_WEB_CONNECTION_STATE_SEND_CONTINUE: {
            i32 left = connection->continue_data_left;
            i32 offset = (i32)pinta_web_continue_header_length - left;
            long written = 0;

            written = send(PINTA_SOCKET(connection->socket), pinta_web_continue_header + offset, (u32)left, 0);
            pinta_web_log(server->configuration.log, "continue write: %d bytes", written);
            if (written < 0)
            {
                pinta_web_log(server->configuration.log, "failed to write 100-continue header");
                connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
                return;
            }
            left -= (i32)written;
            connection->continue_data_left = left;
            if (left == 0)
                connection->state = PINTA_WEB_CONNECTION_STATE_SERVE;
        } break; /* PINTA_WEB_SEND_CONTINUE */

        case PINTA_WEB_CONNECTION_STATE_SERVE: {
            /* Clear I/O buffer for output */
            connection->io_buffer.used = 0;
            /* Switch socket to blocking mode. */
            if (pinta_web_connection_set_blocking(connection) != PINTA_WEB_OK)
                return;

            /* Figure out if this is a request to upgrade to WebSockets */
            if (pinta_web_connection_is_websocket_request(&connection->context))
            {
                pinta_web_log(server->configuration.log, "received a websocket upgrade request");
                if (!server->configuration.on_ws_connect ||
                    server->configuration.on_ws_connect(&connection->context, server->configuration.user_data) != PINTA_WEB_OK)
                {
                    pinta_web_log(server->configuration.log, "user callback failed connection attempt");
                    pinta_web_context_response_begin(&connection->context, 400, -1,
                        pinta_web_plain_text_headers, PINTA_LENGTH(pinta_web_plain_text_headers));
                    pinta_web_printf(&connection->context, "WebSockets not supported at %s\r\n",
                        connection->context.request.uri);
                    pinta_web_context_response_end(&connection->context);
                }
                else
                {
                    /* OK, let's try to upgrade the connection to WebSockets */
                    if (pinta_web_connection_send_websocket_upgrade(connection) != PINTA_WEB_OK)
                    {
                        pinta_web_log(server->configuration.log, "websocket upgrade failed");
                        pinta_web_context_response_begin(&connection->context, 400, -1,
                            pinta_web_plain_text_headers, PINTA_LENGTH(pinta_web_plain_text_headers));
                        pinta_web_printf(&connection->context, "WebSockets couldn't not be enabled\r\n");
                        pinta_web_context_response_end(&connection->context);
                    }
                    else
                    {
                        /* OK, we're now a websocket */
                        connection->flags |= PINTA_WEB_CONNECTION_FLAG_WEBSOCKET;
                        pinta_web_log(server->configuration.log, "connection %d upgraded to websocket",
                            (i32)(connection - server->connections));
                        server->configuration.on_ws_connected(&connection->context, server->configuration.user_data);
                    }
                }
            }
            else if (server->configuration.on_request(&connection->context, server->configuration.user_data) != 0)
            {
                static const struct PintaWebHeader headers[] = { { "Content-Type", "text/plain" } };
                pinta_web_context_response_begin(&connection->context, 404, -1, headers, PINTA_LENGTH(headers));
                pinta_web_printf(&connection->context, "No handler for %s\r\n",
                    connection->context.request.uri);
                pinta_web_context_response_end(&connection->context);
            }

            /* Back to non-blocking mode, can make the socket die. */
            pinta_web_connection_set_nonblocking(connection);
            /* Ready for another request, unless we should close the connection. */
            if (connection->flags & PINTA_WEB_CONNECTION_FLAG_ALIVE)
            {
                if (connection->flags & PINTA_WEB_CONNECTION_FLAG_CLOSE_AFTER_RESPONSE)
                {
                    connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
                    return;
                }
                else
                {
                    /* Reset connection for next request. */
                    pinta_web_connection_reset(connection, server->configuration.request_buffer_length, server->configuration.io_buffer_length);
                    if (!(connection->flags & PINTA_WEB_CONNECTION_FLAG_WEBSOCKET))
                    {
                        /* Loop back to request state */
                        connection->state = PINTA_WEB_CONNECTION_STATE_REQUEST;
                    }
                    else
                    {
                        /* Clear I/O buffer for input */
                        connection->io_buffer.used = 0;
                        /* Go to the web socket serving state */
                        connection->state = PINTA_WEB_CONNECTION_STATE_WEBSOCKET;
                    }
                }
            }
        } break; /* PINTA_WEB_SERVE */

        case PINTA_WEB_CONNECTION_STATE_WEBSOCKET: {
            /* In this state, we're trying to read a websocket frame into the I/O
            * buffer. Once we have enough data, we call the websocket frame
            * callback and let the client read the data through WebbyRead. */
            if (PINTA_WEB_FILL_ERROR == pinta_socket_receive(PINTA_SOCKET(connection->socket),
                &connection->io_buffer, server->configuration.log))
            {
                /* Give up on this connection */
                connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
                return;
            }

            if (pinta_web_scan_websocket_frame(&connection->on_ws_frame, &connection->io_buffer) != PINTA_WEB_OK)
                return;

            connection->body_bytes_read = 0;
            connection->io_data_left = (i32)connection->io_buffer.used - connection->on_ws_frame.header_length;
            pinta_web_log(server->configuration.log, "%d bytes of incoming websocket data buffered",
                (i32)connection->io_data_left);

            /* Switch socket to blocking mode */
            if (pinta_web_connection_set_blocking(connection) != PINTA_WEB_OK)
                return;

            switch (connection->on_ws_frame.opcode)
            {
            case PINTA_WEB_WSOP_CLOSE:
                pinta_web_log(server->configuration.log, "received websocket close request");
                connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
                return;

            case PINTA_WEB_WSOP_PING:
                pinta_web_log(server->configuration.log, "received websocket ping request");
                if (pinta_socket_send(PINTA_SOCKET(connection->socket), pinta_web_websocket_pong, sizeof pinta_web_websocket_pong))
                {
                    connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
                    return;
                }
                break;

            default:
                /* Dispatch frame to user handler. */
                if (server->configuration.on_ws_frame(&connection->context, &connection->on_ws_frame, server->configuration.user_data) != PINTA_WEB_OK)
                {
                    connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
                    return;
                }
            }

            /* Discard any data the client didn't read to retain the socket state. */
            if (connection->body_bytes_read < connection->on_ws_frame.payload_length)
            {
                i32 length = connection->on_ws_frame.payload_length - connection->body_bytes_read;
                if (pinta_web_connection_discard_incoming_data(&connection->context, length) != PINTA_WEB_OK)
                {
                    connection->flags &= (u16)~PINTA_WEB_CONNECTION_FLAG_ALIVE;
                    return;
                }
            }

            /* Back to non-blocking mode */
            if (pinta_web_connection_set_nonblocking(connection) != PINTA_WEB_OK)
                return;

            pinta_web_connection_reset_ws(connection, server->configuration.request_buffer_length, server->configuration.io_buffer_length);
            connection->state = PINTA_WEB_CONNECTION_STATE_WEBSOCKET;
        } break; /* PINTA_WEB_WEBSOCKET */
        } /* switch */
    } /* for */
}

void pinta_web_server_update(struct PintaWebServer *server)
{
    i32 err;
    u32 i, count;
    PintaWebSocket max_socket;
    fd_set read_fds, write_fds, except_fds;
    struct timeval timeout;

    /* Build set of sockets to check for events */
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);
    max_socket = 0;

    /* Only accept incoming connections if we have space */
    if (server->connection_count < server->configuration.connection_max)
    {
        FD_SET(server->socket, &read_fds);
        FD_SET(server->socket, &except_fds);
        max_socket = PINTA_SOCKET(server->socket);
    }

    for (i = 0, count = server->connection_count; i < count; ++i)
    {
        PintaWebSocket socket = PINTA_SOCKET(server->connections[i].socket);
        FD_SET(socket, &read_fds);
        FD_SET(socket, &except_fds);
        if (server->connections[i].state == PINTA_WEB_CONNECTION_STATE_SEND_CONTINUE)
            FD_SET(socket, &write_fds);
        if (socket > max_socket)
            max_socket = socket;
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    err = select((i32)(max_socket + 1), &read_fds, &write_fds, &except_fds, &timeout);
    if (err < 0)
    {
        pinta_web_log(server->configuration.log, "failed to select");
        return;
    }

    if (err == 0)
        return;

    /* Handle incoming connections */
    if (FD_ISSET(PINTA_SOCKET(server->socket), &read_fds))
    {
        do
        {
            pinta_web_log(server->configuration.log, "awake on incoming");
            err = pinta_web_server_on_incoming(server);
        } while (err == 0);
    }

    /* Handle incoming connection data */
    for (i = 0, count = server->connection_count; i < count; ++i)
    {
        struct PintaWebConnection *connection = &server->connections[i];
        if (FD_ISSET(PINTA_SOCKET(connection->socket), &read_fds) ||
            FD_ISSET(PINTA_SOCKET(connection->socket), &write_fds) ||
            connection->flags & PINTA_WEB_CONNECTION_FLAG_FRESH_CONNECTION)
        {
            pinta_web_log(server->configuration.log, "reading from connection %d", i);
            pinta_web_server_update_connection(server, connection);
        }
    }

    /* Close stale connections & compact connection array. */
    for (i = 0; i < server->connection_count; )
    {
        struct PintaWebConnection *connection = &server->connections[i];
        if (!(connection->flags & PINTA_WEB_CONNECTION_FLAG_ALIVE))
        {
            u32 remain;
            pinta_web_log(server->configuration.log, "closing connection %d (%08x)", i, connection->flags);
            if (connection->flags & PINTA_WEB_CONNECTION_FLAG_WEBSOCKET)
                server->configuration.on_ws_closed(&connection->context, server->configuration.user_data);
            remain = server->connection_count - (u32)i - 1;
            pinta_web_connection_close(connection);
            memmove(&server->connections[i], &server->connections[i + 1], remain * sizeof(server->connections[i]));
            --server->connection_count;
        }
        else ++i;
    }
}
