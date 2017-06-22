// memory.cpp
// (C)1998,1999,2000,2008,2009,2010 Kenneth Boyd, license: MIT.txt
// implementation of the following:
//	_msize
//	malloc
//	free
//	calloc
//	realloc

// All allocations have no slack space.  [A request for 1 byte will get just that].
// This will cause micro-leaks on implementations that support address alignment, but
// is needed to allow calculating array size from a dynamically allocated pointer.
// This saves sizeof(size_t) bytes/object in the main program (but as the size is stored twice)
// in the memory manager, there should be no net gain.

// Startup code: reserve a huge block of VRAM
// Commit it only as necessary
// two stacks: bottom-up is stack of pointers
// top-down is the allocated memory

// Note: free is a no-op on NULL pointers (per standard)
// Note: free(pointer that isn't currently allocated) is immediately fatal; ditto for realloc.
// Note: we return NULL when malloc'ing 0 bytes.  If new should be used to allocate 0 bytes, it *will* fail (non-ISO behavior)
// Note: we do overflow checking in calloc, per CERT C MEM07.

//! \todo CERT C MEM03 would like data explicitly cleared before release.  Decide on best metaphor for this.

//! \todo New extensions (extern "C" interface); these affect memory.hxx
//!	void _lock_block(size_t size);
//!  void* _unlock_malloc(void);
//!  void* _unlock_calloc(void);
//
//!	_lock_block instructs the memory manager to recycle free blocks of sufficient
//! size (parameter at least size), rather than totally deallocating them.  This parameter
//! starts at 0.  This recycling should account for memory alignment on appropriate target
//! systems (e.g., Mac)
//!  _unlock_malloc and _unlock_calloc return NULL if no block has been previously locked.
//! Otherwise, returns the memory block and sets the internal block-trap parameter to 0.

//! \todo new extension (extern "C" interface)
//!	void _dual_realloc(void* memblock, size_t size)
//! this function is like realloc, except it truncates data from the *beginning* when reducing
//! the memory block size.

#define ZAIMONI_STL_IN_MEMORY_CPP 1

#include "../z_memory.h"

#undef ZAIMONI_STL_IN_MEMORY_CPP

#ifdef __cplusplus
#include "../OS/AIMutex.hpp"	// pulls in Windows.h
#elif defined(_WIN32)
#include <WINDOWS.H>
#else
#error("Error: headers for memory.cpp not implemented.")
#endif

// OPTIMIZATION NOTES:
// EVERYTHING NOT BOOTSTRAP IS TIME-CRITICAL
// Once stabilized, use ASM w/sourcecode backup
// currently unstable

// report errors using Win95 MessageBox
// this should have an AIMutex for multi-threaded compatibility

//! \todo clean up function call graphs, then improve version number
#define RAMManagerName "Zaimoni.com Memory Manager System V0.04"
#define PointerSizeInfoCorrupted "A pointer with corrupted size information has been detected."
#define MayICleanClipboard "Memory failure imminent.  May I delete the Clipboard contents?"
#define PleaseCloseOtherDocumentsAndApps "Memory failure imminent.  Please close nonessential documents and applications."

#define NoPtrRAM "Unable to allocate pointer index RAM."
#define FreeNonNULLInvalid "free() was given a pointer it shouldn't be able to free."
#define ReallocNonNULLInvalid "realloc() was given a pointer it shouldn't be able to reallocate."
#define ExpandNonNULLInvalid "_expand() was given a pointer it shouldn't be able to reallocate."
#define InvalidWriteDetected "An invalid write to RAM has been detected."
#define AlphaInvalidLocationsDetected "ALPHA: overlapping memory blocks detected."
#define AlphaPointerTableOverextended "ALPHA: pointer table overextended."

size_t AppRunning = 0;	// Controls Microsoft bypass

#ifdef __cplusplus
// This needs the AIMutex.hxx header
static AIMutex RAMBlock;	// RAM Block.  Must unlock for exit()
#endif

struct _track_pointer {
	char* _address;
	size_t _size;
};

#ifndef __cplusplus
typedef struct _track_pointer _track_pointer;
#endif

union _track_toggle {
	char* raw;
	_track_pointer* records;
};

#ifndef __cplusplus
typedef union _track_toggle _track_toggle;
#endif

//! internal usage of RawBlock
//!	<br>RawBlock+0: start of 8-byte records: 1st 4 is address, 2nd 4 is size allocated
//!	<br>Pointers are allocated from the top down.
//! <br>pointer format [char* default]:
//!	<br>** Ptr-sizeof(size_t): size_t recording size of allocated memory
//!	<br>** Ptr: start of allocated memory
//!	<br>** sizeof(void*) after: NULL [guard bytes]

