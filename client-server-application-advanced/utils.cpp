#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sys/epoll.h>
const int port = 8829;
const int client_size = 100;
const int timeout = 100000;

int send_message(int sockd, char* message) {
    int message_len = strlen(message);
    unsigned char str_message_len[2];
    str_message_len[1] = message_len % 256;
    message_len /= 256;
    str_message_len[0] = message_len % 256;
    message_len = strlen(message);
    int sended_bytes = 0;
    while (true) {
        int first_byte_status = send(sockd, (char *)&str_message_len[0], 1, 0);
        if (first_byte_status < 0) {
            perror("send error");
            return -1;
        } else if (first_byte_status == 1) {
            break;
        }
    }
    while (true) {
        int second_byte_status = send(sockd, (char *)&str_message_len[1], 1, 0);
        if (second_byte_status < 0) {
            perror("send error");
            return -1;
        } else if (second_byte_status == 1) {
            break;
        }
    }
    while (sended_bytes < message_len) {
        int bytes = send(sockd, message + sended_bytes, message_len - sended_bytes, 0);
        if (bytes < 0) {
            perror("send error");
            return -1;
        } else {
            sended_bytes += bytes;
        }
    }
    return 1;
}

int get_message(int sockd, char* message) {
    int message_len = 0;
    unsigned char first_byte[1], second_byte[1];
    while (true) {
        int bytes = recv(sockd, first_byte, 1, 0);
        if (bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            perror("recv error");
            return -1;
        } else if (bytes == 1) {
            break;
        }
    }
    while (true) {
        int bytes = recv(sockd, second_byte, 1, 0);
        if (bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            perror("recv error");
            return -1;
        } else if (bytes == 1) {
            break;
        }
    }
    message_len = (int)first_byte[0] * 256 + (int)second_byte[0];
    int geted_bytes = 0;
    while(geted_bytes < message_len) {
        int bytes = recv(sockd, message + geted_bytes, message_len - geted_bytes, 0);
        if (bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            perror("recv error");
            return -1;
        } else {
            geted_bytes += bytes;
        }
    }
    message[message_len] = '\0';
    return message_len;
}