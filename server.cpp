#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "utils.h"

int main() {
    int listen_sockfd, send_sockfd;
    sockaddr_in server_addr, client_addr_from, client_addr_to;
    char buffer[MAX_PACKET_SIZE];
    socklen_t addr_size = sizeof(client_addr_from);
    unsigned int expected_seq_num = 0;
    //Packet ack_pkt;

    // Create a UDP socket for sending
    send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_sockfd < 0) {
        std::cerr << "Could not create send socket.\n";
        return 1;
    }

    // Create a UDP socket for listening
    listen_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_sockfd < 0) {
        std::cerr << "Could not create listen socket.\n";
        return 1;
    }

    // Configure the server address structure
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Setup timeout
    timeval tv = {0, 0};
    tv.tv_usec = 200;// Setup timeout

    setsockopt(listen_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Bind the listen socket to the server address
    if (bind(listen_sockfd, (sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed.\n";
        close(listen_sockfd);
        return 1;
    }

    // Configure the client address structure to which we will send ACKs
    std::memset(&client_addr_to, 0, sizeof(client_addr_to));
    client_addr_to.sin_family = AF_INET;
    client_addr_to.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    client_addr_to.sin_port = htons(CLIENT_PORT_TO);

    // Open the target file for writing (always write to output.txt)
    std::ofstream outputStream("output.txt", std::ios::out);
    if (!outputStream.is_open()) {
        std::cerr << "Error opening file.\n";
        close(listen_sockfd);
        close(send_sockfd);
        return 1;
    }

    // TODO: Receive file from the client and save it as output.txt

    bool last = false;

    while (!last) {
        Packet ackPacket;
        bool resend = false;
        int bytesRead = receivePacket(listen_sockfd, buffer);

        if (bytesRead < 0) {
            // Timeout
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "Timeout!\n";
                ackPacket = Packet(CLIENT_PORT_TO, SERVER_PORT, 0, expected_seq_num, false, true, 0, nullptr);
                resend = true;
            }

            // Some other error occurred
            else {
                std::cerr << "Error reading from socket.\n";
                close(listen_sockfd);
                close(send_sockfd);
                return 1;
            }
        }
        
        // If a packet was read
        else {
            Packet packet(bytesRead, buffer);
            packet.printRecv();

            if (packet.getSeqnum() == expected_seq_num) {
                last = packet.isLast();

                expected_seq_num += packet.getPayloadLength();

                outputStream.write(packet.getPayload(), packet.getPayloadLength());
            }

            else {
                resend = true;
            }

            ackPacket = Packet(CLIENT_PORT_TO, SERVER_PORT, 0, expected_seq_num, false, true, 0, nullptr);
        }

        sendPacket(send_sockfd, &client_addr_to, addr_size, &ackPacket, resend);
    }

    outputStream.close();
    close(listen_sockfd);
    close(send_sockfd);
    return 0;
}
