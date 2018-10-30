#include "utils.cpp"


int main(int argc , char *argv[])
{
    if (argc != 2) {
        printf("Illegal amount of arguments\nFor getting help run ./client help\n");
        return 0;
    }
    std::string arg = argv[1];
    if (arg == "help") {
        printf("For start client run ./client <server-address>\n");
        return 0;
    }
    printf("You must use :\n"
           "+<number> - for adding number in array at server\n"
           "sum       - for getting sum of array at server\n"
           "stop      - for stoping server and client application\n");
    int server_sockd = socket(AF_INET , SOCK_STREAM , 0);
    if (server_sockd < 0)
    {
        printf("socket error");
    }
    puts("Socket created");
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(server_sockd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect error");
        return 1;
    }
    printf("connected to server\n");
    while(true)
    {
        char to_server[1024], from_server[1024];
        scanf("%s", to_server);
        if (send_message(server_sockd, to_server) < 0) {
            printf("send_message: can't send message\n");
            return 1;
        }
        if (get_message(server_sockd, from_server) < 0) {
            printf("get_message: can't get message\n");
            return 1;
        }
        printf("server: %s\n", from_server);
        if (strcmp(from_server, "Server stopped") == 0) {
            break;
        }
    }
    close(server_sockd);
    return 0;
}
