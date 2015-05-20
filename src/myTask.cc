/*************************************************************************
  > File Name: ./src/myTask.cc
  > Author: uangyy
  > Mail:uangyy@gmail.com 
  > Created Time: Mon 18 May 2015 11:21:30 PM CST
 ************************************************************************/

#include "myTask.h"
#include <iostream>
#include <unistd.h>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <pthread.h>

using namespace std;


//用动态规划的办法来计算两个字符串的编辑距离
int dist(const string &str1, const string &str2);

int min(int a, int b, int c);


//处理优先级队列，取出队列中的前十个不重复的元素，并将结果组合成字符串的形式，用来向客户端发送
string handle_result(priority_queue<MyResult, vector<MyResult>, MyCompare> &m_result);

void Task::write2client()
{
    int nwrite;
    if(m_express.size() == 0)
    {
        //收到的单词是空单词，写会一个空格
        nwrite = write(m_fd, " ", 1);
    }
    else
    {
        int nwrite = write(m_fd, m_express.c_str(), m_express.size());
        cout << "m_express: " << m_express << " size: " << m_express.size() << endl;
    }
    if(nwrite == -1)
    {
        close(m_fd);
    }
}

//void Task::execute(Cache &cache)
//版本一：没有chache
void Task::execute()
{
    cout << "m_cache: " << &m_cache << endl;
    string re = "";
    //第一步：首先查询cache中的内容，看看是否命中，命中直接返回结果
    m_cache.lock();
    if(m_express.size() != 0 && (re = m_cache.in_cache(m_express)) != "")
    {
        cout << "bingo!" << endl;
        //命中
        m_cache.unlock();
    }
    else if(m_express.size() != 0)
    {
        cout << "here" << endl;
        m_cache.unlock();
        //execute是根据vector<pair<string, int> > 和 map<string, set<int> >中的内容去查询编辑距离小于3的单词
        string::iterator str_ite = m_express.begin();   //用来遍历查询单词的迭代器
        set<int>::iterator set_ite;                     //用来遍历每一个字母所在单词的下标的迭代器
        set<int>::iterator set_end;

        while(str_ite != m_express.end())
        {
            string ch = "";
            ch += tolower(*str_ite);
            set_ite = (*m_index)[ch].begin();   //获得（比如字母a）的所在m_vec的下标
            set_end = (*m_index)[ch].end();
            while(set_ite != set_end)
            {
                int dis;
                if((dis = dist(m_express, (*m_vec)[*set_ite].first)) < 3)
                {
                    //编辑距离小于给定值，将这个查找的单词封装成一个MyResult的对象，装入优先级队列中去
                    Result ret;
                    ret.word_ = (*m_vec)[*set_ite].first;
                    ret.ifreq_ = (*m_vec)[*set_ite].second;
                    ret.idist_ = dis;
                    //cout << m_express << ", " << ret.word_ << ": " << dist(m_express, ret.word_);
                    //cout << " ret.word: " << ret.word_ << " ret.dist: " << ret.idist_ << " ret.freq: " << ret.ifreq_ << endl; 
                    m_result.push(ret);
                }
                set_ite++;
            }
            str_ite++;
        }

        //从优先级队列中取出前十个写会给客户端
        re = handle_result(m_result);
        string line_to_cache = m_express;
        line_to_cache += " ";
        line_to_cache += re;
        m_cache.lock();
        m_cache.add_to_cache(line_to_cache);
        cout << "add to cache" << endl;
        m_cache.unlock();
    }
    //cout << "execute fd is: " << &m_fd << endl;
    int nwrite;
    if(re.size() == 0)
    {
        //结果为空，写回空格
        re = " ";
        nwrite = write(m_fd, re.c_str(), re.size());
    }
    else
    {
        //结果不为空，正常写回
        nwrite = write(m_fd, re.c_str(), re.size());
    }
    //cout << "&task: " << this << "  ";
    cout << "fd: " << this ->m_fd << "nwrite: " << nwrite << "  ret: " << re << endl;
    if(nwrite == -1)
        close(m_fd);
}

//处理优先级队列，取出队列中的前十个不重复的元素，并将结果组合成字符串的形式，用来向客户端发送
string handle_result(priority_queue<MyResult, vector<MyResult>, MyCompare> &m_result)
{
    int n = 0;  //结果计数器
    vector<string> re_vec;  //保存结果的一个数组
    vector<string>::iterator re_ite;
    string re_word;         //暂时存储优先级队列的top
    while(n < 10 && !m_result.empty())
    {
        re_word = m_result.top().word_;
        //cout << "top is: " << re_word << endl;
        if(re_vec.empty())
        {
            re_vec.push_back(re_word);
            m_result.pop();
            n++;
        }
        else
        {
            //判断取出来的是否已经在vector中了
            for(re_ite = re_vec.begin(); re_ite != re_vec.end(); ++re_ite)
            {
                if(*re_ite == re_word)  //已经存在
                {
                    break;
                }
                else
                    continue;
            }
            if(re_ite == re_vec.end())      //结果不重复
            {
                //cout << "add to re_vec" << endl;
                m_result.pop();
                re_vec.push_back(re_word);
                ++n;
            }
            else
            {
                //cout << "not add  to re_vec" << endl;
                m_result.pop();
            }
        }
    }

    string result = "";
    //将vector中的结果合成字符串
    for(re_ite = re_vec.begin(); re_ite != re_vec.end(); re_ite++)
    {
        result += *re_ite;
        result += " ";
    }
    result.resize(result.size() - 1);
    return result;
}

//用动态规划的办法来计算两个字符串的编辑距离
int dist(const string &str1, const string &str2)
{
    int n = str1.size(), m = str2.size();
    int **dist; //存储dist的二维数组
    dist = new int*[n+1];     //n行
    for(int j = 0; j <= n; j ++)
    {
        dist[j] = new int[m+1];   //设置二维数组的列数
    }

    //计算dist[][]数组
    //初始化:dist[0][0] = 0; dist[0][j] = j; dist[i][0] = i
    dist[0][0] = 0;
    for(int j = 1; j <= m; j++)
    {
        dist[0][j] = j;
    }
    for(int i = 1; i <= n; i++)
    {
        dist[i][0] = i;
    }

    for(int i = 1; i <= n; i++)
    {
        for(int j = 1; j <= m; j ++)
        {
            if(str1[i-1] == str2[j-1])
                dist[i][j] = min(dist[i - 1][j] + 1, dist[i][j - 1] + 1, dist[i - 1][j - 1] + 0);
            else
                dist[i][j] = min(dist[i - 1][j] + 1, dist[i][j - 1] + 1, dist[i - 1][j - 1]+ 1);
        }
    }
    int ret = dist[n][m];
    //销毁数组
    for(int i = 0; i < n; i++)
    {
        delete[] dist[i];
    }
    return ret;
}

int min(int a, int b, int c)
{
    if(a < b)
    {
        if(a < c)
            return a;
        else
            return c;
    }
    else
    {
        if(b < c)
            return b;
        else
            return c;
    }
}