static _track_toggle RawBlock = {NULL};
static size_t SizeOfRawBlock = 0;
static size_t CountPointersAllocated = 0;
static size_t PageSize = 0;
static char* StrictHighBoundIndexPages = NULL;	// lowbound is RawBlock
static char* LowBoundPtrSpace = NULL;				
static char* HighBoundPtrSpace = NULL;

// #define TRULY_WARY
#ifdef TRULY_WARY
static bool WantDebug = false;
static unsigned long DebugCounter = 0;
static char DebugBuffer[25];
#endif

inline void ReportError(const char* const FatalErrorMessage)
{	// FORMALLY CORRECT: Kenneth Boyd, 11/15/1999
#ifdef _WIN32
	MessageBox(NULL,FatalErrorMessage,RAMManagerName,MB_ICONSTOP | MB_OK | MB_SYSTEMMODAL);
#else
#error ERROR: Must implement void ReportError(const char* const FatalErrorMessage);
#endif
}

static void __ReportErrorAndCrash(const char* const FatalErrorMessage)
{	// FORMALLY CORRECT: Kenneth Boyd, 11/15/1999
	ReportError(FatalErrorMessage);
#ifdef __cplusplus
	RAMBlock.UnLock();
#endif
	exit(EXIT_FAILURE);
}

inline void InitBlock(void* memblock, size_t size)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/15/1999
#ifdef __cplusplus
	reinterpret_cast<size_t*>(reinterpret_cast<char*>(memblock)-sizeof(size_t))[0]=size;
	reinterpret_cast<void**>(reinterpret_cast<char*>(memblock)+size)[0]=NULL;
#else
	((size_t*)((char*)(memblock)-sizeof(size_t)))[0]=size;
	((void**)((char*)(memblock)+size))[0]=NULL;
#endif
}

#ifdef __cplusplus
inline size_t& PTRSIZE_FROM_IDX(size_t CurrIdx)
{return RawBlock.records[CurrIdx-1]._size;}
#else
#define PTRSIZE_FROM_IDX(CurrIdx) RawBlock.records[CurrIdx-1]._size
#endif

#ifdef __cplusplus
inline char*& CHARPTR_FROM_IDX(size_t CurrIdx)
{return RawBlock.records[CurrIdx-1]._address;}
#else
#define CHARPTR_FROM_IDX(CurrIdx) RawBlock.records[CurrIdx-1]._address
#endif

inline size_t HOLESIZE(size_t CurrIdxSubOne)
{
	return   RawBlock.records[CurrIdxSubOne-1]._address
			-RawBlock.records[CurrIdxSubOne]._address
			-RawBlock.records[CurrIdxSubOne]._size
			-sizeof(size_t)-sizeof(void*);
}

// uses global variable HighBoundPtrSpace
inline size_t HOLESIZE_AT_IDX1(void)
{return	 HighBoundPtrSpace
		-RawBlock.records[0]._address
		-RawBlock.records[0]._size
		-sizeof(void*);}

static void* __ExpandV2(void* memblock, size_t CurrIdx, size_t size)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/19/1999
	// Using goto to condense exit code
	if      (_msize(memblock)==size)
		goto ExitUnchanged;
	else if (_msize(memblock)>size)
		goto ResizeAndExit;
	else{	// EXPAND INPLACE
		size_t SpareSpace = (1==CurrIdx)	? HOLESIZE_AT_IDX1()
											: HOLESIZE(CurrIdx-1);
		if (_msize(memblock)+SpareSpace>size+sizeof(void*)+sizeof(size_t)) 
			goto ResizeAndExit;
		return NULL;
		};
ResizeAndExit:
	PTRSIZE_FROM_IDX(CurrIdx) = size;
	InitBlock(memblock,size);
ExitUnchanged:
	return memblock;
}

/*!
 * \return 1 above index if found, 0 otherwise
 */
static size_t
__IdxOfPointerInPtrList(const void* const Target, size_t strict_ub, const _track_pointer* const BasePtrIndex)
{	// FORMALLY CORRECT: Kenneth Boyd, 6/11/2010
	// this routine checks to see if Target was allocated by RAMManager, by 
	// binary-searching the array pointed to by BasePtrIndex.  
	// Idx is a strict upper bound to the valid indices.
	size_t lb = 0;
	while(lb<strict_ub)
		{
		const size_t midpoint = lb + (strict_ub-lb)/2;
		if (Target==BasePtrIndex[midpoint]._address)
			return midpoint+1;
		else if ((const char*)Target<BasePtrIndex[midpoint]._address)
			lb = midpoint+1;
		else
			strict_ub = midpoint;
		};
	return 0;
}

