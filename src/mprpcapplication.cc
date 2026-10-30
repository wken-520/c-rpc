#include"mprpcapplication.h"
#include<iostream>
#include<string>
#include<unistd.h>
#include"mprpcconfig.h"
 Mconfig Mapplication:: m_config;
void showArgshelp(){
    std::cout<<"format command -i <configfile> "<<std::endl;
}
Mapplication& Mapplication::Getline(){
    static Mapplication app;
    return app;
}
//初始化
void Mapplication::Init(int argc,char * * argv){
    if(argc<2){
        showArgshelp();
        exit(EXIT_FAILURE);
    }
    int opt=0;
    std::string config_file;
    while((opt=getopt(argc,argv,"i:"))!=-1){
        switch(opt){
            case 'i': 
            config_file=optarg;
            break;
            case '?':
            showArgshelp;
            exit(EXIT_FAILURE);
             case ':':
            showArgshelp;
            exit(EXIT_FAILURE);
            default:
            break;
        }
    }
    //加载配置文件
    m_config.loadMconfig_file(config_file.c_str());
   std::cout<<m_config.load( "rpcserverip")<<std::endl;
   std::cout<<m_config.load( "zookeeperip")<<std::endl;
}
Mconfig& Mapplication::Getconfig(){
    return m_config;
}