#include "../h/thread.h"  // popraviti
#include "../h/pcb.h"  // popraviti
#include "../h/SharedF.h"  // popraviti
#include "../h/kernel.h"
#include "../h/List.h"

#include <stdio.h>

Thread::Thread (StackSize stackSize, Time timeSlice)
{
	 lock_flag();
    //lock_
    myPCB = new PCB(this, timeSlice, stackSize);
    if(!myPCB) SHARED_FILES::error_num++;
   // unlock_
	 unlock_flag();
}

void Thread::start()
{
    if(myPCB->isStarted == 1)
        return;
    myPCB->isStarted = 1;
    Scheduler::put(myPCB);
}

ID Thread::getId() { return myPCB->id; }

ID Thread::getRunningId() { return SHARED_FILES::runningThread->id; }

Thread* Thread::getThreadById(ID id)
{
	lock_flag();
    PCB *pcb = SHARED_FILES::allThreads->get(id);
    unlock_flag();
    return (pcb == 0 ? 0 : pcb->myThread);
}

void Thread::waitToComplete()   // mozda bi bilo dobro izbaciti lock - unlock
{
    lock_flag();
    if(myPCB->isFinished || myPCB->id == SHARED_FILES::runningThread->id || SHARED_FILES::idleThread->id == myPCB->id)
    {
        unlock_flag();
        return;
    }

    myPCB->waitingThreads->add((PCB *) SHARED_FILES::runningThread);
    SHARED_FILES::runningThread->isBlocked = 1;

    dispatch();
    unlock_flag();
}

Thread::~Thread()
{
    lock_flag();	// bilo lock
    SHARED_FILES::allThreads->remove(this->myPCB->id);
    this->myPCB->myThread = 0;
    delete myPCB;
    unlock_flag();	// bilo unlock
}


// ------------------ VILJUSKA ------------------

unsigned bp_fork, sp_fork, ss_fork, parent_sp, parent_ss, parent_bp;
unsigned old_bp = 1, temp = 1;

PCB *child_pcb = 0;
void interrupt save_context()
{
	memcpy(child_pcb->stack, SHARED_FILES::runningThread->stack, SHARED_FILES::runningThread->stackSize * sizeof(unsigned));
	asm{
		mov sp_fork, sp
		mov bp_fork, bp
		mov ss_fork, ss
	}
}

Thread *child;
int errors_num1, errors_num2;
ID Thread::fork()
{
	lock_flag();
	//lock_
	errors_num1 = SHARED_FILES::error_num;
	child = SHARED_FILES::runningThread->myThread->clone();
	errors_num2 = SHARED_FILES::error_num;
	//unlock_
	if(errors_num1 != errors_num2 || child==0 || child->myPCB == 0 || child->myPCB->Error == 1)
	{
		unlock_flag();
		return -1;
	}
	child_pcb = child->myPCB;
	save_context();
	if(SHARED_FILES::runningThread->isChild == 0)
	{
		// KONTEKST RODITELJA
			child_pcb->bp = FP_OFF(child_pcb->stack) + bp_fork - FP_OFF(SHARED_FILES::runningThread->stack);
			child_pcb->sp = FP_OFF(child_pcb->stack) + sp_fork - FP_OFF(SHARED_FILES::runningThread->stack);
			child_pcb->ss = FP_SEG(child_pcb->stack) + ss_fork - FP_SEG(SHARED_FILES::runningThread->stack);


			// prevezivanje BP-ova
				ss_fork = child_pcb->ss;
				sp_fork = child_pcb->sp;
				bp_fork = child_pcb->bp;
				asm{
					mov parent_bp, bp
					mov parent_sp, sp
					mov parent_ss, ss
					mov ss, ss_fork
					mov bp, bp_fork
					mov sp, sp_fork
					mov temp, ax
				}
				while(1)
				{
					asm {
						mov ax, [bp]
						mov old_bp, ax
					}

					if(old_bp == 0) { break; }
					//printf("PR: %d ", old_bp);
					old_bp = FP_OFF(child->myPCB->stack) + old_bp - FP_OFF(SHARED_FILES::runningThread->stack);
					//printf("PO: %d\n", old_bp);
					asm{
						mov ax, old_bp
						mov [bp], ax
						mov bp, ax
					}
				}

				asm{
					mov ax, temp
					mov ss, parent_ss
					mov bp, parent_bp
					mov sp, parent_sp
				}

			// postavljanje flega da je dete
			child->myPCB->isChild = 1;

			child->myPCB->isBlocked =  SHARED_FILES::runningThread->isBlocked;
			child->myPCB->isFinished =  SHARED_FILES::runningThread->isFinished;
			child->myPCB->isStarted =  SHARED_FILES::runningThread->isStarted;
			child->myPCB->isUnlimited =  SHARED_FILES::runningThread->isUnlimited;
			child->myPCB->stackSize = SHARED_FILES::runningThread->stackSize;
			child->myPCB->timeSlice = SHARED_FILES::runningThread->timeSlice;
			child->myPCB->Error = 0;
			child->myPCB->myThread = child;
			//lock_
				child->myPCB->waitingThreads = new List<PCB>();
				child->myPCB->myChildren = new List<PCB>();
			//unlock_

			// ubaciti dete u listu dece
			SHARED_FILES::runningThread->myChildren->add(child->myPCB);
			// ubaciti dete u Scheduler
			Scheduler::put(child->myPCB);

			unlock_flag();
			//unlock_
			return child->getId();
	}
	else
	{
	// KONTEKST DETETA
		lock_flag();
		SHARED_FILES::runningThread->isChild = 0;
		unlock_flag();
		return 0;
	}
}