// This function returns a char* to where to start *putting* the data, after allowing for the
// size_t overhead behind it.
static char*
__FindHole(size_t size, char* AboveThisAddress, size_t AllocPtrCount, char* StrictUpperBoundPtrSpace)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/19/1999
	if (AllocPtrCount)
		{
		const size_t TargetSize = size+sizeof(size_t)+sizeof(void*);
#ifdef _WIN32
		const size_t Target1EffSize = (20>=TargetSize) ? 20 : 16*((TargetSize-5)/16)+20;
		const size_t TargetNEffSize = (16>=TargetSize) ? 16 : 16*((TargetSize-1)/16)+16;
#else
#error must implement alignment code
#endif
		char* Hole1 = NULL;
		size_t Hole1Size = 0;
		size_t Idx = 0;	// logically CurrIdx-1
		size_t HoleSize = HOLESIZE_AT_IDX1();
		// Idx==0 case
		if (HoleSize>=Target1EffSize)
			{
			if (HoleSize==Target1EffSize)
				return StrictUpperBoundPtrSpace-Target1EffSize+sizeof(size_t);
			else{
				Hole1 = StrictUpperBoundPtrSpace-Target1EffSize+sizeof(size_t);
				Hole1Size = HoleSize;
				HoleSize = 0;
				};			
			};
Restart:
		while(HoleSize<TargetNEffSize && ++Idx<AllocPtrCount &&	 AboveThisAddress
															<CHARPTR_FROM_IDX(Idx)-TargetSize)
			HoleSize = HOLESIZE(Idx);

		if		(HoleSize==TargetNEffSize)
			return CHARPTR_FROM_IDX(Idx)-TargetSize;
		else if (HoleSize>TargetNEffSize)
			{
			if (NULL==Hole1 || Hole1Size>HoleSize)
				{
				Hole1Size = HoleSize;
				Hole1 = CHARPTR_FROM_IDX(Idx)-TargetNEffSize;
				}
			HoleSize = 0;
			goto Restart;
			};
		return Hole1;
		};
	return NULL;
}

#if 0	// TODO: REDO memory2.azm
static void __OnePassInsertSort(_track_pointer* OffsetIntoPtrIndex, size_t CurrIdx);
#else	// base form
static void __OnePassInsertSort(_track_pointer* OffsetIntoPtrIndex, size_t CurrIdx)
{	// FORMALLY CORRECT: 9/19/1999
	// STABLE
#pragma message("This routine wants inline ASM: void __OnePassInsertSort(char* OffsetIntoPtrIndex, size_t CurrIdx);")
	while(	2<=CurrIdx
		  && OffsetIntoPtrIndex->_address>(OffsetIntoPtrIndex-1)->_address)
		{
		_track_pointer Tmp = *OffsetIntoPtrIndex;
		*(OffsetIntoPtrIndex) = *(OffsetIntoPtrIndex-1);
		*(OffsetIntoPtrIndex-1) = Tmp;
		CurrIdx--;
		--OffsetIntoPtrIndex;
		};
}
#endif

static void __RegisterPtr(void* memblock, size_t size, _track_pointer* const base)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/20/1999
	// MS C++: ASM already efficient, do not use inline ASM
	if (NULL!=memblock)
		{
		InitBlock(memblock,size);
		{
		_track_pointer* const target = base+ CountPointersAllocated++;
#ifdef __cplusplus
		target->_address=reinterpret_cast<char*>(memblock);
#else
		target->_address=((char*)(memblock));
#endif
		target->_size=size;
		__OnePassInsertSort(target,CountPointersAllocated);
		};
		}
}

static void __ReleaseNextIndexPage(void)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/17/1999
	StrictHighBoundIndexPages-=PageSize;
#ifdef _WIN32
	VirtualFree(StrictHighBoundIndexPages,PageSize-1,MEM_DECOMMIT);
#else
#error Must implement void __ReleaseNextIndexPage(void);
#endif
}

static void __CompactIndexRAM(void)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/15/1999
	size_t SpareIndexRAM = (StrictHighBoundIndexPages-RawBlock.raw)-sizeof(_track_pointer)*CountPointersAllocated;
	if (2*PageSize<SpareIndexRAM) __ReleaseNextIndexPage();
}

static void __MetaFree(size_t CurrIdx)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/19/1999
	// all error checking is done in free() or realloc().
	// remove entry from MasterList
	if (CurrIdx<CountPointersAllocated)
		memmove(RawBlock.records+(CurrIdx-1),
				RawBlock.records+CurrIdx,
				sizeof(_track_pointer)*(CountPointersAllocated-CurrIdx));
	CHARPTR_FROM_IDX(CountPointersAllocated) = NULL;
	PTRSIZE_FROM_IDX(CountPointersAllocated) = 0;
	CountPointersAllocated--;

	__CompactIndexRAM();
}

static void __DesperateCompactIndexRAM(void)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/16/1999
	size_t SpareIndexRAM = (StrictHighBoundIndexPages-RawBlock.raw)-sizeof(_track_pointer)*CountPointersAllocated;
	if (PageSize+sizeof(_track_pointer)<SpareIndexRAM)
		__ReleaseNextIndexPage();
}

