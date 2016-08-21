#include <map>
#include "header.h"
using std::map;

map<int, struct sockaddr_in*> socks;         // 用于记录各个客户端，键是与客户端通信 socket 的文件描述符，值是对应的客户端的 sockaddr_in 的信息

// 群发消息给 socks 中的所有客户端
inline void send_all(const char *buf, int len)
{
    for(auto it = socks.begin(); it != socks.end(); ++it)
        send(it->first, buf, len, 0);
}

// 服务端端接收消息的线程函数
void* recv_func(void* args)
{
    int cfd = *(int*)args;
    char buf[BUF_LEN];
    while(true) {
        int n = recv(cfd, buf, BUF_LEN, 0);
        if(n <= 0)   break;                     // 关键的一句，用于作为结束通信的判断
        write(STDOUT_FILENO, buf, n);
//        printf("buf = %s  len(buf) = %ld\n", buf, strlen(buf));
        if(strcmp(buf, "bye\n") == 0) {         // 如果接收到客户端的 bye，就结束通信并从 socks 中删除相应的文件描述符，动态申请的空间也应在删除前释放
            printf("close connection with client %d.\n", cfd);
            free(socks[cfd]);
            socks.erase(cfd);
            break;
        }
        send_all(buf, n);           // 群发消息给所有已连接的客户端
    }
    close(cfd);                 // 关闭与这个客户端通信的文件描述符
}

// 和某一个客户端通信的线程函数
void* process(void *argv)
{
    pthread_t td;
    pthread_create(&td, NULL, recv_func, (void*)argv);         // 在主处理函数中再新开一个线程用于接收该客户端的消息

    int sc = *(int*)argv;
    char buf[BUF_LEN];
    while(true) {
        int n = read(STDIN_FILENO, buf, BUF_LEN);
        buf[n++] = '\0';                // 和客户端一样需要自己手动添加字符串结束符
        send_all(buf, n);               // 服务端自己的信息输入需要发给所有客户端
    }
    close(sc);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serv;
    bzero(&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_port = htons(PORT);

    int ss = socket(AF_INET, SOCK_STREAM, 0);
    if(ss < 0) {
        perror("socket error");
        return 1;
    }
    int err = bind(ss, (struct sockaddr*)&serv, sizeof(serv));
    if(err < 0) {
        perror("bind error");
        return 2;
    }
    err = listen(ss, 2);
    if(err < 0) {
        perror("listen error");
        return 3;
    }

    socks.clear();          // 清空 map
    socklen_t len = sizeof(struct sockaddr);

    while(true) {
        struct sockaddr_in *cli_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
        int sc = accept(ss, (struct sockaddr*)cli_addr, &len);
        if(sc < 0) {
            free(cli_addr);
            continue;
        }
        if(socks.size() >= MAX_CONNECTION) {            // 当将要超过最大连接数时，就让那个客户端先等一下
            char buf[128] = "connections is too much, please waiting...\n";
            send(sc, buf, strlen(buf) + 1, 0);
            close(sc);
            free(cli_addr);
            continue;
        }
        socks[sc] = cli_addr;                        // 指向对应申请到的 sockaddr_in 空间
        printf("client %d connect me...\n", sc);

        pthread_t td;
        pthread_create(&td, NULL, process, (void*)&sc);       // 开一个线程来和 accept 的客户端进行交互
    }
    return 0;
}
