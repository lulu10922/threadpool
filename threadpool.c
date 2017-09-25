#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

void *thread_routine(void *arg)
{
	printf("thread 0x%x is starting\n", (int)pthread_self());
	threadpool_t *pool = (threadpool_t *)arg;
	while(1)
	{
		condition_lock(&pool->ready);
		pool->idle++;
		while(pool->first == NULL && pool->quit)
		{
			condition_wait(&pool->ready);
		}
		pool->idle--;
		if(pool->first)
		{
			task_t *t = pool->first;
			pool->first = pool->first->next;
			condition_unlock(&pool->ready);
			t->run(t->arg);
			free(t);
			condition_lock(&pool->ready);
		}
		if(pool->quit && pool->first == NULL)
		{
            pool->counter--;
            condition_unlock(&pool->ready);
			break;
		}
        condition_unlock(&pool->ready);
	}
	printf("thread 0x%x is exiting\n", (int)pthread_self());
	return NULL;
}

void threadpool_init(threadpool* pool, int threads)
{
	condition_init(&pool->cond);
	pool->first = NULL;
	pool->last = NULL;
	pool->counter = 0;
    pool->idle = 0;
	pool->max_threads = threads;
	pool->quit = 0;
}

void threadpool_add_task(threadpool* pool, void* (*run)(void* arg), void *arg)
{
	task_t *new_task = (task_t*)malloc(sizeof(task_t));
	new_task->run = run;
	new_task->arg = arg;
	new_task->next = NULL;

    condition_lock(&pool->ready);
	if(pool->first == NULL)
	{
		pool->first = new_task;
	}
	else
		pool->last->next = new_task;

	pool->last = new_task;

//每添加一个任务，检查是否有空闲的线程去执行任务，否则检查线程池中的线程数是否达到最大，如果没有则可以新建线程
	if(pool->idle > 0)
	{
		condition_signal(&pool->ready);
		pool->idle--;
	}
	else if(pool->counter < pool->max_threads)
	{
		pthread_t tid;
		pthread_create(&tid, NULL, thread_routine)
	}
	condition_unlock(&pool->ready);
}