#if !defined(OLD_KLARITY)

#include "WinSock2.h"
#include "Windows.h"
#include <vld.h>

/*
#include "initguid.h"
#include "core.hpp"
#include "Mmdeviceapi.h"
#include "atlbase.h"
#include "Functiondiscoverykeys_devpkey.h"
#include "memory"
#include "AudioClient.h"
*/
#include "stdint.h"
#include "string.h"
#include <typeindex>
#include "core_memory_MemoryWorker.hpp"
namespace klass
{
	class Problem
	{
	public:
		virtual void finalize() = 0;
	};

	class Problems
	{
	public:
		Problems(Problem * p);
		~Problems();
	};

	typedef std::type_index TypeIdentifier;

	class IBoss;
	class IBasic
	{
		virtual bool find(const TypeIdentifier &, IBasic **) = 0;
		virtual void finalize() = 0;
	};

	class IAuditor: public IBasic
	{
	public:
		void report(uint32_t level, uint32_t info, void * extrainfo);
	};

	
	class IWindow
	{
	public:
		virtual Problems resetWindowLocation(uint64_t location) = 0;
		virtual Problems resetWindowSize(size_t sizeMin, size_t sizeMax) = 0;
		virtual Problems slideWindow() = 0;
		//virtual Problems getData(size_t & dataCount, IData ** out);
	};

	class IReadableMemory
	{
	public:
		virtual void finalize() = 0;
		virtual Problems getReadableMemory(const void **) = 0;
		virtual size_t getSize() = 0;
	};

	class IWritableMemory
	{
	public:
		virtual void finalize() = 0;
		virtual Problems getWritableMemory(void **) = 0;
		virtual size_t getSize() = 0;
	};

	class IData
	{
	public:
		//Releases the IData pointer, IData should not be used after this call
		virtual void finalize() = 0;

		virtual Problems openReadableMemory() = 0;
		virtual Problems openWritableMemory() = 0;
		virtual Problems transfer(IReadableMemory * source, size_t sz, size_t sourceoffset, size_t destoffset) = 0;
		virtual Problems transfer(IWritableMemory * dest, size_t sz, size_t sourceoffset, size_t destoffset) = 0;
		virtual Problems createSnapshot(IData *) = 0;
	};

	class IDiary
	{
	public:
		enum Level
		{
			levelDebug,
			levelLog,
			levelWarning,
			levelError,
		};

		virtual void finalize() = 0;
		virtual IDiary * newChildDiary(const char * name) = 0;
		virtual void report(Level l, const char *, ...) = 0;
	};

	class IAllocator{
	public:
		virtual void * createInner(const char * purpose);
		virtual void * reallocate(void *, size_t sz) = 0;
		virtual void finalize() = 0;
	};

	class Allocator
	{
		virtual void * reallocate(void *, size_t sz) = 0;
		virtual void finalize()
		{
			delete this;
		}
	};
	
	class Intent
	{
	public:
		/*
		struct Intent_Array
		{
			uint16_t unit;
			uint16_t count;
			uint16_t offset;
		};

		struct IntentCreationInfo
		{
			const char * name;
			const char * type;
			Intent ** creators;
		};

		*/
		typedef uint32_t LogLevel;
		/*
		static const LogLevel kLogLevel_Debug = 0;
		static const LogLevel kLogLevel_Log = 1;
		static const LogLevel kLogLevel_Warning = 2;
		static const LogLevel kLogLevel_Error = 3;

		virtual Intent * addNewIntent(const IntentCreationInfo &) = 0;
		//virtual const void * getUserData(size_t *) const = 0;
		//virtual void * getUserData(size_t *) = 0;
		virtual void log(LogLevel l, const char * message, ...) = 0;
		virtual void releaseIntent() = 0;
		*/
	};
	

	class IntentMessageInterface
	{
	public:
		typedef uint64_t ProcessID;
		typedef uint64_t TypeID;
		typedef uint64_t IntentID;
		typedef Intent::LogLevel LogLevel;
		struct String
		{
			char * str;
			uint32_t size;
			String():str(nullptr), size(0){}
		};

		virtual bool processIntentMessage_Log(IntentID This, LogLevel l, String logmessage) = 0;
		virtual bool processIntentMessage_Create(IntentID This, IntentID owner, const String name, const String type, uint32_t nOtherIntents, IntentID * others) = 0;
		virtual bool processIntentMessage_Destroy(IntentID This) = 0;

		
		static const int kTypeID_Unknown = 0;
		static const int kTypeID_Log = 1;
		static const int kTypeID_Create = 2;
		static const int kTypeID_Destroy = 3;

		struct Block
		{
			uint32_t size;
			uint32_t offset;
		};

		struct Header
		{
			ProcessID processID;
			TypeID typeID;
			Block arrIntentIDs;
		};

		struct CreateBody
		{
			Block name;
			Block type;
		};

		struct LogBody
		{
			LogLevel level;
			Block message;
		};

		struct DestroyBody
		{
		};
	};

	class IntentMessageSerializer: public IntentMessageInterface
	{
	protected:
		
		kq::core::memory::MemoryWorker & mem;
		ProcessID processID;
		String scratchpad;
		uint64_t allocSize;

		bool makeSize(size_t sz)
		{
			if(allocSize < (uint64_t)sz)
			{
				void * newsp = mem(scratchpad.str, sz);
				if(newsp)
				{
					scratchpad.str = (char *)newsp;
					allocSize = sz;
					goto memavailable;
				}
				goto fail;
			}
			memavailable:
			scratchpad.size = sz;
			return true;
			fail:

			return false;
		}
	public:
		
		IntentMessageSerializer(kq::core::memory::MemoryWorker & mem, ProcessID processID):mem(mem), processID(processID)
		{
			scratchpad.size = 0;
			scratchpad.str = nullptr;
			allocSize = 0;
			if(makeSize(sizeof(Header)))
			{
				Header & h = *(Header *)scratchpad.str;
				h.processID = processID;
				h.typeID = kTypeID_Unknown;
				h.arrIntentIDs.offset = 0;
				h.arrIntentIDs.size = 0;
				return;
			}
			throw std::exception("IntentMessageSerializer::IntentMessageSerializer() filed");
		};

