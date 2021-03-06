#ifndef __hemlock_tests_app_hpp
#define __hemlock_tests_app_hpp

#include "app/single_window_app.h"
#include "app/process/process_base.h"
#include "app/window/state.hpp"
#include "app/window/window_base.h"

#include "test_entry_screen.hpp"
#include "test_render_screen.hpp"
#include "test_voxel_screen.hpp"

class MyApp : public happ::SingleWindowApp {
public:
    virtual ~MyApp() { /* Empty */ };
protected:
    virtual void prepare_screens() override {
        happ::ScreenBase* test_entry_screen = new TestEntryScreen();
        test_entry_screen->init("test_entry_screen", this);
        happ::ScreenBase* test_render_screen = new TestRenderScreen();
        test_render_screen->init("test_render_screen", this);
        happ::ScreenBase* test_voxel_screen = new TestVoxelScreen();
        test_voxel_screen->init("test_voxel_screen", this);

        m_screens.insert({ "test_entry_screen",  test_entry_screen  });
        m_screens.insert({ "test_render_screen", test_render_screen });
        m_screens.insert({ "test_voxel_screen",  test_voxel_screen  });

        m_current_screen = test_entry_screen;
    }
};

#endif // __hemlock_tests_app_hpp
