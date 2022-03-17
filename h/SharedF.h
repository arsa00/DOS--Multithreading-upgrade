// File: SharedF.h
#ifndef _shared_files_h_
#define _shared_files_h_

#define lock asm cli
#define unlock asm sti

#define lock_ asm pushf; \
		      asm cli;

#define unlock_ asm popf;

class PCB;

#include "thread.h"
#include "HashPCB.h"
#include "ListSEM.h"


class idleBody : public Thread
{
    public:
        idleBody() { }
    protected:
        virtual void run() 
        {
            while(1);
        }
};

class SHARED_FILES
{
public:
    static volatile int timer_counter;
    static volatile PCB *runningThread;
    static volatile HashMapPCB *allThreads;  // value == PCB* | key = PCB->id
    static volatile unsigned context_switch_on_demand;
    static PCB *idleThread;
    static idleBody *idle;
	static volatile int lockFlag;
	static volatile ListSem *allSemaphores;
	static volatile int error_num;
};
#endif
