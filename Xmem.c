/*
 * Xmem.c
 *
 *  Created on: Feb 24, 2021
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
#include "Xmem.h"

static uint8_t SigRx[SIG_MEM_SIZE] = {0};
static uint8_t SigTx[SIG_MEM_SIZE] = {0};

int XMemoryREAD(key_t MemName);
int XMemoryWRITE(key_t MemName,uint8_t *buf);

/*---------------------------------------------------------------------------------
 * MEMORY: ReadBLOCK WriteBlock
 * --------------------------------------------------------------------------------*/
int MEMBlockWR(key_t Name,uint8_t *buf,int len)
{
	int i = 0,j = 0,k = 0;
	int Xchushu = 0,Xyushu = 0;
	int BlockNum = 0,lastlen = 0;
	int templen = 0;

	if(len < SIG_MEM_SIZE){
		lastlen = len;
		BlockNum = 1;
	}
	else{
		Xchushu = len / SIG_MEM_SIZE;
		Xyushu  = len % SIG_MEM_SIZE;
		if(Xyushu > 0){
			lastlen = Xyushu;
			BlockNum = Xchushu + 1;
		}
		else{
			lastlen = SIG_MEM_SIZE;
			BlockNum = Xchushu;
		}
	}

	if(BlockNum > TOT_MEM)  return -1;

	for(i = 0 ; i < BlockNum; i++ )
	{
		if((i+1) == BlockNum)
			templen = lastlen;
		else
			templen = SIG_MEM_SIZE;


		bzero(SigTx, SIG_MEM_SIZE);
		for(k = 0; k < templen;k++){
			SigTx[k] = buf[k+(i*SIG_MEM_SIZE)];
		}

		j = XMemoryWRITE((Name+i),SigTx);
		if(j < 0) return -2;
	}
	//wr complete
	return 1;
}
/*---------------------------------------------------------------------------------
 * MEMORY: ReadBLOCK
 * --------------------------------------------------------------------------------*/
int MEMBlockRD(key_t Name,uint8_t *buf,int NeedRdblock)
{
	int i = 0,j = 0,k = 0;

	for(i = 0 ; i < NeedRdblock;i++)
	{
		j = XMemoryREAD(Name+i);
		if(j < 0) return -1;
		else
		{
			for(k = 0 ; k < SIG_MEM_SIZE;k++){
				buf[k + (i * SIG_MEM_SIZE)] = SigRx[k];
			}
		}
	}

	return 1;
}

/*---------------------------------------------------------------------------------
 * MEMORY: Read
 * --------------------------------------------------------------------------------*/
int XMemoryREAD(key_t MemName)
{
    void *shm_addr = NULL;
	pthread_mutex_t * sharedLock;


    int shmid;
    // 使用约定的键值打开共享内存
    shmid = shmget(MemName, SIG_MEM_SIZE, IPC_CREAT);
    if (shmid == -1){
    	return -1;
    }

    // 将共享内存附加到本进程
    shm_addr = shmat(shmid, NULL, 0);
    if (shm_addr == (void *) -1){
    	return -2;
    }

	sharedLock = (pthread_mutex_t *)shm_addr;
    // 读取数据
	pthread_mutex_lock(sharedLock);
	bzero(SigRx, SIG_MEM_SIZE);
	memcpy(SigRx, ((pthread_mutex_t *)shm_addr)+1, SIG_MEM_SIZE);
	pthread_mutex_unlock(sharedLock);

    // 分离
    if (shmdt(shm_addr) == -1){
    	return -3;
    }
    // 删除共享内存
    /*
    if (shmctl(shmid, IPC_RMID, 0) == -1){
        //printf("shmctl error!\n");
        //exit(1);
    	return -4;
    }
    */
    return 1;
}

/*---------------------------------------------------------------------------------
 * MEMORY: WRITE
 * --------------------------------------------------------------------------------*/
int XMemoryWRITE(key_t MemName,uint8_t *buf)
{
    void *shm_addr = NULL;
	pthread_mutex_t * sharedLock;
	pthread_mutexattr_t ma;


    int shmid;
    // 使用约定的键值创建共享内存
    shmid = shmget(MemName, SIG_MEM_SIZE, 0666 | IPC_CREAT);
    if (shmid < 0){
        return -1;
    }

    // 将共享内存附加到本进程
    shm_addr = shmat(shmid, NULL, 0);
    if (shm_addr == (void *) -1){
        return -2;
    }

	sharedLock = (pthread_mutex_t *)shm_addr;

	pthread_mutexattr_init(&ma);
	pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
	pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST);
	pthread_mutex_init(sharedLock,&ma);

    // 写入数据
	pthread_mutex_lock(sharedLock);
	//bzero(SigTx, SIG_MEM_SIZE);
	memcpy(((pthread_mutex_t *)shm_addr)+1, buf, SIG_MEM_SIZE);
	pthread_mutex_unlock(sharedLock);

    // 分离
    if (shmdt(shm_addr) == -1){
        return -3;
    }
    //pthread_mutex_destroy(sharedLock);
    pthread_mutexattr_destroy(&ma);

    return 1;
}
