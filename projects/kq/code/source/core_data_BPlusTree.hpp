#ifndef CORE_DATA_BPT_HPP
#define CORE_DATA_BPT_HPP

#include "core_IntegerTypes.hpp"
#include "core_memory_MemoryWorker.hpp"

namespace kq{
	namespace core{
		namespace data{

			class BPlusTree{

			//Basic B+Tree stuff
			protected:
				//Memory interface
				kq::core::memory::MemoryWorker mem;

				//Data Types
				//Nibble counts, Level Counts, ByteCounts, etc
				typedef kq::core::ui8 USmall;
				//Children count can be 256, USmalls multiplied
				typedef kq::core::ui16 ULarge;
				//When USmall needs to be negative
				typedef kq::core::i16 SSmall;
				//When ULarges need to be negative
				typedef kq::core::i32 SLarge;

				//Expect 0-16
				const USmall m_nBytesInKey;
				//Expect 1-8
				const USmall m_nBitsPerLevel;
				//2^m_nBitsPerLevel, 1-256
				const ULarge m_nChildrenPerNode;
				//8/m_nBitsPerLevel, 1-8
				const USmall m_nNibblesPerByte;
				//m_nBytesInKey*m_nNibblesPerByte, (0-16)*(1-8) = 0,128
				const USmall m_nNibblesInKey;

				//Root of the Tree
				void * m_pRoot;

				//Masks for getting child index out of key bytes
				USmall m_pNibbleMasks[8];

			public:
				BPlusTree(kq::core::memory::MemoryWorker memworker, USmall bytesInKey, USmall bitsPerLevel = 4);
				~BPlusTree();
				bool map(void * key, void * newV, void ** oldV = 0);
				void * lookup(void * key);

			protected:
				void ** find(void * k);
				void ** findOrCreate(void * k);
				bool destroy(void * pKey, void ** oldValue = 0);
				void move(void *** pppCurr, USmall & iKeyNibble, kq::core::ui8 * pKey);

			//Stack Features, Iteration etc
			protected:
				//m_nNibblesInKey + 1, 1 - 129
				const USmall m_nLevelsInStack;
				//m_nLevelsInStack*(sizeof(void*)), 32 - 8192
				const ULarge m_nSizeOfStack;

				typedef void *** Stack;
				Stack stackCreate();
				void stackDestroy(Stack s);
				void stackCopy(Stack sOut, Stack sIn);
				bool getStackFromKey(Stack s, void * k);
				void getKeyFromStack(void * k, Stack s);
				void * readValueAtStack(Stack s);
				void writeValueAtStack(Stack s, void * v);
				bool nextleaf(Stack s, SSmall iDirection = 1);
				void path(Stack s, USmall & iKeyNibble, kq::core::ui8 * pKey);
				void dumpStack(Stack s);
				bool destroyStack(Stack s, void ** oldValue);

			//Public facing class to use for iteration
			public:
				class Path{
					BPlusTree * tree;
					Stack s;
					bool move(void ** v, void * k, SSmall iDirection);
					bool init(void ** v, void * k, SSmall iDirection);
					
				public:
					Path(BPlusTree * pTree);
					~Path();
					const Path & operator = (const Path &);

					bool next(void ** v = 0, void * k = 0);
					bool prev(void ** v = 0, void * k = 0);
					bool init_first(void ** v = 0, void * k = 0);
					bool init_last(void ** v = 0, void * k = 0);
					bool init_moveTo(void * k, void ** v = 0);
					void current(void ** v = 0, void * k = 0);
					void write(void * v);
				};


			//Use Path to dump on stdout
			public:
				void dump();


			};


		}
	}
}

#endif
