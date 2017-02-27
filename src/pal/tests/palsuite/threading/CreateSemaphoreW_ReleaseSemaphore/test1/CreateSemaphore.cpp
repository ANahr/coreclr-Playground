// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*============================================================
**
** Source: CreateSemaphoreW_ReleaseSemaphore/test1/CreateSemaphore.c
**
** Purpose: Test Semaphore operation using classic IPC problem:
** "Producer-Consumer Problem".
**
** Dependencies: CreateThread
**               ReleaseSemaphore
**               WaitForSingleObject
**               Sleep 
**               fflush
** 

**
**=========================================================*/

#define UNICODE
#include <palsuite.h>

#define PRODUCTION_TOTAL 26

#define _BUF_SIZE 10

DWORD dwThreadId;  /* consumer thread identifier */

HANDLE hThread; /* handle to consumer thread */

HANDLE hSemaphoreM; /* handle to mutual exclusion semaphore */

HANDLE hSemaphoreE; /* handle to semaphore that counts empty buffer slots */

HANDLE hSemaphoreF; /* handle to semaphore that counts full buffer slots */

typedef struct Buffer
{
    short readIndex;
    short writeIndex;
    CHAR message[_BUF_SIZE];

} BufferStructure;

CHAR producerItems[PRODUCTION_TOTAL + 1];

CHAR consumerItems[PRODUCTION_TOTAL + 1];

/* 
 * Read next message from the Buffer into provided pointer.
 * Returns:  0 on failure, 1 on success. 
 */
int
readBuf(BufferStructure *Buffer, char *c)
{
    if( Buffer -> writeIndex == Buffer -> readIndex )
    {
	return 0;
    }
    *c = Buffer -> message[Buffer -> readIndex++];
    Buffer -> readIndex %= _BUF_SIZE;
    return 1;
}

/* 
 * Write message generated by the producer to Buffer. 
 * Returns:  0 on failure, 1 on success.
 */
int 
writeBuf(BufferStructure *Buffer, CHAR c)
{
    if( ( ((Buffer -> writeIndex) + 1) % _BUF_SIZE) ==
	(Buffer -> readIndex) )
    {
	return 0;
    }
    Buffer -> message[Buffer -> writeIndex++] = c;
    Buffer -> writeIndex %= _BUF_SIZE;
    return 1;
}

/*
 * Atomic decrement of semaphore value.
 */
VOID
down(HANDLE hSemaphore)
{
    switch ( (WaitForSingleObject (
		  hSemaphore,   
		  10000)))      /* Wait 10 seconds */
    {
    case WAIT_OBJECT_0:  /* 
			  * Semaphore was signaled. OK to access
			  * semaphore. 
			  */
	break;
    case WAIT_ABANDONED: /*
			  * Object was mutex object whose owning
			  * thread has terminated.  Shouldn't occur.
			  */
	Fail("WaitForSingleObject call returned 'WAIT_ABANDONED'.\n"
	     "Failing Test.\n");
	break;
    case WAIT_FAILED:    /* WaitForSingleObject function failed */
	Fail("WaitForSingleObject call returned 'WAIT_FAILED'.\n"
	     "GetLastError returned %d\nFailing Test.\n",GetLastError());
	break;
    default:
	Fail("WaitForSingleObject call returned an unexpected value.\n"
	     "Failing Test.\n");
	break;
    }

}

/* 
 * Atomic increment of semaphore value.
 */
VOID
up(HANDLE hSemaphore) 
{
    if (!ReleaseSemaphore (
	    hSemaphore,  
	    1,           
	    NULL)        
	)
    {
	Fail("ReleaseSemaphore call failed.  GetLastError returned %d\n",
	     GetLastError());
    }
}

/* 
 * Sleep 500 milleseconds.
 */
VOID 
consumerSleep(VOID)
{
    Sleep(500);
}

/* 
 * Sleep between 10 milleseconds.
 */
VOID 
producerSleep(VOID)
{
    Sleep(10);
}

/* 
 * Produce a message and write the message to Buffer.
 */
