#include"mprpcprovider.h"
#include"mprpcapplication.h"
#include"mprpcprotocol.h"
#include<google/protobuf/descriptor.h>
#include"rpcheader.pb.h"
#include"zookeeperuil.h"
#include <arpa/inet.h>
#include <utility>
//注册一个服务
void Rpcprovider::Notice(google::protobuf::Service *server ){
    serverinfo ser;
    const google::protobuf::ServiceDescriptor*prodes =server->GetDescriptor();
    //获取rpc服务的名字
    std::string name=prodes->name();
    std::cout<<name<<std::endl;
    //获取rpc服务的数量
    int methodcnt=prodes->method_count();
    for(int i=0;i<methodcnt;i++){//获取服务对象的方法来填表
       const google::protobuf::MethodDescriptor* methoddes=prodes->method(i);
       std::string str=methoddes->name();
       ser.m_methods_map.insert({str,methoddes});
    }

    // Insert once, after every method has been collected.  Inserting inside the
    // loop silently discarded every method after the first one.
    ser.m_server_one=server;
    const std::pair<std::unordered_map<std::string, serverinfo>::iterator, bool>
        result = m_servers_map.emplace(name, std::move(ser));
    if (!result.second)
    {
        std::cerr << "duplicate rpc service registration: " << name << std::endl;
    }
}
//函数执行完后的回调函数，刚得到respond
void Rpcprovider::sendRpcresult(const muduo::net::TcpConnectionPtr&T,
                                google::protobuf::Message* request,
                                google::protobuf::Message* response){
    // Response wire format: [4-byte big-endian body length][protobuf body].
    // This makes partial and coalesced TCP reads unambiguous for the client.
    std::string res;
    std::string frame;
    if(response->SerializeToString(&res) && mprpc::EncodeFrame(res, &frame)){
        T->send(frame);
    }
    else{
        std::cout<<"respond serialize error"<<std::endl;
    }

    T->shutdown();//模拟http短连接
    delete request;
    delete response;
}
void Rpcprovider::run(){
    //开启muduo网络库
    std::string ip=Mapplication::Getconfig().load("rpcserverip");
    uint16_t port=atoi(Mapplication::Getconfig().load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);
    muduo::net::TcpServer server(&mp_eventLoop,address,"RpcTcpserver");
    server.setThreadNum(4);
    ZkClient zkCli;
    zkCli.start();//链接zkserver
    for(auto &sp:m_servers_map)
    {
        //service_name
        std::string service_path ="/"+sp.first;//拼接路径
        zkCli.Create(service_path.c_str(),nullptr,0);//创建临时性节点
        for(auto &mp:sp.second.m_methods_map)
        {
            //service_name/method_name
            std::string method_path=service_path+"/"+mp.first;//拼接服务器路径和方法路径
            // sprintf(method_path_data,"%s:%d",ip.c_str(),port);//向data中写入路径
            char method_path_data[128] = {0};
            snprintf(method_path_data, sizeof(method_path_data),
                 "%s:%u", ip.c_str(), port);
            //创建节点,ZOO_EPHEMERAL表示临时节点
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }
    //设置链接时的回调函数
    server.setConnectionCallback(std::bind(&Rpcprovider::onconnection,this,std::placeholders::_1));
    //设置有消息到来时的回调函数
    server.setMessageCallback(std::bind(&Rpcprovider::onmessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    server.start();
    std::cout<<"Rpcprovider start now at "<<ip<<" :"<<port<<std::endl;
    mp_eventLoop.loop();
}
void Rpcprovider::onconnection(const muduo::net::TcpConnectionPtr&con){

}
//处理从网络接收到的数据，根据规定好的Tcp协议拆分
void Rpcprovider::onmessage(const muduo::net::TcpConnectionPtr&conn,muduo::net::Buffer*buf,muduo::Timestamp){

    constexpr uint32_t kPrefixSize = sizeof(uint32_t);
    constexpr uint32_t kMaxHeaderSize = 64 * 1024;
    constexpr uint32_t kMaxArgsSize = 16 * 1024 * 1024;

    // 一次回调中可能收到了多个完整 RPC 包，因此循环处理。
    while (true)
    {
        // 包头长度字段尚未收全。
        if (buf->readableBytes() < kPrefixSize)
        {
            return;
        }

        uint32_t network_header_size = 0;
        std::memcpy(&network_header_size, buf->peek(), kPrefixSize);
        const uint32_t header_size = ntohl(network_header_size);

        if (header_size == 0 || header_size > kMaxHeaderSize)
        {
            std::cerr << "invalid rpc header size: "
                      << header_size << std::endl;
            conn->shutdown();
            return;
        }

        // 固定长度字段 + protobuf header 尚未收全。
        if (buf->readableBytes() < kPrefixSize + header_size)
        {
            return;
        }

        mprpc::rpcheader rpc_header;
        const char* header_data = buf->peek() + kPrefixSize;
        if (!rpc_header.ParseFromArray(header_data,
                                       static_cast<int>(header_size)))
        {
            std::cerr << "rpc header parse failed" << std::endl;
            conn->shutdown();
            return;
        }

        const std::string& service_name = rpc_header.server_name();
        const std::string& method_name = rpc_header.method_name();
        const uint32_t args_size = rpc_header.args_size();

        if (args_size > kMaxArgsSize)
        {
            std::cerr << "rpc args too large: " << args_size << std::endl;
            conn->shutdown();
            return;
        }

        const size_t total_size =
            static_cast<size_t>(kPrefixSize) +
            static_cast<size_t>(header_size) +
            static_cast<size_t>(args_size);

        // 参数部分尚未收全：保留 Buffer，等待下次 onmessage。
        if (buf->readableBytes() < total_size)
        {
            return;
        }

        std::string args(buf->peek() + kPrefixSize + header_size,
                         args_size);

        auto service_it = m_servers_map.find(service_name);
        if (service_it == m_servers_map.end())
        {
            std::cerr << "service not found: " << service_name << std::endl;
            conn->shutdown();
            return;
        }

        auto method_it =
            service_it->second.m_methods_map.find(method_name);
        if (method_it == service_it->second.m_methods_map.end())
        {
            std::cerr << "method not found: " << method_name << std::endl;
            conn->shutdown();
            return;
        }

        google::protobuf::Service* service =
            service_it->second.m_server_one;
        const google::protobuf::MethodDescriptor* method =
            method_it->second;

        google::protobuf::Message* request =
            service->GetRequestPrototype(method).New();

        // 注意：ParseFromString 成功时返回 true。
        if (!request->ParseFromString(args))
        {
            std::cerr << "request parse failed" << std::endl;
            delete request;
            conn->shutdown();
            return;
        }

        google::protobuf::Message* response =
            service->GetResponsePrototype(method).New();

        // 先消费完整帧；CallMethod 可能同步完成并触发回调。
        buf->retrieve(total_size);

        google::protobuf::Closure* done =
            google::protobuf::NewCallback<
                Rpcprovider,
                const muduo::net::TcpConnectionPtr&,
                google::protobuf::Message*,
                google::protobuf::Message*
            >(
                this,
                &Rpcprovider::sendRpcresult,
                conn,
                request,
                response
            );

        service->CallMethod(method, nullptr, request, response, done);

        // Do not delete request/response here: an async service can use them
        // until done->Run(). sendRpcresult releases both objects.
    }
}
    //读取网络传输过来的buf,并且处理TCP粘包的问题
    //std::string reve_buf=buf->retrieveAllAsString();
    /*函数	作用	是否消费 Buffer
    peek()	查看数据起始位置	否
    retrieve(n)	消费指定长度的数据	是
    retrieveAllAsString()	取出全部数据并消费	是
    */
    //uint32_t header_size;
    //前4个字节是header_size，通过string.copy的方法直接按二进制赋值
    //reve_buf.copy((char*)&header_size,4,0);
    // 先判断 Buffer 中是否至少有 4 字节
//     if (buf->readableBytes() < 4)
//         {
//          return;
//         }

// // 读取前 4 字节，但不消费 Buffer
//     uint32_t network_header_size = 0;
//     memcpy(
//         &network_header_size,
//         buf->peek(),
//         sizeof(network_header_size)
//         );
//         // 限制 Header 最大长度
//     constexpr uint32_t MAX_HEADER_SIZE = 64 * 1024;
//     //网络字节序转换
//         uint32_t header_size = ntohl(network_header_size);
//     if (header_size == 0 || header_size > MAX_HEADER_SIZE)
//     {
//     std::cout << "invalid header size" << std::endl;
//     mes->shutdown();
//     return;
//     }

// // 判断 Header 是否完整
//     if (buf->readableBytes() < 4 + header_size)
//     {
//         return;
//     }

//     mprpc::rpcheader rpc_header;

// const char* header_data = buf->peek() + 4;

// if (!rpc_header.ParseFromArray(
//        header_data,
//         static_cast<int>(header_size)))
// {
//     mes->shutdown();
//     return;
// }
//       std::string server_name=rpc_header.server_name();
//       std::string method_name=rpc_header.method_name();
//       uint32_t args_size=rpc_header.args_size();
//     // mprpc::rpcheader rpcheader;
//     // std::string str_header=reve_buf.substr(4,header_size);
//     // std::string server_name;
//     // std::string method_name;
//     // uint32_t args_size;
//     // //通过rpcheader进行反序列化，读取成员
//     // if(rpcheader.ParseFromString(str_header)){
//     //     server_name=rpcheader.server_name();
//     //     method_name=rpcheader.method_name();
//     //     args_size=rpcheader.args_size();
//     // }
//     // //反序列化失败
//     // else{

//     // }
//     //调试信息
//     std::cout<<server_name<<std::endl;
//     std::cout<<method_name<<std::endl;
//     std::cout<<args_size<<std::endl;
//     //进行函数参数的拆解,之后proto会自动拆分每个参数
//     //std::string arg_=header_data.substr(4+header_size,args_size);
//     std::string arg_(
//     buf->peek() + 4 + header_size,
//     args_size
// );  
        
//     //之前注册的server map
//     //查看是否注册了当前的服务
//     auto it=m_servers_map.find(server_name);
//     if(it==m_servers_map.end()){
//         std::cout<<"this server is not found"<<std::endl;
//         return ;
//     }
//     google::protobuf::Service*server =it->second.m_server_one;
//    //查看注册的服务中是否包含了当前方法
//    auto mit=it->second.m_methods_map.find(method_name);
//    if(mit==it->second.m_methods_map.end()){
//     std::cout<<"this method is not found"<<std::endl;
//     return ;
//    }
//    const google::protobuf::MethodDescriptor* method_=mit->second;
//    //通过server生成request 和 respond
//    google::protobuf::Message* request=server->GetRequestPrototype(method_).New();
//    //自动拆解函数参数
//    if(request->ParseFromString(arg_)){
//     std::cout<<"request false"<<std::endl;
//    }
//    google::protobuf::Message* respond=server->GetResponsePrototype(method_).New();
//    //接着就可以调用方法了
//    //绑定回调
//    google::protobuf::Closure* done=google::protobuf::NewCallback<Rpcprovider,
//                                             const muduo::net::TcpConnectionPtr&,
//                                             google::protobuf::Message*>
//                                             (this,&Rpcprovider::sendRpcresult,mes,respond);
//     //绑定好就执行，全套流程已完成
//     server->CallMethod(method_,nullptr,request,respond,done);
// }