		bool processIntentMessage_Log(IntentID This, LogLevel l, String logmessage)
		{
			uint32_t sizeReq = sizeof(Header) + sizeof(LogBody) + sizeof(IntentID) + sizeof(logmessage.size); 
			if(makeSize(sizeReq))
			{
				Header & h = *(Header *)scratchpad.str;
				LogBody & log = *(LogBody *)(&h + 1);
				char * temp = (char *)(&log + 1);
				
				h.typeID = kTypeID_Log;
				h.arrIntentIDs.offset = temp - scratchpad.str;
				h.arrIntentIDs.size = sizeof(IntentID) * 1;
				*((IntentID *)(temp)) = This;
				temp += h.arrIntentIDs.size;
				
				log.level = l;
				log.message.offset = temp - scratchpad.str;
				memcpy(temp, logmessage.str, logmessage.size);
				return true;
			}
			return false;
		}
		bool processIntentMessage_Create(IntentID This, IntentID owner, const String name, const String type, uint32_t nOtherIntents, IntentID * others)
		{
			size_t sizeReq = sizeof(Header) + sizeof(CreateBody) + sizeof(IntentID) * (nOtherIntents + 2) + name.size + type.size;
			if(makeSize(sizeReq))
			{
				Header & h = *(Header *)scratchpad.str;
				CreateBody & cmsg = *(CreateBody *)(&h + 1);
				char * temp = (char *)(&cmsg + 1);
				
				h.typeID = kTypeID_Create;
				h.arrIntentIDs.offset = temp - scratchpad.str;
				h.arrIntentIDs.size = sizeof(IntentID) * (nOtherIntents + 2);
				*((IntentID *)(temp) + 0) = This;
				*((IntentID *)(temp) + 1) = owner;
				memcpy((IntentID *)(temp) + 2, others, sizeof(IntentID) * nOtherIntents);
				temp += h.arrIntentIDs.size;

				cmsg.name.offset = temp - scratchpad.str;
				cmsg.name.size = name.size;
				memcpy(temp, name.str, name.size);
				temp += cmsg.name.size;

				cmsg.type.offset = temp - scratchpad.str;
				cmsg.type.size = type.size;
				memcpy(temp, type.str, type.size);
				temp += cmsg.type.size;
				return true;
			}
			return false;
		}

		bool processIntentMessage_Destroy(IntentID This)
		{
			size_t sizeReq = sizeof(Header) + sizeof(DestroyBody);
			if(makeSize(sizeReq)){
				Header & h = *(Header *)scratchpad.str;
				DestroyBody & dmsg = *(DestroyBody *)(&h + 1);
				char * temp = (char *)(&dmsg + 1);

				h.typeID = kTypeID_Destroy;
				h.arrIntentIDs.offset = temp - scratchpad.str;
				h.arrIntentIDs.size = sizeof(IntentID) * (1);
				*((IntentID *)(temp) + 0) = (uintptr_t)This;
				temp += h.arrIntentIDs.size;
				return true;
			}
			return false;
		}

		const String & getSerializedMessage()
		{
			return scratchpad;
		}
	};

	class IntentMessageDeserializer: public IntentMessageInterface
	{
	public:
		IntentMessageDeserializer()
		{
		}

		bool push(String s)
		{
			bool ret = false;
			Header & h = *(Header *)s.str;
			if(s.size >= sizeof(h) && s.size >= sizeof(h) + h.arrIntentIDs.size)
			{
				IntentID * intent = reinterpret_cast<IntentID *>(s.str + h.arrIntentIDs.offset);
				uint32_t nIntents = h.arrIntentIDs.size/sizeof(IntentID);
				uint32_t bodysize = s.size - sizeof(h) - h.arrIntentIDs.size;		
				switch (h.typeID)
				{
				case kTypeID_Unknown:
					break;
				case kTypeID_Create:
					{
						CreateBody & b = *reinterpret_cast<CreateBody *>(&h + 1);
						if(bodysize >= sizeof(b) && bodysize >= sizeof(b) + b.name.size + b.type.size && nIntents >= 2)
						{
							String name; name.size = b.name.size; name.str = s.str + b.name.offset;
							String type; type.size = b.type.size; type.str = s.str + b.type.offset;
							processIntentMessage_Create(intent[0], intent[1], name, type, nIntents - 2, (nIntents - 2)?intent + 2: nullptr);
							ret = true;
						};
					}
					break;
				case kTypeID_Log:
					{
						LogBody & b = *reinterpret_cast<LogBody *>(&h + 1);
						if(bodysize >= sizeof(b) && bodysize >= sizeof(b) + b.message.size && nIntents >= 1)
						{
							String message; message.size = b.message.size; message.str = s.str + b.message.offset;
							processIntentMessage_Log(intent[0], b.level, message);
							ret = true;
						}
					}
					break;
				case kTypeID_Destroy:
					{
						DestroyBody & b = *reinterpret_cast<DestroyBody *>(&h + 1);
						if(bodysize >= sizeof(b) && nIntents >= 1)
						{
							processIntentMessage_Destroy(intent[0]);
							ret = true;
						}
					}
					break;
				}				
			};
			return ret;
		}

		bool processIntentMessage_Log(IntentID This, LogLevel l, String logmessage)
		{
			return true;
		}

		bool processIntentMessage_Create(IntentID This, IntentID owner, const String name, const String type, uint32_t nOtherIntents, IntentID * others)
		{
			return true;
		}

		bool processIntentMessage_Destroy(IntentID This)
		{
			return true;
		}

	};

	/*
	class ProxyIntent: public Intent
	{
		Intent * intent;
	protected:
		void setProxyServer(Intent * intent){this->intent = intent;}
		Intent * getProxyServer(){return this->intent;}
	public:
		ProxyIntent():intent(nullptr){}
		virtual Intent * addNewIntent(const IntentCreationInfo & info){return intent->addNewIntent(info);}
		//virtual void * getUserData(size_t * sz){return intent->getUserData(sz);}
		//virtual const void * getUserData(size_t * sz) const {return intent->getUserData(sz);}
		virtual void log(LogLevel l, const char * message, ...){intent->log(l, message);}
		virtual void releaseIntent(){intent->releaseIntent();}
	};
	*/

	//class BaseIntent: public Intent
	//{
	//	BaseIntent * owner;
	//	kq::core::memory::MemoryWorker & mem;
	//	//void * userDataLocation;
	//	//size_t userDataSize;
	//protected:
	//	BaseIntent(kq::core::memory::MemoryWorker & memw, BaseIntent * owner/*, void * userDataLocation, size_t userDataSize*/):mem(memw), owner(owner)/*, userDataLocation(userDataLocation), userDataSize(userDataSize)*/{}
	//public:
	//	virtual ~BaseIntent(){}
	//	kq::core::memory::MemoryWorker & getMemoryWorker() const{return mem;}

	//	/*
	//	const void * Intent::getUserData(size_t * psz) const
	//	{
	//		if(psz) *psz = userDataSize;
	//		return userDataLocation; 
	//	};		
	//	
	//	void * Intent::getUserData(size_t * psz)
	//	{
	//		if(psz) *psz = userDataSize;
	//		return userDataLocation; 
	//	};
	//	*/
	//	
	//	virtual void Intent::log(LogLevel l, const char * message, ...)
	//	{
	//	}
	//	
	//	virtual void releaseIntent()
	//	{
	//		kq::core::memory::MemoryWorker & memw = getMemoryWorker();
	//		this->~BaseIntent();
	//		memw(this, 0); 
	//	}

	//};

	//class RootIntent: public BaseIntent
	//{

	//	class InnerIntent: public BaseIntent
	//	{

	//	};

	//	kq::core::memory::MemoryWorker * ownedworker;

