#include "kom.hpp"



namespace kq{
	namespace kom{
		//ILifetime {8B8E7C37-82E0-4d2b-B9AF-19E0495ACA89}
		//const InterfaceID ILifetime::IID = { 0x8b8e7c37, 0x82e0, 0x4d2b, { 0xb9, 0xaf, 0x19, 0xe0, 0x49, 0x5a, 0xca, 0x89 } };

		//IObject {5C72078F-FA2F-4213-8E53-54C9A747917A}
		const InterfaceID IObject::IID = { 0x5c72078f, 0xfa2f, 0x4213, { 0x8e, 0x53, 0x54, 0xc9, 0xa7, 0x47, 0x91, 0x7a } };

		//IAllocator {08FFF88C-0D78-4480-A1D4-1990EA542C69}
		const InterfaceID IAllocator::IID = { 0x8fff88c, 0xd78, 0x4480, { 0xa1, 0xd4, 0x19, 0x90, 0xea, 0x54, 0x2c, 0x69 } };

		//IClass {E3DF7609-96A6-4333-8AC0-77AF8F6B2E11}
		const InterfaceID IClass::IID = { 0xe3df7609, 0x96a6, 0x4333, { 0x8a, 0xc0, 0x77, 0xaf, 0x8f, 0x6b, 0x2e, 0x11 } };

		//IClassLibrary {5E28A35F-5FE5-4d10-A399-01A3DB4C3F8F}
		const InterfaceID IClassLibrary::IID = { 0x5e28a35f, 0x5fe5, 0x4d10, { 0xa3, 0x99, 0x1, 0xa3, 0xdb, 0x4c, 0x3f, 0x8f } };

		//IClassStore {929B0A00-C605-4920-B23B-7CD8F9FF857E}
		const InterfaceID IClassStore::IID = { 0x929b0a00, 0xc605, 0x4920, { 0xb2, 0x3b, 0x7c, 0xd8, 0xf9, 0xff, 0x85, 0x7e } };

		//IClassManager {155A7DA2-1C7C-40db-8E66-949ABC54BA9C}
		const InterfaceID IClassManager::IID = { 0x155a7da2, 0x1c7c, 0x40db, { 0x8e, 0x66, 0x94, 0x9a, 0xbc, 0x54, 0xba, 0x9c } };

	}
}

#include "map"

namespace kq{
	namespace kom{

		class BaseLifetime: public ILifetime
		{
		protected:
			size_t count;
		public:
			BaseLifetime():count(1){}
			virtual ~BaseLifetime(){}
			void ILifetime::addRef(){ count++; }
			void ILifetime::releaseRef(){ if(!--count)delete this; }
		};

		template<typename T>
		class DeleterLifetime: public BaseLifetime
		{
			const T * o;
		public:
			DeleterLifetime(const T * o):o(o){}
			void ILifetime::releaseRef(){ if(!--count)delete o; }
		};

		class ClassManager: public IClassManager, public IInterfaceFinder
		{
		public:
			static const ClassID CID;
		private:
			class ClassFactory: public IClass, public IInterfaceFinder
			{
				DeleterLifetime<ClassFactory> del;
			public:
				ClassFactory(): del(this)
				{
					IClass::IObject::lifetime = &del;
					IClass::IObject::finder = this;
				};

				bool IInterfaceFinder::findInterface(const InterfaceID & type, IObject ** out)
				{
					if(type == IClass::IID || type == IObject::IID)
					{
						*out = this;
						(*out)->lifetime->addRef();
						return true;
					}
					return false;
				}

				bool IClass::createObject(const InterfaceID & type, IObject ** out)
				{
					if(type == IClassManager::IID || type == IObject::IID)
					{
						*out = new ClassManager(this);
						return true;
					}
					return false;
				}

				~ClassFactory(){}
			};
			class ClassStore: public IClassStore, public IInterfaceFinder
			{
				ClassManager * owner;
			public:
				ClassStore(ClassManager * owner): owner(owner)
				{
					IClassStore::IObject::lifetime = nullptr;
					IClassStore::IObject::finder = this;
				}
				~ClassStore(){};		