static int __CommitNextPtrPages(size_t PageCount)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/16/1999
	LowBoundPtrSpace-=PageCount*PageSize;
#if _WIN32
	if (!VirtualAlloc(LowBoundPtrSpace,PageCount*PageSize-1,MEM_COMMIT,PAGE_EXECUTE_READWRITE))
		{	// program is about to crash from RAM failure
			// #1: spare space at bottom of Ptrspace
		if (   (__DesperateCompactIndexRAM(),!VirtualAlloc(StrictHighBoundIndexPages,PageCount*PageSize-1,MEM_COMMIT,PAGE_EXECUTE_READWRITE))
			// #2: clean clipboard
			&& (MessageBox(NULL,MayICleanClipboard,RAMManagerName,MB_ICONSTOP | MB_YESNO | MB_SYSTEMMODAL),!VirtualAlloc(StrictHighBoundIndexPages,PageCount*PageSize-1,MEM_COMMIT,PAGE_EXECUTE_READWRITE))
			// #3: ask human to close apps and documents
			&& (MessageBox(NULL,PleaseCloseOtherDocumentsAndApps,RAMManagerName,MB_ICONSTOP | MB_OK | MB_TASKMODAL),!VirtualAlloc(StrictHighBoundIndexPages,PageCount*PageSize-1,MEM_COMMIT,PAGE_EXECUTE_READWRITE)))
			{
			LowBoundPtrSpace+=PageCount*PageSize;
			return 0;
			}
		}
#else
#error Must implement int __CommitNextPtrPages(size_t PageCount);
#endif
	return 1;
}

static void* __CreateAtBottom(size_t size)
{	// FORMALLY CORRECT: Kenneth Boyd, 10/1/1999
	// this routine tries to make space at the bottom of PTRspace for a memory block of
	// size bytes, plus overhead [sizeof(size_t)+sizeof(void*)].  It does *not* actually
	// allocate anything.
	const size_t TargetSize = size+sizeof(size_t)+sizeof(void*);
	size_t TargetEffSize;
	size_t SparePtrRAM;
	char* TargetPtr;
	if (0==CountPointersAllocated)
		{
#ifdef _WIN32
		TargetEffSize = (20>=TargetSize) ? 20 : 16*((TargetSize-5)/16)+20;
#else
#error must implement alignment code
#endif
		SparePtrRAM = HighBoundPtrSpace-LowBoundPtrSpace;
		TargetPtr = HighBoundPtrSpace+sizeof(size_t);
		}
	else{
#ifdef _WIN32
		TargetEffSize = (16>=TargetSize) ? 16 : 16*((TargetSize-1)/16)+16;
#else
#error must implement alignment code
#endif
		SparePtrRAM = RawBlock.records[CountPointersAllocated-1]._address-LowBoundPtrSpace-sizeof(size_t);
		TargetPtr = CHARPTR_FROM_IDX(CountPointersAllocated);
		};
	TargetPtr -= TargetEffSize;
	if (SparePtrRAM>=TargetEffSize) return TargetPtr;
	{
	size_t PageCount = 0;
	do	{
		++PageCount;
		SparePtrRAM+=PageSize;
		}
	while(SparePtrRAM<TargetEffSize);
	return __CommitNextPtrPages(PageCount) ? TargetPtr : NULL;
	};
}

static void __ReleaseNextPtrPages(size_t PageCount)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/17/1999
	size_t PhysicalSize = PageCount*PageSize;
#ifdef _WIN32
	VirtualFree(LowBoundPtrSpace,PhysicalSize-1,MEM_DECOMMIT);
#else
#error Must implement void __ReleaseNextPtrPages(size_t PageCount);
#endif
	LowBoundPtrSpace += PhysicalSize;
}

static void __CompactPtrRAM(void)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/15/1999
	size_t SparePtrRAM = RawBlock.records[CountPointersAllocated-1]._address-LowBoundPtrSpace;
	if (2*PageSize<=SparePtrRAM)
		{
		size_t PageCount = 0;
		do	{
			++PageCount;
			SparePtrRAM-=PageSize;
			}
		while(2*PageSize<=SparePtrRAM);
		__ReleaseNextPtrPages(PageCount);
		};
}

static void __DesperateCompactPtrRAM(void)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/16/1999
	size_t SparePtrRAM = RawBlock.records[CountPointersAllocated-1]._address-LowBoundPtrSpace;
	if (PageSize<=SparePtrRAM)
		{
		size_t PageCount = 0;
		do	{
			++PageCount;
			SparePtrRAM-=PageSize;
			}
		while(PageSize<=SparePtrRAM);
		__ReleaseNextPtrPages(PageCount);
		};
}

