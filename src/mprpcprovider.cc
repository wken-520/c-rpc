#include"mprpcprovider.h"
#include"mprpcapplication.h"
#include<google/protobuf/descriptor.h>
#include"rpcheader.pb.h"
#include"zookeeperuil.h"
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
    //完成一个服务和其方法的封装
    ser.m_server_one=server;
    //加入一个新的封装服务
    m_servers_map.insert({name,ser});
    }
}
//函数执行完后的回调函数，刚得到respond
void Rpcprovider::sendRpcresult(const muduo::net::TcpConnectionPtr&T,google::protobuf::Message*M){
    //进行respond的序列化
    std::string res;
    if(M->SerializeToString(&res)){
        //将结果返回
        T->send(res);
        T->shutdown();//模拟http短连接
    }
    else{
        std::cout<<"respond serialize error"<<std::endl;
        T->shutdown();
    }
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
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);//向data中写入路径

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
void Rpcprovider::onmessage(const muduo::net::TcpConnectionPtr&mes,muduo::net::Buffer*buf,muduo::Timestamp){
    //读取网络传输过来的buf,并且处理TCP粘包的问题
    std::string reve_buf=buf->retrieveAllAsString();
    uint32_t header_size;
    //前4个字节是header_size，通过string.copy的方法直接按二进制赋值
    reve_buf.copy((char*)&header_size,4,0);
    mprpc::rpcheader rpcheader;
    std::string str_header=reve_buf.substr(4,header_size);
    std::string server_name;
    std::string method_name;
    uint32_t args_size;
    //通过rpcheader进行反序列化，读取成员
    if(rpcheader.ParseFromString(str_header)){
        server_name=rpcheader.server_name();
        method_name=rpcheader.method_name();
        args_size=rpcheader.args_size();
    }
    //反序列化失败
    else{

    }
    //调试信息
    std::cout<<server_name<<std::endl;
    std::cout<<method_name<<std::endl;
    std::cout<<args_size<<std::endl;
    //进行函数参数的拆解,之后proto会自动拆分每个参数
    std::string arg_=reve_buf.substr(4+header_size,args_size);
    //之前注册的server map
    //查看是否注册了当前的服务
    auto it=m_servers_map.find(server_name);
    if(it==m_servers_map.end()){
        std::cout<<"this server is not found"<<std::endl;
        return ;
    }
    google::protobuf::Service*server =it->second.m_server_one;
   //查看注册的服务中是否包含了当前方法
   auto mit=it->second.m_methods_map.find(method_name);
   if(mit==it->second.m_methods_map.end()){
    std::cout<<"this method is not found"<<std::endl;
    return ;
   }
   const google::protobuf::MethodDescriptor* method_=mit->second;
   //通过server生成request 和 respond
   google::protobuf::Message* request=server->GetRequestPrototype(method_).New();
   //自动拆解函数参数
   if(request->ParseFromString(arg_)){
    std::cout<<"request false"<<std::endl;
   }
   google::protobuf::Message* respond=server->GetResponsePrototype(method_).New();
   //接着就可以调用方法了
   //绑定回调
   google::protobuf::Closure* done=google::protobuf::NewCallback<Rpcprovider,
                                            const muduo::net::TcpConnectionPtr&,
                                            google::protobuf::Message*>
                                            (this,&Rpcprovider::sendRpcresult,mes,respond);
    //绑定好就执行，全套流程已完成
    server->CallMethod(method_,nullptr,request,respond,done);
}