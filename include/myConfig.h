/*************************************************************************
	> File Name: ./include/myConfig.h
	> Author: uangyy
	> Mail:uangyy@gmail.com 
	> Created Time: Sun 17 May 2015 05:03:57 PM CST
 ************************************************************************/

#ifndef MYCONFIG_H
#define MYCONFIG_H

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <fstream>
#include <utility>
using namespace std;


//本类主要用来读取配置文件config和存储字典
class MyConfig
{
    friend class Task;
public:
    MyConfig()
    {}
    //将m_vec中第i个位置上的pair中的first元素映射到索引m_index
    void index_to_map();

    //获取存放配置文件内容的关联容器m_map
    void get_map(ifstream&);

    //将语料库中的内容读入word_map中，统计词频
    //语料库的目录路径存放在conf中的dataPath中
    void read_file();
    
    //获得词典的指针和索引的指针
    vector<pair<string, int> > *get_vec()
    {
        return &word_vec;
    }
    map<string, set<int> > *get_index()
    {
        return &m_index;
    }

    //获得cache的地址
    string get_cachePath()
    {
        return m_map["cachePath"];
    }
private:
    //存储config的文件路径
    //string m_file;
    map<string, string> m_map;  //存放配置文件的内容
    vector<pair<string, int> > word_vec; //存放字典文件的内容（字典文件以word，frequence的形式存储）
    map<string, set<int> > m_index;   //每个字母的单词下标
    //ifstream fin;   //用来关联配置文件的文件流
};

#endif