static int __CommitNextIndexPage(void)
{	// FORMALLY CORRECT: Kenneth Boyd, 9/16/1999
#if _WIN32
	if (!VirtualAlloc(StrictHighBoundIndexPages,PageSize-1,MEM_COMMIT,PAGE_EXECUTE_READWRITE))
		{	// program is about to crash from RAM failure
			// #1: spare space at bottom of Ptrspace
		if (   (__DesperateCompactPtrRAM(),!VirtualAlloc(StrictHighBoundIndexPages,PageSize-1,MEM_COMMIT,PAGE_EXECUTE_READWRITE))
			// #2: clean clipboard
			&& (MessageBox(NULL,MayICleanClipboard,RAMManagerName,MB_ICONSTOP | MB_YESNO | MB_SYSTEMMODAL),!VirtualAlloc(StrictHighBoundIndexPages,PageSize-1,MEM_COMMIT,PAGE_EXECUTE_READWRITE))
			// #3: ask human to close apps and documents
			&& (MessageBox(NULL,PleaseCloseOtherDocumentsAndApps,RAMManagerName,MB_ICONSTOP | MB_OK | MB_TASKMODAL),!VirtualAlloc(StrictHighBoundIndexPages,PageSize-1,MEM_COMMIT,PAGE_EXECUTE_READWRITE)))
			return 0;
		}
#else
#error Must implement int __CommitNextIndexPage(void)
#endif
	StrictHighBoundIndexPages+=PageSize;
	return 1;
}

static void FlushMemoryManager(void)
{
// work around GCC 4.2.x issues
#if __GNUC__!=4 || __GNUC_MINOR__!=2
#ifdef __cplusplus
	RAMBlock.Lock();
#endif
	if (NULL!=RawBlock.raw)
		{
#ifdef _WIN32
		VirtualFree(RawBlock.raw,SizeOfRawBlock,MEM_DECOMMIT);
		VirtualFree(RawBlock.raw,0,MEM_RELEASE);
#else
#error Must implement FlushMemoryManager()
#endif
		RawBlock.raw = NULL;
		};
#ifdef __cplusplus
	RAMBlock.UnLock();
#endif
#endif
}

static void __EmergencyInitialize(void)
{	// FORMALLY CORRECT: 11/3/1999
	size_t AllocateStep;
	size_t BaseSize;
#ifdef _WIN32
	{
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	PageSize = SystemInfo.dwPageSize;
	AllocateStep = SystemInfo.dwAllocationGranularity;
	BaseSize =	 (size_t)(SystemInfo.lpMaximumApplicationAddress)
				-(size_t)(SystemInfo.lpMinimumApplicationAddress);
	BaseSize -= (BaseSize%AllocateStep);
	};

#ifdef __cplusplus
	RawBlock.raw = reinterpret_cast<char*>(VirtualAlloc(NULL,BaseSize,MEM_RESERVE,PAGE_EXECUTE_READWRITE));
#else
	RawBlock.raw = ((char*)(VirtualAlloc(NULL,BaseSize,MEM_RESERVE,PAGE_EXECUTE_READWRITE)));
#endif
	while(NULL==RawBlock.raw && AllocateStep!=BaseSize)
		{
		BaseSize -= AllocateStep;
#ifdef __cplusplus
		RawBlock.raw = reinterpret_cast<char*>(VirtualAlloc(NULL,BaseSize,MEM_RESERVE,PAGE_EXECUTE_READWRITE));
#else
		RawBlock.raw = ((char*)(VirtualAlloc(NULL,BaseSize,MEM_RESERVE,PAGE_EXECUTE_READWRITE)));
#endif
		};
#else
#error Must implement void __EmergencyInitialize(void);
#endif

	if (NULL!=RawBlock.raw)
		{
		StrictHighBoundIndexPages = RawBlock.raw;
		SizeOfRawBlock = BaseSize;
		HighBoundPtrSpace = RawBlock.raw+SizeOfRawBlock;
		LowBoundPtrSpace = HighBoundPtrSpace;
		atexit(FlushMemoryManager);
		if (__CommitNextIndexPage()) return;
		};
	__ReportErrorAndCrash(NoPtrRAM);
}

