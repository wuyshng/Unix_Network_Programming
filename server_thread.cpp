#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

std::mutex coutMutex; // Mutex for safe printing to the console

// Function to handle individual client communication
void handleClient(int clientSocket) {
    char buffer[1024];
    int bytesReceived;

    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the string
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Client " << clientSocket << " sent: " << buffer << std::endl;

        // Echo message back to the client
        send(clientSocket, buffer, bytesReceived, 0);
    }

    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "Client " << clientSocket << " disconnected." << std::endl;

    close(clientSocket);  // Close the socket once done
}

int main() {
    const int port = 5555;
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // Create the server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return -1;
    }

    // Configure server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address
    serverAddr.sin_port = htons(port);

    // Bind the socket to the address and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        return -1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed." << std::endl;
        return -1;
    }

    std::cout << "Server is listening on port " << port << std::endl;

    // Vector to hold client threads
    std::vector<std::thread> clientThreads;

    // Accept clients in a loop
    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSocket < 0) {
            std::cerr << "Failed to accept connection." << std::endl;
            continue;
        }

        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Accepted connection from client " << clientSocket << std::endl;

        // Launch a new thread to handle the client
        clientThreads.emplace_back(std::thread(handleClient, clientSocket));
    }

    // Clean up the threads (optional, depends on program termination logic)
    for (auto& th : clientThreads) {
        if (th.joinable())
            th.join();
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
