#ifndef UTILS_H
#define UTILS_H
#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <list>
#include <utility>

class Packet;

typedef std::set<Packet> packet_queue;
// Maps packets to whether they've been sent before or not
typedef std::list<std::pair<Packet, bool>> packet_list;

// MACROS
#define SERVER_IP "127.0.0.1"
#define LOCAL_HOST "127.0.0.1"
#define SERVER_PORT_TO 5002
#define CLIENT_PORT 6001
#define SERVER_PORT 6002
#define CLIENT_PORT_TO 5001
#define PAYLOAD_SIZE 1024
#define WINDOW_SIZE 5
#define TIMEOUT 2
#define MAX_SEQUENCE 1024
#define MAX_PACKET_SIZE 1200

class Packet {
public:
    Packet(unsigned short source_port, unsigned short dest_port, unsigned int seqnum, unsigned int acknum, bool last, bool ack, unsigned short payloadLength, const char* payload);
    Packet(unsigned int packetSize, char* packet);
    Packet(const Packet& packet);
    Packet();
    Packet& operator=(const Packet& packet);
    unsigned short getSourcePort() const;
    unsigned short getDestPort() const;
    unsigned int getSeqnum() const;
    unsigned int getAcknum() const;
    bool isAck() const;
    bool isLast() const;
    bool isValid() const;
    unsigned int getPayloadLength() const;
    const char* getPayload() const;
    const char* getPacket() const;
    unsigned short getLength() const;
    void printRecv() const;
    void printSend(bool isResend) const;
private:
    unsigned short source_port;
    unsigned short dest_port;
    unsigned int seqnum;
    unsigned int acknum;
    bool ack;
    bool last;
    bool checksumValid;
    unsigned short payloadLength;
    unsigned short length;
    char payload[PAYLOAD_SIZE];
    char packet[MAX_PACKET_SIZE];
    void makePacket();
    unsigned short makeChecksum();
    void checkChecksum(unsigned short packetSum);
};

Packet::Packet() {

}

Packet::Packet(unsigned short source_port, unsigned short dest_port, unsigned int seqnum, unsigned int acknum, bool last, bool ack, unsigned short payloadLength, const char* payload) {
    this->source_port = source_port;
    this->dest_port = dest_port;
    this->seqnum = seqnum;
    this->acknum = acknum;
    this->ack = ack;
    this->last = last;
    this->payloadLength = payloadLength;
    this->length = payloadLength + 14;
    // 8 for header, 4 for seqnum or acknum, 1 for ack, 1 for last
    std::memcpy(this->payload, payload, payloadLength);

    makePacket();
}

Packet::Packet(unsigned int packetSize, char* packet) {
    unsigned int payloadSize = packetSize - 14;
    char* packetPlace = packet;

    unsigned short checksum;

    this->payloadLength = payloadSize;
    this->length = packetSize;
    std::memcpy(this->packet, packet, packetSize);

    // Extract header
    std::memcpy(&source_port, packetPlace, 2);
    packetPlace += 2;
    std::memcpy(&dest_port, packetPlace, 2);
    packetPlace += 2;
    std::memcpy(&length, packetPlace, 2);
    packetPlace += 2;
    std::memcpy(&checksum, packetPlace, 2);
    packetPlace += 2;

    // Extract reliability requirements
    unsigned int ackOrSeq;
    std::memcpy(&ackOrSeq, packetPlace, 4);
    packetPlace += 4;
    std::memcpy(&ack, packetPlace, 1);
    packetPlace++;
    std::memcpy(&last, packetPlace, 1);
    packetPlace++;

    if (ack) {
        acknum = ackOrSeq;
    }

    else {
        seqnum = ackOrSeq;
    }

    // Extract payload
    std::memcpy(payload, packetPlace, payloadSize);

    checkChecksum(checksum);
}

Packet::Packet(const Packet& packet) {
    this->source_port = packet.source_port;
    this->dest_port = packet.dest_port;
    this->seqnum = packet.seqnum;
    this->acknum = packet.acknum;
    this->ack = packet.ack;
    this->last = packet.last;
    this->payloadLength = packet.payloadLength;
    this->length = payloadLength + 14;
    // 8 for header, 4 for seqnum or acknum, 1 for ack, 1 for last
    std::memcpy(this->payload, packet.payload, payloadLength);
    std::memcpy(this->packet, packet.packet, length);
}