static void __DetectOverwrites(void)
{	// FORMALLY CORRECT: Kenneth Boyd, 11/3/1999
	size_t i = CountPointersAllocated;
	_track_pointer* PtrRecordBase = RawBlock.records;
	if (0<i)
		{
		size_t j = i;
		{
		char* LowBoundLastBlock = NULL;
		char* HighBoundLastBlock = NULL;
		do	{
			_track_pointer* CurrentOffset = PtrRecordBase+ --j;
#ifdef TRULY_WARY
			if ((unsigned long)StrictHighBoundIndexPages<(unsigned long)CurrentOffset+sizeof(_track_pointer))
				__ReportErrorAndCrash(AlphaPointerTableOverextended);
#endif
			char* Target = CurrentOffset->_address;
			char* LowBoundThisBlock = Target-sizeof(size_t);
			char* HighBoundThisBlock = Target+CurrentOffset->_size+sizeof(void*);
			if (NULL!=LowBoundLastBlock && HighBoundLastBlock>LowBoundThisBlock)
				__ReportErrorAndCrash(AlphaInvalidLocationsDetected);
			LowBoundLastBlock = LowBoundThisBlock;
			HighBoundLastBlock = HighBoundThisBlock;
			}
		while(0<j);
		}
		j = i;
		do	{
			_track_pointer* CurrentOffset = PtrRecordBase+ --j;
			if (_msize(CurrentOffset->_address)!=CurrentOffset->_size)	// #1
				__ReportErrorAndCrash(PointerSizeInfoCorrupted);
			}
		while(0<j);
		do	{
			_track_pointer* CurrentOffset = PtrRecordBase+ --i;
			char* Target = CurrentOffset->_address;
			size_t TargetSize = CurrentOffset->_size;
#ifdef __cplusplus
			if (NULL!=reinterpret_cast<void**>(Target+TargetSize)[0])	// #2
#else
			if (NULL!=((void**)(Target+TargetSize))[0])	// #2
#endif
				{	// FATAL ERROR CODE
				char Buffer[10];
				ReportError(InvalidWriteDetected);
#ifdef __cplusplus
				_ltoa((size_t)(reinterpret_cast<void**>(Target+TargetSize)[0]),Buffer,16);
#else
				_ltoa((size_t)(((void**)(Target+TargetSize))[0]),Buffer,16);
#endif
				ReportError(Buffer);
				_ltoa(TargetSize,Buffer,10);
				__ReportErrorAndCrash(Buffer);
				};
			}
		while(0<i);
		};
}

// same as above, but suitable for assert()
// use int to avoid C/C++ issues (what is bool?)
#ifdef __cplusplus
extern "C"
#endif
int _no_obvious_overwrites(void)
{	// FORMALLY CORRECT: Kenneth Boyd, 11/3/1999
	size_t i = CountPointersAllocated;
	_track_pointer* PtrRecordBase = RawBlock.records;
	if (0<i)
		{
		size_t j = i;
		{
		char* LowBoundLastBlock = NULL;
		char* HighBoundLastBlock = NULL;
		do	{
			_track_pointer* CurrentOffset = PtrRecordBase+ --j;
#ifdef TRULY_WARY
			if ((unsigned long)StrictHighBoundIndexPages<(unsigned long)CurrentOffset+sizeof(_track_pointer))
				return 0;
#endif
			char* Target = CurrentOffset->_address;
			char* LowBoundThisBlock = Target-sizeof(size_t);
			char* HighBoundThisBlock = Target+CurrentOffset->_size+sizeof(void*);
			if (NULL!=LowBoundLastBlock && HighBoundLastBlock>LowBoundThisBlock)
				return 0;
			LowBoundLastBlock = LowBoundThisBlock;
			HighBoundLastBlock = HighBoundThisBlock;
			}
		while(0<j);
		}
		j = i;
		do	{
			_track_pointer* CurrentOffset = PtrRecordBase+ --j;
			if (_msize(CurrentOffset->_address)!=CurrentOffset->_size)	// #1
				return 0;
			}
		while(0<j);
		do	{
			_track_pointer* CurrentOffset = PtrRecordBase+ --i;
			char* Target = CurrentOffset->_address;
			size_t TargetSize = CurrentOffset->_size;
#ifdef __cplusplus
			if (NULL!=reinterpret_cast<void**>(Target+TargetSize)[0])	// #2
#else
			if (NULL!=((void**)(Target+TargetSize))[0])	// #2
#endif
				return 0;
			}
		while(0<i);
		};
	return 1;
}

