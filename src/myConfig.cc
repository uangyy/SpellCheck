/*************************************************************************
  > File Name: src/myConfig.cc
  > Author: uangyy
  > Mail:uangyy@gmail.com 
  > Created Time: Sun 17 May 2015 05:14:36 PM CST
 ************************************************************************/

#include "myConfig.h"
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
using namespace std;

void MyConfig::index_to_map()
{
    vector<pair<string, int> >::const_iterator vec_ite = word_vec.begin();
    map<string, set<int> >::iterator i_ite;
    int n = 0;
    while(vec_ite != word_vec.end())
    {
        string word = vec_ite->first;
        string::iterator str_ite = word.begin();
        while(str_ite != word.end())
        {
            string ch = "";
            ch += *str_ite;
            if((i_ite = m_index.find(ch)) != m_index.end())   //在m_index中找到了字符
            {
                i_ite->second.insert(n);
            }
            else                                                    //没有找到
            {
                set<int> set;
                set.insert(n);
                m_index.insert(make_pair(ch, set));
            }
            str_ite++;
        }
        vec_ite++;
        n++;
    }
    //测试index_map
    ofstream os;
    os.open("/home/uangyy/CPP/Project/ProSpellCheck/tmp/ret_index");
    map<string, set<int> >::const_iterator map_ite = m_index.begin();
    while(map_ite != m_index.end())
    {
        os << map_ite->first << ": ";
        set<int>::const_iterator set_ite = map_ite->second.begin();
        while(set_ite != map_ite->second.end())
        {
            os << *set_ite << " ";
            set_ite++;
        }
        os << endl;
        map_ite++;
    }
}

//读取配置文件的内容，并保存在m_map 中
void MyConfig::get_map(ifstream &is)
{
    string line;
    while(getline(is, line))
    {
        //将:变为空格
        string::size_type pos = line.find(':');
        line[pos] = ' ';
        istringstream iss(line);
        string key, val;
        iss >> key >> val;
        m_map.insert(make_pair(key, val));
        cout << key << ":" << val << endl;
    }
}

//读取语料库，统计词频，并保存在word_map<string, int>中。
//语料库的目录路径在conf的dataPath
//使用的是UTF-8的编码方式，所以要注意读入字节的长度
void MyConfig::read_file()
{
    string dir_path = m_map["dataPath"];
    map<string, int> word_map;
    //遍历dataPath目录下的所有文件
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    ifstream is;

    if((dp = opendir(dir_path.c_str())) == NULL)
    {
        printf("can not open a directory\n");
        return;
    }

    chdir(dir_path.c_str());
    while((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &statbuf);
        //发现一个文件夹
        if(S_ISDIR(statbuf.st_mode))
        {
            continue;
        }
        else
        {
            //打开文件
            is.open(entry->d_name);
            string line, word;
            while(getline(is, line))
            {
                //将标点变为空格

                string newline;
                string::iterator it = line.begin();
                for( ; it != line.end(); it++)
                {
                    if(!ispunct(*it))   //不是标点或分隔符
                    {
                        newline += tolower(*it);
                    }
                    else
                    {
                        newline += " ";
                        continue;
                    }
                }

                //处理一行，读入word_map中,用来保存词典中的每一个单词
                istringstream iss(newline);
                while(iss >> word)
                {
                    word_map[word] ++;
                }
            }
            is.close();
        }
    }
    //返回原来的工作目录
    chdir("-");

    //将word_map中的每一项都加入到word_vec中去
    map<string, int>::const_iterator ite = word_map.begin();
    while(ite != word_map.end())
    {
        word_vec.push_back(*ite);
        ite++;
    }

    ofstream os;
    os.open("/home/uangyy/CPP/Project/ProSpellCheck/tmp/ret");
    if(os == NULL)
    {
        cout << "err" << endl;
        return;
    }
    //测试读入的map
    vector<pair<string, int> >::const_iterator vec_ite = word_vec.begin();
    cout << "===========" << endl;
    while(vec_ite != word_vec.end())
    {
        os << vec_ite->first << ": " << vec_ite->second << endl;
        vec_ite++;
    }
    os.close();
}
