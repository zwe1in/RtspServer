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
    addr.sin_addr.s_addr = inet_addr("112.74.181.170");
    addr.sin_port = htons(port);
    if(connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        return -1;

    close(fd);
    // while(true)
    // {
    //     char buf[100] = "Hello world.";
    //     write(fd, buf, sizeof(buf));
    // }
    return 0;
}