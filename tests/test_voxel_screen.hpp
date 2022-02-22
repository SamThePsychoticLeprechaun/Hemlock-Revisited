#ifndef __hemlock_tests_test_voxel_screen_hpp
#define __hemlock_tests_test_voxel_screen_hpp

#include "iomanager.hpp"

class TestVoxelScreen : public happ::ScreenBase {
public:
    TestVoxelScreen() :
        happ::ScreenBase(),
        m_input_manager(nullptr)
    { /* Empty. */ }
    virtual ~TestVoxelScreen() { /* Empty */ };

    virtual void update(TimeData time) override {
        m_sprite_batcher.begin();
        m_sprite_batcher.add_sprite(
            f32v2{
                60.0f + 30.0f * sin(time.total / 1000.0f),
                60.0f + 30.0f * cos(time.total / 1000.0f)
            },
            f32v2{200.0f, 200.0f},
            colour4{255,   0, 0, 255},
            colour4{  0, 255, 0, 255},
            hg::Gradient::LEFT_TO_RIGHT
        );
        m_sprite_batcher.add_string(
            "Hello, world!",
            f32v4{300.0f, 300.0f, 1000.0f, 1000.0f},
            f32v4{295.0f, 295.0f, 1010.0f, 1010.0f},
            hg::f::StringSizing{hg::f::StringSizingKind::SCALED, f32v2{1.0f}},
            colour4{0, 0, 0, 255},
            "fonts/Orbitron-Regular.ttf",
            hg::f::TextAlign::TOP_LEFT,
            hg::f::WordWrap::NONE//,
            // 0.0f,
            // hg::f::FontStyle::NORMAL,
            // hg::f::FontRenderStyle::SOLID
        );
        m_sprite_batcher.end();

        m_chunk_grid.update(time);

        f32 speed_mult = 1.0f;
        if (m_input_manager->key_modifier_state().ctrl) {
            speed_mult = 10.0f;
        }
        if (m_input_manager->key_modifier_state().alt) {
            speed_mult = 50.0f;
        }

        f32v3 delta_pos{0.0f};
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_W)) {
            delta_pos += glm::normalize(m_camera.direction()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_A)) {
            delta_pos -= glm::normalize(m_camera.right()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_S)) {
            delta_pos -= glm::normalize(m_camera.direction()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_D)) {
            delta_pos += glm::normalize(m_camera.right()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_Q)) {
            delta_pos += glm::normalize(m_camera.up()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_E)) {
            delta_pos -= glm::normalize(m_camera.up()) * static_cast<f32>(time.frame) * 0.01f * speed_mult;
        }

#if defined(DEBUG)
        static f64 last_time = 0.0;
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_T)) {
            if (last_time + 1000.0 < time.total) {
                last_time = time.total;
                f32v3 pos = m_camera.position();
                f32v3 dir = m_camera.direction();
                debug_printf("Camera Coords: (%f, %f, %f)\nCamera Direction: (%f, %f, %f)\n", pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);
            }
        }
