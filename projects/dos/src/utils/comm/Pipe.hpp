/*
 * Pipe.hpp
 *
 *  Created on: Mar 17, 2009
 *      Author: k
 */

#ifndef PIPE_HPP_
#define PIPE_HPP_
#include "IPipe.hpp"

namespace dos{
	namespace utils{
		namespace comm{

			class LoopbackPipe: public IPipe{
			protected:
				Fragment * m_pHead;
				Fragment * m_pTail;
			public:
				LoopbackPipe();
				~LoopbackPipe();
				void push(Fragment *);
				void pop(Fragment &);

			};
		}
	}
}

#endif /* PIPE_HPP_ */
