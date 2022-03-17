#ifndef _kernel_semaphor_h_
#define _kernel_semaphor_h_

#include "List.h"
#include "pcb.h"

class KernelSem
{

struct SemElement
{
    PCB* pcb;
    unsigned int TimeSlice;
    unsigned int PassedTimeSlices;
    int unblocked_by_signal;

    SemElement() : pcb(0), TimeSlice(0), PassedTimeSlices(0), unblocked_by_signal(0) { }
};


public:
    int wait(unsigned int TimeToWait);
    void signal();
    void tickSem();
    int getValue() const { return val; }

    KernelSem(int init_val);
    ~KernelSem();

    int id;
private:
    int val;
    List<SemElement> *blocked_all;
    List<SemElement> *blocked_limited;

    static int ID;

    int block(unsigned int TimeSlice);
    void unblock();
};


#endif
