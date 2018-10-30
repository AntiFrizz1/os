#include "utils.cpp"

int main(int argc , char *argv[])
{
    if (argc != 2) {
        printf("Illegal amount of arguments\nFor getting help run ./client help\n");
        return 0;
    }
    std::string arg = argv[1];
    if (arg == "help") {
        printf("For starting client run ./client <server-address>\n");
        return 0;
    }
    printf("You must use :\n"
           "+<number> - for adding number in array at server\n"
           "sum       - for getting sum of array at server\n"
           "exit      - for stopping client application\n"
           "stop      - for stopping server and client application\n");


    int server_sockd = socket(AF_INET , SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_sockd < 0)
    {
        perror("socket error");
    }
    printf("Socket created\n");
    int epfd = epoll_create(client_size);
    if (epfd < 0) {
        perror("epoll_create error");
        return 1;
    }
    static struct epoll_event events[client_size];
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);


    bool connected = true;
    if (connect(server_sockd , (struct sockaddr *)&server , sizeof(server)) < 0) {
        if (errno == EINPROGRESS) {
            connected = false;
        } else {
            perror("connect error");
            return 1;
        }
    }
    static struct epoll_event server_event;
    server_event.events = EPOLLIN;
    server_event.data.fd = STDIN_FILENO;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &server_event) < 0) {
        perror("epoll_ctl error");
        close(epfd);
        close(server_sockd);
        return 1;
    }


    if (!connected) {
        server_event.events = EPOLLIN | EPOLLOUT;
        server_event.data.fd = server_sockd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_sockd, &server_event) < 0) {
            perror("epoll_ctl error");
            close(epfd);
            close(server_sockd);
            return 1;
        }


    } else {
        printf("connected to server\n");
        server_event.events = EPOLLIN;
        server_event.data.fd = server_sockd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_sockd, &server_event) < 0) {
            perror("epoll_ctl error");
            close(epfd);
            close(server_sockd);
            return 1;
        }
    }


    bool f = true;
    char to_server[1024], from_server[1024];
    while (f) {
        int res = epoll_wait(epfd, events ,client_size, timeout);
        for (int i = 0; i < res && f; i++) {
            if (events[i].data.fd == server_sockd && events[i].events & EPOLLOUT) {
                if (!connected) {
                    int len = sizeof(int);
                    int err = 0;
                    int i = getsockopt(server_sockd, SOL_SOCKET, SO_ERROR, &err, reinterpret_cast<socklen_t *>(&len));
                    if (i != 1) {
                        if (err == 0) {
                            printf("connected to server\n");
                            static struct epoll_event event;
                            event.events = EPOLLIN;
                            event.data.fd = server_sockd;
                            if (epoll_ctl(epfd, EPOLL_CTL_MOD, server_sockd, &event) < 0) {
                                perror("epoll_ctl error");
                                f = false;
                                break;
                            }
                        }
                        else {
                            perror("");
                            static struct epoll_event event;
                            event.events = 0;
                            event.data.fd = server_sockd;
                            epoll_ctl(epfd, EPOLL_CTL_DEL, server_sockd, &event);
                            close(server_sockd);
                        }
                    }
                }
            } else if (events[i].data.fd == STDIN_FILENO) {
                int size1 = read(STDIN_FILENO, to_server, 1024);
                to_server[size1 - 1] = '\0';
                if (strcmp(to_server, "exit") == 0) {
                    close(events[i].data.fd);
                    f = false;
                    break;
                }
                if (send_message(server_sockd, to_server) < 0) {
                    printf("send_message: can't send message\n");
                    close(events[i].data.fd);
                    f = false;
                    break;
                }
            } else if (events[i].data.fd == server_sockd) {
                if (get_message(events[i].data.fd, from_server) < 0) {
                    printf("get_message: can't get message\n");
                    close(events[i].data.fd);
                    f = false;
                    break;
                }
                printf("server: %s\n", from_server);
                if (strcmp(from_server, "Server stopped") == 0) {
                    close(events[i].data.fd);
                    f = false;
                    break;
                }
            } else {
                f = false;
                close(events[i].data.fd);
            }
        }
    }


    if (close(epfd) < 0) {
        perror("close");
    }
    return 0;
}