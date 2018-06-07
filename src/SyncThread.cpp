//============================================================================
// Name        : SyncThread.cpp
// Author      : wenbing.wang
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
//#include <mutex>
using namespace std;

#define MAX_THREAD_NUM 3
typedef enum {
    THREAD_NUM_00 = 0x0,    //bit 0
    THREAD_NUM_01 = 0x1,    //bit 1
    THREAD_NUM_02 = 0x2,    //bit 2
};

#define MAX_BIT_FLAG (1<<THREAD_NUM_00 | 1<<THREAD_NUM_01 | 1<<THREAD_NUM_02)

typedef struct work_thread_t {
    pthread_t tId;

    int flag;
    struct main_context_t *main;
} WorkThreadCtx;

typedef struct main_context_t {
    WorkThreadCtx ctx[MAX_THREAD_NUM];

    pthread_mutex_t mtx;
    pthread_cond_t cond;

    bool exit = false;
    int sync_flag = 0;
} MainContext;

void process(WorkThreadCtx *thd) {
    //do some process
    //....

    pthread_mutex_lock(&thd->main->mtx);
    thd->main->sync_flag |= ((1<<thd->flag) & MAX_BIT_FLAG);
    printf("thread%d process, sync_flag:%d\n", thd->flag, thd->main->sync_flag);

    if (thd->main->sync_flag == MAX_BIT_FLAG) {
        pthread_cond_signal(&thd->main->cond);
        printf("receive condtion, signal exit\n");
    }
    pthread_mutex_unlock(&thd->main->mtx);
    usleep(1000);
}

void* pthread_func (void *args) {
    if (!args)
        return NULL;

    WorkThreadCtx *ctx = static_cast<WorkThreadCtx *>(args);

    do {
        process(ctx);
    } while (!ctx->main->exit);
    return NULL;
}

int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	MainContext mainCtx;
    pthread_mutex_init(&mainCtx.mtx, NULL);
    pthread_cond_init(&mainCtx.cond, NULL);

	for (int i=0;i<MAX_THREAD_NUM;i++) {
	    pthread_create(&(mainCtx.ctx[i].tId), NULL, pthread_func, &mainCtx.ctx[i]);
	    mainCtx.ctx[i].flag = i;
	    mainCtx.ctx[i].main = &mainCtx;

	    printf("%d flag:%d\n", i, i);
	}

	pthread_mutex_lock(&mainCtx.mtx);
	while (mainCtx.sync_flag != MAX_BIT_FLAG) {
	    pthread_cond_wait(&mainCtx.cond, &mainCtx.mtx);
	}
	mainCtx.sync_flag = 0;
	pthread_mutex_unlock(&mainCtx.mtx);

	mainCtx.exit = true;
	for (int i=0;i<MAX_THREAD_NUM;i++) {
        pthread_join(mainCtx.ctx[i].tId, NULL);
        printf("thread%d exit\n", i);
    }

	pthread_mutex_destroy(&mainCtx.mtx);
	pthread_cond_destroy(&mainCtx.cond);

	printf("main exit\n");
	return 0;
}
