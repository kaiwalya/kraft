extern "C"{
    //#include "kq.h"
}

#include "IntegerTypes.hpp"

#include "windows.h"

#include "crtdbg.h"

#include "malloc.h"
#include "string.h"

using namespace kq::core;

/*
MemoryManager
    allocate
    deallocate
    reallocate

FileSystem(System Network Memory)
VirtualFileSystem -
    Interact with Real File Systems
    Directories       
    Files
    Iterator
        Move In, MoveOut, OpenChildFile, CloseChildFile;
        Mount/Umount

*/

inline void setBit(void * pStart, ui32 nBit){
    ui8 * pByte = ((ui8 *)pStart) + (nBit >> 3);
    *pByte = (*pByte | (1 << (nBit & 7)));
};

inline void resetBit(void * pStart, ui32 nBit){
    ui8 * pByte = ((ui8 *)pStart) + (nBit >> 3);
    *pByte = *pByte & ~(1 << (nBit & 7));
};

inline ui8 getBit(void * pStart, ui32 nBit){   
    return
        (
            *(((ui8 *)pStart) + (nBit >> 3))
            &
            (1 << (nBit & 7))
        );
};

class RefCounter{
public:
    void * object;
    ui32 count;
    bool (*AtLast)(RefCounter *);

    RefCounter(void * object = 0, ui32 count = 0, bool (*AtLast)(RefCounter *) = 0){   
        this->object = object;
        this->count = count;
        this->AtLast = AtLast;
    }


    RefCounter(void * object, bool (*AtLast)(RefCounter *)){

        this->count = 0;
        this->object = object;
        this->AtLast = AtLast;
    }

};

static RefCounter nullCounter;

template<typename t>
class Pointer{
    RefCounter * m_pRefCounter;
    void * m_pBufferedObject;

    void setReference(RefCounter * pRefCounter){
        m_pRefCounter = pRefCounter;
        m_pBufferedObject = pRefCounter->object;
    };

    void attach(RefCounter * pRefCounter){
        setReference(pRefCounter);

        m_pRefCounter->count++;       
    };
   
    void detach(){
        m_pRefCounter->count--;
        if(!m_pRefCounter->count && m_pRefCounter->object){
            if( !(m_pRefCounter->AtLast) || !(*(m_pRefCounter->AtLast))(m_pRefCounter) ){
                delete (t *)m_pRefCounter->object;
                delete m_pRefCounter;
            }
        }
        setReference(&nullCounter);

    }

   
public:

    Pointer(RefCounter * pRefCounter){
        attach(pRefCounter);
    };

    Pointer(const Pointer<t> & pointer){
        attach(pointer.m_pRefCounter);
    };

    Pointer(){
        attach(&nullCounter);
    }

    ~Pointer(){
        detach();
    }

    Pointer<t> & operator = (const Pointer<t> & oprand){
        if(m_pRefCounter->object != oprand.m_pRefCounter->object){
            detach();
            attach(oprand.m_pRefCounter);           
        }
        return *this;
    };

    operator Pointer<const t>()const{

        Pointer<const t> ret(m_pRefCounter);
        return ret;
    }

    bool operator == (const Pointer<t> & oprand) const{
        return (oprand.m_pRefCount->object == m_pBufferedObject);

    }

    bool operator != (const Pointer<t> & oprand) const{
        return (oprand.m_pRefCount->object != m_pBufferedObject);
    }

    t * operator ->()const {
        return (t *)(m_pBufferedObject);
    };

};

class ITicker{
public:
    virtual ui64 getTickCount() const = 0;
    virtual ui64 getTicksPerSecond() const = 0;
    virtual ~ITicker(){}
};

class IClock{
public:
    virtual i64 waitForNextTick() const = 0;
};

class WindowsAPITicker:public ITicker{

    ui64 iTicksPerSecond;   

public:
    WindowsAPITicker(){

        LARGE_INTEGER li;
        if(QueryPerformanceFrequency(&li)){           
            iTicksPerSecond = li.QuadPart;           
        }else{
            iTicksPerSecond = 0;
           
        }
    }

    ui64 getTicksPerSecond() const{
        return iTicksPerSecond;
    }

    ui64 getTickCount() const{

        LARGE_INTEGER li;
        ui64 iTicks;
        if(QueryPerformanceCounter(&li)){
            iTicks = li.QuadPart;
        }else{
            iTicks = 0;
        }
       
        return iTicks;       
    }
};

class IStopWatch:public ITicker{
public:
    virtual void reset() = 0;
};

class StopWatch:public IStopWatch{
    Pointer<const ITicker> m_pProvider;
    ui64 m_iInitialTime;
public:
    StopWatch(Pointer<const ITicker> pTimeProvider):m_pProvider(pTimeProvider){
        reset();
    }

    void reset(){
        m_iInitialTime = m_pProvider->getTickCount();
    };

    ui64 getTickCount() const{
        return m_pProvider->getTickCount() - m_iInitialTime;
    }

    ui64 getTicksPerSecond() const{
        return m_pProvider->getTicksPerSecond();
    };
};

class ISleeper{
public:
    virtual ui64 milliSleep(ui64 nMilli) const = 0;
};

class WindowsAPISleeper :public ISleeper{
public:
    ui64 milliSleep(ui64 nMilli) const{
        Sleep((DWORD)nMilli);
        return (DWORD)nMilli;
    };
};

class Clock:public IClock{

    Pointer<IStopWatch> m_pStopWatch;
    Pointer<const ISleeper> m_pSleeper;   
    double m_dMilliSecondsPerCycle;

public:
    Clock(double dTargetCyclesPerSecond, Pointer<IStopWatch> pStopWatch, Pointer<const ISleeper> pSleeper){
        m_dMilliSecondsPerCycle = 1000.0 / dTargetCyclesPerSecond;
        m_pStopWatch = pStopWatch;
        m_pSleeper = pSleeper;

    };

    i64 waitForNextTick() const{
        double dTicksPerSecond = (double)m_pStopWatch->getTicksPerSecond();
        double dConsumedTicksThisCycle = (double)m_pStopWatch->getTickCount();
        m_pStopWatch->reset();

        double dConsumedMilliSecondsThisCycle = 1000.0 * dConsumedTicksThisCycle / dTicksPerSecond;
        double dRemainingMilliSecondsThisCycle = m_dMilliSecondsPerCycle - dConsumedMilliSecondsThisCycle;

        if(dRemainingMilliSecondsThisCycle >= 1.0){
            m_pSleeper->milliSleep((ui64)dRemainingMilliSecondsThisCycle);           
        }

        return (i64)dRemainingMilliSecondsThisCycle;

    }

};