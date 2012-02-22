extern "C"{
    //#include "kq.h"
}

#include "core.hpp"

#include "windows.h"

//#include "crtdbg.h"

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
