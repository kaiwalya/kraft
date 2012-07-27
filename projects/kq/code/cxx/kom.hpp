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
			virtual void addRef() = 0;
			virtual void releaseRef() = 0;
		};

		struct IInterfaceFinder
		{
			virtual bool findInterface(const InterfaceID & type, IObject *, IObject ** out) = 0;
			virtual bool getSupportedInterfaceCount(size_t & sz) = 0;
			virtual bool getSupportedInterfaceType(size_t index, InterfaceID & type) = 0;
			virtual bool isTypeSupported(const InterfaceID & type) = 0;
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

		struct IObjectFactory: public IObject
		{
			static const InterfaceID IID;
			struct ISpecification: public IObject
			{
				static const InterfaceID IID;
				//typedef uint32_t ParameterIndex;
				//static const ParameterIndex kParamIndex_NotIndexed;				
				//virtual bool findParameter(const InterfaceID & type, ParameterIndex idx, IObject ** out) = 0;
				//virtual bool findAllocatedObject(IObject ** out) = 0;
			};
			virtual bool getObjectSize(const InterfaceID & type, ISpecification *, size_t & sz) = 0;
			virtual bool initializeObject(const InterfaceID & type, ISpecification *, IObject * obj) = 0;
			virtual bool createObject(const InterfaceID & type, ISpecification *, IObject ** out) = 0;		
		};		

		struct IClassLibrary: public IObject
		{
			static const InterfaceID IID;
			virtual bool findObjectFactory(const ClassID & type, IObjectFactory ** out) = 0;
		};

		struct IClassStore: public IObject
		{
			static const InterfaceID IID;
			virtual bool registerClass(const ClassID & type, IObjectFactory * factory, TokenID * token) = 0;
			virtual bool unregisterClass(TokenID token) = 0;
		};

		struct IClassManager: public IObject
		{
			static const InterfaceID IID;
			virtual bool getClassStore(IClassStore **) = 0;
			virtual bool getClassLibrary(IClassLibrary **) = 0;
		};

		IObjectFactory * getKOMFactory();
		
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

	}
}