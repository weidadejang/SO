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
#include "Xmem.h"

int xtest = 0;
#define BUF_SIZE 4096
static uint8_t RxTemp[BUF_SIZE] = {30,31,32,33,34};

int MemoryWRITE(uint8_t *buf,int len);
int MemoryREAD(void);

void print_buf(char *fmt, uint8_t *buf, int size,int id)
{
	int i=0;
	printf("\n--------------------------------------------------------------------------------\n");
	printf("Current id :[%d] data:[%d]\n",id,size);
	for(; i<size; i++) printf(fmt,buf[i]);
}

JNIEXPORT jint JNICALL Java_XShare_J2C_1WrBuffer(JNIEnv *env, jobject obj, jbyteArray j_array)
{
	jint i = 0;
	jint j_array_len;
	jbyte *c_array;
	//1. 获取数组长度
	j_array_len = (*env)->GetArrayLength(env,j_array);
	//2. 根据数组长度和数组元素的数据类型申请存放java数组元素的缓冲区
	c_array = (jbyte*)malloc(sizeof(jbyte) * j_array_len);
	//3. 初始化缓冲区
	memset(c_array,0,sizeof(jbyte)*j_array_len);
	//4. 拷贝Java数组中的所有元素到缓冲区中
	(*env)->GetByteArrayRegion(env,j_array,0,j_array_len,c_array);

	//print_buf("%02X ", (uint8_t *)c_array, j_array_len,Java2C_WRMEMName);
	//5. 1 OK
	i = MEMBlockWR(Java2C_WRMEMName,(uint8_t *)c_array,j_array_len);

	free(c_array);
	return i;
}



/*---------------------------------------------------------------------------------
 * Java:
 * --------------------------------------------------------------------------------*/
/*
JNIEXPORT void JNICALL Java_XShare_sayHello(JNIEnv *env, jobject obj)
{
	printf("Check test value xtest : %d\n",xtest);
	return;
}
*/
/*---------------------------------------------------------------------------------
 * Java: Array communication Download into MEM
 * --------------------------------------------------------------------------------*/
/*
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

	//for (i = 0; i < arr_len; i++) {
	//	sum += c_array[i];  //5. 累加数组元素的和
	//}

	sum = MemoryWRITE((uint8_t *)c_array,arr_len);
	free(c_array);  //6. 释放存储数组元素的缓冲区

	return sum;
}
*/
/*
JNIEXPORT jbyteArray JNICALL Java_XShare_GetArray(JNIEnv *env, jobject obj)
{
	int i = 0;
	jbyteArray c_result = (*env)->NewByteArray(env,5);

	i = MemoryREAD();
	if(i){
		(*env)->SetByteArrayRegion(env,c_result,0,5,(const jbyte *)RxTemp);
	}

	return c_result;
}*/