				bool IClassStore::registerClass(const ClassID & type, IClass * factory, TokenID * token)
				{
					return owner->registerClass(type, factory, token);
				}

				bool IClassStore::unregisterClass(TokenID token)
				{
					return owner->unregisterClass(token);
				} 

				bool IInterfaceFinder::findInterface(const InterfaceID & type, IObject ** out)
				{
					if(type == IClassStore::IID || type == IObject::IID)
					{
						*out = this;
						lifetime->addRef();
						return true;
					}
					return false;
				}
			};

			class ClassLibrary:public IClassLibrary, public IInterfaceFinder
			{
				ClassManager * owner;
			public:
				ClassLibrary(ClassManager * owner): owner(owner)
				{
					IClassLibrary::IObject::lifetime = nullptr;
					IClassLibrary::IObject::finder = this;
				}
				~ClassLibrary(){};
				
				bool IClassLibrary::findClass(const ClassID & type, IClass ** out)
				{
					return owner->findClass(type, out);
				}

				bool IInterfaceFinder::findInterface(const InterfaceID & type, IObject ** out)
				{
					if(type == IClassLibrary::IID || type == IObject::IID)
					{
						*out = this;
						lifetime->addRef();
						return true;
					}
					return false;
				}
			};

			struct ClassInfo
			{
				ClassID cid;
				TokenID tid;
				IClass * iclass;
			};

			TokenID factoryRegistration;
			std::map<ClassID, ClassInfo> classMap;
			ClassStore store;
			ClassLibrary library;			
			DeleterLifetime<ClassManager> del;

		
			ClassManager(IClass * factory):store(this), library(this), del(this)
			{
				store.lifetime = library.lifetime = IClassManager::IObject::lifetime = &del;
				IClassManager::IObject::finder = this;
				store.registerClass(CID, factory, &factoryRegistration);				
			}

		public:
			static bool createObject(const InterfaceID & type, IObject ** out)
			{
				bool bret = false;
				if(type == IClassManager::IID)
				{
					ClassFactory * factory = new ClassFactory();
					bret = factory->createObject(type, out);
					factory->lifetime->releaseRef();
				}
				return bret;
			}

			~ClassManager()
			{
				store.unregisterClass(factoryRegistration);
			}

			bool IInterfaceFinder::findInterface(const InterfaceID & type, IObject ** out)
			{
				if(type == IClassManager::IID || type == IObject::IID)
				{					
					*out = this;
					lifetime->addRef();
					return true;
				}
				return false;
			}

			bool registerClass(const ClassID & type, IClass * factory, TokenID * token)
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

			bool findClass(const ClassID & type, IClass ** out)
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

		class KOM: public IObject, IInterfaceFinder
		{
		public:
			static const ClassID CID;
			IClassManager * classmanager;
			ILifetime * classmanager_lifetime;
			IInterfaceFinder * classmanager_interfaceFinder;
			DeleterLifetime<KOM> del;
			~KOM()
			{
				if(classmanager)
				{
					classmanager->lifetime = classmanager_lifetime;
					classmanager->finder = classmanager_interfaceFinder;
					classmanager->lifetime->releaseRef();
				}
			}
		public:
			KOM():del(this)
			{
				IObject::lifetime = &del;
				IObject::finder = this;
				classmanager = nullptr;
			}
			
			bool findInterface(const InterfaceID & type, IObject ** out)
			{
				if(type == IClassManager::IID)
				{
					if(!classmanager)
					{
						if(ClassManager::createObject(IClassManager::IID, (IObject**)&classmanager))
						{
							classmanager_lifetime = classmanager->lifetime;
							classmanager_interfaceFinder = classmanager->finder;
							classmanager->lifetime = &del;
							classmanager->finder = this;
							lifetime->addRef();
							*out = classmanager;
							return true;
						}
					}
				}
				else if(type == IObject::IID)
				{
					*out = this;
					lifetime->addRef();
					return true;
				}
				return false;
			}
		};
		
		bool createKOM(IObject ** out)
		{
			*out = new KOM();
			return true;
		};
	}
}
