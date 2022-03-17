#ifndef _ivt_entry_h_
#define _ivt_entry_h_

typedef void interrupt (*pInterrupt)(...);

#include "HashIVTE.h"
class KernelEv;

class IVTEntry
{
private:
    pInterrupt oldRoutine;
    unsigned char entryNumber;
    KernelEv *kernelEvent;

    //static const unsigned char TIMER_INTERRUPT_NEW_ROUTINE,  TIMER_INTERRUPT_OLD_ROUTINE;
    const unsigned char ENTRY_LIMIT;
public:
    IVTEntry(unsigned char entryNo, pInterrupt newInterruptRoutine);
    ~IVTEntry();

    void call_old_routine();
    void signal();
    void set_kernel_event(KernelEv *event);

    static IVTEntry *get_ptr_to_IVTEntry(unsigned char entryNo);    // vraca pokazivac na IVTEntry instancu za zadati ulaz
    static HashMapIVTEntry *allIVTEntries;  // hash mapa svih IVTEntry instanci
};

#endif
