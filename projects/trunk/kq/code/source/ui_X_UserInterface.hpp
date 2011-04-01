#if !defined(KQ_UI_X_USERINTERFACE_HPP)
#define KQ_UI_X_USERINTERFACE_HPP

#include "ui_UserInterface.hpp"

namespace kq{
    namespace ui{
        namespace X{
            kq::core::memory::Pointer<kq::ui::UserInterface> createInstance(kq::core::memory::MemoryWorker &mem);
        }
    }
}

#endif
