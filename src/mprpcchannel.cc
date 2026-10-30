#include"mprpcchannel.h"
#include"google/protobuf/descriptor.h"
#include"rpcheader.pb.h"
#include<sys/socket.h>
#include <arpa/inet.h>
#include"mprpcapplication.h"
#include<unistd.h>
#include"zookeeperuil.h"
//当用户发起方法的调用时，通过callmethod对调用做统一的管理
  void Rpcchannel::CallMethod(const google::protobuf::MethodDescriptor*method,
                          google::protobuf::RpcController*controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done)
{
    // 获取header_size+server_name+method_name+args_size; 完成rpcheader的构建
    
    const google::protobuf::ServiceDescriptor* server=method->service();
    std::string server_name=server->name();
    std::string method_name=method->name();
    std::string str;
    request->SerializeToString(&str);
    mprpc::rpcheader header;
    header.set_method_name(method_name);
    header.set_server_name(server_name);
    int args_size=str.size();
    header.set_args_size(args_size);
    int header_size;
    std::string str1;
    header.SerializeToString(&str1);
    header_size=str1.size();
    std::string result="";
    result.insert(0,(char*)&header_size,4);
    result+=str1;
    result+=str;

    //向服务器发送请求 通过TCP向对方发送方法，等待结果的返回
    int client_fd=socket(AF_INET,SOCK_STREAM,0);
    if(client_fd==-1){
        std::cout<<"error creat socket"<<std::endl;
        controller->SetFailed("error creat socket");
        close(client_fd);
        return ;
    }
    // std::string ip=Mapplication::Getconfig().load("rpcserverip");
    // uint16_t port=atoi(Mapplication::Getconfig().load("rpcserverport").c_str())
     ZkClient zkCli;
    zkCli.start();
    std::string method_path="/"+server_name+"/"+method_name;
    //获取ip地址和端口号
    std::string host_data=zkCli.GetData(method_path.c_str());
    if(host_data=="")
    {
        controller->SetFailed(method_path+" is not exist!");
        return;
    }
    int idx=host_data.find(":");//分割符
    if(idx==-1)
    {
        controller->SetFailed(method_path+" address is invalid!");
        return;
    };
      std::string ip=host_data.substr(0,idx);
    uint32_t port=atoi(host_data.substr(idx+1,host_data.size()-idx).c_str());
    struct sockaddr_in server_in;
    server_in.sin_family=AF_INET;
     server_in.sin_port=htons(port);
    server_in.sin_addr.s_addr=inet_addr(ip.c_str());
    if(-1==connect(client_fd,(struct sockaddr*)&server_in,sizeof(server_in))){
        std::cout<<"error connect"<<std::endl;
        controller->SetFailed("error connect");
        close(client_fd);
        return ;
    }
    size_t total = 0;
    //循环发送确保发完
while (total < result.size()) {
    ssize_t n = send(client_fd, result.c_str() + total, result.size() - total, 0);
    if (n <= 0) {
        perror("send");
        break;
    }
    total += n;
}
    std::cout<<"send success"<<std::endl;
    //接受服务端的返回
    char buf[1024];
    int nums=0;
    if((nums=recv(client_fd,buf,sizeof(buf),0))==-1){
        std::cout<<" recv error"<<std::endl;
        controller->SetFailed(" recv error");
        close(client_fd);
        return ;
    }
    std::string respond_str(buf,nums);
    if(!response->ParseFromString(respond_str)){
        std::cout<<"parsefrom error"<<std::endl;
        controller->SetFailed("parsefrom error");
        close(client_fd);
        return ;
    }
    close(client_fd);
}
