#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define PORT 8000

using namespace std;

int sock = 0;

void *recvmg(void *sock)
{
    int sockfd = *((int *)sock);
    char buffer[1024];
    int len;

    while ((len = recv(sockfd, buffer, 1024, 0)) > 0)
    {
        buffer[len] = '\0';
        cout << buffer << endl;
    }
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, recvmg, (void *)&sock) < 0)
    {
        printf("\nCould not create thread\n");
        return -1;
    }

    char message[1024];

    while (fgets(message, 1024, stdin) != NULL)
    {
        if (send(sock, message, strlen(message), 0) < 0)
        {
            printf("\nSend failed\n");
            return -1;
        }
    }

    return 0;
}
