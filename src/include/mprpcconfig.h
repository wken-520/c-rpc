#pragma once
#include<string>
#include<unordered_map>
class Mconfig{
    public:
    //加载读取配置文件
    void loadMconfig_file(std::string str);
    //查询配置文件键值对
    std::string load(std::string key);
    private:
    std::unordered_map<std::string,std::string> M_config_kv;
};