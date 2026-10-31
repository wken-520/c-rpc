1. 项目介绍
RPC（Remote Procedure Call，远程过程调用）允许调用者像调用本地函数一样调用远程服务器上的服务。
本项目实现了一个基础的 RPC 框架，主要包括服务注册、服务发现、网络通信、数据序列化以及远程方法调用等模块。
项目整体采用 C++ 开发，使用面向对象的方式组织代码。
| 技术 | 用途 |
|---|---|
| C++ | 核心开发语言 |
| Muduo | 高性能 TCP 网络通信 |
| Protobuf | RPC 请求与响应的序列化 |
| ZooKeeper | 服务注册与服务发现 |
| CMake | 项目构建 |
| Linux | 项目运行环境 |
| pthread | 多线程支持 |
3. 项目功能
已实现的主要功能
- RPC 服务注册：通过 Protobuf Service 注册 RPC 服务及其方法。
- RPC 服务调用：通过自定义 RpcChannel 实现远程方法调用。
- 数据序列化：使用 Protobuf 序列化 RPC 请求和响应。
- TCP 网络通信：基于 Muduo 实现服务端网络通信。
- 服务注册与发现：通过 ZooKeeper 注册服务节点，客户端根据服务名和方法名获取服务地址。
- 配置文件管理：支持通过配置文件读取 RPC 服务端和 ZooKeeper 的连接信息。
- RPC 控制器：通过 RpcController 管理 RPC 调用状态和错误信息。
- 日志模块：实现基础日志输出及线程安全队列。
4. 项目架构
                 Client
                    |
                    v
              RpcChannel
                    |
                    v
             Protobuf 序列化
                    |
                    v
                TCP Socket
                    |
                    v
              Muduo Server
                    |
                    v
              RpcProvider
                    |
                    v
             Protobuf 反序列化
                    |
                    v
              RPC Service
                    |
                    v
              执行远程方法
                    |
                    v
              返回 RPC 结果


              ZooKeeper
                  |
          +-------+-------+
          |               |
      服务注册          服务发现
          |               |
       Provider         Client