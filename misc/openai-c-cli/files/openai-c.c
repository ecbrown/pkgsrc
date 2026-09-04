/*
 * Copyright (c) 2026 The openai-c contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#define PROGRAM_NAME "openai-c"
#define PROGRAM_VERSION "0.1"
#define DEFAULT_MODEL "gpt-5-mini"
#define DEFAULT_BASE_URL "https://api.openai.com/v1"
#define DEFAULT_CA_FILE "@PREFIX@/share/mozilla-rootcerts/cacert.pem"
#define MAX_INPUT_SIZE (16U * 1024U * 1024U)
#define MAX_RESPONSE_SIZE (64U * 1024U * 1024U)

struct buffer {
	char *data;
	size_t len;
	size_t cap;
};

struct endpoint {
	char *host;
	char *port;
	char *host_header;
	char *request_path;
};

enum json_type {
	JSON_UNDEFINED,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_STRING,
	JSON_PRIMITIVE
};

struct json_token {
	enum json_type type;
	int start;
	int end;
	int parent;
};

struct json_tokens {
	struct json_token *v;
	size_t len;
	size_t cap;
};

static void
buffer_init(struct buffer *b)
{
	b->data = NULL;
	b->len = 0;
	b->cap = 0;
}

static void
buffer_free(struct buffer *b)
{
	free(b->data);
	buffer_init(b);
}

static int
buffer_reserve(struct buffer *b, size_t extra)
{
	size_t needed, cap;
	char *p;

	if (extra > (size_t)-1 - b->len - 1)
		return -1;
	needed = b->len + extra + 1;
	if (needed <= b->cap)
		return 0;
	cap = b->cap ? b->cap : 256;
	while (cap < needed) {
		if (cap > (size_t)-1 / 2) {
			cap = needed;
			break;
		}
		cap *= 2;
	}
	p = (char *)realloc(b->data, cap);
	if (p == NULL)
		return -1;
	b->data = p;
	b->cap = cap;
	return 0;
}

static int
buffer_append(struct buffer *b, const void *data, size_t len)
{
	if (buffer_reserve(b, len) != 0)
		return -1;
	if (len != 0)
		memcpy(b->data + b->len, data, len);
	b->len += len;
	b->data[b->len] = '\0';
	return 0;
}

static int
buffer_append_string(struct buffer *b, const char *s)
{
	return buffer_append(b, s, strlen(s));
}

static int
buffer_append_format(struct buffer *b, const char *fmt, ...)
{
	char local[128];
	char *dynamic;
	int n;
	va_list ap;

	va_start(ap, fmt);
	n = vsnprintf(local, sizeof(local), fmt, ap);
	va_end(ap);
	if (n < 0)
		return -1;
	if ((size_t)n < sizeof(local))
		return buffer_append(b, local, (size_t)n);
	dynamic = (char *)malloc((size_t)n + 1);
	if (dynamic == NULL)
		return -1;
	va_start(ap, fmt);
	n = vsnprintf(dynamic, (size_t)n + 1, fmt, ap);
	va_end(ap);
	if (n < 0) {
		free(dynamic);
		return -1;
	}
	if (buffer_append(b, dynamic, (size_t)n) != 0) {
		free(dynamic);
		return -1;
	}
	free(dynamic);
	return 0;
}

static char *
duplicate_range(const char *s, size_t len)
{
	char *p;

	p = (char *)malloc(len + 1);
	if (p == NULL)
		return NULL;
	memcpy(p, s, len);
	p[len] = '\0';
	return p;
}

static int
contains_newline(const char *s)
{
	return strchr(s, '\r') != NULL || strchr(s, '\n') != NULL;
}

static void
endpoint_free(struct endpoint *ep)
{
	free(ep->host);
	free(ep->port);
	free(ep->host_header);
	free(ep->request_path);
	memset(ep, 0, sizeof(*ep));
}

static int
parse_endpoint(const char *url, struct endpoint *ep)
{
	const char *authority, *authority_end, *path, *host_start, *host_end;
	const char *port_start;
	size_t host_len, path_len, base_len;
	int bracketed;
	struct buffer out;

	memset(ep, 0, sizeof(*ep));
	buffer_init(&out);
	if (strncmp(url, "https://", 8) != 0) {
		fprintf(stderr, "%s: OPENAI_BASE_URL must use https://\n",
		    PROGRAM_NAME);
		return -1;
	}
	authority = url + 8;
	authority_end = strchr(authority, '/');
	if (authority_end == NULL)
		authority_end = authority + strlen(authority);
	if (authority == authority_end || memchr(authority, '@',
	    (size_t)(authority_end - authority)) != NULL) {
		fprintf(stderr, "%s: invalid OPENAI_BASE_URL authority\n",
		    PROGRAM_NAME);
		return -1;
	}
	path = *authority_end == '/' ? authority_end : NULL;
	bracketed = *authority == '[';
	port_start = NULL;
	if (bracketed) {
		host_start = authority + 1;
		host_end = memchr(host_start, ']',
		    (size_t)(authority_end - host_start));
		if (host_end == NULL) {
			fprintf(stderr, "%s: malformed IPv6 address in base URL\n",
			    PROGRAM_NAME);
			return -1;
		}
		if (host_end + 1 < authority_end) {
			if (host_end[1] != ':') {
				fprintf(stderr, "%s: invalid base URL authority\n",
				    PROGRAM_NAME);
				return -1;
			}
			port_start = host_end + 2;
		}
	} else {
		const char *colon;

		host_start = authority;
		colon = memchr(authority, ':',
		    (size_t)(authority_end - authority));
		if (colon != NULL) {
			host_end = colon;
			port_start = colon + 1;
		} else {
			host_end = authority_end;
		}
	}
	host_len = (size_t)(host_end - host_start);
	if (host_len == 0 || (port_start != NULL && port_start == authority_end)) {
		fprintf(stderr, "%s: invalid host or port in base URL\n",
		    PROGRAM_NAME);
		return -1;
	}
	ep->host = duplicate_range(host_start, host_len);
	ep->port = port_start != NULL
	    ? duplicate_range(port_start, (size_t)(authority_end - port_start))
	    : duplicate_range("443", 3);
	if (ep->host == NULL || ep->port == NULL || contains_newline(ep->host) ||
	    contains_newline(ep->port))
		goto nomem_or_invalid;

	if (bracketed && buffer_append_string(&out, "[") != 0)
		goto nomem_or_invalid;
	if (buffer_append_string(&out, ep->host) != 0)
		goto nomem_or_invalid;
	if (bracketed && buffer_append_string(&out, "]") != 0)
		goto nomem_or_invalid;
	if (strcmp(ep->port, "443") != 0 &&
	    (buffer_append_string(&out, ":") != 0 ||
	    buffer_append_string(&out, ep->port) != 0))
		goto nomem_or_invalid;
	ep->host_header = out.data;

	buffer_init(&out);
	if (path == NULL) {
		if (buffer_append_string(&out, "/responses") != 0)
			goto nomem_or_invalid;
	} else {
		path_len = strlen(path);
		if (strchr(path, '?') != NULL || strchr(path, '#') != NULL ||
		    contains_newline(path)) {
			fprintf(stderr, "%s: base URL cannot contain a query or fragment\n",
			    PROGRAM_NAME);
			buffer_free(&out);
			endpoint_free(ep);
			return -1;
		}
		base_len = path_len;
		while (base_len > 1 && path[base_len - 1] == '/')
			base_len--;
		if (base_len == 1 && path[0] == '/')
			base_len = 0;
		if (buffer_append(&out, path, base_len) != 0 ||
		    buffer_append_string(&out, "/responses") != 0)
			goto nomem_or_invalid;
	}
	ep->request_path = out.data;
	return 0;

nomem_or_invalid:
	buffer_free(&out);
	endpoint_free(ep);
	fprintf(stderr, "%s: unable to parse base URL\n", PROGRAM_NAME);
	return -1;
}

static int
append_json_string(struct buffer *b, const char *s)
{
	const unsigned char *p;
	char escaped[7];

	if (buffer_append_string(b, "\"") != 0)
		return -1;
	for (p = (const unsigned char *)s; *p != '\0'; p++) {
		switch (*p) {
		case '"':
			if (buffer_append_string(b, "\\\"") != 0)
				return -1;
			break;
		case '\\':
			if (buffer_append_string(b, "\\\\") != 0)
				return -1;
			break;
		case '\b':
			if (buffer_append_string(b, "\\b") != 0)
				return -1;
			break;
		case '\f':
			if (buffer_append_string(b, "\\f") != 0)
				return -1;
			break;
		case '\n':
			if (buffer_append_string(b, "\\n") != 0)
				return -1;
			break;
		case '\r':
			if (buffer_append_string(b, "\\r") != 0)
				return -1;
			break;
		case '\t':
			if (buffer_append_string(b, "\\t") != 0)
				return -1;
			break;
		default:
			if (*p < 0x20) {
				(void)snprintf(escaped, sizeof(escaped), "\\u%04x",
				    (unsigned int)*p);
				if (buffer_append_string(b, escaped) != 0)
					return -1;
			} else if (buffer_append(b, p, 1) != 0) {
				return -1;
			}
		}
	}
	return buffer_append_string(b, "\"");
}

static int
build_json_body(struct buffer *body, const char *model, const char *input,
	const char *instructions)
{
	buffer_init(body);
	if (buffer_append_string(body, "{\"model\":") != 0 ||
	    append_json_string(body, model) != 0 ||
	    buffer_append_string(body, ",\"input\":") != 0 ||
	    append_json_string(body, input) != 0 ||
	    buffer_append_string(body, ",\"store\":false") != 0)
		goto fail;
	if (instructions != NULL &&
	    (buffer_append_string(body, ",\"instructions\":") != 0 ||
	    append_json_string(body, instructions) != 0))
		goto fail;
	if (buffer_append_string(body, "}") != 0)
		goto fail;
	return 0;
fail:
	buffer_free(body);
	return -1;
}

static int
connect_tcp(const char *host, const char *port)
{
	struct addrinfo hints, *result, *ai;
	int fd, error;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	error = getaddrinfo(host, port, &hints, &result);
	if (error != 0) {
		fprintf(stderr, "%s: cannot resolve %s: %s\n", PROGRAM_NAME,
		    host, gai_strerror(error));
		return -1;
	}
	fd = -1;
	for (ai = result; ai != NULL; ai = ai->ai_next) {
		fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (fd < 0)
			continue;
		if (connect(fd, ai->ai_addr, ai->ai_addrlen) == 0)
			break;
		(void)close(fd);
		fd = -1;
	}
	freeaddrinfo(result);
	if (fd < 0)
		fprintf(stderr, "%s: cannot connect to %s:%s: %s\n",
		    PROGRAM_NAME, host, port, strerror(errno));
	return fd;
}

static int
ssl_write_all(SSL *ssl, const char *data, size_t len)
{
	size_t sent;
	int n, error;

	sent = 0;
	while (sent < len) {
		size_t remaining;

		remaining = len - sent;
		if (remaining > INT_MAX)
			remaining = INT_MAX;
		n = SSL_write(ssl, data + sent, (int)remaining);
		if (n > 0) {
			sent += (size_t)n;
			continue;
		}
		error = SSL_get_error(ssl, n);
		if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
			continue;
		fprintf(stderr, "%s: TLS write failed\n", PROGRAM_NAME);
		ERR_print_errors_fp(stderr);
		return -1;
	}
	return 0;
}

static int
ssl_read_all(SSL *ssl, struct buffer *response)
{
	char block[16384];
	int n, error;

	buffer_init(response);
	for (;;) {
		n = SSL_read(ssl, block, sizeof(block));
		if (n > 0) {
			if (response->len + (size_t)n > MAX_RESPONSE_SIZE) {
				fprintf(stderr, "%s: response exceeds %u bytes\n",
				    PROGRAM_NAME, (unsigned int)MAX_RESPONSE_SIZE);
				buffer_free(response);
				return -1;
			}
			if (buffer_append(response, block, (size_t)n) != 0) {
				buffer_free(response);
				return -1;
			}
			continue;
		}
		error = SSL_get_error(ssl, n);
		if (error == SSL_ERROR_ZERO_RETURN)
			break;
		if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
			continue;
		if (error == SSL_ERROR_SYSCALL && n == 0)
			break;
		fprintf(stderr, "%s: TLS read failed\n", PROGRAM_NAME);
		ERR_print_errors_fp(stderr);
		buffer_free(response);
		return -1;
	}
	return 0;
}

static int
make_request(const struct endpoint *ep, const char *api_key,
	const char *organization, const char *project, const char *ca_file,
	const struct buffer *body, struct buffer *response)
{
	SSL_CTX *ctx;
	SSL *ssl;
	X509_VERIFY_PARAM *verify;
	struct buffer request;
	unsigned char address[16];
	int fd, is_ip, rc;

	if (contains_newline(api_key) ||
	    (organization != NULL && contains_newline(organization)) ||
	    (project != NULL && contains_newline(project))) {
		fprintf(stderr, "%s: authentication header contains a newline\n",
		    PROGRAM_NAME);
		return -1;
	}
	fd = connect_tcp(ep->host, ep->port);
	if (fd < 0)
		return -1;
	ctx = NULL;
	ssl = NULL;
	rc = -1;
	if (OPENSSL_init_ssl(0, NULL) != 1) {
		fprintf(stderr, "%s: cannot initialize OpenSSL\n", PROGRAM_NAME);
		goto done;
	}
	ctx = SSL_CTX_new(TLS_client_method());
	if (ctx == NULL) {
		fprintf(stderr, "%s: cannot create TLS context\n", PROGRAM_NAME);
		ERR_print_errors_fp(stderr);
		goto done;
	}
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	if (SSL_CTX_load_verify_locations(ctx, ca_file, NULL) != 1) {
		fprintf(stderr, "%s: cannot load CA certificates from %s\n",
		    PROGRAM_NAME, ca_file);
		ERR_print_errors_fp(stderr);
		goto done;
	}
	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		fprintf(stderr, "%s: cannot create TLS connection\n", PROGRAM_NAME);
		ERR_print_errors_fp(stderr);
		goto done;
	}
	is_ip = inet_pton(AF_INET, ep->host, address) == 1 ||
	    inet_pton(AF_INET6, ep->host, address) == 1;
	verify = SSL_get0_param(ssl);
	if ((is_ip && X509_VERIFY_PARAM_set1_ip_asc(verify, ep->host) != 1) ||
	    (!is_ip &&
	    (X509_VERIFY_PARAM_set_hostflags(verify,
	    X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS),
	    X509_VERIFY_PARAM_set1_host(verify, ep->host, 0) != 1))) {
		fprintf(stderr, "%s: cannot configure TLS hostname checking\n",
		    PROGRAM_NAME);
		goto done;
	}
	if (!is_ip && SSL_set_tlsext_host_name(ssl, ep->host) != 1) {
		fprintf(stderr, "%s: cannot configure TLS server name\n",
		    PROGRAM_NAME);
		ERR_print_errors_fp(stderr);
		goto done;
	}
	if (SSL_set_fd(ssl, fd) != 1 || SSL_connect(ssl) != 1) {
		fprintf(stderr, "%s: TLS connection to %s failed\n",
		    PROGRAM_NAME, ep->host);
		ERR_print_errors_fp(stderr);
		goto done;
	}

	buffer_init(&request);
	if (buffer_append_format(&request, "POST %s HTTP/1.1\r\n",
	    ep->request_path) != 0 ||
	    buffer_append_format(&request, "Host: %s\r\n", ep->host_header) != 0 ||
	    buffer_append_format(&request, "User-Agent: %s/%s\r\n",
	    PROGRAM_NAME, PROGRAM_VERSION) != 0 ||
	    buffer_append_format(&request, "Authorization: Bearer %s\r\n",
	    api_key) != 0 ||
	    (organization != NULL &&
	    buffer_append_format(&request, "OpenAI-Organization: %s\r\n",
	    organization) != 0) ||
	    (project != NULL &&
	    buffer_append_format(&request, "OpenAI-Project: %s\r\n", project) != 0) ||
	    buffer_append_string(&request, "Content-Type: application/json\r\n") != 0 ||
	    buffer_append_string(&request, "Accept: application/json\r\n") != 0 ||
	    buffer_append_string(&request, "Accept-Encoding: identity\r\n") != 0 ||
	    buffer_append_string(&request, "Connection: close\r\n") != 0 ||
	    buffer_append_format(&request, "Content-Length: %lu\r\n\r\n",
	    (unsigned long)body->len) != 0 ||
	    buffer_append(&request, body->data, body->len) != 0) {
		fprintf(stderr, "%s: out of memory while making request\n",
		    PROGRAM_NAME);
		buffer_free(&request);
		goto done;
	}
	if (ssl_write_all(ssl, request.data, request.len) != 0) {
		buffer_free(&request);
		goto done;
	}
	buffer_free(&request);
	if (ssl_read_all(ssl, response) != 0)
		goto done;
	rc = 0;

done:
	if (ssl != NULL) {
		if (rc == 0)
			(void)SSL_shutdown(ssl);
		SSL_free(ssl);
	}
	SSL_CTX_free(ctx);
	(void)close(fd);
	return rc;
}

static const char *
find_bytes(const char *data, size_t len, const char *needle, size_t needle_len)
{
	size_t i;

	if (needle_len == 0 || needle_len > len)
		return NULL;
	for (i = 0; i + needle_len <= len; i++) {
		if (memcmp(data + i, needle, needle_len) == 0)
			return data + i;
	}
	return NULL;
}

static int
case_equal(const char *a, size_t alen, const char *b)
{
	size_t i, blen;

	blen = strlen(b);
	if (alen != blen)
		return 0;
	for (i = 0; i < alen; i++) {
		if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i]))
			return 0;
	}
	return 1;
}

static int
header_value_contains(const char *headers, size_t len, const char *name,
	const char *word)
{
	const char *p, *end, *line_end, *colon, *v;
	size_t word_len, i;

	p = headers;
	end = headers + len;
	word_len = strlen(word);
	while (p < end) {
		line_end = find_bytes(p, (size_t)(end - p), "\r\n", 2);
		if (line_end == NULL)
			line_end = end;
		colon = memchr(p, ':', (size_t)(line_end - p));
		if (colon != NULL && case_equal(p, (size_t)(colon - p), name)) {
			v = colon + 1;
			while (v < line_end && isspace((unsigned char)*v))
				v++;
			for (i = 0; i + word_len <= (size_t)(line_end - v); i++) {
				if (case_equal(v + i, word_len, word))
					return 1;
			}
		}
		p = line_end == end ? end : line_end + 2;
	}
	return 0;
}

static int
parse_content_length(const char *headers, size_t len, size_t *value)
{
	const char *p, *end, *line_end, *colon, *v;
	unsigned long n;
	char *number_end;

	p = headers;
	end = headers + len;
	while (p < end) {
		line_end = find_bytes(p, (size_t)(end - p), "\r\n", 2);
		if (line_end == NULL)
			line_end = end;
		colon = memchr(p, ':', (size_t)(line_end - p));
		if (colon != NULL && case_equal(p, (size_t)(colon - p),
		    "Content-Length")) {
			v = colon + 1;
			while (v < line_end && isspace((unsigned char)*v))
				v++;
			errno = 0;
			n = strtoul(v, &number_end, 10);
			if (errno != 0 || number_end == v || number_end > line_end ||
			    n > (unsigned long)MAX_RESPONSE_SIZE)
				return -1;
			*value = (size_t)n;
			return 1;
		}
		p = line_end == end ? end : line_end + 2;
	}
	return 0;
}

static int
hex_digit(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int
decode_chunked(const char *input, size_t len, struct buffer *body)
{
	const char *p, *end, *line_end, *q;
	size_t chunk;
	int digit, saw_digit;

	buffer_init(body);
	p = input;
	end = input + len;
	for (;;) {
		line_end = find_bytes(p, (size_t)(end - p), "\r\n", 2);
		if (line_end == NULL)
			goto malformed;
		chunk = 0;
		saw_digit = 0;
		for (q = p; q < line_end && *q != ';'; q++) {
			if (isspace((unsigned char)*q))
				continue;
			digit = hex_digit((unsigned char)*q);
			if (digit < 0 || chunk > ((size_t)-1 - (size_t)digit) / 16)
				goto malformed;
			chunk = chunk * 16 + (size_t)digit;
			saw_digit = 1;
		}
		if (!saw_digit)
			goto malformed;
		p = line_end + 2;
		if (chunk == 0)
			return 0;
		if (chunk > (size_t)(end - p) || chunk > MAX_RESPONSE_SIZE - body->len)
			goto malformed;
		if (buffer_append(body, p, chunk) != 0)
			goto malformed;
		p += chunk;
		if (end - p < 2 || p[0] != '\r' || p[1] != '\n')
			goto malformed;
		p += 2;
	}

malformed:
	buffer_free(body);
	fprintf(stderr, "%s: malformed chunked HTTP response\n", PROGRAM_NAME);
	return -1;
}

static int
parse_http_response(const struct buffer *response, int *status,
	struct buffer *body)
{
	const char *separator, *body_start;
	size_t header_len, body_len, content_length;
	int has_content_length;

	separator = find_bytes(response->data, response->len, "\r\n\r\n", 4);
	if (separator == NULL || sscanf(response->data, "HTTP/%*u.%*u %d", status) != 1) {
		fprintf(stderr, "%s: malformed HTTP response\n", PROGRAM_NAME);
		return -1;
	}
	header_len = (size_t)(separator - response->data);
	body_start = separator + 4;
	body_len = response->len - (size_t)(body_start - response->data);
	if (header_value_contains(response->data, header_len,
	    "Transfer-Encoding", "chunked"))
		return decode_chunked(body_start, body_len, body);
	has_content_length = parse_content_length(response->data, header_len,
	    &content_length);
	if (has_content_length < 0 ||
	    (has_content_length > 0 && content_length > body_len)) {
		fprintf(stderr, "%s: invalid HTTP content length\n", PROGRAM_NAME);
		return -1;
	}
	if (has_content_length > 0)
		body_len = content_length;
	buffer_init(body);
	if (buffer_append(body, body_start, body_len) != 0) {
		fprintf(stderr, "%s: out of memory while reading response\n",
		    PROGRAM_NAME);
		return -1;
	}
	return 0;
}

static void
json_tokens_free(struct json_tokens *tokens)
{
	free(tokens->v);
	tokens->v = NULL;
	tokens->len = 0;
	tokens->cap = 0;
}

static int
json_add_token(struct json_tokens *tokens, enum json_type type, int start,
	int parent)
{
	struct json_token *p;
	size_t cap;

	if (tokens->len == tokens->cap) {
		cap = tokens->cap ? tokens->cap * 2 : 256;
		p = (struct json_token *)realloc(tokens->v,
		    cap * sizeof(*tokens->v));
		if (p == NULL)
			return -1;
		tokens->v = p;
		tokens->cap = cap;
	}
	tokens->v[tokens->len].type = type;
	tokens->v[tokens->len].start = start;
	tokens->v[tokens->len].end = -1;
	tokens->v[tokens->len].parent = parent;
	return (int)tokens->len++;
}

static int
json_tokenize(const char *json, size_t len, struct json_tokens *tokens)
{
	int *stack;
	size_t stack_len, stack_cap, i, start;
	int parent, index, digit;
	char c, expected;

	memset(tokens, 0, sizeof(*tokens));
	stack = NULL;
	stack_len = 0;
	stack_cap = 0;
	if (len > INT_MAX)
		goto invalid;
	for (i = 0; i < len; i++) {
		c = json[i];
		if (isspace((unsigned char)c) || c == ':' || c == ',')
			continue;
		parent = stack_len ? stack[stack_len - 1] : -1;
		if (c == '{' || c == '[') {
			index = json_add_token(tokens,
			    c == '{' ? JSON_OBJECT : JSON_ARRAY, (int)i, parent);
			if (index < 0)
				goto invalid;
			if (stack_len == stack_cap) {
				int *new_stack;
				size_t new_cap;

				new_cap = stack_cap ? stack_cap * 2 : 32;
				new_stack = (int *)realloc(stack,
				    new_cap * sizeof(*stack));
				if (new_stack == NULL)
					goto invalid;
				stack = new_stack;
				stack_cap = new_cap;
			}
			stack[stack_len++] = index;
			continue;
		}
		if (c == '}' || c == ']') {
			if (stack_len == 0)
				goto invalid;
			index = stack[stack_len - 1];
			expected = tokens->v[index].type == JSON_OBJECT ? '}' : ']';
			if (c != expected)
				goto invalid;
			tokens->v[index].end = (int)i + 1;
			stack_len--;
			continue;
		}
		if (c == '"') {
			start = ++i;
			for (; i < len; i++) {
				c = json[i];
				if ((unsigned char)c < 0x20)
					goto invalid;
				if (c == '"')
					break;
				if (c != '\\')
					continue;
				if (++i >= len || strchr("\"\\/bfnrtu", json[i]) == NULL)
					goto invalid;
				if (json[i] == 'u') {
					size_t j;

					if (i + 4 >= len)
						goto invalid;
					for (j = 1; j <= 4; j++) {
						digit = hex_digit((unsigned char)json[i + j]);
						if (digit < 0)
							goto invalid;
					}
					i += 4;
				}
			}
			if (i >= len)
				goto invalid;
			index = json_add_token(tokens, JSON_STRING, (int)start, parent);
			if (index < 0)
				goto invalid;
			tokens->v[index].end = (int)i;
			continue;
		}
		start = i;
		while (i < len && !isspace((unsigned char)json[i]) &&
		    json[i] != ',' && json[i] != ']' && json[i] != '}')
			i++;
		if (i == start)
			goto invalid;
		index = json_add_token(tokens, JSON_PRIMITIVE, (int)start, parent);
		if (index < 0)
			goto invalid;
		tokens->v[index].end = (int)i;
		i--;
	}
	if (stack_len != 0 || tokens->len == 0)
		goto invalid;
	free(stack);
	return 0;

invalid:
	free(stack);
	json_tokens_free(tokens);
	return -1;
}

static int
json_raw_string_equal(const char *json, const struct json_token *token,
	const char *s)
{
	size_t len;

	if (token->type != JSON_STRING)
		return 0;
	len = (size_t)(token->end - token->start);
	return len == strlen(s) && memcmp(json + token->start, s, len) == 0;
}

static int
json_object_get(const char *json, const struct json_tokens *tokens,
	int object, const char *key)
{
	size_t i;
	int key_token;

	if (object < 0 || (size_t)object >= tokens->len ||
	    tokens->v[object].type != JSON_OBJECT)
		return -1;
	key_token = -1;
	for (i = (size_t)object + 1; i < tokens->len &&
	    tokens->v[i].start < tokens->v[object].end; i++) {
		if (tokens->v[i].parent != object)
			continue;
		if (key_token < 0) {
			if (tokens->v[i].type != JSON_STRING)
				return -1;
			key_token = (int)i;
		} else {
			if (json_raw_string_equal(json, &tokens->v[key_token], key))
				return (int)i;
			key_token = -1;
		}
	}
	return -1;
}

static int
append_utf8(struct buffer *out, unsigned long cp)
{
	unsigned char bytes[4];
	size_t len;

	if (cp <= 0x7f) {
		bytes[0] = (unsigned char)cp;
		len = 1;
	} else if (cp <= 0x7ff) {
		bytes[0] = (unsigned char)(0xc0 | (cp >> 6));
		bytes[1] = (unsigned char)(0x80 | (cp & 0x3f));
		len = 2;
	} else if (cp <= 0xffff) {
		bytes[0] = (unsigned char)(0xe0 | (cp >> 12));
		bytes[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3f));
		bytes[2] = (unsigned char)(0x80 | (cp & 0x3f));
		len = 3;
	} else if (cp <= 0x10ffff) {
		bytes[0] = (unsigned char)(0xf0 | (cp >> 18));
		bytes[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3f));
		bytes[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3f));
		bytes[3] = (unsigned char)(0x80 | (cp & 0x3f));
		len = 4;
	} else {
		return -1;
	}
	return buffer_append(out, bytes, len);
}

static int
read_hex4(const char *p, unsigned long *value)
{
	int i, digit;
	unsigned long v;

	v = 0;
	for (i = 0; i < 4; i++) {
		digit = hex_digit((unsigned char)p[i]);
		if (digit < 0)
			return -1;
		v = v * 16 + (unsigned long)digit;
	}
	*value = v;
	return 0;
}

static int
json_decode_string(const char *json, const struct json_token *token,
	struct buffer *out)
{
	const char *p, *end;
	unsigned long cp, low;
	char c;

	if (token->type != JSON_STRING)
		return -1;
	buffer_init(out);
	p = json + token->start;
	end = json + token->end;
	while (p < end) {
		if (*p != '\\') {
			if (buffer_append(out, p, 1) != 0)
				goto fail;
			p++;
			continue;
		}
		if (++p >= end)
			goto fail;
		c = *p++;
		switch (c) {
		case '"': case '\\': case '/':
			if (buffer_append(out, &c, 1) != 0)
				goto fail;
			break;
		case 'b': c = '\b'; goto one_character;
		case 'f': c = '\f'; goto one_character;
		case 'n': c = '\n'; goto one_character;
		case 'r': c = '\r'; goto one_character;
		case 't': c = '\t';
one_character:
			if (buffer_append(out, &c, 1) != 0)
				goto fail;
			break;
		case 'u':
			if (end - p < 4 || read_hex4(p, &cp) != 0)
				goto fail;
			p += 4;
			if (cp >= 0xd800 && cp <= 0xdbff) {
				if (end - p < 6 || p[0] != '\\' || p[1] != 'u' ||
				    read_hex4(p + 2, &low) != 0 ||
				    low < 0xdc00 || low > 0xdfff)
					goto fail;
				cp = 0x10000 + ((cp - 0xd800) << 10) +
				    (low - 0xdc00);
				p += 6;
			} else if (cp >= 0xdc00 && cp <= 0xdfff) {
				goto fail;
			}
			if (append_utf8(out, cp) != 0)
				goto fail;
			break;
		default:
			goto fail;
		}
	}
	return 0;
fail:
	buffer_free(out);
	return -1;
}

static int
print_decoded_token(const char *json, const struct json_token *token)
{
	struct buffer text;

	if (json_decode_string(json, token, &text) != 0)
		return -1;
	if (text.len != 0)
		(void)fwrite(text.data, 1, text.len, stdout);
	if (text.len == 0 || text.data[text.len - 1] != '\n')
		(void)putchar('\n');
	buffer_free(&text);
	return 0;
}

static int
print_response_text(const char *json, size_t len)
{
	struct json_tokens tokens;
	size_t i;
	int type, text, found, root_text;

	if (json_tokenize(json, len, &tokens) != 0)
		return -1;
	found = 0;
	for (i = 0; i < tokens.len; i++) {
		if (tokens.v[i].type != JSON_OBJECT)
			continue;
		type = json_object_get(json, &tokens, (int)i, "type");
		text = json_object_get(json, &tokens, (int)i, "text");
		if (type >= 0 && text >= 0 &&
		    json_raw_string_equal(json, &tokens.v[type], "output_text") &&
		    tokens.v[text].type == JSON_STRING) {
			if (print_decoded_token(json, &tokens.v[text]) != 0) {
				json_tokens_free(&tokens);
				return -1;
			}
			found++;
		}
	}
	if (found == 0 && tokens.v[0].type == JSON_OBJECT) {
		root_text = json_object_get(json, &tokens, 0, "output_text");
		if (root_text >= 0 && tokens.v[root_text].type == JSON_STRING) {
			if (print_decoded_token(json, &tokens.v[root_text]) != 0) {
				json_tokens_free(&tokens);
				return -1;
			}
			found = 1;
		}
	}
	json_tokens_free(&tokens);
	return found;
}

static char *
extract_error_message(const char *json, size_t len)
{
	struct json_tokens tokens;
	struct buffer decoded;
	int error, message;
	char *result;

	if (json_tokenize(json, len, &tokens) != 0 ||
	    tokens.v[0].type != JSON_OBJECT)
		return NULL;
	error = json_object_get(json, &tokens, 0, "error");
	message = error >= 0 && tokens.v[error].type == JSON_OBJECT
	    ? json_object_get(json, &tokens, error, "message") : -1;
	if (message < 0 || tokens.v[message].type != JSON_STRING ||
	    json_decode_string(json, &tokens.v[message], &decoded) != 0) {
		json_tokens_free(&tokens);
		return NULL;
	}
	result = decoded.data;
	json_tokens_free(&tokens);
	return result;
}

static int
read_stream(FILE *stream, struct buffer *input, size_t limit)
{
	char block[8192];
	size_t n;

	buffer_init(input);
	errno = 0;
	while ((n = fread(block, 1, sizeof(block), stream)) != 0) {
		if (input->len + n > limit || buffer_append(input, block, n) != 0) {
			buffer_free(input);
			return -1;
		}
	}
	if (ferror(stream)) {
#ifdef __VMS
		/* GNV stream pipes set both EOF and error at normal mailbox EOF. */
		if (feof(stream)) {
			clearerr(stream);
		} else
