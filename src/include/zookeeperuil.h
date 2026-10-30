#pragma once
#include<zookeeper/zookeeper.h>
#include<string>
class ZkClient{
    public:
    ZkClient();
    ~ZkClient();
    void start();
    std::string GetData(const char *path);
    void Create(const char* path, const char* data,
                int datalen, int state = 0);    
    private:
      zhandle_t*  m_zhandle;
};