	//	RootIntent( kq::core::memory::MemoryWorker * owned, kq::core::memory::MemoryWorker & m/*, void * userDataLocation, size_t userDataSize*/): BaseIntent(m, nullptr/*, userDataLocation, userDataSize*/), ownedworker(owned)
	//	{
	//	}

	//	~RootIntent(){
	//		if(ownedworker){
	//			ownedworker->operator()(ownedworker, 0);
	//		}
	//	}

	//	static void * myrealloc(void *, void * mem, kq::core::ui64 sz){
	//		return realloc(mem, (size_t)sz);
	//	}

	//public:
	//	static Intent * createRootIntent(const IntentCreationInfo & info, kq::core::memory::MemoryWorker * pmemworker = 0)
	//	{
	//		kq::core::memory::MemoryWorker * owned, * use;
	//		void * intentmem;

	//		if(!pmemworker){
	//			owned = (kq::core::memory::MemoryWorker *)myrealloc(nullptr, nullptr, sizeof(kq::core::memory::MemoryWorker));
	//			if(owned)
	//			{
	//				new (owned) kq::core::memory::MemoryWorker();
	//				owned->set(nullptr, myrealloc);
	//				use = owned;
	//			}
	//			else{
	//				use = nullptr;
	//			}
	//		}
	//		else{
	//			use = pmemworker;
	//			owned = nullptr;
	//		}

	//		if(use)
	//		{
	//			kq::core::memory::MemoryWorker & mem = *use;
	//			intentmem = mem(0, sizeof(RootIntent)/* + info.szUserData*/);
	//			if(intentmem)
	//				new (intentmem) RootIntent(owned, mem/*, (unsigned char *)intentmem + sizeof(RootIntent), info.szUserData*/);
	//		}
	//		return reinterpret_cast<RootIntent *>(intentmem);
	//	}

	//	virtual Intent * addNewIntent(const IntentCreationInfo &)
	//	{
	//		return nullptr;
	//	}

	//	virtual void releaseIntent()
	//	{
	//		kq::core::memory::MemoryWorker & memw = getMemoryWorker();
	//		this->~RootIntent();
	//		memw(this, 0); 
	//	}
	//	
	//};
	
}

#include "iostream"
#include "fstream"
#include "memory"

#include "core_memory_StandardLibraryMemoryAllocator.hpp"
#include "core_oops.hpp"

namespace kq
{
	namespace core
	{
		namespace kom
		{
			struct Identifier
			{
				union
				{
					GUID guid;
					uint8_t key8[16];
					uint16_t key16[8];
					uint32_t key32[4];
					uint64_t key64[2];
				};

				bool operator == (const Identifier & other) const
				{
					
					return this->key64[0] == other.key64[0] && this->key64[1] == other.key64[1];
				}

				bool operator < (const Identifier & other) const
				{
					return this->key64[1] < other.key64[1] || this->key64[0] < other.key64[0];
				}

				bool operator > (const Identifier & other) const
				{
					return this->key64[1] > other.key64[1] || this->key64[0] > other.key64[0];
				}

				bool operator <= (const Identifier & other) const
				{
					return other > *this;
				}

				bool operator >= (const Identifier & other) const
				{
					return other < * this;
				}
			};

			typedef Identifier InterfaceID;
			typedef Identifier ClassID;
			typedef Identifier TokenID;

			struct IObject;
			struct ICore
			{
				//static const InterfaceID IID;
				virtual bool findInterface(const InterfaceID & type, IObject ** out) = 0;
				virtual void addRef() = 0;
				virtual void releaseRef() = 0;
			};

			struct IObject
			{
				static const InterfaceID IID;
				virtual ICore * core() = 0;
				//void addRef(){core()->addRef();}
				//void releaseRef(){core()->releaseRef();}
				//void findInterface(const InterfaceID & type, IObject ** out){core()->findInterface(type, out);}
			};

			struct IClass: public IObject
			{
				static const InterfaceID IID;
				virtual bool createObject(const InterfaceID & type, IObject * owner, IObject ** out) = 0;
			};

			struct IClassLibrary: public IObject
			{
				static const InterfaceID IID;
				virtual bool findClass(const ClassID & type, IClass ** out) = 0;
			};

			struct IClassStore: public IObject
			{
				static const InterfaceID IID;
				virtual bool registerClass(const ClassID & type, IClass * factory, TokenID * token) = 0;
				virtual bool unregisterClass(TokenID token) = 0;
			};

			struct IClassManager: public IObject
			{
				static const InterfaceID IID;
				virtual bool getClassStore(IClassStore **) = 0;
				virtual bool getClassLibrary(IClassLibrary **) = 0;
			};
			
			bool createKOM(IObject * owner, IObject ** out);
		}
	}
}

using namespace kq::core::kom;

//ICore {8B8E7C37-82E0-4d2b-B9AF-19E0495ACA89}
//const InterfaceID ICore::IID = { 0x8b8e7c37, 0x82e0, 0x4d2b, { 0xb9, 0xaf, 0x19, 0xe0, 0x49, 0x5a, 0xca, 0x89 } };

//IObject {5C72078F-FA2F-4213-8E53-54C9A747917A}
const InterfaceID IObject::IID = { 0x5c72078f, 0xfa2f, 0x4213, { 0x8e, 0x53, 0x54, 0xc9, 0xa7, 0x47, 0x91, 0x7a } };

//IClass {E3DF7609-96A6-4333-8AC0-77AF8F6B2E11}
const InterfaceID IClass::IID = { 0xe3df7609, 0x96a6, 0x4333, { 0x8a, 0xc0, 0x77, 0xaf, 0x8f, 0x6b, 0x2e, 0x11 } };

//IClassLibrary {5E28A35F-5FE5-4d10-A399-01A3DB4C3F8F}
const InterfaceID IClassLibrary::IID = { 0x5e28a35f, 0x5fe5, 0x4d10, { 0xa3, 0x99, 0x1, 0xa3, 0xdb, 0x4c, 0x3f, 0x8f } };

//IClassStore {929B0A00-C605-4920-B23B-7CD8F9FF857E}
const InterfaceID IClassStore::IID = { 0x929b0a00, 0xc605, 0x4920, { 0xb2, 0x3b, 0x7c, 0xd8, 0xf9, 0xff, 0x85, 0x7e } };

//IClassManager {155A7DA2-1C7C-40db-8E66-949ABC54BA9C}
const InterfaceID IClassManager::IID = { 0x155a7da2, 0x1c7c, 0x40db, { 0x8e, 0x66, 0x94, 0x9a, 0xbc, 0x54, 0xba, 0x9c } };

#include "map"
	

class Core: public ICore
{
	union{
		size_t count;
		ICore * owner;
	};

