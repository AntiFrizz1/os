#include "utils.cpp"
int main(int argc , char *argv[]) {
    if (argc != 2) {
        printf("Illegal amount of arguments\nFor getting help run ./server help");
        return 0;
    }
    std::string arg = argv[1];
    if (arg == "help") {
        printf("For starting server run ./server <client-address>\n");
        return 0;
    }

    int server_sockd = socket(AF_INET , SOCK_STREAM , 0);
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

    listen(server_sockd , 4);
    int client_addr_length = sizeof(struct sockaddr);
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(argv[1]);
    client.sin_port = htons(port);
    int client_sockd;
    do {
        client_sockd = accept(server_sockd, (struct sockaddr *) &client, (socklen_t *) &client_addr_length);
        if (client_sockd < 0) {
            perror("accept error");
            return 1;
        }
    }while (client.sin_addr.s_addr != inet_addr(argv[1]));
    int a[1024], size = 0;
    int buf_size = 1024, code = 0;
    char buf[1024];
    do{
        code = 0;
        if ((buf_size = get_message(client_sockd, buf)) && buf_size < 0) {
            printf("get_message: can't get message\n");
            return 1;
        }
        printf("client: %s\n", buf);
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
                return 1;
            }
        }
    } while(code == 0);
    close(server_sockd);
    return 0;
}
