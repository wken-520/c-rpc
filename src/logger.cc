#include"logger.h"
#include<thread>
#include<time.h>
#include<fstream>
#include<iostream>
#include<string>
void logger::setloggerlevel(loglevel level){
    M_level=level;
}
void logger::setloggertext(std::string msg)
    {
        M_queue.push(msg);
    }
    logger& logger::GetInence(){
        static logger logger;
        return logger;
    }
    logger::logger(){
        //用于写磁盘IO
        std::thread M_thread([&]()->void{
            for(;;){
                time_t now=time(nullptr);
                tm* local=localtime(&now);
                char file_name[128];
                char time_now[128];
                sprintf(file_name,"%d-%d-%d-logger.txt",local->tm_year+1900,local->tm_mon+1,local->tm_mday);
                sprintf(time_now,"%d-%d-%d->",local->tm_hour,local->tm_min,local->tm_sec);
                std::ofstream fout("test.txt", std::ios::app);   // 不存在则创建，存在则追加
                if(fout.is_open()){
                    std::cout<<"open file_name error"<<std::endl;
                    exit(EXIT_FAILURE);
                }
                std::string msg=M_queue.pop();
                fout<<time_now<<msg<<std::endl;
            }
        });
        M_thread.detach();
    }