	void (Core::*addRef_dynamic)();
	void (Core::*releaseRef_dynamic)();
	bool (Core::*findInterface_dynamic)(const InterfaceID & type, IObject ** out);
public:
	Core(ICore * owner):owner(owner), addRef_dynamic(&Core::addRef_owner), releaseRef_dynamic(&Core::releaseRef_owner), findInterface_dynamic(&Core::findInterface_owner){}
	Core():count(1), addRef_dynamic(&Core::addRef_self), releaseRef_dynamic(&Core::releaseRef_self), findInterface_dynamic(&Core::findInterface_self){}
	virtual ~Core(){}

	void addRef_owner(){owner->addRef();}
	void releaseRef_owner(){owner->releaseRef();}
	bool findInterface_owner(const InterfaceID & type, IObject ** out){return owner->findInterface(type, out);}
	void addRef_self(){count++;}
	void releaseRef_self(){
		count--;
		if(!count) finalize();
	};
	virtual bool findInterface_self(const InterfaceID & type, IObject ** out) = 0;
	virtual void finalize(){delete this;}
	void addRef(){(this->*addRef_dynamic)();}
	void releaseRef(){(this->*releaseRef_dynamic)();}
	bool findInterface(const InterfaceID & type, IObject ** out){return (this->*findInterface_dynamic)(type, out);}

};

template<typename T>
class CoreCover: public Core, public IObject
{
public:
	CoreCover(){}
	~CoreCover(){uncover()->~T();}
	void * operator new (size_t sz)
	{
		return new uint8_t[sz + sizeof(T)];
	}

	void operator delete(void * p)
	{
		delete [] ((uint8_t *)p);
	}

	ICore * core(){return this;}
	T * uncover()
	{
		return (T *)(((uint8_t *)(this)) + sizeof(CoreCover));
	}

	bool findInterface_self(const InterfaceID & type, IObject ** out)
	{
		return uncover()->findInterface_self(type, out);
	}

	static bool createObject(const InterfaceID & type, IObject * owner, IObject ** out)
	{
		bool bret = false;
		IObject * oret;
		if(owner)
		{
			if(type == IObject::IID)
			{
				auto cover = new CoreCover<T>();
				new (cover->uncover())T(owner->core()); 
				oret = cover;
				bret = true;
			}
		}
		if(bret)
		{
			*out = oret;
		}
		return bret;
	}
};

template<typename T>
class CoreFactory: public IClass, public Core
{
public:
	CoreFactory(){}
	CoreFactory(ICore * owner): Core(owner){}
	~CoreFactory(){}

	bool findInterface_self(const InterfaceID & type, IObject ** out)
	{
		bool bret = false;
		IObject * oret;
		if(type == IClass::IID || type == IObject::IID)
		{
			addRef();
			bret = true;
			oret = this;
		}
		if(bret)
		{
			*out = oret;
		}
		return bret;
	}

	ICore * core(){return this;}

	bool createObject(const InterfaceID & type, IObject * owner, IObject ** out)
	{
		return create(type, owner, out);
	}

	static bool create(const InterfaceID & type, IObject * owner, IObject ** out)
	{
		if(owner == nullptr){
			bool bret = false;
			IObject * oret;
			if(type == IClassManager::IID || type == IObject::IID){
				bret = true;
				oret = new T();
			}
			if(bret)
			{
				*out = oret;
			}
			return bret;
		}
		else
		{
			return CoreCover<T>::createObject(type, owner, out);
		}
	}
};

class ClassManager: public IClassManager, private Core
{
public:
	static const ClassID CID;
private:
	class ClassStore: public IClassStore, private Core
	{
		ClassManager * owner;
	public:
		ClassStore(ClassManager * owner): owner(owner){}
		ClassStore(ClassManager * owner, ICore * facade): Core(facade), owner(owner){} 
		~ClassStore(){};
		bool findInterface_self(const InterfaceID & type, IObject ** out)
		{

			bool bret = false;
			IObject * oret;
			if(type == IClassStore::IID || type == IObject::IID)
			{
				addRef();
				bret = true;
				oret = this;
			}
			if(bret)
			{
				*out = oret;
			}
			return bret;
		} 

		ICore * core(){return this;}

		bool registerClass(const ClassID & type, IClass * factory, TokenID * token)
		{
			return owner->registerClass(type, factory, token);
		}

		bool unregisterClass(TokenID token)
		{
			return owner->unregisterClass(token);
		} 
	};

	class ClassLibrary:public IClassLibrary, private Core
	{
		ClassManager * owner;
	public:
		ClassLibrary(ClassManager * owner): owner(owner){}
		ClassLibrary(ClassManager * owner, ICore * facade): Core(facade), owner(owner){} 
		~ClassLibrary(){};
		bool findInterface_self(const InterfaceID & type, IObject ** out)
		{
			bool bret = false;
			IObject * oret;
			if(type == IClassLibrary::IID || type == IObject::IID)
			{
				addRef();
				bret = true;
				oret = this;
			}
			if(bret)
			{
				*out = oret;
			}
			return bret;
		}
		ICore * core(){return this;}


		bool findClass(const ClassID & type, IClass ** out)
		{
			return owner->findClass(type, out);
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
	
public:
	ClassManager():store(this), library(this)
	{
		auto factory = new CoreFactory<ClassManager>;
		registerClass(CID, factory, &factoryRegistration);
		factory->releaseRef();
	}

	ClassManager(ICore * owner): Core(owner), store(this), library(this)
	{
		auto factory = new CoreFactory<ClassManager>;
		registerClass(CID, factory, &factoryRegistration);
		factory->releaseRef();
	}

	~ClassManager()
	{
		//store->core()->releaseRef();
		//library->core()->releaseRef();
		unregisterClass(factoryRegistration);
	}
	bool findInterface_self(const InterfaceID & type, IObject ** out)
	{
		bool bret = false;
		IObject * oret;
		if(type == IClassManager::IID || type == IObject::IID)
		{
			addRef();
			bret = true;
			oret = this;
		}
		if(bret)
		{
			*out = oret;
		}
		return bret;
	}
	ICore * core(){return this;}
	
	bool registerClass(const ClassID & type, IClass * factory, TokenID * token)
	{
		ClassInfo c = {type, type, factory};
		std::pair<ClassID, ClassInfo> entry(type, std::move(c));
		auto pr = classMap.emplace(std::move(entry));
		if(pr.second){
			factory->core()->addRef();
			*token = type;
		}
		return pr.second;
	}
	
	bool unregisterClass(TokenID token)
	{
		auto it = classMap.find(token);
		if(it != classMap.end())
		{
			it->second.iclass->core()->releaseRef();
			return (classMap.erase(token) == 1);
		}
		return false;
	}

	bool findClass(const ClassID & type, IClass ** out)
	{
		auto it = classMap.find(type);
		if(it != classMap.end())
		{
			it->second.iclass->core()->addRef();
			*out = it->second.iclass;
			return true;
		}
		return false;
	}

	bool getClassStore(IClassStore ** out)
	{
		*out = &store;
		(*out)->core()->addRef();
		return true;
	}

	bool getClassLibrary(IClassLibrary ** out)
	{
		*out = &library;
		(*out)->core()->addRef();
		return true;
	}

};
const ClassID ClassManager::CID = { 0x4972fe0d, 0x25e, 0x45cc, { 0x9c, 0xad, 0x34, 0x95, 0x36, 0x98, 0xa5, 0x74 } };

class KOM: public Core, public IObject
{
	IObject * classmanager;
	void init()
	{
		ClassManager c;
		IClassLibrary * library;
		c.getClassLibrary(&library);
		IClass * factory;
		library->findClass(ClassManager::CID, &factory);
		factory->createObject(IObject::IID, this, &classmanager);
		factory->core()->releaseRef();
		library->core()->releaseRef();
	}
	void fini()
	{
		classmanager->core()->releaseRef();
	}
public:
	static const ClassID CID;
	KOM(){init();}
	KOM(ICore * owner): Core(owner){init();}
	~KOM(){fini();}
	bool findInterface_self(const InterfaceID & type, IObject ** out)
	{
		bool bret = false;
		IObject * oret;
		if(type == IClassManager::IID)
		{
			bret = classmanager->core()->findInterface(type, &oret);
		}
		if(bret)*out = oret;
		return bret;
	};

