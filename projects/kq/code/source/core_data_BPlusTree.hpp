#ifndef CORE_DATA_BPT_HPP
#define CORE_DATA_BPT_HPP

#include "core_IntegerTypes.hpp"
#include "core_memory_MemoryWorker.hpp"

namespace kq{
	namespace core{
		namespace data{

			class BPlusTree{

				typedef kq::core::ui16 ULarge;
				typedef kq::core::ui8 USmall;
				typedef kq::core::i16 SSmall;
				typedef kq::core::i32 SLarge;

				void * pRoot;

				const USmall nBytesInKey;
				const ULarge nChildrenPerNode;
				const USmall nBitsPerLevel;
				const USmall nNibblesPerByte;
				const ULarge nNibblesInKey;
				const ULarge nLevelsInStack;
				const ULarge nSizeOfStack;

				USmall * pNibbleMasks;

				//Start(Allocation, Deallocation and UserData)
				kq::core::memory::MemoryWorker mem;
				//End(Allocation, Deallocation and UserData)

				void path(void *** pppCurr, USmall & iKeyNibble, kq::core::ui8 * pKey);

				void move(void *** pppCurr, USmall & iKeyNibble, kq::core::ui8 * pKey);

				typedef void *** Stack;
				bool nextleaf(Stack s, SSmall iDirection = 1);

				void dumpStack(void *** pppCurr);

			public:
				BPlusTree(kq::core::memory::MemoryWorker memworker, USmall bytesInKey, USmall bitsPerLevel = 4);
			protected:

				void ** find(void * k);
				void ** findOrCreate(void * k);
				bool destroy(void * pKey, void ** oldValue = 0);
				Stack createStack();
				void destroyStack(Stack s);
				

				bool getStackFromKey(Stack s, void * k);

				void getKeyFromStack(void * k, Stack s);
				void * readValueAtStack(Stack s);

				void writeValueAtStack(Stack s, void * v);

			public:

				bool map(void * key, void * newV, void ** oldV = 0);

				void * lookup(void * key);

				void dump();

				/*
				class path{
					BPlusTree * tree;
					Stack s;

					bool move(void ** v, void * k, SSmall iDirection);

					bool init(void ** v, void * k, SSmall iDirection);

				public:
					path(BPlusTree * pTree);

					~path();

					bool next(void ** v, void * k);

					bool prev(void ** v, void * k);

					bool init_first(void ** v, void * k);

					bool init_last(void ** v, void * k);

					void current(void ** v, void * k);

					bool moveTo(void * k, void ** v);

					void write(void * v);

				};
				*/

				~BPlusTree();
			};
		}
	}
}

#endif
