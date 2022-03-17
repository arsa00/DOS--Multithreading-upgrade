#ifndef _kernel_event_h_
#define _kernel_event_h_

#include "IVTEntry.h"
#include "pcb.h"

class KernelEv
{
public:
    KernelEv(unsigned char ivtNo);
    ~KernelEv();
    void wait();
    void signal();
private:
    IVTEntry *MyIVTEntry;
    PCB *owner;
    int val;
    unsigned isOwnerBlocked;
};

#endif