Packet& Packet::operator=(const Packet& packet) {
    if (this == &packet) {
        return *this;
    }

    this->source_port = packet.source_port;
    this->dest_port = packet.dest_port;
    this->seqnum = packet.seqnum;
    this->acknum = packet.acknum;
    this->ack = packet.ack;
    this->last = packet.last;
    this->payloadLength = packet.payloadLength;
    this->length = payloadLength + 14;
    // 8 for header, 4 for seqnum or acknum, 1 for ack, 1 for last
    std::memcpy(this->payload, packet.payload, payloadLength);
    std::memcpy(this->packet, packet.packet, length);

    return *this;
}

void Packet::makePacket() {
    unsigned short checksum = makeChecksum();
    // Create header
    char* packetPlace = packet;

    std::memcpy(packetPlace, &source_port, 2);
    packetPlace += 2;
    std::memcpy(packetPlace, &dest_port, 2);
    packetPlace += 2;
    std::memcpy(packetPlace, &length, 2);
    packetPlace += 2;
    std::memcpy(packetPlace, &checksum, 2);
    packetPlace += 2;
    
    // Add additional data for reliable transfering
    std::memcpy(packetPlace, ack ? &acknum : &seqnum, 4);
    packetPlace += 4;
    std::memcpy(packetPlace, &ack, 1);
    packetPlace++;
    std::memcpy(packetPlace, &last, 1);
    packetPlace++;

    // Copy payload
    std::memcpy(packetPlace, payload, payloadLength);
}

unsigned short Packet::makeChecksum() {
    return 0;
}

void Packet::checkChecksum(unsigned short checksum) {
    checksumValid = true;

    /* 
    checksumValid = ~(makeChecksum() + checksum) == 0;
    */
}

inline unsigned short Packet::getSourcePort() const {
    return source_port;
}

inline unsigned short Packet::getDestPort() const {
    return dest_port;
}

inline unsigned int Packet::getSeqnum() const {
    return seqnum;
}

inline unsigned int Packet::getAcknum() const {
    return acknum;
}

inline bool Packet::isAck() const {
    return ack;
}

inline bool Packet::isLast() const {
    return last;
}

inline bool Packet::isValid() const {
    return checksumValid;
}

inline unsigned int Packet::getPayloadLength() const {
    return payloadLength;
}

inline const char* Packet::getPayload() const {
    return payload;
}

inline const char* Packet::getPacket() const {
    return packet;
}

unsigned short Packet::getLength() const {
    return length;
}

inline void Packet::printRecv() const {
    std::cout << "RECV " << seqnum << ' ' << acknum << (last ? " LAST" : "") << (ack ? " ACK" : "") << '\n';
}

inline void Packet::printSend(bool isResend) const {
    if (isResend)
        std::cout << "RESEND " << seqnum << ' ' << acknum << (last ? " LAST" : "") << (ack ? " ACK" : "") << '\n';
    else
        std::cout << "SEND " << seqnum << ' ' << acknum << (last ? " LAST" : "") << (ack ? " ACK" : "") << '\n';
}

void sendPacket(int fd, sockaddr_in* addr, socklen_t addr_size, Packet* packet, bool resend) {
    sendto(fd, packet->getPacket(), packet->getLength(), 0, (sockaddr*) addr, addr_size);

    /*std::cout << "Source port: " << packet->getSourcePort() << ", Dest port: " << packet->getDestPort() << ", Length: " << packet->getLength() << '\n';
    std::cout.write(packet->getPayload(), packet->getPayloadLength());
    std::cout << '\n';*/

    /*std::cout << "Packet sent has contents ";
    std::cout.write(packet->getPacket(), packet->getLength());*/
    packet->printSend(resend);
}

int receivePacket(int fd, char* buffer) {
    int bytesRead =  recv(fd, buffer, MAX_PACKET_SIZE, 0);

    /*if (bytesRead > 0) {
        Packet packet(bytesRead, buffer);
    
        std::cout << "Received Source port: " << packet.getSourcePort() << ", Dest port: " << packet.getDestPort() << ", Length: " << packet.getLength() << '\n';
        std::cout.write(packet.getPayload(), packet.getPayloadLength());
        std::cout << '\n';
    }

    if (bytesRead > 0) {
        std::cout << "Read packet ";
        std::cout.write(buffer + 14, 3);
        std::cout << '\n';
    }*/
    return bytesRead;
}