#endif

        // const f32 turn_clamp_on_at = 60.0f / 360.0f * 2.0f * M_PI;
        // f32 up_angle = glm::acos(glm::dot(m_camera.up(), hcam::ABSOLUTE_UP));
        // if (up_angle < -1.0f * turn_clamp_on_at || up_angle > turn_clamp_on_at)
        //     m_camera.set_clamp_enabled(true);

        m_camera.offset_position(delta_pos);
        m_camera.update();
    }
    virtual void draw(TimeData time) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_shader.use();

        glUniformMatrix4fv(m_shader.uniform_location("view_proj"),  1, GL_FALSE, &m_camera.view_projection_matrix()[0][0]);

        glBindTextureUnit(0, m_default_texture);
        glUniform1i(m_shader.uniform_location("tex"), 0);

        m_chunk_grid.draw(time);
        // for (auto& chunk : m_chunk_grid.chunks()) {
        //     if (chunk.second->state != hvox::ChunkState::MESH_UPLOADED) continue;

        //     glBindVertexArray(chunk.second->mesh_handles.vao);

        //     auto chunk_position = hvox::block_world_position(chunk.second->position, 0);
        //     auto translation_matrix = glm::translate(f32m4{1.0f}, f32v3{chunk_position});
        //     glUniformMatrix4fv(m_shader.uniform_location("model"),  1, GL_FALSE, &translation_matrix[0][0]);

        //     glDrawArrays(GL_TRIANGLES, 0, chunk.second->mesh.vertex_count);
        // }
        // glBindVertexArray(0);

        // Deactivate our shader.
        m_shader.unuse();

        // happ::WindowDimensions dims = m_process->window()->dimensions();
        // m_sprite_batcher.render(f32v2{dims.width, dims.height});
        m_sprite_batcher.render(f32m4{1.0f}, m_camera.view_projection_matrix());
    }

    virtual void init(const std::string& name, happ::ProcessBase* process) override {
        happ::ScreenBase::init(name, process);

        m_state = happ::ScreenState::RUNNING;

        m_input_manager = static_cast<happ::SingleWindowApp*>(m_process)->input_manager();

        m_camera.attach_to_window(m_process->window());
        m_camera.set_position(f32v3{270.0f, 230.0f, -470.0f});
        m_camera.rotate_from_mouse_with_absolute_up(-110.0f, 110.0f, 0.005f);
        m_camera.set_fov(90.0f);
        // m_camera.set_clamp({false, 30.0f / 360.0f * 2.0f * M_PI});
        m_camera.update();

        // std::cout << std::endl << my_shader_parser("shaders/default_sprite.frag", &my_iom) << std::endl << std::endl;
        m_shader_cache.init(&m_iom, hg::ShaderCache::Parser(
            [](const hio::fs::path& path, hio::IOManagerBase* iom) -> std::string {
                std::string buffer;
                if (!iom->read_file_to_string(path, buffer)) return "";

                return buffer;
            }
        ));

        m_shader.init(&m_shader_cache);

        m_shader.set_attribute("v_position",      0);
        m_shader.set_attribute("v_texture_coord", 1);

        m_shader.add_shaders("shaders/test_vox.vert", "shaders/test_vox.frag");

        m_shader.link();

        m_default_texture = hg::load_texture("test_tex.png");

        m_chunk_grid.init(10);

#define NUM 5
        for (auto x = -NUM; x < NUM; ++x) {
            for (auto z = -NUM; z < NUM; ++z) {
                for (auto y = -2 * NUM; y < 0; ++ y) {
                    m_chunk_grid.preload_chunk_at({ x, y, z });
                }
            }
        }
        for (auto x = -NUM; x < NUM; ++x) {
            for (auto z = -NUM; z < NUM; ++z) {
                for (auto y = -2 * NUM; y < 0; ++ y) {
                    m_chunk_grid.load_chunk_at({ x, y, z });
                }
            }
        }
#undef NUM

        handle_mouse_move = hemlock::Subscriber<hui::MouseMoveEvent>(
            [&](hemlock::Sender, hui::MouseMoveEvent ev) {
                if (m_input_manager->is_pressed(static_cast<ui8>(hui::MouseButton::LEFT))) {
                    m_camera.rotate_from_mouse_with_absolute_up(
                        -1.0f * static_cast<f32>(ev.dx),
                        -1.0f * static_cast<f32>(ev.dy),
                        0.005f
                    );
                }
            }
        );

        hui::InputDispatcher::instance()->on_mouse.move += &handle_mouse_move;

        m_font_cache.init(&m_iom, hg::f::FontCache::Parser(
            [](const hio::fs::path& path, hio::IOManagerBase* iom) -> hg::f::Font {
                hio::fs::path actual_path;
                if (!iom->resolve_path(path, actual_path)) return hg::f::Font{};

                hg::f::Font font;
                font.init(actual_path.string());

                return font;
            }
        ));


        auto font = m_font_cache.fetch("fonts/Orbitron-Regular.ttf");
        font->set_default_size(50);
        font->generate(/*hg::f::FontStyle::NORMAL, hg::f::FontRenderStyle::SOLID*/);
        // auto font_instance = font->get_instance(/*hg::f::FontStyle::NORMAL, hg::f::FontRenderStyle::SOLID*/);
        // font_instance.save("test.png", {hio::img::png::save});

        m_sprite_batcher.init(&m_shader_cache, &m_font_cache);
    }
protected:
    hemlock::Subscriber<hui::MouseMoveEvent>      handle_mouse_move;

    ui32 m_default_texture;

    MyIOManager                  m_iom;
    hg::ShaderCache              m_shader_cache;
    hg::f::FontCache             m_font_cache;
    hg::s::SpriteBatcher         m_sprite_batcher;
    hcam::BasicFirstPersonCamera m_camera;
    hui::InputManager*           m_input_manager;
    hvox::ChunkGrid              m_chunk_grid;
    hg::GLSLProgram              m_shader;
};

#endif // __hemlock_tests_test_voxel_screen_hpp
