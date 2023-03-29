#include <iostream>
#include <unistd.h>
#include <string.h>
#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

mutex mtx;

void client_thread(int client_socket, vector<int>& clients, const string& username) {
    mtx.lock();
    cout << "Client " << username << " connected" << endl;
    mtx.unlock();

    while (true) {
        char buffer[1024] = {0};
        int bytes_received = recv(client_socket, buffer, 1024, 0);
        if (bytes_received == 0) {
            mtx.lock();
            cout << "Client " << username << " disconnected" << endl;
            // Remove client from the list
            clients.erase(remove(clients.begin(), clients.end(), client_socket), clients.end());
            mtx.unlock();
            break;
        }
        // message is already binded with username
        string message = string(buffer, bytes_received);
        message = message;

        // Send the message to all connected clients except the sender
        for (int client : clients) {
            if (client != client_socket) {
                send(client, message.c_str(), message.size(), 0);
            }
        }
    }

    close(client_socket);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "Failed to create socket" << endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "Failed to set socket options" << endl;
        return 1;
    }

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    memset(&address, 0, addrlen);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9000);

    if (bind(server_socket, (struct sockaddr *)&address, addrlen) < 0) {
        cerr << "Failed to bind socket" << endl;
        return 1;
    }

    if (listen(server_socket, 3) < 0) {
        cerr << "Failed to listen on socket" << endl;
        return 1;
    }

    vector<int> clients;

    while (true) {
        int client_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            cerr << "Failed to accept connection" << endl;
            continue;
        }

        // Receive the username from the client
        char buffer[1024] = {0};
        int bytes_received = recv(client_socket, buffer, 1024, 0);
        if (bytes_received == 0) {
            cerr << "Failed to receive username" << endl;
            close(client_socket);
            continue;
        }
        string username = string(buffer, bytes_received);

        mtx.lock();
        // Add the client to the list
        clients.push_back(client_socket);
        mtx.unlock();

        // Create a thread to handle the client
        thread t(client_thread, client_socket, ref(clients), username);
        t.detach();
    }

    close(server_socket);
    return 0;
}
