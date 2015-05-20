/*************************************************************************
  > File Name: include/myTask.h
  > Author: uangyy
  > Mail:uangyy@gmail.com 
  > Created Time: Mon 18 May 2015 09:38:33 PM CST
 ************************************************************************/
#ifndef MYTASK_H
#define MYTASK_H

#include "myConfig.h"
#include "myCache.h"
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <set>
#include <vector>
#include <stack>
#include <queue>
using namespace std;

typedef struct MyResult
{
    string word_;
    int ifreq_;
    int idist_;
}Result;

struct MyCompare
{
    bool operator()(const MyResult &lhs, const MyResult &rhs)
    {
        if(lhs.idist_ > rhs.idist_)
            return true;
        else if(lhs.idist_ == rhs.idist_ && 
                lhs.ifreq_ < rhs.ifreq_)
            return true;
        else if(lhs.idist_ == rhs.idist_ &&
                lhs.ifreq_ == rhs.ifreq_ &&
                lhs.word_ > rhs.word_)
            return true;
        else
            return false;
    }
};

class Task
{
public:
    Task(const string &expr, int sockfd, vector<pair<string, int> > *vec, map<string, set<int> > *index, Cache &cache)
        :m_express(expr), m_fd(sockfd), m_vec(vec), m_index(index), m_cache(cache)
    {
        //要取出单词最后一个换行！
        if(m_express[m_express.size() - 1] == '\n')
            m_express.resize(m_express.size() - 1);
    }
    Task(const char *expr, int sockfd, vector<pair<string, int> > *vec, map<string, set<int> > *index, Cache &cache)
        :m_express(expr), m_fd(sockfd), m_vec(vec), m_index(index), m_cache(cache)
    {
        //要取出单词最后一个换行！
        if(m_express[m_express.size() - 1] == '\n')
            m_express.resize(m_express.size() - 1);
    }

    //void Task::execute(Cache &cache)
    //版本一：没有chache
    //处理查找单词的函数
    void execute();

    //测试代码
    void write2client();
private:
    void query_idx_table();
    void statistic(set<int> &iset);
    int distance(const string &rhs);
    void response(Cache &cache);
private:
    string m_express;  //保存用户输入的查询单词
    int m_fd;   //与用户通信的socket描述符
    vector<pair<string, int> > *m_vec;   //指向保存数据词典的指针
    map<string, set<int> > *m_index;    //的指针
    priority_queue<MyResult, vector<MyResult>, MyCompare> m_result; //保存查询结果的优先级队列
    Cache &m_cache;    //一个cache
};

#endif
