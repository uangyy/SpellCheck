/*************************************************************************
  > File Name: src/main.cc
  > Author: uangyy
  > Mail:uangyy@gmail.com 
  > Created Time: Sun 17 May 2015 03:23:04 PM CST
 ************************************************************************/

#include "thread_pool.h"
#include "myConfig.h"
#include "myTask.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/epoll.h>
using namespace std;
#define ERR_EXIT(m)\
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    }while(0)


ifstream &open_file(ifstream &is, const string &file)
{
    is.close();
    is.clear();

    is.open(file.c_str());
    return is;
}

//线程函数
void *func(void *arg)
{
    Task &task = *(Task *)arg;
    cout << "i am work func!" << endl;
    task.execute();
    return NULL;
}

int main(int argc, char *argv[])
{
    /*
     * 测试线程池
     if(tpool_create(5) != 0)
     {
     printf("tpool_create failed\n");
     exit(EXIT_FAILURE);
     }
     int i;
     for(i = 0; i != 10; i++)
     {
     tpool_add_work(func, (void *)i);

     sleep(2);
     }
     tpool_destroy;
     */

    /*
     * 测试读配置文件
     */
    MyConfig myCon;
    ifstream is;
    if(argc != 2 || !open_file(is,argv[1]))
    {
        printf("no input file");
        exit(EXIT_FAILURE);
    }
    //读取配置文件
    myCon.get_map(is);
    //读取语料库
    myCon.read_file();
    //生成索引
    myCon.index_to_map();
    is.close();

    //创建一个线程池
    if(tpool_create(10) != 0)
        ERR_EXIT("create a thread pool");

    /*
     *测试socket
     */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
        ERR_EXIT("socket");

    //地址复用
    int on = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt");

    //internet环境下套接字的地址形式
    //所以在网络编程中我们会对sockaddr_in结构体进行操作
    //使用sockaddr_in来建立所需的信息，最后使用类型转化就可以了
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.4.89");
    addr.sin_port = htons(8888);
    if(bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        ERR_EXIT("bind");
    cout << "182.168.4.89:8888" << endl;
    if(listen(listenfd, SOMAXCONN) == -1)
        ERR_EXIT("listen");

    //创建epoll事件循环
    int efd = epoll_create(2048);
    if(efd == -1)
        ERR_EXIT("epoll_create");
    struct epoll_event evt_listen;
    evt_listen.data.fd = listenfd;
    evt_listen.events = EPOLLIN | EPOLLET;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &evt_listen);
    if(ret == -1)
        ERR_EXIT("epoll_ctr add");
    struct epoll_event *events = (epoll_event *)calloc(2048, sizeof(struct epoll_event));

    while(1)
    {
        int nready = epoll_wait(efd, events, 2048, -1);
        if(nready == -1)
        {
            ERR_EXIT("epoll_wait");
            break;
        }
        for(int i = 0; i != nready; i++)
        {
            int fd = events[i].data.fd;
            //新客户到来
            if(fd == listenfd)
            {
                cout << "welcome !" << endl;
                if(events[i].events & EPOLLIN)
                {
                    int peerfd = accept(fd, NULL, NULL);
                    if(peerfd == -1)
                        ERR_EXIT("accept");
                    struct epoll_event ev;
                    ev.data.fd = peerfd;
                    ev.events = EPOLLIN;
                    ret = epoll_ctl(efd, EPOLL_CTL_ADD, peerfd, &ev);
                    if(ret == -1)
                        ERR_EXIT("epollctl_add");
                }
            }
            else
            {
                //与客户端已经建立的连接
                if(events[i].events & EPOLLIN)
                {
                    char recvbuf[1024] = {0};
                    int nread = read(fd, recvbuf, sizeof(recvbuf));
                    if(nread == 0)
                    {
                        struct epoll_event ev;
                        ev.data.fd = fd;
                        ret = epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ev);
                        close(fd);
                        if(ret == -1)
                            ERR_EXIT("epoll_ctl_del");
                        continue;
                    }
                    cout << "recv msg: " << recvbuf ;
                    //封装一个Task对象，并把这个对象加入到线程池中去
                    Task *task = new Task(recvbuf, fd, myCon.get_vec(), myCon.get_index());
                    tpool_add_work(func, (void *)task);
                }
            }
        }
    }
    close(efd);
    close(listenfd);
    tpool_destroy();
    return 0;
}