VOID
producer(BufferStructure *Buffer)
{

    int n = 0;
    char c;
    
    while (n < PRODUCTION_TOTAL) 
    {
	c = 'A' + n ;   /* Produce Item */   
                   
	down(hSemaphoreE);
	down(hSemaphoreM);
    
	if (writeBuf(Buffer, c)) 
	{
            Trace("Producer produces %c.\n", c);
	    fflush(stdout);
	    producerItems[n++] = c;
	}
    
	up(hSemaphoreM);
	up(hSemaphoreF);
    
	producerSleep();
    }

    return;
}

/* 
 * Read and "Consume" the messages in Buffer. 
 */
DWORD
PALAPI 
consumer( LPVOID lpParam )
{
    int n = 0;
    char c;   

    consumerSleep();

    while (n < PRODUCTION_TOTAL) 
    {
    
	down(hSemaphoreF);
	down(hSemaphoreM);

	if (readBuf((BufferStructure*)lpParam, &c)) 
	{
	    Trace("\tConsumer consumes %c.\n", c);
	    fflush(stdout);
	    consumerItems[n++] = c;
	}
    
	up(hSemaphoreM);
	up(hSemaphoreE);

	consumerSleep();
    }
    
    return 0;
}

int __cdecl main (int argc, char **argv) 
{

    BufferStructure Buffer, *pBuffer;

    pBuffer = &Buffer;
   
    if(0 != (PAL_Initialize(argc, argv)))
    {
        return ( FAIL );
    }

    /*
     * Create Semaphores
     */
    hSemaphoreM = CreateSemaphoreW (
	NULL,            
	1,               
	1,               
	NULL);           

    if ( NULL == hSemaphoreM ) 
    {
	Fail ( "hSemaphoreM = CreateSemaphoreW () - returned NULL\n"
	       "Failing Test.\nGetLastError returned %d\n", GetLastError());
    }


    hSemaphoreE = CreateSemaphoreW (
	NULL,            
	_BUF_SIZE ,      
	_BUF_SIZE ,      
	NULL);           

    if ( NULL == hSemaphoreE ) 
    {
	Fail ( "hSemaphoreE = CreateSemaphoreW () - returned NULL\n"
	       "Failing Test.\nGetLastError returned %d\n", GetLastError());
    }
    
    hSemaphoreF = CreateSemaphoreW (
	NULL,            
	0,               
	_BUF_SIZE ,      
	NULL);           

    if ( NULL == hSemaphoreF ) 
    {
	Fail ( "hSemaphoreF = CreateSemaphoreW () - returned NULL\n"
	       "Failing Test.\nGetLastError returned %d\n", GetLastError());
    }


    /* 
     * Initialize Buffer
     */
    pBuffer->writeIndex = pBuffer->readIndex = 0;

    /* 
     * Create Consumer
     */
    hThread = CreateThread(
	NULL,            
	0,               
	consumer,        
	&Buffer,         
	0,               
	&dwThreadId);    

    if ( NULL == hThread ) 
    {
	Fail ( "CreateThread() returned NULL.  Failing test.\n"
	       "GetLastError returned %d\n", GetLastError()); 
    }
    
    /* 
     * Start producing 
     */
    producer(pBuffer);
    
    /*
     * Wait for consumer to complete
     */
    WaitForSingleObject (hThread, INFINITE);

    /* 
     * Compare items produced vs. items consumed
     */
    if ( 0 != strncmp (producerItems, consumerItems, PRODUCTION_TOTAL) )
    {
	Fail("The producerItems string %s\n and the consumerItems string "
	     "%s\ndo not match. This could be a problem with the strncmp()"
	     " function\n FailingTest\nGetLastError() returned %d\n", 
	     producerItems, consumerItems, GetLastError());
    }

    Trace ("producerItems and consumerItems arrays match.  All %d\nitems "
	   "were produced and consumed in order.\nTest passed.\n",
	   PRODUCTION_TOTAL);

    PAL_Terminate();
    return ( PASS );

}
