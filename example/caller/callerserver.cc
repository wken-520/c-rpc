#include<iostream>
#include"user.pb.h"
#include"mprpcapplication.h"
#include"mprpcchannel.h"
int main(int argc,char**argv){
    //进行完整的调用流程
    //通过./后面的参数对rpc进行初始化
    Mapplication::Init(argc,argv);
    //创建一个stub来管理一个服务的方法的调用
    fixbug::userrpd_Stub sub(new Rpcchannel());
    //调用每一个方法要创建request respond 
    fixbug::request input;
    input.set_name("zhangshan");
    input.set_pwd("wken");
    fixbug::respond respond;
    //调用方法
    MyRpccontroller contro;
    sub.login(&contro,&input,&respond,nullptr);
    //通过controller来判断是否完成了rpc
    if(contro.Failed()){
        std::cout<<contro.ErrorText()<<std::endl;
        return 0;
    }
    //获取方法的结果
    std::cout<<respond.result().errornum()<<std::endl;
    std::cout<<respond.result().errors()<<std::endl;
    return 0;
}
