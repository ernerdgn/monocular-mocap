#pragma once
#include "ui_types.hpp"

namespace mocap {

    class ConsolePanel : public IPanel
    {
        public:
        ConsolePanel();
        void render() override;
        private:
        bool m_autoScroll;
    };

}