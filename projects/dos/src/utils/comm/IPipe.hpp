/*
 * Endpoint.hpp
 *
 *  Created on: Mar 17, 2009
 *      Author: k
 */

#ifndef IPIPE_HPP_
#define IPIPE_HPP_

#include "stdint.h"

namespace dos{
	namespace utils{
		namespace comm{
			typedef uint32_t Size;
			typedef uint64_t Location;
			typedef uint64_t ID;


			class Fragment{

			public:

				Size size;
				Location location;
				Location next;

				Size getAggregateSize();
				void getAggregateData(void * pData);
			};


			class IPipe{
			public:
				virtual ~IPipe();

				virtual void push(Fragment *) = 0;
				//Message will be owned by the called after this call.
				virtual void pop(Fragment &) = 0;
			};

		}
	}
}

#endif /* ENDPOINT_HPP_ */
