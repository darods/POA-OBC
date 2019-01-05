//
//  Kakao_ImgProc_Thread.h
//
//  Created by Maverick Park on 2014. 11. 21.
//  Copyright (c) 2015년 Kakao Corp. All rights reserved.
//

#ifndef __KAKAO_IMGPROC_THREAD_H__
#define __KAKAO_IMGPROC_THREAD_H__

#include <stdio.h>
#include <stdlib.h>
#include "Kakao_ImgProc_Platform.h"
#include "Kakao_ImgProc_Typedefs.h"
#include "Kakao_ImgProc_Define.h"
#include "Kakao_ImgProc_Misc.h"


#define MAKE_SEMA_NAME(pSemaBaseName, nIndex, pDstSemaName)                     {\
                                                                                    sprintf(pDstSemaName, "%s%02d", pSemaBaseName, nIndex);\
                                                                                    pDstSemaName[strlen(pDstSemaName)+1] = '\0';\
                                                                                }


#if USE_MULTI_THREAD
    #define MAX_THREAD_NUM                                                      (32)

    #if ENABLE_ISET(CPU_ARMX)
        #if ENABLE_OS(OS_IOS)
            #include <pthread.h>
            #include <semaphore.h>
            #include <mach/mach_init.h>
            #include <mach/semaphore.h>
            #include <mach/task.h>
            #include <time.h>
            #include <unistd.h>
        #elif ENABLE_OS(OS_ANDROID)
            #include <semaphore.h>
            #include <sched.h>
        #else

        #endif
    #elif ENABLE_ISET(CPU_INTEL_IA32) | ENABLE_ISET(CPU_INTEL_IA64)
        #if ENABLE_OS(OS_WIN32) | ENABLE_OS(OS_WIN64)
            #include <process.h>
            #include <windows.h>
        #elif ENABLE_OS(OS_OSX)
            #include <pthread.h>
            #include <semaphore.h>
            #include <mach/mach_init.h>
            #include <mach/semaphore.h>
            #include <mach/task.h>
            #include <time.h>
            #include <unistd.h>
        #else

        #endif
    #else

    #endif


    #if ENABLE_ISET(CPU_ARMX)
        #if ENABLE_OS(OS_IOS)
            #define ThreadID                                                    pthread_t
            #define sem_t                                                       semaphore_t
            #define SEMA_CREATE(sema, param0, param1)                           semaphore_create(mach_task_self(), sema, SYNC_POLICY_FIFO, param1)
            #define SEMA_WAIT(sema)                                             (semaphore_wait(*sema))
            #define SEMA_POST(sema)                                             semaphore_signal(*sema)
            #define SEMA_DESTROY(sema)                                          semaphore_destroy(mach_task_self(), *sema)
            #define THREAD_CREATE(threadid, attr, funcname, threadparam)        pthread_create(&threadid, attr, (void *)(funcname), (void *)(threadparam));\
                                                                                if(NULL == threadid)\
                                                                                    goto Error;
            #define THREAD_JOIN(threadid)                                       pthread_join(threadid, 0);
        #elif ENABLE_OS(OS_ANDROID)
            #define ThreadID                                                    UINT32
            #define SEMA_CREATE(sema, param0, param1)                           sem_init(sema, param0, param1)
            #define SEMA_WAIT(sema)                                             sem_wait((sema))
            #define SEMA_POST(sema)                                             sem_post((sema))
            #define SEMA_DESTROY(sema)                                          sem_destroy((sema))
            #define THREAD_CREATE(threadid, attr, funcname, threadparam)        pthread_create(&threadid, attr, (void *)(funcname), (void *)(threadparam));\
                                                                                if(0 == threadid)\
                                                                                    goto Error;
            #define THREAD_JOIN(threadid)                                       pthread_join(threadid, 0);
        #else
            #define SEMA_CREATE(sema, param0, param1)
            #define SEMA_WAIT(sema)
            #define SEMA_POST(sema)
            #define SEMA_DESTROY(sema)
            #define THREAD_CREATE(threadid, attr, funcname, threadparam)
            #define THREAD_JOIN(threadid)
        #endif
    #elif ENABLE_ISET(CPU_INTEL_IA32) | ENABLE_ISET(CPU_INTEL_IA64)
        #if ENABLE_OS(OS_WIN32) | ENABLE_OS(OS_WIN64)
            #define pthread_t                                                   HANDLE
            #define ThreadID                                                    pthread_t

            #define sem_t                                                       HANDLE
            #define SEMA_CREATE(sema, param0, param1)                           (int)((*sema = CreateSemaphore(NULL, 0, 32768, NULL)) == NULL)
            #define SEMA_WAIT(sema)                                             (int)(WAIT_OBJECT_0 != WaitForSingleObject(*sema, INFINITE))
            #define SEMA_POST(sema)                                             ReleaseSemaphore(*sema, 1, NULL)
            #define SEMA_DESTROY(sema)                                          if(*sema)((int)(CloseHandle(*sema))==TRUE)
            #define THREAD_CREATE(threadid, attr, funcname, threadparam)        (int)((threadid = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall *)(void *))funcname, threadparam, 0, NULL)) == NULL);\
                                                                                if (NULL == threadid)\
                                                                                    goto Error;
            #define THREAD_JOIN(threadid)                                       ((WaitForSingleObject((threadid),INFINITE)!=WAIT_OBJECT_0) || !CloseHandle(threadid));
        #elif ENABLE_OS(OS_OSX)
            #define ThreadID                                                    pthread_t
            #define sem_t                                                       semaphore_t
            #define SEMA_CREATE(sema, param0, param1)                           semaphore_create(mach_task_self(), sema, SYNC_POLICY_FIFO, param1)
            #define SEMA_WAIT(sema)                                             (semaphore_wait(*sema))
            #define SEMA_POST(sema)                                             semaphore_signal(*sema)
            #define SEMA_DESTROY(sema)                                          semaphore_destroy(mach_task_self(), *sema)
            #define THREAD_CREATE(threadid, attr, funcname, threadparam)        pthread_create(&threadid, attr, (void *)(funcname), (void *)(threadparam));\
                                                                                if(NULL == threadid)\
                                                                                    goto Error;
            #define THREAD_JOIN(threadid)                                       pthread_join(threadid, 0);
        #else
            #define SEMA_CREATE(sema, param0, param1)
            #define SEMA_WAIT(sema)
            #define SEMA_POST(sema)
            #define SEMA_DESTROY(sema)
            #define THREAD_CREATE(threadid, attr, funcname, threadparam)
            #define THREAD_JOIN(threadid)
        #endif
    #else
        #define SEMA_CREATE(sema, param0, param1)
        #define SEMA_WAIT(sema)
        #define SEMA_POST(sema)
        #define SEMA_DESTROY(sema)
        #define THREAD_CREATE(threadid, attr, funcname, threadparam)
        #define THREAD_JOIN(threadid)
    #endif
#endif


#endif




