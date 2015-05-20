/*************************************************************************
  > File Name: ./src/myCache.cc
  > Author: uangyy
  > Mail:uangyy@gmail.com 
  > Created Time: Tue 19 May 2015 10:50:20 PM CST
 ************************************************************************/

#include "myCache.h"
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <utility>
#include <cstdlib>
#include <string>
using namespace std;

//从给定的文件流中读取cache
void Cache::get_cache()
{
    ifstream is;
    is.open(m_path.c_str());
    cout << m_path << endl;
    if(!is)
    {
        cout << "open file err" << endl;
        exit(EXIT_FAILURE);
    }
    string line, key, val;
    while(getline(is, line))
    {
        istringstream iss(line);
        iss >> key;
        val.assign(line, key.size() + 1, line.size() - key.size() - 1);
        m_cache.insert(make_pair(key, val));
    }
    is.close();
}


//判断给定的单词是否在cache中，若是的话返回该字符串
string Cache::in_cache(const string &word)
{
    map<string, string>::iterator map_ite;
    if((map_ite = m_cache.find(word)) != m_cache.end())
    {
        //找到了单词
        return map_ite->second;
    }
    else
        return "";
}

//将cache中的内容写回到文件中去
void Cache::write_back()
{
    ofstream os;
    os.open(m_path.c_str());
    if(!os)
    {
        cout << "open file err!" << endl;
        exit(EXIT_FAILURE);
    }
    map<string, string>::iterator map_ite = m_cache.begin();
    while(map_ite != m_cache.end())
    {
        string line, key, val;
        key = map_ite->first;
        val = map_ite->second;
        line = "";
        line += key;
        line +=" ";
        line += val;
        os << line << endl;
        map_ite++;
    }
    os.close();
}

//向m_cache中添加一条
void Cache::add_to_cache(const string &line)
{
    string key, val;
    istringstream iss(line);
    iss >> key;
    val.assign(line, key.size() + 1, line.size() - key.size() - 1);
    m_cache.insert(make_pair(key, val));
}