inline std::strong_ordering operator<=>(Packet left, Packet right) {
    if (left.isAck()) {
        return left.getAcknum() <=> right.getAcknum();
    }

    return left.getSeqnum() <=> right.getSeqnum();
}

// Returns true if the next packet(s) was(were) found
bool checkQueue(packet_queue* queue, unsigned int* expected_seq_num, bool* last, std::ofstream* outputStream) {
    packet_queue::iterator it;
    Packet queueTop;

    bool foundNextPacket = false;

    for (it = queue->begin(), queueTop = *it; !queue->empty() && queueTop.getSeqnum() == *expected_seq_num; it = queue->begin()) {
        //std::cout << "Found next packet in queue.\n";

        foundNextPacket = true;

        *last = queueTop.isLast();

        *expected_seq_num += queueTop.getPayloadLength();

        outputStream->write(queueTop.getPayload(), queueTop.getPayloadLength());

        queue->erase(it);
    }

    return foundNextPacket;
}

// Returns -1 on fail.
int readAndEnqueuePackets(int fd, unsigned int* expectedSeqNum, packet_queue* queue, 
                            std::ofstream* outputStream, bool* last, bool* foundNextPacket) {
    std::vector<char> buffer(MAX_PACKET_SIZE);

    bool timedOut = false;
    while (!timedOut) {
        int bytesRead = receivePacket(fd, buffer.data());

        if (bytesRead < 0) {
            // Timeout
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "Timeout!\n";
                timedOut = true;
            }

            // Some other error occurred
            else {
                return -1;
            }
        }

        else {
            Packet temp(bytesRead, buffer.data());

            temp.printRecv();

            /*std::cout << "Read packet ";
            std::cout.write(temp.getPacket(), temp.getLength());
            std::cout << '\n';*/

            unsigned int seqnum = temp.getSeqnum();

            if (seqnum > *expectedSeqNum) { 
                /*auto result = */queue->emplace(bytesRead, buffer.data());
                /*if (result.second) {
                    std::cout << "Successfully enqueued.\n";
                }*/
            }

            else if (seqnum == *expectedSeqNum) {
                //std::cout << "Wrote next packet instead of enqueuing.\n";

                *foundNextPacket = true;
                outputStream->write(temp.getPayload(), temp.getPayloadLength());

                *expectedSeqNum += temp.getPayloadLength();

                *last = temp.isLast();

                if (!queue->empty()) {
                    checkQueue(queue, expectedSeqNum, last, outputStream);
                }
            }

            else {
                std::cout << "Duplicate packet detected. Skipping.\n";
            }
        }
    }

    return 0;
}

void fillWindow(std::ifstream* textStream, packet_list* window, unsigned int* seq_num, unsigned int windowSize) {
    std::vector<char> buffer(PAYLOAD_SIZE);

    while (!textStream->eof() && window->size() < windowSize) {
        textStream->read(buffer.data(), PAYLOAD_SIZE);

        int readBytes = textStream->gcount();

        Packet packet(CLIENT_PORT, SERVER_PORT_TO, *seq_num, 0, textStream->eof(), false, readBytes, buffer.data());

        /*std::cout.write(buffer.data(), 3);
        std::cout << '\n';*/

        window->push_back(std::make_pair(packet, false));

        *seq_num += readBytes;
    }
}

// Sends everything in the window
void sendWindow(int fd, packet_list* window, sockaddr_in* addr, socklen_t addr_size) {
    auto end = window->end();
    for (auto it = window->begin(); it != end; it++) {
        Packet packet = it->first;
        bool resend = it->second;

        /*std::cout << "Packet " << packet.getSeqnum() << " ";
        std::cout.write(packet.getPayload(), 3);
        std::cout << '\n';*/

        it->second = true;

        sendPacket(fd, addr, addr_size, &packet, resend);
    }
}

// Returns whether the last packet was popped
bool removeAckedPackets(packet_list* window, unsigned int ack) {
    Packet windowFront;
    bool last = false;

    while (!window->empty() && (windowFront = window->front().first).getSeqnum() < ack) {
        last = windowFront.isLast();
        window->pop_front();
    }

    return last;
}

#endif