#endif
		{
		buffer_free(input);
		return -1;
		}
	}
	if (input->data == NULL && buffer_append(input, "", 0) != 0)
		return -1;
	return 0;
}

static int
join_arguments(int argc, char **argv, int first, struct buffer *input)
{
	int i;

	buffer_init(input);
	for (i = first; i < argc; i++) {
		if (i != first && buffer_append_string(input, " ") != 0)
			goto fail;
		if (buffer_append_string(input, argv[i]) != 0 ||
		    input->len > MAX_INPUT_SIZE)
			goto fail;
	}
	if (input->data == NULL && buffer_append(input, "", 0) != 0)
		goto fail;
	return 0;
fail:
	buffer_free(input);
	return -1;
}

static void
usage(FILE *stream)
{
	fprintf(stream,
	    "usage: %s [-m model] [-s instructions] [--base-url url] [--ca-file file] [--raw] [prompt ...]\n"
	    "       %s --extract < response.json\n",
	    PROGRAM_NAME, PROGRAM_NAME);
}

static const char *
option_value(int argc, char **argv, int *index, const char *option)
{
	if (*index + 1 >= argc) {
		fprintf(stderr, "%s: %s requires an argument\n",
		    PROGRAM_NAME, option);
		return NULL;
	}
	(*index)++;
	return argv[*index];
}

