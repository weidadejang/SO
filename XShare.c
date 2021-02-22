/*
 * XShare.c
 *
 *  Created on: Feb 20, 2021
 *      Author: root
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "XShare.h"

int xtest = 0;

#define BUF_SIZE 4096
static uint8_t RxTemp[BUF_SIZE] = {30,31,32,33,34};
//const jbyte RxTemp[BUF_SIZE] = {30,31,32,33,34};

int MemoryWRITE(uint8_t *buf,int len);
int MemoryREAD(void);

/*---------------------------------------------------------------------------------
 * Java:
 * --------------------------------------------------------------------------------*/
JNIEXPORT void JNICALL Java_XShare_sayHello(JNIEnv *env, jobject obj)
{
	printf("Check test value xtest : %d\n",xtest);
	return;
}
/*---------------------------------------------------------------------------------
 * Java: Array communication Download into MEM
 * --------------------------------------------------------------------------------*/
JNIEXPORT jint JNICALL Java_XShare_sumArray(JNIEnv *env, jobject obj, jbyteArray j_array)
{
	//jint i = 0;
	jint sum = 0;
	jbyte *c_array;
	jint arr_len;
	//1. 获取数组长度
	arr_len = (*env)->GetArrayLength(env,j_array);
	//2. 根据数组长度和数组元素的数据类型申请存放java数组元素的缓冲区
	c_array = (jbyte*)malloc(sizeof(jbyte) * arr_len);
	//3. 初始化缓冲区
	memset(c_array,0,sizeof(jbyte)*arr_len);
	//printf("arr_len = %d ", arr_len);
	//4. 拷贝Java数组中的所有元素到缓冲区中
	(*env)->GetByteArrayRegion(env,j_array,0,arr_len,c_array);
	/*
	for (i = 0; i < arr_len; i++) {
		sum += c_array[i];  //5. 累加数组元素的和
	}
	*/
	sum = MemoryWRITE((uint8_t *)c_array,arr_len);
	free(c_array);  //6. 释放存储数组元素的缓冲区

	return sum;
}


JNIEXPORT jbyteArray JNICALL Java_XShare_GetArray(JNIEnv *env, jobject obj)
{
	int i = 0;
	jbyteArray c_result = (*env)->NewByteArray(env,5);

	i = MemoryREAD();
	if(i){
		(*env)->SetByteArrayRegion(env,c_result,0,5,(const jbyte *)RxTemp);
	}

	return c_result;
}

/*---------------------------------------------------------------------------------
 * Java: Array communication
 * --------------------------------------------------------------------------------*/
int test(void){
	return 77;
}


/*---------------------------------------------------------------------------------
 * MEMORY:
 * --------------------------------------------------------------------------------*/
int MemoryREAD(void)
{
    void *shm_addr = NULL;
	pthread_mutex_t * sharedLock;
	unsigned char *tmp;

    int shmid;
    // 使用约定的键值打开共享内存
    shmid = shmget((key_t) 1234, BUF_SIZE, IPC_CREAT);
    //printf("shmid : %u\n", shmid);
    if (shmid == -1){
        //perror("shmget error!");
        //exit(1);
    	return -1;
    }
    // 将共享内存附加到本进程
    shm_addr = shmat(shmid, NULL, 0);
    if (shm_addr == (void *) -1){
        //perror("shmat error!");
        //exit(1);
    	return -2;
    }

	sharedLock = (pthread_mutex_t *)shm_addr;
    // 读取数据
	pthread_mutex_lock(sharedLock);
	//tmp = RxTemp;
	bzero(RxTemp, BUF_SIZE);
	memcpy(RxTemp, ((pthread_mutex_t *)shm_addr)+1, BUF_SIZE);
	//printf("read from shared memory: %s\n", tmp);
	pthread_mutex_unlock(sharedLock);

    // 分离
    if (shmdt(shm_addr) == -1){
        //printf("shmdt error!\n");
        //exit(1);
    	return -3;
    }
    // 删除共享内存
//    if (shmctl(shmid, IPC_RMID, 0) == -1){
//        //printf("shmctl error!\n");
//        //exit(1);
//    	return -4;
//    }

    return 1;
}

/*---------------------------------------------------------------------------------
 * MEMORY:
 * --------------------------------------------------------------------------------*/
int MemoryWRITE(uint8_t *buf,int len)
{
    void *shm_addr = NULL;
    unsigned char buffer[BUF_SIZE];
	pthread_mutex_t * sharedLock;
	pthread_mutexattr_t ma;

    int shmid;
    // 使用约定的键值创建共享内存
    shmid = shmget((key_t) 1234,  BUF_SIZE, 0666 | IPC_CREAT);
    //printf("shmid : %u\n", shmid);
    if (shmid < 0)
    {
        //perror("shmget error!");
        //exit(1);
        return 1;
    }

    // 将共享内存附加到本进程
    shm_addr = shmat(shmid, NULL, 0);
    if (shm_addr == (void *) -1)
    {
        //perror("shmat error!");
        //exit(1);
        return 2;
    }

	sharedLock = (pthread_mutex_t *)shm_addr;

	pthread_mutexattr_init(&ma);
	pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
	pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST);
	pthread_mutex_init(sharedLock,&ma);

    // 写入数据
	pthread_mutex_lock(sharedLock);
	bzero(buffer, BUF_SIZE);
	//sprintf(buffer, "Hello, My PID is %u\n", (unsigned int) getpid());
	//printf("send data: %s\n", buffer);
	//memcpy(((pthread_mutex_t *)shm_addr)+1, buffer, strlen(buffer));
	memcpy(((pthread_mutex_t *)shm_addr)+1, buf, len);
	pthread_mutex_unlock(sharedLock);
    //sleep(5);

    // 分离
    if (shmdt(shm_addr) == -1)
    {
        //printf("shmdt error!\n");
        //exit(1);
        return 3;
    }
    //pthread_mutex_destroy(sharedLock);
    pthread_mutexattr_destroy(&ma);

    return 4;
}

