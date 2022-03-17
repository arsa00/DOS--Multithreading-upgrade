#include "../h/pcb.h"  // popraviti
#include <dos.h>
#include "../h/kernel.h"

#include <stdio.h>

unsigned long PCB::Thread_ID = 0;
const unsigned long PCB::MAX_ARRAY_SIZE = 65535;

PCB::PCB(Thread *thr, unsigned int time, unsigned long stackS) : myThread(thr), timeSlice(time), stackSize(stackS), id(Thread_ID++)
{
        if(stackSize > MAX_ARRAY_SIZE) stackSize = MAX_ARRAY_SIZE;
        stackSize /= sizeof(unsigned); // mozda stavi 2

       // lock
        stack = new unsigned[stackSize];    // proveriti moze li
       // unlock

        if(timeSlice == 0)
            isUnlimited = 1;
        else
            isUnlimited = 0;
        isStarted = 0;
        isFinished = 0;
        isBlocked = 0;
        isChild = 0;

      //  lock
		myChildren = new List<PCB>();
        waitingThreads = new List<PCB>();    // proveriti moze li
       // unlock

        if(myChildren == 0 || waitingThreads == 0 || stackSize == 0)
        {
              Error = 1;
              SHARED_FILES::error_num++;
        }
        else
        {
              Error = 0;
              // inicijalizacija konteksta niti:
                      stack[stackSize - 1] = 0x200;                 // setovan I fleg u pocetnom PSWu za nit
                      stack[stackSize - 2] = FP_SEG(PCB::wrapper);       // PC za nit predstavlja adresa metode wrapper
                      stack[stackSize - 3] = FP_OFF(PCB::wrapper);
                      stack[stackSize -12] = 0;	// proveriti
                      sp = (unsigned)FP_OFF(stack + stackSize - 12);    // svi sacuvani registri pri ulasku u interrupt rutinu
                      ss = (unsigned)FP_SEG(stack + stackSize - 12);
                      bp = (unsigned)FP_OFF(stack + stackSize - 12);    // Izmena: pocetni bp treba da pokazuje na poziciju na kojoj se cuva stara vrednost bp

                      SHARED_FILES::allThreads->add(this, this->id);
        }


}

void PCB::PCB_main_init()
{
    PCB *main = new PCB(); // proveriti moze li
    main->myThread = 0;
    main->timeSlice = 2;
   // main->stackSize = 4096;
    main->isUnlimited = 0;
    main->isStarted = 1;
    main->isFinished = 0;
    main->isBlocked = 0;
    main->isChild = 0;
    main->waitingThreads = new List<PCB>();    // proveriti moze li
    main->myChildren = new List<PCB>();
    SHARED_FILES::allThreads->add(main, main->id);
    SHARED_FILES::runningThread = main;
}

void PCB::wrapper()
{
    //if(SHARED_FILES::runningThread->myThread != 0)
        SHARED_FILES::runningThread->myThread->run();

	lock_flag();
     //  lock
    List<PCB>::Iterator iter = SHARED_FILES::runningThread->waitingThreads->getIter();

    while (iter.hasNext())
    {
        PCB* old = iter.next();
        old->isBlocked = 0;
        if(old->isFinished == 0)
            Scheduler::put(old);

        SHARED_FILES::runningThread->waitingThreads->remove(old);
    }

    SHARED_FILES::runningThread->isFinished = 1;
    unlock_flag();

   // unlock
	dispatch();
}

PCB::~PCB()
{
	    List<PCB>::Iterator iter = this->waitingThreads->getIter();
	    while (iter.hasNext())
	    {
	        PCB* old = iter.next();
	        old->isBlocked = 0;
	        if(old->isFinished == 0)
	            Scheduler::put(old);
	        this->waitingThreads->remove(old);
	    }

	    iter = this->myChildren->getIter();
	   	while (iter.hasNext())
	   	{
	   	    PCB* old = iter.next();
	   	    this->myChildren->remove(old);
	   	}
	   	lock_
	   		delete [] stack;
	   		delete this->myChildren;
	   		delete this->waitingThreads;
	   	unlock_
}
