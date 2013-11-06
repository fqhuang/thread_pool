
#ifndef THREAD_POOL_HPP_INCLUDED
#define THREAD_POOL_HPP_INCLUDED

#include <queue>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

template<typename T>
class TaskQueue:boost::noncopyable
{
public:
    TaskQueue() : m_queue(), cond(), m_mutex() {}
    ~TaskQueue(){}

    void push_task(const T& task_func)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        m_queue.push(task_func);
        cond.notify_one();			
    }
    T get_task()
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        if(m_queue.size()==0)
        {
            cond.wait(lock);
        }

        T task = NULL;
        if (m_queue.size() != 0)
        {
            task = m_queue.front();
            m_queue.pop();
        }		
        return task;
    }
    int get_size()
    {
        return m_queue.size();
    }

    void notify_all()
    {
        cond.notify_all();
    }

    void notify_any()
    {
        cond.notify_one();
    }

private:
    std::queue<T> m_queue;
    boost::condition_variable_any cond;
    boost::mutex m_mutex;
};


typedef boost::function<void(void)> threadTask;

class ThreadPool:boost::noncopyable
{
public:
    ThreadPool(int num):threadNum(num),isRun(false){}
    ~ThreadPool(){}
    void init()
    {
        isRun=true;
        if(threadNum<=0)
            return;
        for(int i=0;i<threadNum;++i)
        {

            threadGroup.add_thread(new boost::thread(boost::bind(&ThreadPool::run,this)));
        }	
    }

    void stop()
    {
        isRun=false;
        taskQueue.notify_all();
    }

    void  post(const threadTask & task)
    {
        taskQueue.push_task(task);
    }

    void wait()
    {
        threadGroup.join_all();
    }

private:
    TaskQueue<threadTask> taskQueue;
    boost::thread_group threadGroup;
    int threadNum;
    volatile bool isRun;
    void run()
    {
        while(isRun)
        {

            threadTask task=taskQueue.get_task();
            if (task)
                task();
            else
                printf("exit\n");
        }
    }
};


#endif