#ifdef __cplusplus
namespace std
{
// actual implementation
extern "C"
#endif
void* _cdecl malloc(size_t size)
{	// FORMALLY CORRECT: Kenneth Boyd, 2/24/2008
	// We have the option, in C99, of returning NULL rather than allocating 0 bytes.  Do It.
	if (0==size) return NULL;

#ifdef __cplusplus
	RAMBlock.Lock();
#endif
	if (NULL==RawBlock.raw) __EmergencyInitialize();
	__DetectOverwrites();
	{	// space check: can we store the allocated pointer in the pointer index?
	size_t SpareIndexRAM = StrictHighBoundIndexPages-RawBlock.raw-sizeof(_track_pointer)*CountPointersAllocated;
	if (sizeof(_track_pointer)>SpareIndexRAM && !__CommitNextIndexPage())
		{
#ifdef __cplusplus
		RAMBlock.UnLock();
#endif
		return NULL;
		};
	};
	{
	void* Tmp = __FindHole(size,LowBoundPtrSpace,CountPointersAllocated,HighBoundPtrSpace);
	if (NULL==Tmp) Tmp = __CreateAtBottom(size);
	if (NULL!=Tmp) __RegisterPtr(Tmp,size,RawBlock.records);
#ifdef __cplusplus
	RAMBlock.UnLock();
#endif
	return Tmp;
	}
}

#ifdef __cplusplus
extern "C"
#endif
void* _cdecl calloc(size_t num, size_t size)
{	// FORMALLY CORRECT: Kenneth Boyd, 2/24/2008
	void* Tmp = NULL;
	if ((size_t)(-1)/size>=num)
	{	// CERT-mandated fix (MEM07): we NULL out rather than take an integer overflow.
		Tmp = malloc(num*size);
		if (NULL!=Tmp) memset(Tmp,'\0',_msize(Tmp));
	}
	return Tmp;
}

#ifdef __cplusplus
extern "C"
#endif
void _cdecl free(void* memblock)
{	// FORMALLY CORRECT: Kenneth Boyd, 6/2/2008
	if (	NULL!=memblock	/* C90 */
		&&  NULL!=RawBlock.raw)	/* handle race condition issues at program shutdown */
		{
#ifdef __cplusplus
		RAMBlock.Lock();
#endif
		size_t CurrIdx = __IdxOfPointerInPtrList(memblock,CountPointersAllocated,RawBlock.records);
		if (!CurrIdx)
			// error reporting
			__ReportErrorAndCrash(FreeNonNULLInvalid);
		__MetaFree(CurrIdx);
		if (CurrIdx>CountPointersAllocated)
			// we deleted the lowest memory block.  We need to check whether we have
			// excessive Ptrspace below the last pointer.
			__CompactPtrRAM();
		__DetectOverwrites();
#ifdef __cplusplus
		RAMBlock.UnLock();
#endif
		};
}
#ifdef __cplusplus
}	// namespace std
#endif

static void* __SlideUp(char* memblock, size_t CurrIdx, size_t size)
{	// FORMALLY CORRECT: Kenneth Boyd, 11/3/1999
	// this routine tries to slide up a block [i.e., move the hole from above the block
	// to under the block].  The block retains its current size.
	size_t HoleSize = (1==CurrIdx)	? HOLESIZE_AT_IDX1()
									: HOLESIZE(CurrIdx-1);
	if ((size_t)(HighBoundPtrSpace-LowBoundPtrSpace)<HoleSize+sizeof(_track_pointer))
		__ReportErrorAndCrash(PointerSizeInfoCorrupted);
	if (0<HoleSize)
		{
		memmove(memblock+HoleSize-sizeof(size_t),
				memblock-sizeof(size_t),
				size+sizeof(size_t)+sizeof(void*));
		CHARPTR_FROM_IDX(CurrIdx) = memblock+HoleSize;
		};
	return CHARPTR_FROM_IDX(CurrIdx);
}

