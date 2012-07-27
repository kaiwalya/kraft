#include "kom.hpp"

namespace kq{
	namespace kom{
		//IObject {5C72078F-FA2F-4213-8E53-54C9A747917A}
		const InterfaceID IObject::IID
			= { 0x5c72078f, 0xfa2f, 0x4213, { 0x8e, 0x53, 0x54, 0xc9, 0xa7, 0x47, 0x91, 0x7a } };

		//IAllocator {08FFF88C-0D78-4480-A1D4-1990EA542C69}
		const InterfaceID IAllocator::IID
			= { 0x8fff88c, 0xd78, 0x4480, { 0xa1, 0xd4, 0x19, 0x90, 0xea, 0x54, 0x2c, 0x69 } };

		//IObjectFactory {E3DF7609-96A6-4333-8AC0-77AF8F6B2E11}
		const InterfaceID IObjectFactory::IID
			= { 0xe3df7609, 0x96a6, 0x4333, { 0x8a, 0xc0, 0x77, 0xaf, 0x8f, 0x6b, 0x2e, 0x11 } };

		//IObjectFactory::ISpecification {9CD8F680-2BA8-4679-B036-76CC6E750121}
		const InterfaceID IObjectFactory::ISpecification::IID
			= { 0x9cd8f680, 0x2ba8, 0x4679, { 0xb0, 0x36, 0x76, 0xcc, 0x6e, 0x75, 0x1, 0x21 } };
		//const IObjectFactory::ISpecification::ParameterIndex IObjectFactory::ISpecification::kParamIndex_NotIndexed
		//	= 0;

		//IClassLibrary {5E28A35F-5FE5-4d10-A399-01A3DB4C3F8F}
		const InterfaceID IClassLibrary::IID
			= { 0x5e28a35f, 0x5fe5, 0x4d10, { 0xa3, 0x99, 0x1, 0xa3, 0xdb, 0x4c, 0x3f, 0x8f } };

		//IClassStore {929B0A00-C605-4920-B23B-7CD8F9FF857E}
		const InterfaceID IClassStore::IID
			= { 0x929b0a00, 0xc605, 0x4920, { 0xb2, 0x3b, 0x7c, 0xd8, 0xf9, 0xff, 0x85, 0x7e } };

		//IClassManager {155A7DA2-1C7C-40db-8E66-949ABC54BA9C}
		const InterfaceID IClassManager::IID
			= { 0x155a7da2, 0x1c7c, 0x40db, { 0x8e, 0x66, 0x94, 0x9a, 0xbc, 0x54, 0xba, 0x9c } };

	}
}

#include "core_oops.hpp"
#include "map"
#include "set"

static void localassert(bool b)
{
	kq::core::oops::assume(b);
}

static void localCheckLifetime(kq::kom::ILifetime * l)
{
	localassert(l != nullptr);
	l->addRef();
	l->releaseRef();
}

namespace kq{
	namespace kom{
		
		class Lifetime: public ILifetime
		{
		public:			
			enum class TearDownLogic
			{
				kLogicNone,
				kLogicFree,
				kLogicDelete,
				kLogicDestroy,
				kLogicCall,
				kLogicRelayOnly,
			};
			
			Lifetime(void * v, void(*deletor)(void *)):v(v), count(0), deletor(deletor), logic(TearDownLogic::kLogicCall)
			{
			}
			/*
			Lifetime(IObject * o, TearDownLogic logic = TearDownLogic::kLogicDelete, ILifetime * relay = nullptr):o(o), count(1), relay(relay), logic(logic)
			{
				if(relay)
				{
					relay->addRef();
				}
			}
			*/
			virtual ~Lifetime()
			{
				localassert(count == 0);
			}

			void ILifetime::addRef()
			{
				count++;
				localassert(count != 0);
			}

			void ILifetime::releaseRef()
			{
				if(!--count)
				{
					finalize();
				}
				localassert(count != (decltype(count))-1);
			}

