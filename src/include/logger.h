#pragma once
#include"lockqueue.h"
#include<string>
enum loglevel{
    info,
    error,
};
class logger{
    private:
    int M_level;
    lockqueue<std::string> M_queue;
    public:
    static logger& GetInence();
    void setloggerlevel(loglevel level);
    void setloggertext(std::string);
    logger();
    logger(const logger&)=delete;
    logger(logger&&)=delete;
};
#define LOG_INFO(logmegformat,...)\
do\
{\
    logger& log=logger::GetInence();\
    log.setloggerlevel(loglevel::info);\
    char buf[1024]={0};\
    snprintf(buf,1024,logmegformat,##__VA_ARGS__);\
    log.setloggertext(buf);\
}while(0)