	ICore * core(){return this;}

	static bool createObject(IObject * owner, IObject ** out)
	{
		return CoreFactory<KOM>::create(IObject::IID, owner, out);
	}
};
//{89D0360A-59B2-47c6-9974-4629DEAB5DA9}
const ClassID KOM::CID = { 0x89d0360a, 0x59b2, 0x47c6, { 0x99, 0x74, 0x46, 0x29, 0xde, 0xab, 0x5d, 0xa9 } };

bool kq::core::kom::createKOM(IObject * owner, IObject ** out)
{
	return KOM::createObject(owner, out);
};

void classManagerTest(IClassManager * s, int iDepth)
{
	IClassStore * store = nullptr;
	IClassLibrary * library = nullptr;
	{
		s->getClassStore(&store);
		s->getClassLibrary(&library);
	}
	if(library && store)
	{
		
		IObject * lib2;
		kq::core::oops::assume(library->core()->findInterface(library->IID, &lib2) && lib2 == library);
		lib2->core()->releaseRef();
		IClass * classManager;
		if(library->findClass(ClassManager::CID, &classManager))
		{
			IClassManager * manager;
			if(iDepth && classManager->createObject(IClassManager::IID, nullptr, (IObject **)&manager))
			{
				classManagerTest(manager, iDepth - 1);
				manager->core()->releaseRef();
			}
			classManager->core()->releaseRef();
		}
		library->core()->releaseRef();
		store->core()->releaseRef();
	}
}

void komtest()
{
	IObject * kom;
	if(createKOM(nullptr, &kom))
	{
		IClassManager * manager;
		if(kom->core()->findInterface(IClassManager::IID, (IObject **)&manager))
		{
			classManagerTest(manager, 0);
			manager->core()->releaseRef();
		}
		kom->core()->releaseRef();
	}
}
/*
struct InterfaceID
{
	uint8_t data[16];
	bool operator == (const InterfaceID & other) const
	{
		return (memcmp(data, other.data, sizeof(data)) == 0);
	}
};
*/
//
//struct ICounted
//{
//private:
//	static const InterfaceID __InterfaceID;
//	static InterfaceID getInterfaceID()
//	{
//		return __InterfaceID;
//	}
//protected:
//	struct RefInfo
//	{
//		size_t count;
//	};
//
//	struct ICounted_Deleter
//	{
//		void operator ()(void const * p) const
//		{
//			((ICounted *)p)->releaseRef();
//		}
//	};
//
//	RefInfo * info;
//	virtual void finalize() const
//	{
//		delete this;
//	}
//
//	virtual ~ICounted()
//	{
//		delete info;
//	}
//
//	
//	virtual bool findInterface(const InterfaceID * InterfaceID, ICounted ** out)
//	{
//		if(*InterfaceID == __InterfaceID)
//		{
//			*out = this;
//			this->addRef();
//			return true;
//		}
//		return false;
//	}
//
//public:
//
//	ICounted()
//	{
//		info = new RefInfo();
//		info->count = 1;
//	}
//	
//
//	virtual void addRef() const
//	{
//		info->count++;
//	}
//
//	virtual void releaseRef() const
//	{
//		info->count--;
//		if(!info->count)
//		{
//			finalize();
//		}
//	}
//
//	size_t getRefCount() const
//	{
//		return info->count;
//	}
//	
//};
//
//struct IMemoryBlock:public virtual ICounted
//{
//private:
//	static const InterfaceID __InterfaceID;
//public:
//
//	virtual size_t MemoryBlock_getSize() const = 0;
//	virtual const char * MemoryBlock_getLocation() const = 0;
//	virtual char * MemoryBlock_getLocation() = 0;
//};
//
//
//class MemoryBlock:public IMemoryBlock
//{	
//	char * location;
//	const size_t size;
//
//public:
//	MemoryBlock(size_t sz):size(sz), location(new char[sz]){}
//	~MemoryBlock(){delete [] location;}
//	
//	virtual size_t MemoryBlock_getSize() const {return size;}
//	virtual const char * MemoryBlock_getLocation() const {return location;}
//	virtual char * MemoryBlock_getLocation(){return location;}
//};
//
//struct ProxyMemoryBlock: public IMemoryBlock
//{
//	IMemoryBlock * server;
//public:
//	ProxyMemoryBlock(IMemoryBlock * server):server(server){server->addRef();}
//	~ProxyMemoryBlock(){server->releaseRef();}
//	virtual size_t MemoryBlock_getSize() const {return server->MemoryBlock_getSize();}
//	virtual const char * MemoryBlock_getLocation() const {return server->MemoryBlock_getLocation();}
//	virtual char * MemoryBlock_getLocation(){return server->MemoryBlock_getLocation();}
//};
//
//class MemoryBlockContainer: public virtual ICounted
//{
//	struct ContainerInfo
//	{
//		IMemoryBlock * block;
//		size_t openForRWCount;
//		size_t openExclusiveCount;
//	};
//
//	ContainerInfo * info;
//
//	struct OpenMemoryBlock: public ProxyMemoryBlock
//	{
//		const MemoryBlockContainer * container;
//		size_t * count;
//		OpenMemoryBlock(const MemoryBlockContainer * container, size_t * count): ProxyMemoryBlock(container->info->block), container(container), count(count)
//		{
//			container->addRef();
//			(*count)++;
//		}
//
//		~OpenMemoryBlock()
//		{
//			(*count)--;
//			container->releaseRef();
//		}
//	};
//
//public:
//	MemoryBlockContainer(IMemoryBlock * block)
//	{
//		info = new ContainerInfo();
//		info->openExclusiveCount = 0;
//		info->openForRWCount = 0;
//		info->block = block;
//		if(info->block)
//			info->block->addRef();
//	}
//
//	~MemoryBlockContainer()
//	{		
//		if(info->block)
//			info->block->releaseRef();
//		delete info;
//	}
//
//	bool MemoryBlockCotainer_open(IMemoryBlock ** blk)
//	{
//		if(!info->openExclusiveCount)
//		{
//			*blk = new OpenMemoryBlock(this, &info->openForRWCount);
//			return true;
//		}
//		return false;
//	}
//
//	bool MemoryBlockCotainer_openForRead(const IMemoryBlock ** blk) const
//	{
//		if(!info->openExclusiveCount)
//		{
//			*blk = new OpenMemoryBlock(this, &info->openForRWCount);
//			return true;
//		}
//		return false;
//	}
//
//	bool MemoryBlockContainer_openExclusive(IMemoryBlock ** blk)
//	{
//		if(!info->openForRWCount && !info->openExclusiveCount)
//		{
//			*blk = new OpenMemoryBlock(this, &info->openExclusiveCount);
//			return true;
//		}
//		return false;
//	}
//};
//
//struct Data:public virtual ICounted
//{
//	typedef size_t BlockID;
//	
//	virtual size_t Data_getSize() const = 0;
//	virtual size_t Data_getMemoryBlockCount() const = 0;	
//	virtual bool Data_openBlock_ReadOnly(BlockID, const IMemoryBlock **) const = 0;
//
//
//	virtual bool Data_openBlock(BlockID, IMemoryBlock **) = 0;
//	
//};
//
//const InterfaceID ICounted::__InterfaceID = {0, 1};
//const InterfaceID IMemoryBlock::__InterfaceID = {0, 2};

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{	
	komtest();
	/*
	{
		IMemoryBlock * block = new MemoryBlock(100);	
		MemoryBlockContainer container(block);
		block->releaseRef();

		if(container.MemoryBlockCotainer_open(&block))
		{
			if(!container.MemoryBlockContainer_openExclusive(&block))
			{
				block->releaseRef();
				if(container.MemoryBlockContainer_openExclusive(&block))
				{
					block->releaseRef();
				}
			}
		}
	}
	*/

	_CrtDumpMemoryLeaks();

	/*
	kq::core::memory::StandardLibraryMemoryAllocator alloc;
	kq::core::memory::MemoryWorker mem;
	alloc.getMemoryWorker(mem);

	klass::IntentMessageSerializer msg(mem, 0);
	char type[] = "type";
	char name[] = "name";
	klass::IntentMessageSerializer::String sType, sName;
	sType.size = sizeof(type);
	sType.str = type;

	sName.size = sizeof(name);
	sName.str = name;

	msg.processIntentMessage_Create(0, 0, sName, sType, 0, nullptr);
	auto & outstr = msg.getSerializedMessage();
	
	klass::IntentMessageSerializer::String outstr2 = outstr;
	outstr2.str = (char *) malloc(outstr2.size);
	if(outstr2.str)
	{
		memcpy(outstr2.str, outstr.str, outstr2.size);
		klass::IntentMessageDeserializer msg2;
		msg2.push(outstr2);
	}
	*/
}


#else
#include "core.hpp"

using namespace kq::core;
using namespace kq::core::memory;


#define _WIN32_WINNT 0x0502
#define WINVER 0x0502
#include "windows.h"
#include "stdio.h"

template<typename DataType>
class DataQueue{
public:
	kq::core::memory::MemoryWorker mem;
public:

