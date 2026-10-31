#include"mprpcchannel.h"
#include"google/protobuf/descriptor.h"
#include"rpcheader.pb.h"
#include<sys/socket.h>
#include <arpa/inet.h>
#include"mprpcapplication.h"
#include<unistd.h>
#include"zookeeperuil.h"
#include <poll.h>
#include <cerrno>
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
    if(!request->SerializeToString(&str)){
        controller->SetFailed("request serialize error");
        return ;
    }
    mprpc::rpcheader header;
    header.set_method_name(method_name);
    header.set_server_name(server_name);
    int args_size=str.size();
    header.set_args_size(args_size);
    // int header_size;
    // std::string str1;
    // header.SerializeToString(&str1);
    // header_size=str1.size();
    // std::string result="";
    // result.insert(0,(char*)&header_size,4);
    // result+=str1;
    // result+=str;
    // 序列化 RPC Header
        std::string str1;
        if (!header.SerializeToString(&str1))
            {
            controller->SetFailed("header serialize error");
            return;
            }

    // Header 长度转换为网络字节序
        uint32_t header_size = static_cast<uint32_t>(str1.size());
        uint32_t network_header_size = htonl(header_size);

// 组装完整 RPC 请求
        std::string result;
        result.append(reinterpret_cast<const char*>(&network_header_size), 4);
        result += str1;
        result += str;
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
//     size_t total = 0;
//     //循环发送确保发完
// while (total < result.size()) {
//     ssize_t n = send(client_fd, result.c_str() + total, result.size() - total, 0);
//     if (n <= 0) {
//         perror("send");
//         break;
//     }
//     total += n;
// }
    size_t total = 0;

while (total < result.size())
{
    ssize_t n = send(
        client_fd,
        result.data() + total,
        result.size() - total,
        MSG_NOSIGNAL
    );

    if (n > 0)
    {
        total += static_cast<size_t>(n);
    }
    else if (n == -1 && errno == EINTR)
    {
        continue;
    }
    else
    {
        perror("send");
        controller->SetFailed("send request error");
        close(client_fd);
        return;
    }
}
    std::cout<<"send success"<<std::endl;
    // 等待服务端响应，单位：毫秒。
// 建议后续改为从配置文件读取。
//通过poll的参数实现了超时控制
constexpr int kRpcTimeoutMs = 3000;

pollfd pfd{};
pfd.fd = client_fd;
pfd.events = POLLIN;

int poll_result;
do
{
    poll_result = poll(&pfd, 1, kRpcTimeoutMs);
} while (poll_result == -1 && errno == EINTR);

if (poll_result == 0)
{
    controller->SetFailed("rpc response timeout");
    close(client_fd);
    return;
}

if (poll_result < 0)
{
    controller->SetFailed("wait rpc response error");
    close(client_fd);
    return;
}

if ((pfd.revents & (POLLERR | POLLNVAL)) != 0)
{
    controller->SetFailed("rpc connection error");
    close(client_fd);
    return;
}

char buf[1024];
ssize_t nums;
do
{
    nums = recv(client_fd, buf, sizeof(buf), 0);
} while (nums == -1 && errno == EINTR);

if (nums < 0)
{
    controller->SetFailed("recv rpc response error");
    close(client_fd);
    return;
}

if (nums == 0)
{
    controller->SetFailed("rpc server closed connection");
    close(client_fd);
    return;
}

std::string response_str(buf, static_cast<size_t>(nums));

if (!response->ParseFromString(response_str))
{
    controller->SetFailed("parse rpc response error");
    close(client_fd);
    return;
}

close(client_fd);

if (done != nullptr)
{
    done->Run();
}
    //接受服务端的返回
    // char buf[1024];
    // int nums=0;
    // if((nums=recv(client_fd,buf,sizeof(buf),0))==-1){
    //     //std::cout<<" recv error"<<std::endl;
    //     controller->SetFailed(" recv error");
    //     close(client_fd);
    //     return ;
    // }
    // std::string respond_str(buf,nums);
    // if(!response->ParseFromString(respond_str)){
    //     //std::cout<<"parsefrom error"<<std::endl;
    //     controller->SetFailed("parsefrom error");
    //     close(client_fd);
    //     return ;
    // }
    // close(client_fd);
}