void Thread::waitForForkChildren()
{
	//lock
	lock_flag();
	    List<PCB>::Iterator iter = SHARED_FILES::runningThread->myChildren->getIter();
	unlock_flag();
	    while (iter.hasNext())
	    {
	        PCB* old = iter.next();
	        old->myThread->waitToComplete();
	    }
	//unlock
}

void Thread::exit()
{
	lock_flag();
	//lock_
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

	//unlock_
	dispatch();
}






// backup

// prevezivanje BP-ova
/*
	ss_fork = child_pcb->ss;
	sp_fork = child_pcb->sp;
	bp_fork = child_pcb->bp;
	asm{
		cli
		mov parent_bp, bp
		mov parent_sp, sp
		mov parent_ss, ss
		mov ss, ss_fork
		mov bp, bp_fork
		mov sp, sp_fork
		mov temp, ax
	}
	while(1)
	{
		asm {
			mov ax, [bp]
			mov old_bp, ax
		}

		if(old_bp == 0) { break; }
		//printf("PR: %d ", old_bp);
		old_bp = FP_OFF(child->myPCB->stack) + old_bp - FP_OFF(SHARED_FILES::runningThread->stack);
		//printf("PO: %d\n", old_bp);
		asm{
			mov ax, old_bp
			mov [bp], ax
			mov bp, ax
		}
	}

	asm{
		mov ax, temp
		mov ss, parent_ss
		mov bp, parent_bp
		mov sp, parent_sp
		sti
	}
*/








/*
 * PCB *child_pcb = 0;
int i;
void interrupt save_context()
{
	memcpy(child_pcb->stack, SHARED_FILES::runningThread->stack, SHARED_FILES::runningThread->stackSize * sizeof(unsigned));
	asm{
		mov ax, [bp]
		mov temp, ax
		mov sp_fork, sp
		mov bp_fork, bp
		mov ss_fork, ss
	}

	// prevezivanje SP-a i BP-a
	//printf("PARENT\nSTACK\tSEG: %d\tOFF: %d\n\n", FP_SEG(SHARED_FILES::runningThread->stack), FP_OFF(SHARED_FILES::runningThread->stack));
	//printf("CHILD\nSTACK\tSEG: %d\tOFF: %d\nSS: %d\n\n", FP_SEG(child->myPCB->stack), FP_OFF(child->myPCB->stack), child->myPCB->ss);
	//printf("SAVE_CONT: %d i %d\n", bp_fork, sp_fork);
	//printf("SAVE_CONT: %d i %d; TMEP: %d\n", sp_fork, bp_fork, temp);
	for(i = SHARED_FILES::runningThread->stackSize - 1; i >= 0; i--)
	{
		//if(SHARED_FILES::runningThread->stack[i] != child_pcb->stack[i])
		printf("P: %d   C: %d; %d\n", SHARED_FILES::runningThread->stack[i], child_pcb->stack[i], i);
	}
}

Thread *child;
ID Thread::fork()
{

	lock_flag();
	child = SHARED_FILES::runningThread->myThread->clone();
	child_pcb = child->myPCB;
	save_context();
	if(SHARED_FILES::runningThread->isChild == 0)
	{
		child_pcb->bp = FP_OFF(child_pcb->stack) + bp_fork - FP_OFF(SHARED_FILES::runningThread->stack);
		child_pcb->sp = FP_OFF(child_pcb->stack) + sp_fork - FP_OFF(SHARED_FILES::runningThread->stack);
		child_pcb->ss = FP_SEG(child_pcb->stack) + ss_fork - FP_SEG(SHARED_FILES::runningThread->stack);
		//printf("FORK:\t SEG: %d OFF: %d\n", cs_fork, ip_fork);
		// KONTEKST RODITELJA
			// formiranje deteta
			// set-ovati fleg da je dete
			child->myPCB->isChild = 1;
			// formirati kontekst i dodati u Scheduler

			// formiranje (kopiranje) PCB-a
			child->myPCB->isBlocked =  SHARED_FILES::runningThread->isBlocked;
			child->myPCB->isFinished =  SHARED_FILES::runningThread->isFinished;
			child->myPCB->isStarted =  SHARED_FILES::runningThread->isStarted;
			child->myPCB->isUnlimited =  SHARED_FILES::runningThread->isUnlimited;
			child->myPCB->stackSize = SHARED_FILES::runningThread->stackSize;
			child->myPCB->timeSlice = SHARED_FILES::runningThread->timeSlice;
			child->myPCB->myThread = child;
			child->myPCB->waitingThreads = 0; // nisam siguran za ovo, proveriti!
			Scheduler::put(child->myPCB);


			for(int i = child->myPCB->stackSize - 1; i >= 0; i--)
			{
				if(SHARED_FILES::runningThread->stack[i] != child->myPCB->stack[i])
				printf("P: %d   C: %d; %d\n", SHARED_FILES::runningThread->stack[i], child->myPCB->stack[i], i);
			}

			//printf("SP nakon init: %d\nSS nakon init: %d\nBP nakon init: %d\n", child->myPCB->sp, child->myPCB->ss, child->myPCB->bp);
			//printf("PROC:\tSP nakon init: %d\nPROC:\tSS nakon init: %d\nPROC:\tBP nakon init: %d\n", parent_sp, parent_ss, parent_bp);

		// dodati dete u listu dece

			//printf("\nRoditelj prosao\n");
			unlock_flag();
			return child->getId();
	}
	else
	{
		//printf("\nDete proslo\n");
		SHARED_FILES::runningThread->isChild = 0;
		unlock_flag();

		return 0;
	}
}
 * */





