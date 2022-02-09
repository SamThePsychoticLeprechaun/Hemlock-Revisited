#ifndef __hemlock_graphics_window_window_h
#define __hemlock_graphics_window_window_h

#include "graphics/window/base.h"
#include "ui/input/events.hpp"

namespace hemlock {
    namespace graphics {
        class Window : public WindowBase {
        public:
            Window();
            virtual ~Window() { /* Empty. */ }

            virtual WindowError init(WindowSettings settings = {}) override;
            virtual void        dispose()                          override;

            virtual void set_name                (const std::string& name)                override;
            virtual void set_dimensions          (WindowDimensions dimensions)            override;
            virtual void set_width               (ui32 width)                             override;
            virtual void set_height              (ui32 height)                            override;
            virtual void set_display             (ui32 display_idx)                       override;
            virtual void set_fake_fullscreen     (bool fake_fullscreen)                   override;
            virtual void set_is_fullscreen       (bool is_fullscreen)                     override;
            virtual void set_is_resizable        (bool is_resizable)                      override;
            virtual void set_is_borderless       (bool is_borderless)                     override;
            virtual void set_is_maximised        (bool is_maximised)                      override;
            virtual void set_swap_interval       (SwapInterval swap_interval)             override;
            virtual void set_allowed_resolutions (WindowDimensionMap allowed_resolutions) override;
            virtual void set_fullscreen_mode     (FullscreenMode fullscreen_mode)         override;

            virtual void sync() override;
        protected:
            virtual void check_display_occupied() override;
        private:
            void determine_modes();

            SDL_Window*        m_window;
            SDL_GLContext      m_context;
        };
    }
}
namespace hg = hemlock::graphics;

#endif // __hemlock_graphics_window_window_h
