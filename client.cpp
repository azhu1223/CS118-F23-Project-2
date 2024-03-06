#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include <string>
#include <fstream>
#include <vector>
#include <cerrno>

#include "utils.h"


int main(int argc, char *argv[]) {
    int listen_sockfd, send_sockfd;
    sockaddr_in client_addr, server_addr_to, server_addr_from;
    socklen_t addr_size = sizeof(server_addr_to);
    unsigned int seq_num = 0;
    unsigned int ack_num = 0;
    bool last = false;
    bool ack = false;

    // read filename from command line argument
    if (argc != 2) {
        std::cerr << "Usage: ./client <filename>\n";
        return 1;
    }

    std::string filename = argv[1];

    // Create a UDP socket for listening
    listen_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_sockfd < 0) {
        std::cerr << "Could not create listen socket.\n";
        return 1;
    }

    // Create a UDP socket for sending
    send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_sockfd < 0) {
        std::cerr << "Could not create send socket.\n";
        return 1;
    }

    // Configure the server address structure to which we will send data
    std::memset(&server_addr_to, 0, sizeof(server_addr_to));
    server_addr_to.sin_family = AF_INET;
    server_addr_to.sin_port = htons(SERVER_PORT_TO);
    server_addr_to.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Configure the client address structure
    std::memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Setup timeout
    timeval tv = {0, 0};
    tv.tv_usec = 200;
    setsockopt(listen_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));


    // Bind the listen socket to the client address
    if (bind(listen_sockfd, (sockaddr*) &client_addr, sizeof(client_addr)) < 0) {
        std::cerr << "Bind failed.\n";
        close(listen_sockfd);
        return 1;
    }

    // Open file for reading
    std::ifstream textStream(filename, std::ios::in);
    if (!textStream.is_open()) {
        std::cerr << "Error opening file.\n";
        close(listen_sockfd);
        close(send_sockfd);
        return 1;
    }

    // TODO: Read from file, and initiate reliable data transfer to the server

    std::vector<char> buffer(MAX_PACKET_SIZE);

    while (!last) {
        textStream.read(buffer.data(), PAYLOAD_SIZE);

        if (textStream.eof())
        {
            last = true;
        }

        unsigned short numCharRead = textStream.gcount();

        Packet packet(SERVER_PORT_TO, CLIENT_PORT, seq_num, 0, last, ack, numCharRead, buffer.data());

        bool validAck = false;
        bool resend = false;
        ack_num += packet.getPayloadLength();

        while (!validAck) {
            sendPacket(send_sockfd, &server_addr_to, addr_size, &packet, resend);
            int bytesRead = receivePacket(listen_sockfd, buffer.data());

            // If there was a non-timeout error
            if (bytesRead <= 0 && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                std::cerr << "Error reading from socket.\n";
                close(listen_sockfd);
                close(send_sockfd);
                textStream.close();
                return 1;
            }
            // Received an ACK
            else if (bytesRead > 0)
            {
                Packet ack(bytesRead, buffer.data());

                ack.printRecv();

                // Check if ACK is the expected ACK
                if (ack.isAck() && ack.getAcknum() == ack_num) {
                    std::cout << "Valid ACK.\n";
                    validAck = true;
                }

                else {
                    resend = true;
                }
            }

            // There was a timeout
            else {
                std::cout << "Timeout!\n";
                resend = true;
            }
        }

        seq_num = ack_num;


        /*sendto(send_sockfd, packet.getPacket(), packet.getLength(), 0, (sockaddr*) &server_addr_to, addr_size);

        packet.printSend(false);

        int bytesRead = recv(listen_sockfd, buffer.data(), MAX_PACKET_SIZE, 0);
        while (bytesRead < 0)
        {
            std::cout << "Timeout!\n";
            // If there was a non-timeout error
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "Error reading from socket.\n";
                close(listen_sockfd);
                close(send_sockfd);
                textStream.close();
                return 1;
            }

            sendto(send_sockfd, packet.getPacket(), packet.getLength(), 0, (sockaddr*) &server_addr_to, addr_size);

            packet.printSend(true);

            bytesRead = recv(listen_sockfd, buffer.data(), MAX_PACKET_SIZE, 0);
        }

        Packet ack(bytesRead, buffer.data());

        ack.printRecv();

        while (ack.getAcknum() == seq_num) {
            sendto(send_sockfd, packet.getPacket(), packet.getLength(), 0, (sockaddr*) &server_addr_to, addr_size);

            packet.printSend(true);

            bytesRead = recv(listen_sockfd, buffer.data(), MAX_PACKET_SIZE, 0);
            if (bytesRead < 0) {
                std::cerr << "Error reading from socket.\n";
                close(listen_sockfd);
                close(send_sockfd);
                textStream.close();
                return 1;
            }

            ack = Packet(bytesRead, buffer.data());

            ack.printRecv();
        }

        seq_num = ack.getAcknum();*/
    }
    
    textStream.close();
    close(listen_sockfd);
    close(send_sockfd);
    return 0;
}