	class LinkNode; class DataNode;


	kq::core::memory::Pointer<LinkNode> createLinkNode(){		
		return kq_core_memory_workerRefCountedClassNew(mem, LinkNode);
	};

	kq::core::memory::Pointer<DataNode> createDataNode(){
		return kq_core_memory_workerRefCountedClassNew(mem, DataNode);
	};

	DataQueue(kq::core::memory::MemoryWorker & worker){
		mem = worker;
		m_pTail = createLinkNode();
	};

	class DataNode{
	public:
		kq::core::memory::Pointer<DataType> m_pData;
		kq::core::memory::Pointer<LinkNode> m_pNext;
	};

	class LinkNode{
		//Lock is acquired at construction and released when data is written
		//To do a blocking read, do lock, then unlock
		HANDLE m_lock;
	public:
		kq::core::memory::Pointer<DataNode> m_pNext;
		LinkNode(){
			m_pNext = 0;
			m_lock = CreateMutex(0, 0, 0);
			WaitForSingleObject(m_lock, INFINITE);
		}

		~LinkNode(){
			CloseHandle(m_lock);
		};

		void waitForData(){
			WaitForSingleObject(m_lock, INFINITE);
			ReleaseMutex(m_lock);
		}

	};
	
	kq::core::memory::Pointer<LinkNode> m_pTail;

	void push(kq::core::memory::Pointer<DataType> pData){
		kq::core::memory::Pointer<LinkNode> m_pNewTail = kq_core_memory_workerRefCountedClassNew(mem, LinkNode);
		kq::core::memory::Pointer<DataNode> m_pNewTailData = kq_core_memory_workerRefCountedClassNew(mem, DataNode);

		m_pNewTailData->m_pNext = m_pNewTail;
		m_pNewTailData->m_pData = pData;
		
		//Push into tail		
		m_pTail->m_pNext = m_pNewTailData;
		m_pTail = m_pNewTailData->m_pNext;
		ReleaseMutex(m_pTail->m_lock);
		
		
	};

	kq::core::memory::Pointer<LinkNode> getTail(){
		kq::core::memory::Pointer<LinkNode> ret;
		ret = m_pTail;
		return ret;
	};

	class Viewer{
	public:
		kq::core::memory::Pointer<LinkNode> m_pTail;

		bool hasData(){
			return (m_pTail->m_pNext != 0);
		};

		bool getData(kq::core::memory::Pointer<DataNode> & pDataNodeOut){
			if(hasData()){
				kq::core::memory::Pointer<LinkNode> pOldTail = getTail();
				kq::core::memory::Pointer<DataNode> pDataNode = pOldTail->m_pNext;
				pDataNodeOut = pDataNode->m_pData;
				m_pTail = pDataNode->m_pNext;
				return true;
			}
			return false;
		};

		bool waitAndGetData(kq::core::memory::Pointer<DataNode> & pDataNodeOut){
			
			m_pTail->waitForData();

			kq::core::memory::Pointer<LinkNode> pOldTail = getTail();
			kq::core::memory::Pointer<DataNode> pDataNode = pOldTail->m_pNext;
			pDataNodeOut = pDataNode->m_pData;
			m_pTail = pDataNode->m_pNext;
			return true;
		};
	};

	kq::core::memory::Pointer<Viewer> createViewer(){
		kq::core::memory::Pointer<Viewer> pRet = kq_core_memory_workerRefCountedClassNew(mem, Viewer);
		pRet->m_pTail = getTail();
		return pRet;
	};

};




class WindowsConsoleOutputStream{

	HANDLE m_hOut;

