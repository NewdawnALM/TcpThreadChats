#include "header.h"

// 客户端接收消息的线程函数
void* recv_func(void *args)
{
    char buf[BUF_LEN];
    int sock_fd = *(int*)args;
    while(true) {
        int n = recv(sock_fd, buf, BUF_LEN, 0);
        if(n <= 0)   break;                  // 这句很关键，一开始不知道可以用这个来判断通信是否结束，用了其它一些很奇葩的做法来结束并关闭 sock_fd 以避免 CLOSE_WAIT 和 FIN_WAIT2 状态的出现T.T
        write(STDOUT_FILENO, buf, n);
    }
    close(sock_fd);
    exit(0);
}

// 客户端和服务端进行通信的处理函数
void process(int sock_fd)
{
    pthread_t td;
    pthread_create(&td, NULL, recv_func, (void*)&sock_fd);      // 新开个线程来接收消息，避免了一读一写的原始模式，一开始竟把它放进 while 循环里面了，泪崩。。。

    char buf[BUF_LEN];
    while(true) {
        int n = read(STDIN_FILENO, buf, BUF_LEN);
        buf[n++] = '\0';                            // 貌似标准读入不会有字符串结束符的，需要自己手动添加
        send(sock_fd, buf, n, 0);
    }
    close(sock_fd);
}

int main(int argc, char *argv[])
{
    assert(argc == 2);

    struct sockaddr_in cli;
    bzero(&cli, sizeof(cli));
    cli.sin_family = AF_INET;
    cli.sin_addr.s_addr = htonl(INADDR_ANY);
    cli.sin_port = htons(PORT);                     // 少了 htons 的话就连接不上了，因为小端机器的原因？？？

    int sc = socket(AF_INET, SOCK_STREAM, 0);
    if(sc < 0) {
        perror("socket error");
        exit(-1);
    }
    inet_pton(AF_INET, argv[1], &(cli.sin_addr));           // 用第一个参数作为连接服务器端的地址

    int err = connect(sc, (struct sockaddr*)&cli, sizeof(cli));
    if(err < 0) {
        perror("connect error");
        exit(-2);
    }
    process(sc);
    close(sc);

    return 0;
}
