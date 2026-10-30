#pragma once
#include"google/protobuf/service.h"
//对于调用rpc方法的方式进行统一管理
class Rpcchannel :public google::protobuf::RpcChannel{
    void CallMethod(const google::protobuf::MethodDescriptor*method,
                          google::protobuf::RpcController*controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);

};