			virtual void finalize()
			{
				ILifetime * relay = this->relay;
				IObject * o = this->o;
				void * v = this->v;
				decltype(deletor) del = this->deletor;
				decltype(logic) logic = this->logic;
				switch(logic)
				{
				case TearDownLogic::kLogicNone:
					break;
				case TearDownLogic::kLogicRelayOnly:
					if(relay) relay->releaseRef();
					break;
				case TearDownLogic::kLogicFree:
					free(reinterpret_cast<void*>(o));
					if(relay) relay->releaseRef();
					break;
				case TearDownLogic::kLogicDelete:					
					delete o;
					if(relay) relay->releaseRef();
					break;
				case TearDownLogic::kLogicDestroy:
					o->~IObject();
					if(relay) relay->releaseRef();
					break;
				case TearDownLogic::kLogicCall:
					del(v);
					break;
				};				
			}
		protected:
			union
			{
				IObject * o;				
				void * v;
			};
			size_t count;
			TearDownLogic logic;
			union
			{
				ILifetime * relay;
				void (*deletor)(void *);
			};
			
		};


		
		

		struct InterfaceFinder: public IInterfaceFinder
		{
			const InterfaceID & iidMain;
			
		public:
			InterfaceFinder(const InterfaceID & iidMain):iidMain(iidMain)
			{}

			bool IInterfaceFinder::findInterface(const InterfaceID & type, IObject * in, IObject ** out)
			{
				if(isTypeSupported(type))
				{
					*out = in;
					(*out)->lifetime->addRef();
					return true;
				}
				return false;
			}

			bool IInterfaceFinder::isTypeSupported(const InterfaceID & type)
			{
				return (type == iidMain || type == IObject::IID);
			}

			bool IInterfaceFinder::getSupportedInterfaceCount(size_t & sz)
			{
				sz = 2;
				return true;
			}

			bool IInterfaceFinder::getSupportedInterfaceType(size_t index, InterfaceID & type)
			{
				bool bret;
				switch(index)
				{
				case 0:
					type = IObject::IID;
				case 1:
					type = iidMain;
					bret = true;
					break;
				default:
					bret = false;
					break;
				}
				return bret;
			}
		};


		template<typename T>
		class ObjectFactory: public IObjectFactory
		{
			InterfaceFinder finder2;
			InterfaceFinder objfinder;
			struct ObjectHeader{
				Lifetime life;
				IObject obj;
			};

			static void destroyObject(void * d)
			{
				ObjectHeader * header = (ObjectHeader*) d;
				(&(header->obj))->~IObject();
				header->life.~Lifetime();
				free(header);
			}

			static bool createObject(IObjectFactory * factory, const InterfaceID & type, IObjectFactory::ISpecification * spec, IObject ** out)
			{
				
			}

			static bool createFactory(IObjectFactory ** out)
			{
				
			}			
		public:
			ObjectFactory(const InterfaceID & mainiid):finder2(IObjectFactory::IID), objfinder(mainiid)
			{
				lifetime->addRef();
				IObjectFactory::IObject::finder = &finder2;
			}

			static bool create(IObjectFactory ** out)
			{
				ObjectHeader * header = (ObjectHeader * )malloc(sizeof(ObjectHeader) - sizeof(decltype(ObjectHeader::obj)) + sizeof(ObjectFactory<T>));
				if(header)
				{
					header->obj.lifetime = new (&header->life)Lifetime(header, destroyObject);
					*out = new (&header->obj) ObjectFactory<T>(T::IID);
					return true;
				}
				return false;
			}

			bool IObjectFactory::createObject(const InterfaceID & type, ISpecification * spec, IObject ** out)
			{
				if(objfinder.isTypeSupported(type))
				{
					size_t szObj;
					if(getObjectSize(type, spec, szObj))
					{
						size_t szLifetime = sizeof(Lifetime);
						ObjectHeader * header = (ObjectHeader * )malloc(sizeof(ObjectHeader) - sizeof(decltype(ObjectHeader::obj)) + szObj);
						header->obj.lifetime = new (&header->life)Lifetime(header, destroyObject);
						header->obj.finder = &objfinder;
						if(initializeObject(type, spec, &header->obj))
						{
							*out = &header->obj;
							return true;
						}
						header->life.~Lifetime();
						free(header);
					}
				}
				return false;
			}

