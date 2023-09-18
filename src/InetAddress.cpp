#include "InetAddress.h"

Ipv4Address::Ipv4Address()
{}

Ipv4Address::Ipv4Address(std::string ip, uint16_t port)
    : mIp(ip), mPort(port)
{
    mAddr.sin_family = AF_INET;
    mAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    mAddr.sin_port = htons(port);
}

Ipv4Address::Ipv4Address(const Ipv4Address& other)
{
    mIp = other.mIp;
    mPort = other.mPort;
    mAddr = other.mAddr;
}

Ipv4Address::Ipv4Address(Ipv4Address&& other)
    : mIp("0.0.0.0"), mPort(-1)
{
    std::swap(mIp, other.mIp);
    std::swap(mPort, other.mPort);
    std::swap(mAddr, other.mAddr);
}

Ipv4Address& Ipv4Address::operator= (const Ipv4Address& other)
{
    if(this != &other)
    {
        mIp = other.mIp;
        mPort = other.mPort;
        mAddr = other.mAddr;
    }
    return *this;
}

Ipv4Address& Ipv4Address::operator= (Ipv4Address&& other)
{
    if(this != &other)
    {
        std::swap(mIp, other.mIp);
        std::swap(mPort, other.mPort);
        std::swap(mAddr, other.mAddr);
    }
    return *this;
}

void Ipv4Address::setAddr(std::string ip, uint16_t port)
{
    mIp = ip;
    mPort = port;
    mAddr.sin_family = AF_INET;
    mAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    mAddr.sin_port = htons(port);
}

std::string Ipv4Address::getIp() const
{
    return mIp;
}

uint16_t Ipv4Address::getPort() const
{
    return mPort;
}

struct sockaddr* Ipv4Address::getAddr() const
{
    return (struct sockaddr*)&mAddr;
}