/*************************************************************************
	> File Name: include/myCache.h
	> Author: uangyy
	> Mail:uangyy@gmail.com 
	> Created Time: Tue 19 May 2015 10:41:29 PM CST
 ************************************************************************/

#ifndef MYCACHE_H
#define MACACHE_H

#include <iostream>
#include <map>
#include <fstream>
#include <pthread.h>

using namespace std;

class Cache
{
public:
    Cache()
    {
    }
    void set(const string &path, pthread_mutex_t &lock)
    {
        m_path = path;
        cache_lock = lock;
    }

    //从给定的文件流中读取cache
    void get_cache();

    //判断，给定的单词是否在cache中，若是的话返回该字符串，不是的话返回NULL
    string in_cache(const string&);

    //将cache中的内容写回到文件中去
    void write_back();

    //向m_cache中添加一条
    void add_to_cache(const string &);

    //上锁和解锁
    void lock()
    {
        pthread_mutex_lock(&cache_lock);
    }
    void unlock()
    {
        pthread_mutex_unlock(&cache_lock);
    }
private:
    string m_path;                    //保存cache的路径
    map<string, string> m_cache;    //保存这<查询词，查询结果> 的一个map
    pthread_mutex_t cache_lock;    //cache锁
};

#endif
