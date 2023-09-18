#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    uint16_t port = 8554;
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        return -1;
    
    if(listen(fd, 1024) < 0)
        return -1;
    printf("Initialize successfully.\n");

    while(true)
    {
        struct sockaddr_in clnAddr;
        socklen_t len = sizeof(clnAddr);
        int clnFd = accept(fd, (sockaddr*)&clnAddr, &len);

        printf("Connection with fd: %d, ip: %s, port: %d\n", clnFd, inet_ntoa(clnAddr.sin_addr), ntohs(clnAddr.sin_port));

        char buf[100] = {0};
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);
    }

    close(fd);
    return 0;
}