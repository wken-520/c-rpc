//用于判断rpc服务的状态
#pragma once
#include<string>
#include"google/protobuf/service.h"
class MyRpccontroller:public google::protobuf::RpcController{
    public:
    MyRpccontroller();
    void Reset() ;
    bool Failed() const;
    std::string ErrorText() const ;
    void StartCancel();
    void SetFailed(const std::string& reason) ;
    bool IsCanceled() const ;
    void NotifyOnCancel(google::protobuf::Closure* callback) ;
    private:
        bool M_failed;//判断rpc成功与否
        std::string errortext;//错误信息
};