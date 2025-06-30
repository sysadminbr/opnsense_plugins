#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAX_LINE 2048
#define MAX_ARGS 64
#define HTTP_PORT 9080

void strip_newline(char *s) {
    while (*s) {
        if (*s == '\n' || *s == '\r') *s = '\0';
        s++;
    }
}

void extract_domain(const char *url, char *domain, size_t maxlen) {
    const char *start = strstr(url, "://");
    if (start)
        start += 3;
    else
        start = url;

    const char *end = strchr(start, '/');
    size_t len = end ? (size_t)(end - start) : strlen(start);

    if (len >= maxlen) len = maxlen - 1;
    strncpy(domain, start, len);
    domain[len] = '\0';
}

void url_decode(char *src, char *dst) {
    while (*src) {
        if (*src == '%' &&
            isxdigit((unsigned char)*(src + 1)) &&
            isxdigit((unsigned char)*(src + 2))) {
            char hex[3];
            hex[0] = *(src + 1);
            hex[1] = *(src + 2);
            hex[2] = '\0';
            *dst++ = (char) strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

int http_get(const char *domain, char *response, size_t maxlen) {
    int sock;
    struct hostent *host;
    struct sockaddr_in server_addr;
    char request[512];
    char buffer[2048];
    int bytes;

    host = gethostbyname("localhost");
    if (!host) return -1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(HTTP_PORT);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return -1;
    }

    snprintf(request, sizeof(request),
             "GET /dnscat.jsp?domain=%s HTTP/1.0\r\nHost: localhost\r\n\r\n", domain);
    send(sock, request, strlen(request), 0);

    bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes < 0) {
        close(sock);
        return -1;
    }

    buffer[bytes] = '\0';

    char *body = strstr(buffer, "\r\n\r\n");
    if (body)
        strncpy(response, body + 4, maxlen - 1);
    else
        response[0] = '\0';

    close(sock);
    return 0;
}

int main(void) {
    char line[MAX_LINE];
    char *args[MAX_ARGS];
    char decoded_args[MAX_ARGS][MAX_LINE];
    char domain[256];
    char response[1024];

    while (1) {
        if (!fgets(line, sizeof(line), stdin)) break;
        strip_newline(line);

        int count = 0;
        char *token = strtok(line, " ");
        while (token && count < MAX_ARGS) {
            url_decode(token, decoded_args[count]);
            args[count] = decoded_args[count];
            count++;
            token = strtok(NULL, " ");
        }

        if (count == 0) {
            printf("ERR\n");
            fflush(stdout);
            continue;
        }

        extract_domain(args[0], domain, sizeof(domain));
        fprintf(stderr, "Extracted domain: %s\n", domain);

        if (http_get(domain, response, sizeof(response)) != 0) {
            printf("ERR\n");
            fflush(stdout);
            continue;
        }

        strip_newline(response);
        fprintf(stderr, "API response: %s\n", response);

        int match = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(response, args[i]) == 0) {
                match = 1;
                break;
            }
        }

        if (match)
            printf("OK\n");
        else
            printf("ERR\n");

        fflush(stdout);
    }

    return 0;
}

