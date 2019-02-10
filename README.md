# 用线程池实现一个并发Web服务器
1.用信号量（互斥锁来实现线程同步）  
2.解析客户请求时，主状态所处的状态  
3.服务器处理HTTP请求的可能结果  
4.所有socket上的事件都被注册到同一个epoll内核事件中，所以将epoll文件描述符设置为静态的  
5.统计用户数量  
6.服务器响应客户端的请求  
7.往缓冲区中写入待发送的数据
