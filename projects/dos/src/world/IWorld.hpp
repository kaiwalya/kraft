#ifndef DOS_WORLD_IWORLD_HPP_
#define DOS_WORLD_IWORLD_HPP_

#include "../layout.hpp"

namespace dos{
	namespace world{

		struct WorldInit{
			dos::system::Engine * m_pEngine;
		};

		class IWorld{
		public:
			virtual void up(void **) = 0;
			virtual void down(void **) = 0;
			virtual void process(void **) = 0;

			virtual void move(void **) = 0;
			virtual void render(void **) = 0;
			virtual ~IWorld(){};

		};
	}
}


#endif /* IWORLD_HPP_ */
