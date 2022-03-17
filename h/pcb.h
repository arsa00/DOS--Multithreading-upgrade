// File: pcb.h
#ifndef _pcb_h_
#define _pcb_h_

#include "List.h"
#include "thread.h"
#include "SharedF.h"


/*typedef unsigned long StackSize;
typedef unsigned int Time; // time, x 55ms
typedef int ID;*/

class PCB 
{
public:
    PCB(Thread *thr, unsigned int time, unsigned long stackS);
    PCB() { id = Thread_ID++; }
    ~PCB();

    static void PCB_main_init();
    static void wrapper();

protected:
    friend class Thread;
public:
    unsigned ss;
    unsigned sp;
    unsigned bp; // pokazivac na stek segment, pointer, base-ptr
    unsigned int timeSlice; // vremenski kvant
    unsigned *stack;    // stek niti
    unsigned long stackSize;
    unsigned isFinished;
    unsigned isBlocked;
    unsigned isStarted;
    unsigned isUnlimited; // flag-ovi: zavrsena nit, blokirana, startovana, vremenski neogranicena;
    unsigned isChild;	// dodato zbog fork-a
    unsigned Error;

    Thread *myThread;
   // PCB *parent;

    unsigned long id;   // id svake niti (PCB-a)
    static unsigned long Thread_ID;     // staticna promenljiva koja generise id za svaku nit

    List<PCB> *waitingThreads;
    List<PCB> *myChildren;

    static const unsigned long MAX_ARRAY_SIZE; // proveriti ovo ogranicenje (64KB == 65536) ???
};

#endif
