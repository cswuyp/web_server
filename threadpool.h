#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

template< typename T >
class threadpool
{
public:
    threadpool( int thread_number = 8, int max_requests = 10000 );
    ~threadpool();
    bool append( T* request );

private:
    static void* worker( void* arg );
    void run();

private:
    int m_thread_number;//线程池中线程数量
    int m_max_requests;
    pthread_t* m_threads;
    std::list< T* > m_workqueue;//用链表来实现工作队列
    locker m_queuelocker;
    sem m_queuestat;//信号量
    bool m_stop;
};

//创建线程池中的线程
template< typename T >
threadpool< T >::threadpool( int thread_number, int max_requests ) : 
        m_thread_number( thread_number ), m_max_requests( max_requests ), m_stop( false ), m_threads( NULL )
{
    if( ( thread_number <= 0 ) || ( max_requests <= 0 ) )
    {
        throw std::exception();
    }

    m_threads = new pthread_t[ m_thread_number ];
    if( ! m_threads )
    {
        throw std::exception();
    }

    for ( int i = 0; i < thread_number; ++i )//创建8个线程放于线程池中
    {
        printf( "create the %dth thread\n", i );
        if( pthread_create( m_threads + i, NULL, worker, this ) != 0 )
        {
            delete [] m_threads;
            throw std::exception();
        }
        if( pthread_detach( m_threads[i] ) )//主线程与子线程分离，子线程结束后，资源自动回收
        {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

//释放线程池中的线程
template< typename T >
threadpool< T >::~threadpool()
{
    delete [] m_threads;
    m_stop = true;
}

//把新到的请求任务加入工作队列中
template< typename T >
bool threadpool< T >::append( T* request )
{
    m_queuelocker.lock();//向工作队列中加入任务前需要先对工作队列加锁
    if ( m_workqueue.size() > m_max_requests )//如果工作队列中的任务数量已经大于请求数量，此时不能继续往工作队列中加入请求，只能往工作队列中取出请求
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back( request );
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

//
template< typename T >
void* threadpool< T >::worker( void* arg )
{
    threadpool* pool = ( threadpool* )arg;
    pool->run();
    return pool;
}


//从工作队列中取出一个任务
template< typename T >
void threadpool< T >::run()
{
    while ( ! m_stop )
    {
        m_queuestat.wait();//信号量减1
        m_queuelocker.lock();//对工作队列加锁
        if ( m_workqueue.empty() )//如果此时工作队列为空
        {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();//指向链表的第一个结点
        m_workqueue.pop_front();//删除链表的第一个结点
        m_queuelocker.unlock();//取出任务完毕，对工作队列解锁
        if ( ! request )
        {
            continue;
        }
        request->process();
    }
}

#endif
