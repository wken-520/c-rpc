#pragma once
#include<memory>
#include"google/protobuf/service.h"
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<string>
#include<functional>
#include<unordered_map>
class Rpcprovider{
    public:
    //传父类，继承多态
    void Notice(google::protobuf::Service *server);
    //开启rpc框架的运行
    void run();
    private:
    std::unique_ptr<muduo::net::TcpServer> mp_Tcpserver_ptr;
    muduo::net::EventLoop mp_eventLoop;
    void onconnection(const muduo::net::TcpConnectionPtr&con);
    void onmessage(const muduo::net::TcpConnectionPtr&mes,muduo::net::Buffer*,muduo::Timestamp);
    struct serverinfo{//单个服务
    //保存服务对象
     google::protobuf::Service* m_server_one;
    //保存对象的方法
    std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methods_map;
    };
    //全部封装好的服务对象
    std::unordered_map<std::string,serverinfo> m_servers_map;
    //定义当rpc函数执行完后的回调
    void sendRpcresult(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);
};