			bool IObjectFactory::initializeObject(const InterfaceID & type, ISpecification * spec, IObject * obj)
			{
				if(objfinder.isTypeSupported(type))
				{
					new (obj) T(this, spec);
					return true;
				}
				return false;
			}

			bool IObjectFactory::getObjectSize(const InterfaceID & type, ISpecification *, size_t & sz)
			{
				if(objfinder.isTypeSupported(type))
				{
					sz = sizeof(T);
					return true;
				}
				return false;
			}
		};

		class ClassManager: public IClassManager
		{
		public:
			static const ClassID CID;
		private:
			class ClassStore: public IClassStore
			{
				ClassManager * owner;
			public:
				ClassStore(ClassManager * owner):owner(owner){}
				~ClassStore(){};

				bool IClassStore::registerClass(const ClassID & type, IObjectFactory * factory, TokenID * token)
				{
					return owner->registerClass(type, factory, token);
				}

				bool IClassStore::unregisterClass(TokenID token)
				{
					return owner->unregisterClass(token);
				} 
			};

			class ClassLibrary:public IClassLibrary
			{
				ClassManager * owner;
			public:
				ClassLibrary(ClassManager * owner): owner(owner){}
				~ClassLibrary(){};
				
				bool IClassLibrary::findObjectFactory(const ClassID & type, IObjectFactory ** out)
				{
					return owner->findObjectFactory(type, out);
				}
			};

			struct ClassInfo
			{
				ClassID cid;
				TokenID tid;
				IObjectFactory * iclass;
			};

			InterfaceFinder finderStore;
			InterfaceFinder finderLibrary;
			TokenID factoryRegistration;
			std::map<ClassID, ClassInfo> classMap;
			
			ClassStore store;
			ClassLibrary library;
			
			ClassManager(IObjectFactory * factory, IObjectFactory::ISpecification * spec):finderStore(IClassStore::IID), finderLibrary(IClassLibrary::IID), store(this), library(this)
			{		
				store.lifetime = library.lifetime = IClassManager::IObject::lifetime;
				store.finder = &finderStore;
				library.finder = &finderLibrary;
				registerClass(CID, factory, &factoryRegistration);	
				this->lifetime->addRef();
			}
			friend class ObjectFactory<ClassManager>;
		public:

			~ClassManager()
			{
				unregisterAllClasses();
				library.~ClassLibrary();
				//store().~ClassStore();
			}			

			bool registerClass(const ClassID & type, IObjectFactory * factory, TokenID * token)
			{
				ClassInfo c = {type, type, factory};
				std::pair<ClassID, ClassInfo> entry(type, std::move(c));
				auto pr = classMap.emplace(std::move(entry));
				if(pr.second){
					factory->lifetime->addRef();
					*token = type;
				}
				return pr.second;
			}

			bool unregisterClass(TokenID token)
			{
				auto it = classMap.find(token);
				if(it != classMap.end())
				{
					it->second.iclass->lifetime->releaseRef();
					return (classMap.erase(token) == 1);
				}
				return false;
			}

			bool unregisterAllClasses()
			{
				decltype(classMap.begin()) it;
				if((it = classMap.begin()) != classMap.end())
				{
					it->second.iclass->lifetime->releaseRef();
					classMap.erase(it);
				}
				return false;
			}

			bool findObjectFactory(const ClassID & type, IObjectFactory ** out)
			{
				auto it = classMap.find(type);
				if(it != classMap.end())
				{
					it->second.iclass->lifetime->addRef();
					*out = it->second.iclass;
					return true;
				}
				return false;
			}

			bool IClassManager::getClassStore(IClassStore ** out)
			{
				*out = &store;
				(*out)->lifetime->addRef();
				return true;
			}

			bool IClassManager::getClassLibrary(IClassLibrary ** out)
			{
				*out = &library;
				(*out)->lifetime->addRef();
				return true;
			}
		};
		const ClassID ClassManager::CID = { 0x4972fe0d, 0x25e, 0x45cc, { 0x9c, 0xad, 0x34, 0x95, 0x36, 0x98, 0xa5, 0x74 } };

		IObjectFactory * getKOMFactory()
		{
			IObjectFactory * f;
			if(ObjectFactory<ClassManager>::create(&f))
			{
				return f;
			}
			return nullptr;
		}
	}
}
