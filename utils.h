#ifndef UTILS_H
#define UTILS_H
#include <cstdlib>
#include <iostream>

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

class Packet {
public:
    Packet(unsigned short seqnum, unsigned short acknum, bool last, bool ack, unsigned int length, const char* payload);
    unsigned short getSeqnum();
    unsigned short getAcknum();
    bool isAck();
    bool isLast();
    unsigned int getLength();
    char* getPayload();
    void printRecv();
    void printSend(bool isResend);
private:
    unsigned short seqnum;
    unsigned short acknum;
    bool ack;
    bool last;
    unsigned int length;
    char payload[PAYLOAD_SIZE];
};

Packet::Packet(unsigned short seqnum, unsigned short acknum, bool last, bool ack, unsigned int length, const char* payload) {
    this->seqnum = seqnum;
    this->acknum = acknum;
    this->ack = ack;
    this->last = last;
    this->length = length;
    std::memcpy(this->payload, payload, length);
}

inline unsigned short Packet::getSeqnum()
{
    return seqnum;
}

inline unsigned short Packet::getAcknum()
{
    return acknum;
}

inline bool Packet::isAck()
{
    return ack;
}

inline bool Packet::isLast()
{
    return last;
}

unsigned int Packet::getLength()
{
    return length;
}

inline char* Packet::getPayload()
{
    return payload;
}

inline void Packet::printRecv() {
    std::cout << "RECV " << seqnum << ' ' << acknum << (last ? " LAST" : "") << (ack ? " ACK" : "") << '\n';
}

inline void Packet::printSend(bool isResend) {
    if (isResend)
        std::cout << "RESEND " << seqnum << ' ' << acknum << (last ? " LAST" : "") << (ack ? " ACK" : "") << '\n';
    else
        std::cout << "SEND " << seqnum << ' ' << acknum << (last ? " LAST" : "") << (ack ? " ACK" : "") << '\n';
}

#endif