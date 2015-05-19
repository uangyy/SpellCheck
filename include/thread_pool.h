/*************************************************************************
	> File Name: include/thread_pool.h
	> Author: uangyy
	> Mail:uangyy@gmail.com 
	> Created Time: Sun 17 May 2015 11:23:57 AM CST
 ************************************************************************/

#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <iostream>
#include <pthread.h>
using namespace std;

//要执行的任务链表
typedef struct tpool_work{
    //函数指针,任务函数
    void *(*routine)(void *);
    
    //传入任务函数的参数
    void *arg;

    struct tpool_work *next;

}tpool_work_t;

typedef struct tpool{
    //线程池是否销毁
    int shutdown;

    //最大线程数
    int max_thr_num;

    //线程ID数组
    pthread_t *thr_id;

    //线程任务链表
    tpool_work_t *queue_head;

    pthread_mutex_t queue_lock;
    pthread_cond_t queue_ready;
}tpool_t;

//创建线程池
int tpool_create(int man_thr_num);

//销毁线程池
void tpool_destroy();

//向线程池中添加任务
//两个参数：任务函数指针和任务函数参数
int tpool_add_work(void *(*routine)(void *), void *arg);

#endif
