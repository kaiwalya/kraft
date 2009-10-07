/*
 * DynamicVariables.hpp
 *
 *  Created on: Feb 25, 2009
 *      Author: k
 */

#ifndef DYNAMICVARIABLES_HPP_
#define DYNAMICVARIABLES_HPP_
#include "map"
#include "string"

namespace dos {

	namespace utils {

		class DynamicVariables {

		public:
			virtual ~DynamicVariables();
			enum ValueType{
				vtNone,
				vtString,
				vtUser,
			};

			struct Value{
				ValueType type;
				union{
					std::string * str;
					void * usr;
				}val;
			};

			typedef std::string  Name;
			typedef std::map<Name, Value> MapType;
			MapType m_map;

			void set(Name n, ValueType type, void * pVal);
			void getRawValue(Name n, Value * pRet);
			std::string * getString(Name n);

		protected:
			void clean(Value * v);
		};

	}

}

#endif /* DYNAMICVARIABLES_HPP_ */
