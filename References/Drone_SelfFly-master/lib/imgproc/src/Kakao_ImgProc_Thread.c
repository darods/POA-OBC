


/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"


/*----------------------------------------------------------------------------------------
 Constant Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/
#define Thread                      signed int
#define TSK_YIELD()                 usleep(1)        //Sleep(0)      // take care of YIELD
#define TSK_SLEEP(n)                usleep(n)
#define CREATE_THREAD               CreateThread
#define RESUME_THREAD               ResumeThread
#define EXIT_THREAD                 GetExitCodeThread
#define SUSPEND_THREAD              SuspendThread

//#define __USE_LOCK_OBJ__

#ifdef __USE_LOCK_OBJ__
#define CS                      CRITICAL_SECTION
#define InitCS(a)               InitializeCriticalSection(&(a))
#define EnterCS(a)              EnterCriticalSection(&(a))
#define ExitCS(a)               LeaveCriticalSection(&(a))
#define UnInitCS(a)             DeleteCriticalSection(&(a))
#else
#define CS                      pthread_mutex_t
#define InitCS(a)               pthread_mutex_init(&(a), NULL)
#define EnterCS(a)              pthread_mutex_lock(&(a))    // 100 : time out
#define ExitCS(a)               pthread_mutex_unlock(&(a))
#define UnInitCS(a)             pthread_mutex_destroy(&(a))
#endif



/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
