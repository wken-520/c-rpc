#pragma once
#include<mutex>
#include<condition_variable>
#include<queue>
template<class T>
class lockqueue{
    private:
    std::mutex M_mutex;
    std::condition_variable condition_v;
    std::queue<T> M_queue;
    public:
    void push(T& data){
        std::lock_guard<std::mutex> lock(M_mutex);
        M_queue.push(data);
        condition_v.notify_one();
    }
    T pop(){
        std::unique_lock<std::mutex> lock(M_mutex);
        while(M_queue.empty()){
            condition_v.wait(lock);
        }
        T p=M_queue.front();
        M_queue.pop();
        return p;
    }
};