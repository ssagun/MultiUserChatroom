#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <mutex>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// Define port number and maximum number of connections
const int PORT = 8000;
const int MAX_CONNECTIONS = 5;

// Mutex to synchronize access to client list
std::mutex client_mutex;

// Vector to hold all connected clients
std::vector<int> clients;

// Function to handle each client connection
void handle_client(int client_socket) {
    // Add client to client list
    client_mutex.lock();
    clients.push_back(client_socket);
    client_mutex.unlock();

    // Send welcome message to client
    std::string welcome_msg = "Welcome to the chatroom!\n";
    send(client_socket, welcome_msg.c_str(), welcome_msg.length(), 0);

    // Receive and broadcast messages from client
    char buffer[1024];
    while (true) {
        // Receive message from client
        int bytes_received = recv(client_socket, buffer, 1024, 0);
        if (bytes_received <= 0) {
            // Connection closed or error occurred
            break;
        }

        // Add null terminator to received message
        buffer[bytes_received] = '\0';

        // Lock client list and broadcast message to all clients
        client_mutex.lock();
        for (int client : clients) {
            if (client != client_socket) {
                send(client, buffer, bytes_received, 0);
            }
        }
        client_mutex.unlock();
    }

    // Remove client from client list
    client_mutex.lock();
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    client_mutex.unlock();

    // Close client socket
    close(client_socket);
}

int main() {
    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Bind socket to port
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Error binding socket to port" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CONNECTIONS) < 0) {
        std::cerr << "Error listening for connections" << std::endl;
        return 1;
    }

    std::cout << "Server started. Listening for incoming connections..." << std::endl;

    // Handle incoming connections in separate threads
    while (true) {
        // Accept incoming connection
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        // Start thread to handle connection
        std::thread t(handle_client, client_socket);
        t.detach();
    }

    // Close server socket
    close(server_socket);

    return 0;
}
