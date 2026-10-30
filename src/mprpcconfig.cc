//加载配置文件，读取键值对
#include "mprpcconfig.h"
#include<fstream>
#include<iostream>
static void trim(std::string& s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
        s.erase(s.begin());
    }
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) {
        s.pop_back();
    }
}
void Mconfig::loadMconfig_file(std::string str) {
    std::ifstream fs(str);
    if(!fs){
        std::cout<<"fs defaule"<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::string line;
    while (std::getline(fs, line)) {
        // 去掉 Windows 换行 \r
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') continue;

        // 找 '='
        std::size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key   = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        trim(key);
        trim(value);

        if (key.empty()) continue;

        M_config_kv[key] = value;
    }
    fs.close();
}
std::string Mconfig::load(std::string key){
    auto it=M_config_kv.find(key);
    if(it==M_config_kv.end()){
        return "";
    }
    return M_config_kv[key];
}