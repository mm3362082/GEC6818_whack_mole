#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "thread_pool.h"


/**
 * @brief 线程函数
 * @param args 线程执行需要的参数:pool
 * @return NULL
 */
void *start_routine(void *args)
{
    thread_pool_t *pool = (thread_pool_t *)args;
    /* 
        循环
        条件阻塞，等待被唤醒之后去执行任务
     */
    do
    {
        /* 上锁操作 */
        pthread_mutex_lock(&(pool->shared_mutex));

        /* 等待被唤醒 */
        pthread_cond_wait(&(pool->shared_cond),&(pool->shared_mutex));

        task_t *task = NULL;
        /* 从任务链表中获取任务并执行 */
        if(pool->task_list == NULL)
        {
            pthread_mutex_unlock(&(pool->shared_mutex));
            goto cont;
        }
        task            =  pool->task_list;
        pool->task_list =  pool->task_list->next;

        pthread_mutex_unlock(&(pool->shared_mutex));

        pthread_t tid = pthread_self();
        /* 执行任务 */
        (task->task_point)(&tid);// task_point(task->args);

        /* 销毁工作 */
        task->next = NULL;
        free(task);
cont:
        task = NULL;

    } while (pool->pool_status);
    
    return NULL;
}

/**
 * @brief 线程池初始化
 * @param thread_count  线程池中线程的数量
 * @return thread_pool_t 返回一个初始化的线程池
 */
thread_pool_t *thread_pool_init(int count)
{
    /* 为线程池开辟一个动态内存空间 */
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));

    /*
        线程池的成员初始化操作
        1）线程池中线程的个数
        2）通过线程池中的线程个数成员，为线程集合指针开辟动态空间
        3）通过线程池中的线程个数成员，为线程id集合指针开辟动态空间
    */
    pool->thread_count = count;
    pool->thread_id    = malloc(sizeof(pthread_t)*count);
    pool->pool_status  = 1;
    
    /* 通过内存设置，将thread_id成员进行置零操作 */
    memset(pool->thread_id,0,sizeof(pthread_t)*count);

    pool->task_list    = NULL;

    /* 初始化条件变量和互斥锁 */
    pthread_mutex_init(&(pool->shared_mutex),NULL);
    pthread_cond_init(&(pool->shared_cond),NULL);

    /* 创建线程 */
    for(int i = 0;i < count;i++)
    {
        if(pthread_create((pool->thread_id+i),NULL,start_routine,pool) != 0)
        {
            i--;
            continue;
        }
        
    }

    /* 返回初始化成功的线程池指针 */
    return pool;
}

/**
 * @brief 添加任务到任务链表
 * @param pool 需要添加任务的线程池
 * @param args 任务执行时需要的参数
 * @param task_rountine 任务函数
 * @return void
 */
void add_new_task(thread_pool_t *pool,void *args,thread_t task_rountine)
{
    task_t *task = malloc(sizeof(task_t));
    task->args = args;
    task->next = NULL;
    task->task_point = task_rountine;

    pthread_mutex_lock(&(pool->shared_mutex));
    if(pool->task_list == NULL)
    {
        pool->task_list = task;
    }
    else
    {
        /* 找尾结点 */
        task_t *temp = pool->task_list;
        while(temp->next)
        {
            temp = temp->next;
        }

        temp->next = task;
    }

    /* 唤醒线程去执行任务 */
    pthread_cond_signal(&(pool->shared_cond));

    pthread_mutex_unlock(&(pool->shared_mutex));

    return;
}

/**
 * @brief 销毁线程池
 * @param pool 二级指针 需要销毁的线程池指针
 */
void thread_destroy(thread_pool_t **pool)
{
    /* 关闭线程池 */
    pthread_mutex_lock(&((*pool)->shared_mutex));
    (*pool)->pool_status = 0;
    pthread_mutex_unlock(&((*pool)->shared_mutex));

    /* 唤醒所有线程 */
    for(int i = 0;i < (*pool)->thread_count;i++)
    {
        pthread_mutex_lock(&((*pool)->shared_mutex));
        pthread_cond_signal(&((*pool)->shared_cond));
        pthread_mutex_unlock(&((*pool)->shared_mutex));
    }
        

    /* 结束所有线程 */
    for(int i = 0;i < (*pool)->thread_count;i++)
    {
        pthread_cancel(*((*pool)->thread_id+i));
        pthread_join(*((*pool)->thread_id+i),NULL);
    }
    /* 销毁任务链表 */
    task_t *temp =(*pool)->task_list;
    while(temp)
    {
        (*pool)->task_list = (*pool)->task_list->next;
        temp->next         = NULL;
        free(temp);
        temp = (*pool)->task_list;
    }
    (*pool)->task_list = NULL;

    /* 销毁线程id空间 */
    free((*pool)->thread_id);
    (*pool)->thread_id = NULL;

    /* 销毁互斥锁和条件变量 */
    pthread_mutex_destroy(&(*pool)->shared_mutex);
    pthread_cond_destroy(&(*pool)->shared_cond);

    /* 将线程池指针置空 */
    free((*pool));
    (*pool) = NULL;
}