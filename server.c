#include <arpa/inet.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "db.h"



#define SIZE 1024

void generate_short_url(char *output) {
    static int counter = 1;
    char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int num = counter++;
    int i = 0;
    char temp[10];
    while (num > 0) {
        temp[i++] = charset[num % 62];
        num /= 62;
    }
    int j = 0;
    while (i > 0) {
        output[j++] = temp[--i];
    }
    output[j] = '\0';
}

void handle_get(int client_fd, char* path) {

    char* short_id = path + 1; 
    char* long_url = find_long_url(short_id);
    if (long_url != NULL) {
        char response[1024];
        sprintf(response,
            "HTTP/1.1 302 Found\r\n"
            "Location: %s\r\n"
            "\r\n",
            long_url
        );
        write(client_fd, response, strlen(response));

    } else {
        char *response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Not Found";
        write(client_fd, response, strlen(response));
    }
}

void handle_post(int client_fd, char* buffer){
    char url[1024] = {0};
    char* ptr = strstr(buffer, "\r\n\r\n");
    if(ptr == NULL){
        // send 400
        return;
    }
    ptr += 4;
    if(strncmp(ptr, "url=", 4) != 0){
        // send 400
        return;
    }
    ptr += 4;
    int i = 0;
    while(*ptr != '\n' && *ptr != '\0' && *ptr != '\r' && i < 1023){
        url[i++] = *ptr;
        ptr++;
    }
    url[i] = '\0';
    char short_url[10];
    generate_short_url(short_url);
    save_info(short_url, url);
    char response[1024];
    sprintf(response,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "%s",
        short_url
    );
    write(client_fd, response, strlen(response));
}
int main() {

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sock, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server running on port 8080...\n");
    //init_storage();
    while (1) {

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        char buffer[SIZE];

        int bytes = read(client_fd, buffer, SIZE - 1);
        if (bytes < 0) {
            perror("read failed");
            close(client_fd);
            continue;
        }

        buffer[bytes] = '\0';

        printf("\n--- Incoming Request ---\n%s\n", buffer);
        regex_t reg;
        regmatch_t matches[4];
        if (regcomp(&reg, "^([A-Z]+) ([^ ]+) (HTTP/[0-9.]+)", REG_EXTENDED) != 0) {
            printf("Regex compile failed\n");
            close(client_fd);
            continue;
        }
        char method[10] = {0};
        char path[100] = {0};
        char version[20] = {0};
        if (regexec(&reg, buffer, 4, matches, 0) == 0) {
            int len;
            len = matches[1].rm_eo - matches[1].rm_so;
            snprintf(method, sizeof(method), "%.*s", len, buffer + matches[1].rm_so);
            len = matches[2].rm_eo - matches[2].rm_so;
            snprintf(path, sizeof(path), "%.*s", len, buffer + matches[2].rm_so);
            len = matches[3].rm_eo - matches[3].rm_so;
            snprintf(version, sizeof(version), "%.*s", len, buffer + matches[3].rm_so);
            printf("Method: %s\nPath: %s\nVersion: %s\n", method, path, version);
        } else {
            printf("No match\n");
        }
        regfree(&reg);

        if(strcmp(method,"GET") == 0){
            handle_get(client_fd, path);
        }
        if(strcmp(method, "POST") == 0){
            handle_post(client_fd, buffer);
        }
        close(client_fd);
    }
    return 0;
}
