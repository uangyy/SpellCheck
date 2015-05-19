/*************************************************************************
  > File Name: src/thread_pool.cc
  > Author: uangyy
  > Mail:uangyy@gmail.com 
  > Created Time: Sun 17 May 2015 11:33:55 AM CST
 ************************************************************************/

#include "thread_pool.h"
#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <string>
#include <errno.h>
#include <cstdio>
#include <cstring>
using namespace std;

//线程池
static tpool_t *tpool = NULL;

//任务函数，从任务链表中取出任务并执行
//函数被创建就要一直循环执行的（就是一个线程）
static void *thread_routine(void *arg)
{
    //线程要执行的工作函数，就是真正想要他来计算结果的函数work
    tpool_work_t *work = NULL;
    while(1){
        //如果线程池没有被撤销并且线程池任务列表中没有任务，则挂起
        pthread_mutex_lock(&tpool->queue_lock);
        while(!tpool->shutdown && !tpool->queue_head)
        {
            pthread_cond_wait(&tpool->queue_ready, &tpool->queue_lock);
        }

        if(tpool->shutdown)
        {
            pthread_mutex_unlock(&tpool->queue_lock);
            pthread_exit(NULL);
        }
        //从工作列表tpool->queue_head中取出work执行
        work = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        pthread_mutex_unlock(&tpool->queue_lock);

        cout << "i am thread: " << pthread_self() << endl;
        work->routine(work->arg);
        free(work);
    }

}

//创建线程池
int tpool_create(int max_thr_num)
{
    int i;
    tpool = (tpool_t *)calloc(1, sizeof(tpool_t));
    if(!tpool)
    {
        printf("%s: calloc failed\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }

    //初始化
    tpool->max_thr_num = max_thr_num;
    tpool->shutdown = false;
    tpool->queue_head = NULL;
    if(pthread_mutex_init(&tpool->queue_lock, NULL) != 0)
    {
        printf("%s: pthread_mutex_init failed, errno: %d, error:%s\n", __FUNCTION__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    //创建工作者线程
    tpool->thr_id = (pthread_t *)calloc(max_thr_num, sizeof(pthread_t));
    if(!tpool->thr_id)
    {
        printf("%s: calloc failed\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }

    for(i = 0; i != max_thr_num; ++i)
    {
        if(pthread_create(&tpool->thr_id[i], NULL, thread_routine, NULL) != 0)
        {
            printf("%s: ptread_create failed\n", __FUNCTION__);
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

//销毁线程池
void tpool_destroy()
{
    int i;
    tpool_work_t *member;
    if(tpool->shutdown)
        return;
    tpool->shutdown = true;

    //通知所有正在等待的线程
    pthread_mutex_lock(&tpool->queue_lock);
    pthread_cond_broadcast(&tpool->queue_ready);
    pthread_mutex_unlock(&tpool->queue_lock);

    for(int i = 0; i != tpool->max_thr_num; ++i)
    {
        pthread_join(tpool->thr_id[i], NULL);
    }
    free(tpool->thr_id);
    while(tpool->queue_head)
    {
        member = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        free(member);
    }

    pthread_mutex_destroy(&tpool->queue_lock);
    pthread_cond_destroy(&tpool->queue_ready);
    free(tpool);
}



//想线程池中添加任务
int tpool_add_work(void *(routine)(void *), void *arg)
{
    tpool_work_t *work, *member;

    if(!routine)
    {
        printf("Invalid argument\n");
        return -1;
    }

    work = (tpool_work_t *)malloc(sizeof(tpool_work_t));
    if(!work)
    {
        printf("malloc failed\n");
        return -1;
    }
    work->routine = routine;
    work->arg = arg;
    work->next = NULL;

    pthread_mutex_lock(&tpool->queue_lock);
    member = tpool->queue_head;
    //线程池的任务列表是空的，直接插入
    if(!member)
    {
        tpool->queue_head = work;
    }
    else        //线程池的任务列表不是空的，在尾部插入
    {
        while(member->next)
            member = member->next;
        member->next = work;
    }

    //通知所有工作者线程，有新任务添加
    pthread_cond_signal(&tpool->queue_ready);
    pthread_mutex_unlock(&tpool->queue_lock);

}
