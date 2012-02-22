namespace kq{
	namespace core{
		namespace data{
			class IDMap{
				memory::MemoryWorker & mem;
				BPlusTree tree;
				typedef ui32 IDVal;
				IDVal m_iNext;
				IDVal m_nAlive;
			public:
				IDMap(memory::MemoryWorker & memworker)
				:mem(memworker), tree(memworker, sizeof(IDVal)), m_iNext(1), m_nAlive(0){}

				~IDMap(){}

				IDVal getAliveIDCount(){return m_nAlive;}

				template<typename t> IDVal create(t * p){
					IDVal ret = 0;
					if(tree.map(&(m_iNext), p)){
						ret = m_iNext;
						m_nAlive++;
						m_iNext++;
					}
					return ret;
				}

				template<typename t> t * get(const IDVal id){
					t * ret = 0;
					if(id && id < m_iNext){
						ret = tree.lookup(&id);
					}
					return ret;
				}

				template<typename t> t * destroy(const IDVal id){
					t * ret = 0;
					if(id && id < m_iNext){
						if(tree.map(&id, 0, (void **)&ret)){
							m_nAlive--;
						}
					}
					return ret;
				}

			public:
				kq::core::data::BPlusTree::Path getIterator(){
					return kq::core::data::BPlusTree::Path(&tree);
				}

			};
		}
	}
}
