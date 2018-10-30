#include "utils.cpp"
#include <sys/epoll.h>

int a[1024], size = 0;
int get_request(char* buf, int buf_size, int client_sockd) {
    int code = 0;
    std::string buf_str = buf;
    if (buf_str == "stop") {
        code = 1;
    } else if (buf_size > 1 && buf_str[0] == '+') {
        int number = 0;
        for (size_t i = 1; i < buf_str.length(); i++) {
            number *= 10;
            if (buf_str[i] <= '9' && buf_str[i] >= '0') {
                number += buf_str[i] - '0';
            } else {
                code = 2;
                break;
            }
        }
        if (code == 0) {
            a[size] = number;
            size++;
        }
    } else if (buf_str == "sum") {
        code = -1;
    } else {
        code = 2;
    }
    int sum = 0;
    std::string message = "";
    switch (code) {
        case -1:
            code = 0;
            for (int i = 0; i < size; i++) {
                sum += a[i];
            }
            message = "Sum = " + std::to_string(sum);
            break;
        case 0:
            code = 0;
            message = "OK";
            break;
        case 1:
            message = "Server stopped";
            break;
        case 2:
            code = 0;
            message = "Error: expected number";
            break;

    }
    if (message != "") {
        char *msg = (char*)message.c_str();
        if (send_message(client_sockd, msg) < 0) {
            printf("send_message: can't send message\n");
            return -1;
        }
    }
    return code;
}

int main() {
    int server_sockd = socket(AF_INET , SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_sockd < 0) {
        perror("socket error");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if(bind(server_sockd,(struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("bind error");
        return 1;
    }

    listen(server_sockd , 1);

    int epfd = epoll_create(client_size);
    if (epfd < 0) {
        perror("epoll_create error");
        return 1;
    }
    static struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_sockd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_sockd, &event) < 0) {
        perror("epoll_ctl error");
        return 1;
    }
    bool f = true;
    while (f) {
        static struct epoll_event events[client_size];
        int res = epoll_wait(epfd, events ,client_size, timeout);
        if (res < 0) {
            close(epfd);
            close(server_sockd);
            f = false;
        }
        for (int i = 0; i < res && f; i++) {
            if (events[i].data.fd == server_sockd) {
                int client_addr_length = sizeof(struct sockaddr);
                struct sockaddr_in client;
                int client_sockd = accept(server_sockd, (struct sockaddr *)&client,
                                          reinterpret_cast<socklen_t *>(&client_addr_length));
                if (client_sockd == -1) {
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                        continue;
                    } else {
                        perror("accept error");
                    }   
                } else {
                    static struct epoll_event client_event;
                    client_event.data.fd = client_sockd;
                    client_event.events = EPOLLIN|EPOLLRDHUP;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockd, &client_event) < 0) {
                        perror("epoll_ctl error");
                    }
                }
            } else if (events[i].events == EPOLLIN) {
                char buf[1024];
                int buf_size = get_message(events[i].data.fd, buf);
                if (buf_size < 0) {
                    printf("get_message error: can't get message");
                    close(events[i].data.fd);
                    continue;
                }
                printf("client id = %d: %s\n", events[i].data.fd, buf);
                int code = get_request(buf, buf_size, events[i].data.fd);
                if (code == 1) {
                    f = false;
                    continue;
                } else if (code == -1) {
                    close(events[i].data.fd);
                }
            } else {
                close(events[i].data.fd);
            }
        }
    }


    if (close(epfd) < 0) {
        perror("close");
    }
    return 0;
}