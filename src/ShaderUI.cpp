#include "ShaderUI.h"

#include <filesystem>
#include <variant>

#include <Window.h>
#include <FrameBuffer.h>

#include "DrawLayer.h"
#include "GameWorld.h"
#include "LuaWrapper.h"
#include "Utils/IO.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <magic_enum.hpp>
#include <magic_enum_utility.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>

UIWindow::UIWindow(std::string name) : name(name)
{
}

UI::UI(Window &window, TextureHolder &textures,
       LayersHolder &layers, Renderer &window_canvas)
    : m_layers(layers)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window.getHandle(), window.getContext());
    ImGui_ImplOpenGL3_Init();

    auto shaders_window = std::make_unique<ShadersWindow>(textures, m_layers, window_canvas);
    auto lua_window = std::make_unique<LuaWindow>();

    m_window_data[UIWindowType::Shaders].p_window = std::move(shaders_window);
    m_window_data[UIWindowType::Shaders].name = "Shaders";
    m_window_data[UIWindowType::Shaders].is_active = false;
    m_window_data[UIWindowType::Lua].p_window = std::move(lua_window);
    m_window_data[UIWindowType::Lua].name = "Lua";
    m_window_data[UIWindowType::Lua].is_active = true;
}

std::vector<std::string> extractFragmentShaderNames(const std::filesystem::path shader_dir = "../Resources/")
{
    std::vector<std::string> shader_names;
    for (auto const &dir_entry : std::filesystem::directory_iterator{shader_dir})
        shader_names.push_back(dir_entry.path());
    return shader_names;
}

ShadersWindow::ShadersWindow(TextureHolder &textures, LayersHolder &layers, Renderer &window_canvas)
    : UIWindow("Shaders"), m_textures(textures)
{
    for (auto &[depth, l_ptr] : layers.m_layers)
    {
        m_slots.emplace_back(*l_ptr);
    }
    for (auto &[id, shader] : GameWorld::getWorld().m_shaders.getShaders())
    {
        m_shader_slots.emplace_back(*shader, id);
    }
    ;
}

LuaWindow::LuaWindow()
    : UIWindow("Lua")
{
    m_current_command.reserve(200);
    m_script_name.reserve(200);
    m_entered_name.reserve(200);
}

void LuaWindow::draw()
{
    ImGui::Begin("Lua");

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;

    if (ImGui::InputText("Command", m_current_command.data(), 200, flags))
    {
        auto lua = LuaWrapper::getSingleton();
        if (!lua->doString(m_current_command.c_str()))
        {
            m_last_error_msg = lua->getLastError();
        }
        else
        {
            m_last_error_msg = "";
        }
        //! add command to history and remove old commands
        m_command_history.push_back(m_current_command);
        if (m_command_history.size() > 20)
        {
            m_command_history.pop_front();
        }
    }
     if (ImGui::InputText("Script", m_script_name.data(), 200, flags))
    {
        auto lua = LuaWrapper::getSingleton();
        if (!lua->doFile(m_script_name.c_str()))
        {
            m_last_error_msg = lua->getLastError();
        }
    }

    auto &scene = GameWorld::getWorld().m_scene;
    for (auto root_id : scene.m_roots)
    {
        std::queue<int> to_visit;
        to_visit.push(root_id);
        while (!to_visit.empty())
        {
            auto current_id = to_visit.front();
            auto &current = scene.m_nodes.at(current_id);
            to_visit.pop();
            assert(current.p_object);
            auto node_name = GameWorld::getWorld().getName(current.p_object->getId());

            bool opened = ImGui::TreeNode(node_name.c_str());
            ImGui::SameLine();
            bool selected = false;
            if (ImGui::Selectable(node_name.c_str(), selected))
            {
                m_selected_entity_id = current.p_object->getId();
            }
            if (opened)
            {
                for (auto child : current.children)
                {
                    to_visit.push(child);
                }
                ImGui::TreePop();

                static char buf[69] = "";
                ImGuiInputTextFlags flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;
                if (ImGui::InputText("Name", buf, 69, flags))
                {
                    GameWorld::getWorld().setName(current.p_object->getId(), {buf});
                }
            }
        }
    }
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
    ImGui::Text("%s", m_last_error_msg.c_str());
    ImGui::PopStyleColor();

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
    {
        ImGui::SetNextFrameWantCaptureKeyboard(true);
        if (ImGui::IsKeyChordPressed(ImGuiKey_DownArrow) && m_command_history.size() > 0)
        {
            m_selected_command_ind = (m_selected_command_ind + 1) % m_command_history.size();
            m_current_command = m_command_history.at(m_selected_command_ind);
        }
    }

    if (ImGui::InputFloat2("Coordinates: ", &m_coords.x))
    {
        auto p_object = GameWorld::getWorld().get(m_selected_entity_id);
        p_object->setPosition(m_coords);
    }

    ImGui::End();
}

ShadersWindow::~ShadersWindow() {}

template <class T>
using uncvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
template <typename T>
struct fail : std::false_type
{
};

