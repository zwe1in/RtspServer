#ifndef INETADDRESS_H
#define INETADDRESS_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

class Ipv4Address{
public:
    Ipv4Address();
    Ipv4Address(std::string ip, uint16_t port);
    Ipv4Address(const Ipv4Address& other);
    Ipv4Address(Ipv4Address&& other);
    Ipv4Address& operator= (const Ipv4Address& other);
    Ipv4Address& operator= (Ipv4Address&& other);
    void setAddr(std::string ip, uint16_t port);
    std::string getIp() const;
    uint16_t getPort() const;
    struct sockaddr* getAddr() const;

private:
    std::string mIp;
    uint16_t mPort;
    struct sockaddr_in mAddr;
};

#endif