	static const i32 nMaxBytesInLogMessage = 2048;
	char m_sBuffer[nMaxBytesInLogMessage];
	FILE * m_fLog;

public:

	WindowsConsoleOutputStream(HANDLE hOut = 0){
		if(hOut != 0){
			m_hOut = hOut;
		}else{
			m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		}

		m_fLog = 0;
		
	}

	i32 output(c8 * pFormat, ...){
	   va_list args;
	   va_start( args, pFormat );	   
	   i64 nChars = vsprintf(m_sBuffer, pFormat, args);
	   //Add null termination
	   nChars++;
	   i64 iRet;
	   if(nChars < nMaxBytesInLogMessage){
		   DWORD nCharsOut = 0;
		   //Remove null termination from count
		   nChars--;
		   if(WriteConsoleA(m_hOut, m_sBuffer, (DWORD)nChars, &nCharsOut, 0) == TRUE){
			   iRet = nCharsOut;
		   }else{
			   iRet = -1;
		   };
	   }else{
		   iRet = -2;
	   }

	   return (i32)iRet;
	   
	};

	i32 log(c8 * pFormat, ...){
		/*
	   va_list args;
	   va_start( args, pFormat );	   
	   i64 nChars = vsprintf_s<nMaxBytesInLogMessage>(m_sBuffer, pFormat, args);

		if(!m_fLog){
			if(0 != fopen_s(&m_fLog, "out.txt", "w")){
			}
		}

		if(m_fLog){
			fwrite(m_sBuffer, (size_t)nChars, 1, m_fLog);
			fflush(m_fLog);
		}
		*/

		i64 iRet = 0;
		/*
	   //Add null termination
	   nChars++;
	   i64 iRet;
	   if(nChars < nMaxBytesInLogMessage){
		   DWORD nCharsOut = 0;
		   //Remove null termination from count
		   nChars--;
		   if(WriteConsoleA(m_hOut, m_sBuffer, (DWORD)nChars, &nCharsOut, 0) == TRUE){
			   iRet = nCharsOut;
		   }else{
			   iRet = -1;
		   };
	   }else{
		   iRet = -2;
	   }
	   */


	   return (i32)iRet;
	   
	};

	i32 output(c16 * pFormat, ...){
	   va_list args;
	   va_start( args, pFormat );	   
	   i64 nChars = vswprintf(*(wchar_t (*)[nMaxBytesInLogMessage/2])m_sBuffer, pFormat, args);
	   //Add null termination
	   nChars++;
	   i64 iRet;
	   if(nChars < nMaxBytesInLogMessage){
		   DWORD nCharsOut = 0;
		   //Remove null termination from count
		   nChars--;
		   if(WriteConsoleW(m_hOut, m_sBuffer, (DWORD)nChars, &nCharsOut, 0) == TRUE){
			   iRet = nCharsOut;
		   }else{
			   iRet = -1;
		   };
	   }else{
		   iRet = -2;
	   }

	   return (i32)iRet;
	   
	};


};


class Globals{
public:
	class GlobalsPtr{
	public:
		Pointer<Globals> pGlobals;

		GlobalsPtr(Pointer<Globals> pGlobals){
			this->pGlobals = pGlobals;
		};

	};
private:
	struct ThreadEntryParam{

		Pointer<Globals> pGlobals;
		DWORD (Globals::* pfnThread)(void *);		
		void * pParam;

	};

	static const int kBufferInOneSec = 4;

	static DWORD __stdcall threadEntry(void * pParam){
		ThreadEntryParam * p = (ThreadEntryParam *)pParam;
		Globals * pG = p->pGlobals.operator ->();
		return (pG->*(p->pfnThread))(p->pParam);
	};

	HANDLE createThread(ThreadEntryParam * p){		
		return CreateThread(0, 0, threadEntry, p, 0, 0);
	};
		