void ShadersWindow::drawUniformValue(const char *name, UniformType &value)
{

    std::visit([&name](auto &value)
               {
                    using T = uncvref_t<decltype(value)>; 
                   if constexpr (std::is_same_v<T, int>)
                    {                       
                       if(ImGui::InputInt(name, &value)){
                            
                       }
                    }
                    else if constexpr (std::is_same_v<T, float>)
                    {
                       if(ImGui::SliderFloat(name, &value, 0., 1.))
                       {
                       }
                     }
                   else if constexpr (std::is_same_v<T, bool>)
                    {   
                        if(ImGui::Button(name))
                        {
                            value = !value; 
                        }
                    }
                   else if constexpr (std::is_same_v<T, glm::vec2>)
                   {
                        float* val = &value[0];
                        ImGui::InputFloat2(name, val);
                   }
                   else if constexpr (std::is_same_v<T, glm::vec3>)
                   {
                        float* val = &value[0];
                        ImGui::ColorPicker3(name, val);
                   }
                   else if constexpr (std::is_same_v<T, glm::vec4>)
                   {
                        float* val = &value[0];
                        ImGui::InputFloat4(name, val);
                   }
                   else 
                   {
                         static_assert(fail<T>{}, "we should not get here!");
                   } },
               value);
}

void ShadersWindow::drawShaderSlot(ShaderSlot &slot)
{
    auto window_size = ImGui::GetWindowSize();
    ImVec2 shader_box_size = {0.45f * window_size.x, window_size.y / 4.2f};

    auto &selected_shader = slot.m_shader;
    auto &shader_variables = selected_shader.getVariables();

    ImGui::SameLine();
    if (ImGui::BeginListBox("", shader_box_size))
    {
        for (auto &[name, value] : shader_variables.uniforms)
        {
            const bool is_selected = (slot.m_selected_uniform == name);
            if (ImGui::Selectable(name.c_str(), is_selected))
            {
                slot.m_selected_uniform = name;
                selected_shader.saveUniformValue(name, value);
            }
        }
        ImGui::EndListBox();
    }
    for (auto &[texture_name, value] : shader_variables.textures)
    {
        if (ImGui::BeginListBox(texture_name.c_str(), shader_box_size))
        {
            for (auto &[texture_identifier, texture_ptr] : m_textures.m_textures)
            {
                const bool is_selected = (value.handle == texture_ptr->getHandle());
                if (ImGui::Selectable(texture_identifier.c_str(), is_selected))
                {
                    value.handle = texture_ptr->getHandle(); //! set shader texture to selected texture
                    slot.m_selected_texture = texture_identifier;
                }
            }
            ImGui::EndListBox();
        }
    }

    if (slot.m_selected_uniform != "")
    {
        drawUniformValue(slot.m_selected_uniform.c_str(),
                         shader_variables.uniforms.at(slot.m_selected_uniform));
    }
}

void ShadersWindow::refresh()
{
    // std::filesystem::path path = "../Resources/";
    // auto shader_filenames = extractNamesInDirectory(path, ".frag");

    // std::vector<std::string> new_shader_paths;

    // auto &default_slot = m_slots.at(0);
    // for (auto &shader_filename : shader_filenames)
    // {

    //     auto pos_right = shader_filename.find_last_of('.');
    //     std::string shader_name = shader_filename.substr(0, pos_right);
    //     if (default_slot.m_canvas.getShaders().getAllData().count(shader_name) == 0)
    //     {
    //         for (auto &slot : m_slots)
    //         {
    //             slot.m_canvas.addShader(shader_name, path.string() + "basicinstanced.vert",
    //                                     path.string() + shader_filename);
    //         }
    //     }

    //     new_shader_paths.push_back(shader_filename);
    // }
}

void ShadersWindow::draw()
{
    {
        std::filesystem::path path = "../Resources/";
        auto names = extractNamesInDirectory(path, ".png");

        ImGui::Begin("Shader Manager");
        auto shader_names = extractNamesInDirectory(path, ".frag");
        if (ImGui::Button("Refresh"))
        {
            refresh();
        }
        ImGui::End();

        ImGui::Begin("Layers");
        int i = 0;
        for (auto &slot : m_shader_slots)
        {
            if (ImGui::TreeNode(slot.m_shader.getName().c_str()))
            {
                drawShaderSlot(slot);
                m_output_image_name.resize(50);
                if (ImGui::InputText("File name: ", m_output_image_name.data(), m_output_image_name.size()))
                {
                }
                if (ImGui::Button("Draw Texture"))
                {
                    // writeTextureToFile("./", m_output_image_name, slot.m_pixels);
                }
                ImGui::TreePop();
            }

            i++;
        }
        ImGui::End();
    }
}

UIWindow::~UIWindow()
{
}

void UI::showWindow()
{
}

void UI::draw(Window &window)
{

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    bool show_demo_window = true;
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    std::vector<UIWindowType> active_windows;
    ImGui::Begin("Control Panel"); // Create a window called "Hello, world!" and append into it.
    for (auto &[type, data] : m_window_data)
    {
        if (ImGui::Button(data.name.c_str()))
            data.is_active = !data.is_active;
    }

    ImGui::Begin("Colors");

    ImGui::InputFloat4("Init Color", &m_particle_init_color.r);
    ImGui::InputFloat4("End Color", &m_particle_end_color.r);
    ImGui::InputFloat4("BackgRound", &m_background_color.r);
    ImGui::InputFloat4("Light", &m_light_color.r);

    ImGui::InputText("Selected Layer:", m_selected_layer.data(), 10);

    ImGui::ColorPicker4("Background:", &m_layer_background.r);

    if (m_layers.hasLayer(m_selected_layer))
    {
        auto p_layer = m_layers.getLayer(m_selected_layer);
        if (ImGui::Button("Activate Layer"))
        {
            m_layers.activate(m_selected_layer);
        }
        p_layer->setBackground(m_layer_background);
    }

    ImGui::End();

    ImGui::End();

    for (auto &[type, window_data] : m_window_data)
    {
        if (window_data.is_active)
        {
            window_data.p_window->draw();
        }
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::handleEvent(SDL_Event event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}