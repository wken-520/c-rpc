#pragma once
#include"mprpcconfig.h"
#include<mprpcprovider.h>
#include"mprpccontroller.h"
class Mapplication{
    public:
    //单列模式
    static void Init(int argc,char ** argv);
    static Mapplication&Getline();
    static Mconfig& Getconfig();
    private:
    static Mconfig m_config;
    Mapplication(){}
    Mapplication(const Mapplication& )=delete;
    Mapplication( Mapplication&&)=delete;
};