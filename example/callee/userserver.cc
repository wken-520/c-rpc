#include"mprpcapplication.h"
#include<iostream>
#include"user.pb.h"
//框架的模拟使用，首先继承框架的类并重写调用
class Userver:public fixbug::userrpd  {
    //设计本地服务
    bool login(std::string name,std::string pwd){
        std::cout<<name<<std::endl;
        std::cout<<pwd<<std::endl;
        return true;
    }
    //想要注册一个服务，需要重写
    void login(::google::protobuf::RpcController* controller,
                       const ::fixbug::request* request,
                       ::fixbug::respond* response,
                       ::google::protobuf::Closure* done){
                         std::string name=request->name();
                         std::string pwd=request->pwd();
                        //调用本地服务
                         bool loginflag=login(name,pwd);
            fixbug::respondnode *code=response->mutable_result();
            code->set_errors("");
            code->set_errornum(0);
            response->set_success(loginflag);
            done->Run();
     }

};
int main(int argc,char**argv){
    //启动需要读配置文件
    Mapplication::Init(argc,argv);
    Rpcprovider rpcprovider;
    //注册服务
    rpcprovider.Notice(new Userver);
    rpcprovider.run();
    return 0;
}