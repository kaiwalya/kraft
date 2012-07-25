#ifdef _MSC_VER
#include "rpc.h"
#endif		

#include "stdint.h"

namespace kq
{
	namespace kom
	{
		typedef GUID GUID;
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
				return other < *this;
			}
		};

		typedef Identifier InterfaceID;
		typedef Identifier ClassID;
		typedef Identifier TokenID;

		struct IObject;
		struct ILifetime
		{
			//static const InterfaceID IID;
			virtual void addRef() = 0;
			virtual void releaseRef() = 0;
		};

		struct IInterfaceFinder
		{
			virtual bool findInterface(const InterfaceID & type, IObject ** out) = 0;
		};

		struct IObject
		{
			static const InterfaceID IID;
			ILifetime * lifetime;
			IInterfaceFinder * finder;
			IObject(){}
			IObject(ILifetime * lifetime, IInterfaceFinder * finder):lifetime(lifetime), finder(finder){}
			virtual ~IObject(){}
		};

		struct IAllocator: public IObject
		{
			static const InterfaceID IID;
			virtual void * memsize(void *, size_t) = 0;	
		};

		struct IForwardIterator
		{
			static const InterfaceID IID;
			bool getNext(IObject **);
			bool restart();
		};

		struct ILogger: public IObject
		{
			static const InterfaceID IID;
				
			typedef uint32_t LogLevel;
			static const LogLevel kLogLevel_Debug;
			static const LogLevel kLogLevel_Log;
			static const LogLevel kLogLevel_Warning;
			static const LogLevel kLogLevel_Error;
			virtual void log(LogLevel, const char * message) = 0;
		};
			
		struct IIntent: public IObject
		{
			static const InterfaceID IID;
			struct IntentCreationInfo
			{
			const char * name;
			const char * type;
			IIntent ** creators;
			};

			virtual bool createInnerIntent(const IntentCreationInfo &, IIntent **) = 0;
		};

		struct IClass: public IObject
		{
			static const InterfaceID IID;
			virtual bool createObject(const InterfaceID & type, IObject ** out) = 0;
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

		bool createKOM(IObject ** out);
	}
}