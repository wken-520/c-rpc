#include"mprpccontroller.h"
    //进行初始化
    MyRpccontroller::MyRpccontroller(){
        M_failed=false;
        errortext="";
    }
    //重置
    void MyRpccontroller::Reset(){
         M_failed=false;
        errortext="";
    }
    //判断是否失败
    bool MyRpccontroller::Failed() const{
        return M_failed;
    }
    //设置失败
    void MyRpccontroller::SetFailed(const std::string& reason) {
        M_failed=true;
        errortext=reason;
    }
    //设置失败信息
    std::string MyRpccontroller::ErrorText() const{
        return errortext;
    }
    void MyRpccontroller::StartCancel(){

    }
    bool MyRpccontroller::IsCanceled() const{
        return true;
    }
    void MyRpccontroller::NotifyOnCancel(google::protobuf::Closure* callback){

    }