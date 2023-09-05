#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <pthread.h>

typedef void *(*thread_t)(void *);


typedef struct tasks
{
    // 任务指针
    void *(*task_point)(void *);

    // 任务参数
    void *args;

    // 下一个任务
    struct tasks *next;
}task_t;

/**
 * @brief 线程池结构体类型申明
 * 
 * 
 */
typedef struct thread_pool
{
    // 线程个数
    int             thread_count;

    // 标明线程池是否启动
    int             pool_status;

    // 线程id的集合
    pthread_t      *thread_id;

    // 线程共享的互斥锁
    pthread_mutex_t shared_mutex;

    // 线程共享的条件变量
    pthread_cond_t  shared_cond;

    // 任务链表
    task_t         *task_list;

    /*
        最大线程数目：表示可以支持线程并发的最大线程数
        当前服役线程数目：表示当前可以用执行任务的线程
        当前休眠线程数目：表示当前可支持同时并发的线程数
        ...
    */

}thread_pool_t;

/**
 * @brief 线程池初始化
 * @param thread_count 线程池中线程的数量
 * @return thread_pool_t 返回一个初始化的线程池
 */
thread_pool_t *thread_pool_init(int count);

/**
 * @brief 添加任务到任务链表
 * @param pool 需要添加任务的线程池
 * @param args 任务执行时需要的参数
 * @param task_rountine 任务函数指针
 * @return void
 */
void add_new_task(thread_pool_t *pool,void *args,thread_t task_rountine);

/**
 * @brief 销毁线程池
 * @param pool 二级指针 需要销毁的线程池指针
 */
void thread_destroy(thread_pool_t **pool);

/*
    add_task_thread:增加线程数
    del_task_thread:删除线程数
    ...
*/

#endif//__THREAD_POOL_H__