	WAVEHDR hdrIn[kBufferInOneSec];
	WAVEHDR hdrOut[kBufferInOneSec];
	DWORD openWaveIn(void * pGPtr){
		DWORD dwRet = 0;
		GlobalsPtr * pPtr = (GlobalsPtr *) pGPtr;
		Pointer<Globals> pGlobals = pPtr->pGlobals;

		WAVEFORMATEX fmtIn;
		fmtIn.cbSize = sizeof(fmtIn);

		fmtIn.wFormatTag = WAVE_FORMAT_PCM;
		fmtIn.nSamplesPerSec = 44100;
		fmtIn.nChannels = 2;
		fmtIn.wBitsPerSample = 16;

		fmtIn.nBlockAlign = fmtIn.nChannels * fmtIn.wBitsPerSample / 8;			
		fmtIn.nAvgBytesPerSec = (WORD)(fmtIn.nSamplesPerSec * fmtIn.nBlockAlign);

		HWAVEIN hWaveIn = 0;
		MMRESULT mmres = waveInOpen(&hWaveIn, WAVE_MAPPER, &fmtIn, (DWORD_PTR)&waveInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
		if(mmres == MMSYSERR_NOERROR){
			pGlobals->pConsole->output("Wave In Device opened\n");
			pGlobals->hWaveIn = hWaveIn;
			
			memset(hdrIn, 0, sizeof(hdrIn));
			for(i32 i = 0; i < sizeof(hdrIn)/sizeof(hdrIn[0]); i++){				
				DWORD nBytes = fmtIn.nSamplesPerSec * fmtIn.nBlockAlign/kBufferInOneSec;
				hdrIn[i].lpData = (LPSTR)mem(0, nBytes);
				hdrIn[i].dwBufferLength = nBytes;
				hdrIn[i].dwUser = i;
				
				if(MMSYSERR_NOERROR == waveInPrepareHeader(hWaveIn, &hdrIn[i], sizeof(hdrIn[i])))
					if(MMSYSERR_NOERROR == waveInAddBuffer(hWaveIn, &hdrIn[i], sizeof(hdrIn[i]))){
						
					};
			}			
			pGlobals->pConsole->output("Wave In Ready\n");
			
		}else{
			pGlobals->pConsole->output("Cannot Open Device\n");
			dwRet = 1;
		};

		return dwRet;
	};

	DWORD openWaveOut(void * pGPtr){
		DWORD dwRet = 0;
		GlobalsPtr * pPtr = (GlobalsPtr *) pGPtr;
		Pointer<Globals> pGlobals = pPtr->pGlobals;

		WAVEFORMATEX fmtIn;
		fmtIn.cbSize = sizeof(fmtIn);

		fmtIn.wFormatTag = WAVE_FORMAT_PCM;
		fmtIn.nSamplesPerSec = 44100;
		fmtIn.nChannels = 2;
		fmtIn.wBitsPerSample = 16;

		fmtIn.nBlockAlign = fmtIn.nChannels * fmtIn.wBitsPerSample / 8;			
		fmtIn.nAvgBytesPerSec = (WORD)(fmtIn.nSamplesPerSec * fmtIn.nBlockAlign);

		HWAVEOUT hWaveOut = 0;
		MMRESULT mmres = waveOutOpen(&hWaveOut, WAVE_MAPPER, &fmtIn, (DWORD_PTR)&waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
		if(mmres == MMSYSERR_NOERROR){
			pGlobals->hWaveOut = (HWAVEOUT)hWaveOut;
			pGlobals->pConsole->output("Wave Out Device Opened\n");

			memset(hdrOut, 0, sizeof(hdrIn));			
			for(i32 i = 0; i < sizeof(hdrOut)/sizeof(hdrOut[0]); i++){
				DWORD nBytes = fmtIn.nSamplesPerSec * fmtIn.nBlockAlign/kBufferInOneSec;
				hdrOut[i].lpData = (LPSTR)mem(0, nBytes);
				hdrOut[i].dwBufferLength = nBytes;
				hdrOut[i].dwUser = i;				
				if(MMSYSERR_NOERROR == waveOutPrepareHeader(hWaveOut, &hdrOut[i], sizeof(hdrOut[i])))
					{
						int j = 0;
						j++;
					};
			}			
			
			pGlobals->pConsole->output("Wave Out Ready\n");
		}else{
			pGlobals->pConsole->output("Cannot Open Device\n");
			dwRet = 1;
		};

		return dwRet;
	};
	

public:


	//Modified by Constructiong/Destruction
	MemoryWorker mem;
	//Pointer<Globals> pThis;

	//Modified by up/down
	Pointer<WindowsConsoleOutputStream> pConsole;
	HWAVEIN hWaveIn;
	HWAVEOUT hWaveOut;

	Globals(MemoryWorker &worker):mem(worker){
		pConsole = 0;
		hWaveIn = 0;
		hWaveOut = 0;
	}

	bool up(Pointer<Globals> pGlobals){

		GlobalsPtr globalsPtr(pGlobals);
		HANDLE hWaveThreads[2];

		
		ThreadEntryParam p[2];
		p[0].pGlobals = pGlobals;
		p[0].pfnThread = (&Globals::openWaveIn);
		p[0].pParam = &globalsPtr;
		
		hWaveThreads[0] = createThread(&p[0]);
		
		p[1] = p[0];
		p[1].pfnThread = (&Globals::openWaveOut);
		hWaveThreads[1] = createThread(&p[1]);

		if(hWaveThreads[0] && hWaveThreads[1]){
			WaitForMultipleObjects(2, hWaveThreads, TRUE, INFINITE);
			return true;
		}
		return false;
	};

	bool down(){
		if(hWaveIn){
			waveInClose(hWaveIn);
			hWaveIn = 0;
		}
		if(hWaveOut){
			waveOutClose(hWaveOut);
			hWaveOut = 0;
		};		
		return true;
	}

	~Globals(){
		down();
	};

	static void __stdcall waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2){
		hwi;
		uMsg;
		dwInstance;
		dwParam1;
		dwParam2;

		if(uMsg == WIM_DATA){
			Globals * pGlobals = (Globals *)dwInstance;

			WAVEHDR * pHdrIn = (WAVEHDR *)dwParam1;
			DWORD bufferIndex = pHdrIn->dwUser;
			WAVEHDR * pHdrOut = &pGlobals->hdrOut[bufferIndex];

			DWORD nShorts = pHdrIn->dwBytesRecorded/2;
			DWORD iShorts;

			double fPowerIn = 0;
			i16 * pSampleIn = (i16 *)pHdrIn->lpData;
			for(iShorts = 0; iShorts < nShorts; iShorts += 2){
				double l = pSampleIn[iShorts];
				double r = pSampleIn[iShorts + 1];
				l/= 32786.0;
				r/= 32786.0;

				fPowerIn += ((l*l + r*r)/(2 * nShorts));
			};


			if(!bufferIndex){
				pGlobals->pConsole->output("\n");
			};
						
			pGlobals->pConsole->output("%f ", fPowerIn);
			

			memset(pHdrOut->lpData, 0, pHdrIn->dwBytesRecorded);
			//memcpy(pHdrOut->lpData, pHdrIn->lpData, pHdrIn->dwBytesRecorded);
			
			i16 * pSample = (i16 *)pHdrOut->lpData;
			if(bufferIndex == 0){
				pSample[0] = 32767;
				pSample[1] = -32768;
			}

			/*
			while(nShorts){
				i16 i1, i2;
				i1 = *pSample;
				i2 = *(pSample + 1);

				*pSample = -(i1 + i2)/2;
				*(pSample + 1) = -(i1 + i2)/2;
				nShorts--;
			};
			*/
			pHdrOut->dwBytesRecorded = pHdrIn->dwBytesRecorded;
			pHdrOut->dwLoops = 1;

			MMRESULT res;
			if(MMSYSERR_NOERROR == (res = waveOutWrite(pGlobals->hWaveOut, pHdrOut, sizeof(*pHdrOut)))){
				int j;
				j = 0;
			}
			else{
				res = 0;
			}
		}
	}

	static void __stdcall waveOutProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2){
		hwi;
		uMsg;
		dwInstance;
		dwParam1;
		dwParam2;

		
		if(uMsg == WOM_DONE){
			Globals * pGlobals = (Globals *)dwInstance;

			WAVEHDR * pHdrOut = (WAVEHDR *)dwParam1;
			DWORD bufferIndex = pHdrOut->dwUser;
			WAVEHDR * pHdrIn = &pGlobals->hdrIn[bufferIndex];
			waveInAddBuffer(pGlobals->hWaveIn, pHdrIn, sizeof(*pHdrIn));
		}
	}
};




int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR /*resDir*/, int){

	if(!GetStdHandle(STD_OUTPUT_HANDLE)){
		if(AttachConsole(ATTACH_PARENT_PROCESS) == FALSE){
			if(AllocConsole() == FALSE){
				return -1;
				
			}
		}	
	}

	WindowsConsoleOutputStream console;
	WindowsConsoleOutputStream * pConsole = &console;
	pConsole->output("Console Init...\tAttached.\n");

	
	//Create standard allocator
	pConsole->output("Creating Standard Allocator\n");
	StandardLibraryMemoryAllocator allocStd;
	MemoryWorker memStd;
	allocStd.getMemoryWorker(memStd);

	//Create the pooled memory allocator
	pConsole->output("Creating Pooled Allocator\n");
	PooledMemoryAllocator allocPooled(memStd);
	MemoryWorker mem;
	allocPooled.getMemoryWorker(mem);


	{
		//Create Globals
		Pointer<Globals> pGlobals = kq_core_memory_workerRefCountedClassNew(mem, Globals, mem);
		if(pGlobals){
			pGlobals->pConsole = kq_core_memory_workerRefCountedClassNew(mem, WindowsConsoleOutputStream);
			if(pGlobals->up(pGlobals)){

				waveInStart(pGlobals->hWaveIn);
				bool b = true;
				while(b){
					Sleep(10);
				};
				pGlobals->down();
			}
		}
	}



	return 0;
};
#endif