/*
 * /*	asm {
				mov sp_fork, sp
				mov ss_fork, ss
				mov bp_fork, bp
				mov temp, ax
				mov ax, [bp]
				mov old_bp, ax
				mov ax, temp
			}

		/*
*/
			// prevezivanje SS-a, SP-a i BP-a
		/*	child->myPCB->bp = FP_OFF(child->myPCB->stack) + bp_fork - FP_OFF(SHARED_FILES::runningThread->stack);
			child->myPCB->sp = FP_OFF(child->myPCB->stack) + sp_fork - FP_OFF(SHARED_FILES::runningThread->stack);
			child->myPCB->ss = FP_SEG(child->myPCB->stack) + ss_fork - FP_SEG(SHARED_FILES::runningThread->stack);
			old_bp = FP_OFF(child->myPCB->stack) + old_bp - FP_OFF(SHARED_FILES::runningThread->stack);*/
			//printf("OLD BP: %d\tRET_IP: %d\tRET_CS: %d\tCODE SEG: %d\n", old_bp, ret_ip, ret_cs, -1);
			/*ret_ip = FP_OFF(child->run) + ret_ip - FP_OFF(SHARED_FILES::runningThread->myThread->run);
			ret_cs = FP_SEG(child->run) + ret_cs - FP_SEG(SHARED_FILES::runningThread->myThread->run);*/
			//asm mov ss_fork, ds
			//printf("OLD BP::::::::::::::::: %d\n", old_bp);
			//printf("PARENT\nSTACK\tSEG: %d\tOFF: %d\nSS: %d\tSP: %d\tBP: %d\n\n", FP_SEG(SHARED_FILES::runningThread->stack), FP_OFF(SHARED_FILES::runningThread->stack), ss_fork, sp_fork, bp_fork);
			//printf("CHILD\nSTACK\tSEG: %d\tOFF: %d\nSS: %d\tSP: %d\tBP: %d\tOLD: %d\n\n", FP_SEG(child->myPCB->stack), FP_OFF(child->myPCB->stack), child->myPCB->ss, child->myPCB->sp, child->myPCB->bp, old_bp);












/*void Thread::fork()
{
	ret_ip = FP_OFF(example);
	ret_cs = FP_SEG(example);
printf("Pre dete %d %d\n", ret_ip, ret_cs);

asm {
								mov temp_sp, sp
								mov sp, bp
								mov temp, ax
								pop ax
								mov old_bp, ax
								pop ax
								mov ret_ip, ax
								pop ax
								mov ret_cs, ax
								mov ax, ret_cs
								push ax
								mov ax, ret_ip
								push ax
								mov ax, old_bp
								push ax
								mov ax, temp
								mov sp, temp_sp
				}
printf("Proslo dete %d, %d, %d\n", old_bp, ret_cs, ret_ip);
ret_ip = FP_OFF(example);
	ret_cs = FP_SEG(example);

			asm {
				mov temp_sp, sp
				mov temp, bp
				mov sp, bp
				pop old_bp
			//	mov old_bp, ax
				//pop ax
				//mov sp_fork, ax
				//pop ax
				//mov ss_fork, ax
				//mov ax, ret_cs
				push ret_cs
				//mov ax, ret_ip
				push ret_ip
				//mov ax, old_bp
				push old_bp
				mov bp, sp
				//mov ax, temp
				mov sp, temp_sp
			}

			asm {
								mov temp_sp, sp
								mov sp, bp
								mov temp, ax
								pop ax
								mov old_bp, ax
								pop ax
								mov ret_ip, ax
								pop ax
								mov ret_cs, ax
								mov ax, ret_cs
								push ax
								mov ax, ret_ip
								push ax
								mov ax, old_bp
								push ax
								mov ax, temp
								mov sp, temp_sp
				}
printf("Proslo dete2 %d, %d, %d\n", old_bp, ret_cs, ret_ip);

}*/
