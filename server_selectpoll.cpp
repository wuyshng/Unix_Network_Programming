#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <set>

#define PORT 5555
#define MAX_CLIENTS 10

int main() {
    int serverSocket, clientSocket, max_sd, activity, newSocket, valread, sd;
    int clientSockets[MAX_CLIENTS] = {0}; // Array to hold client socket descriptors
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    fd_set readfds; // Set of socket descriptors

    char buffer[1024];

    const char *message = "Hello from the server\n";

    // Create the server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket failed" << std::endl;
        return -1;
    }

    // Set up server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(serverSocket);
        return -1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        // Clear and set up the file descriptor set
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);  // Add server socket to set
        max_sd = serverSocket;

        // Add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            sd = clientSockets[i];

            // If valid socket descriptor, add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // Update max_sd
            if (sd > max_sd)
                max_sd = sd;
        }

        // Wait for activity on one of the sockets using select
        activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);

        if ((activity < 0) && (errno != EINTR)) {
            std::cerr << "Select error" << std::endl;
        }

        // If something happened on the server socket, it's an incoming connection
        if (FD_ISSET(serverSocket, &readfds)) {
            newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
            if (newSocket < 0) {
                std::cerr << "Accept failed" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::cout << "New connection, socket fd is " << newSocket 
                      << ", IP: " << inet_ntoa(clientAddr.sin_addr)
                      << ", PORT: " << ntohs(clientAddr.sin_port) << std::endl;

            // Send welcome message
            send(newSocket, message, strlen(message), 0);

            // Add new socket to array of clients
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = newSocket;
                    std::cout << "Adding new client socket at index " << i << std::endl;
                    break;
                }
            }
        }

        // Handle IO for each client
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            sd = clientSockets[i];

            if (FD_ISSET(sd, &readfds)) {
                // Check if it was for closing, and read the incoming message
                if ((valread = recv(sd, buffer, 1024, 0)) == 0) {
                    // Client disconnected, close the socket
                    getpeername(sd, (struct sockaddr*)&clientAddr, &addrLen);
                    std::cout << "Client disconnected, IP " << inet_ntoa(clientAddr.sin_addr)
                              << ", PORT " << ntohs(clientAddr.sin_port) << std::endl;
                    close(sd);
                    clientSockets[i] = 0; // Mark the socket as empty
                } else {
                    // Echo the message back to the client
                    buffer[valread] = '\0';  // Null-terminate the buffer
                    std::cout << "Received message from client: " << buffer << std::endl;
                    send(sd, buffer, valread, 0);  // Echo back
                }
            }
        }
    }

    close(serverSocket);
    return 0;
}
