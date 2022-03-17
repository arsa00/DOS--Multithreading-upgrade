#include "../h/SharedF.h"  // popraviti
#include "../h/pcb.h"

volatile ListSem *SHARED_FILES::allSemaphores = 0;
volatile HashMapPCB *SHARED_FILES::allThreads = new HashMapPCB();
volatile unsigned SHARED_FILES::context_switch_on_demand = 0;
volatile PCB *SHARED_FILES::runningThread = 0;
volatile int SHARED_FILES::timer_counter = 1;
volatile int SHARED_FILES::lockFlag = 0;
volatile int SHARED_FILES::error_num = 0;


idleBody *SHARED_FILES::idle = new idleBody();
PCB * SHARED_FILES::idleThread = allThreads->get(idle->getId());