int
main(int argc, char **argv)
{
	const char *model, *instructions, *base_url, *ca_file, *api_key;
	const char *organization, *project, *value;
	struct endpoint endpoint;
	struct buffer input, json_body, raw_response, response_body;
	char *error_message;
	int i, raw, extract, status, printed, rc;

	model = getenv("OPENAI_MODEL");
	if (model == NULL || *model == '\0')
		model = DEFAULT_MODEL;
	instructions = NULL;
	base_url = getenv("OPENAI_BASE_URL");
	if (base_url == NULL || *base_url == '\0')
		base_url = DEFAULT_BASE_URL;
	ca_file = getenv("SSL_CERT_FILE");
	if (ca_file == NULL || *ca_file == '\0')
		ca_file = DEFAULT_CA_FILE;
	raw = 0;
	extract = 0;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--") == 0) {
			i++;
			break;
		} else if (strcmp(argv[i], "-h") == 0 ||
		    strcmp(argv[i], "--help") == 0) {
			usage(stdout);
			return 0;
		} else if (strcmp(argv[i], "-V") == 0 ||
		    strcmp(argv[i], "--version") == 0) {
			printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
			return 0;
		} else if (strcmp(argv[i], "-m") == 0 ||
		    strcmp(argv[i], "--model") == 0) {
			value = option_value(argc, argv, &i, argv[i]);
			if (value == NULL)
				return 1;
			model = value;
		} else if (strcmp(argv[i], "-s") == 0 ||
		    strcmp(argv[i], "--instructions") == 0) {
			value = option_value(argc, argv, &i, argv[i]);
			if (value == NULL)
				return 1;
			instructions = value;
		} else if (strcmp(argv[i], "--base-url") == 0) {
			value = option_value(argc, argv, &i, argv[i]);
			if (value == NULL)
				return 1;
			base_url = value;
		} else if (strcmp(argv[i], "--ca-file") == 0) {
			value = option_value(argc, argv, &i, argv[i]);
			if (value == NULL)
				return 1;
			ca_file = value;
		} else if (strcmp(argv[i], "--raw") == 0) {
			raw = 1;
		} else if (strcmp(argv[i], "--extract") == 0) {
			extract = 1;
		} else if (argv[i][0] == '-') {
			fprintf(stderr, "%s: unknown option: %s\n",
			    PROGRAM_NAME, argv[i]);
			usage(stderr);
			return 1;
		} else {
			break;
		}
	}

	if (extract) {
		if (i != argc) {
			fprintf(stderr, "%s: --extract reads JSON from standard input\n",
			    PROGRAM_NAME);
			return 1;
		}
		if (read_stream(stdin, &input, MAX_RESPONSE_SIZE) != 0) {
			fprintf(stderr, "%s: cannot read response JSON\n", PROGRAM_NAME);
			return 4;
		}
		printed = print_response_text(input.data, input.len);
		buffer_free(&input);
		if (printed <= 0) {
			fprintf(stderr, "%s: response contains no output_text\n",
			    PROGRAM_NAME);
			return 4;
		}
		return 0;
	}

	if (i < argc) {
		if (join_arguments(argc, argv, i, &input) != 0) {
			fprintf(stderr, "%s: prompt is too large or memory is exhausted\n",
			    PROGRAM_NAME);
			return 1;
		}
	} else if (read_stream(stdin, &input, MAX_INPUT_SIZE) != 0) {
		fprintf(stderr, "%s: cannot read prompt or prompt exceeds %u bytes\n",
		    PROGRAM_NAME, (unsigned int)MAX_INPUT_SIZE);
		return 1;
	}
	api_key = getenv("OPENAI_API_KEY");
	if (api_key == NULL || *api_key == '\0') {
		fprintf(stderr, "%s: OPENAI_API_KEY is not set\n", PROGRAM_NAME);
		buffer_free(&input);
		return 1;
	}
	organization = getenv("OPENAI_ORGANIZATION");
	if (organization != NULL && *organization == '\0')
		organization = NULL;
	project = getenv("OPENAI_PROJECT");
	if (project != NULL && *project == '\0')
		project = NULL;
	if (parse_endpoint(base_url, &endpoint) != 0) {
		buffer_free(&input);
		return 1;
	}
	if (build_json_body(&json_body, model, input.data, instructions) != 0) {
		fprintf(stderr, "%s: cannot construct request JSON\n", PROGRAM_NAME);
		endpoint_free(&endpoint);
		buffer_free(&input);
		return 1;
	}
	buffer_free(&input);
	if (make_request(&endpoint, api_key, organization, project, ca_file,
	    &json_body, &raw_response) != 0) {
		endpoint_free(&endpoint);
		buffer_free(&json_body);
		return 2;
	}
	endpoint_free(&endpoint);
	buffer_free(&json_body);
	if (parse_http_response(&raw_response, &status, &response_body) != 0) {
		buffer_free(&raw_response);
		return 2;
	}
	buffer_free(&raw_response);
	if (status < 200 || status >= 300) {
		error_message = extract_error_message(response_body.data,
		    response_body.len);
		if (error_message != NULL) {
			fprintf(stderr, "OpenAI API error (HTTP %d): %s\n",
			    status, error_message);
			free(error_message);
		} else {
			fprintf(stderr, "OpenAI API error (HTTP %d)\n", status);
			(void)fwrite(response_body.data, 1, response_body.len, stderr);
			if (response_body.len == 0 ||
			    response_body.data[response_body.len - 1] != '\n')
				(void)fputc('\n', stderr);
		}
		buffer_free(&response_body);
		return 3;
	}
	if (raw) {
		(void)fwrite(response_body.data, 1, response_body.len, stdout);
		if (response_body.len == 0 ||
		    response_body.data[response_body.len - 1] != '\n')
			(void)putchar('\n');
		rc = 0;
	} else {
		printed = print_response_text(response_body.data, response_body.len);
		if (printed < 0) {
			fprintf(stderr, "%s: malformed JSON in API response\n",
			    PROGRAM_NAME);
			rc = 4;
		} else if (printed == 0) {
			fprintf(stderr, "%s: API response contains no output_text; use --raw to inspect it\n",
			    PROGRAM_NAME);
			rc = 4;
		} else {
			rc = 0;
		}
	}
	buffer_free(&response_body);
	return rc;
}