#ifdef __cplusplus
namespace std
{
extern "C"
#endif
void* _cdecl realloc(void* memblock, size_t size)
{	// FORMALLY CORRECT: Kenneth Boyd, 11/3/1999
	if (NULL==memblock)
		return malloc(size);
	if (0==size)
		{
		free(memblock);
		return NULL;
		};
#ifdef __cplusplus
	RAMBlock.Lock();
#endif
	__DetectOverwrites();
	size_t CurrIdx = __IdxOfPointerInPtrList(memblock,CountPointersAllocated,RawBlock.records);
	if (!CurrIdx)
		// error reporting
		__ReportErrorAndCrash(ReallocNonNULLInvalid);
#if 0
	if (!AppRunning)
		{	// Microsoft bypass: size request is a non-ANSI *multiplier*
			// need this to start in VC++ 4 (at least)
			// disable once in main()/WinMain()
		size *= _msize(memblock);
		}
#endif

	{
	// This code tries to "flip up" the memory block.
#ifdef __cplusplus
	void* Tmp = __FindHole(size,reinterpret_cast<char*>(memblock)+_msize(memblock)+sizeof(void*),
#else
	void* Tmp = __FindHole(size,((char*)(memblock))+_msize(memblock)+sizeof(void*),
#endif
						 CountPointersAllocated,HighBoundPtrSpace);
	if (NULL!=Tmp)
		{	// MOVE THE BLOCK
		memmove(Tmp,memblock,size);
		InitBlock(Tmp,size);
		PTRSIZE_FROM_IDX(CurrIdx) = size;
#ifdef __cplusplus
		CHARPTR_FROM_IDX(CurrIdx) = reinterpret_cast<char*>(Tmp);
		__OnePassInsertSort(RawBlock.records+CurrIdx-1,CurrIdx);
		RAMBlock.UnLock();
#else
		CHARPTR_FROM_IDX(CurrIdx) = ((char*)(Tmp));
		__OnePassInsertSort(RawBlock.records+CurrIdx-1,CurrIdx);
#endif
		return Tmp;
		};
	if (__ExpandV2(memblock,CurrIdx,size))
#ifdef __cplusplus
		Tmp=__SlideUp(reinterpret_cast<char*>(memblock),CurrIdx,size);
#else
		Tmp=__SlideUp(((char*)(memblock)),CurrIdx,size);
#endif
	if (NULL==Tmp)
		{
		Tmp = __CreateAtBottom(size);
		if (NULL!=Tmp)
			{
			memmove(Tmp,memblock,size);
			__MetaFree(__IdxOfPointerInPtrList(memblock,CountPointersAllocated,RawBlock.records));
			__RegisterPtr(Tmp,size,RawBlock.records);
			};
		};
#ifdef __cplusplus
	RAMBlock.UnLock();
#endif
	return Tmp;
	}
}
#ifdef __cplusplus
}	// namespace std

extern "C"
#endif
void* _expand(void* memblock, size_t size)
{	// FORMALLY CORRECT: Kenneth Boyd, 11/3/1999
	if (NULL==memblock) return malloc(size);
#ifdef __cplusplus
	RAMBlock.Lock();
#endif
	__DetectOverwrites();
	size_t CurrIdx = __IdxOfPointerInPtrList(memblock,CountPointersAllocated,RawBlock.records);
	if (!CurrIdx)
		// error reporting
		__ReportErrorAndCrash(ExpandNonNULLInvalid);
	{
	void* Tmp = __ExpandV2(memblock,CurrIdx,size);
#ifdef __cplusplus
	RAMBlock.UnLock();
#endif
	return Tmp;
	}
}

#ifdef __cplusplus
extern "C"
#endif
int _memory_block_start_valid(const void* x)
{
#ifdef __cplusplus
	RAMBlock.Lock();
#endif
	size_t i = __IdxOfPointerInPtrList(x,CountPointersAllocated,RawBlock.records);
#ifdef __cplusplus
	RAMBlock.UnLock();
#endif
	return 0!=i;
}

#if 0
// META: declare this when necessary
#ifdef __cplusplus
extern "C"
#endif
void
_DynamicRAMIsNotObviouslyCorrupted(void)
{	// FORMALLY CORRECT: Kenneth Boyd, 3/10/2000
#ifdef __cplusplus
	RAMBlock.Lock();
#endif
	__DetectOverwrites();
#ifdef __cplusplus
	RAMBlock.UnLock();
#endif
}
#endif

#ifdef __cplusplus

std::new_handler ZaimoniNewHandler = NULL;

void* operator new(size_t NewSize) throw (std::bad_alloc)
{
	void* Tmp = calloc(1,NewSize);
	if (ZaimoniNewHandler)
		while(!Tmp) Tmp = (ZaimoniNewHandler(),calloc(1,NewSize));

	if (!Tmp) throw std::bad_alloc();
	return Tmp;
}

void* operator new[](std::size_t NewSize) throw (std::bad_alloc)
{
	void* Tmp = calloc(1,NewSize);
	if (ZaimoniNewHandler)
		while(!Tmp) Tmp = (ZaimoniNewHandler(),calloc(1,NewSize));

	if (!Tmp) throw std::bad_alloc();
	return Tmp;
}

void* operator new(size_t NewSize, const std::nothrow_t&) throw ()
{
	void* Tmp = calloc(1,NewSize);
	if (ZaimoniNewHandler)
		try	{
			while(!Tmp) Tmp = (ZaimoniNewHandler(),calloc(1,NewSize));
			}
		catch(const std::bad_alloc&)
			{
			return NULL;
			}

	return Tmp;
}

void* operator new[](std::size_t NewSize, const std::nothrow_t&) throw ()
{
	void* Tmp = calloc(1,NewSize);
	if (ZaimoniNewHandler)
		try	{
			while(!Tmp) Tmp = (ZaimoniNewHandler(),calloc(1,NewSize));
			}
		catch(const std::bad_alloc&)
			{
			return NULL;
			}

	return Tmp;
}

void operator delete(void* Target) throw ()
{/* FORMALLY CORRECT: 4/16/98, Kenneth Boyd */ free(Target);}

void operator delete(void* Target, const std::nothrow_t&) throw ()
{/* FORMALLY CORRECT: 4/16/98, Kenneth Boyd */ free(Target);}

void operator delete[](void* Target) throw()
{/* FORMALLY CORRECT: 9/27/2005, Kenneth Boyd */ free(Target);}

void operator delete[](void* Target, const std::nothrow_t&) throw()
{/* FORMALLY CORRECT: 9/27/2005, Kenneth Boyd */ free(Target);}

#endif
