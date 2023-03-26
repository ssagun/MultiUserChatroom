#include <iostream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>

using namespace std;

const int BUFFER_SIZE = 1024;
char username[BUFFER_SIZE];

void receive_messages(int sockfd) {
    while (true) {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received == -1) {
            cerr << "Error in recv()" << endl;
            break;
        }
        if (bytes_received == 0) {
            cout << "Server disconnected" << endl;
            break;
        }
        cout << buffer << endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./client <IP address> <port>" << endl;
        return 1;
    }

    char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cerr << "Error creating socket" << endl;
        return 1;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_address.sin_addr);

    if (connect(sockfd, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
        cerr << "Error connecting to server" << endl;
        return 1;
    }

    cout << "Connected to server" << endl;

    cout << "Enter your username: ";
    cin.getline(username, BUFFER_SIZE);
    send(sockfd, username, strlen(username), 0);

    thread receive_thread(receive_messages, sockfd);

    while (true) {
        string message;
        getline(cin, message);
        message = string(username) + ": " + message;
        send(sockfd, message.c_str(), message.size(), 0);
    }

    receive_thread.join();

    close(sockfd);
    return 0;
}
