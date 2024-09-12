#pragma once

#include <string>
#include <vector>

#include <FrameBuffer.h>
#include <Renderer.h>

#include "PostEffects.h"

class DrawLayer
{

public:
    DrawLayer(int width, int height);
    DrawLayer(int width, int height, TextureOptions options);

    void toggleActivate();

    bool isActive() const;

    void draw(Renderer &window_rend);

    void drawDirectly(Renderer &canvas);

    void addEffect(std::unique_ptr<PostEffect> effect);

    void setBackground(Color c);
    Color getBackground();

public:
    Renderer m_canvas;
    FrameBuffer m_pixels;

    Renderer m_tmp_canvas1;
    FrameBuffer m_tmp_pixels1;

    Renderer m_tmp_canvas2;
    FrameBuffer m_tmp_pixels2;

    std::vector<std::unique_ptr<PostEffect>> m_effects;

    Color m_background_color = {0, 0, 0, 1};

private:
    bool m_is_active = true;
};

struct LayersHolder
{

    DrawLayer &addLayer(std::string name, int depth, TextureOptions options = {});
    // DrawLayer &addLayerOnTop(std::string name);
    // DrawLayer &addLayerDown(std::string name);
    bool hasLayer(std::string name)
    {
        return m_name2depth.count(name) > 0;
    }

    std::shared_ptr<DrawLayer> at(std::string name)
    {
        return m_layers.at(m_name2depth.at(name));
    }

    void clearAllLayers();

    Renderer &getCanvas(std::string name);
    FrameBuffer &getPixels(std::string name);
    void activate(std::string name)
    {
        at(name)->toggleActivate();
    }

    void draw(Renderer &target, const View &view);

    std::map<int, std::shared_ptr<DrawLayer>> m_layers;
    std::map<std::string, int> m_name2depth;
};
