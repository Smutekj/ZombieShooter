#pragma once

#include <FrameBuffer.h>
#include <Renderer.h>


class PostEffect
{
public:
    virtual void process(Texture &source, Renderer &target) = 0;

protected:
};

class Bloom : public PostEffect
{

public:
    Bloom(int width, int height);
    virtual void process(Texture &source, Renderer &target)override;
    virtual ~Bloom() = default;

private:
    FrameBuffer m_bloom_pass1;
    FrameBuffer m_bloom_pass2;
    FrameBuffer m_downsampled_pixels3;
    FrameBuffer m_downsampled_pixels33;

    Renderer m_bloom_renderer1;
    Renderer m_bloom_renderer2;
    Renderer m_downsampler3;
    Renderer m_downsampler33;
};

class Bloom2 : public PostEffect
{

public:
    Bloom2(int width, int height, TextureOptions options = {});

    virtual void process(Texture &source, Renderer &target) override;
    virtual ~Bloom2() = default;

    struct TexMip
    {
        TexMip(int width, int height)
            : m_pixels(width, height), m_pixels2(width, height), m_canvas(m_pixels), m_canvas2(m_pixels2)
        {
        }

        FrameBuffer m_pixels;
        FrameBuffer m_pixels2;
        Renderer m_canvas;
        Renderer m_canvas2;
    };

    void initMips(int n_levels)
    {
        int width = 800;
        int height = 600;
        for (int i = 0; i < n_levels; ++i)
        {
            m_mips.emplace_back(width, height);
            width /= 2;
            height /= 2;
        }
    }

private:
    FrameBuffer m_downsampled_pixels3;
    FrameBuffer m_downsampled_pixels33;

    std::vector<FrameBuffer> m_mips;

    Renderer m_downsampler3;
    Renderer m_downsampler33;
};

class Bloom3 : public PostEffect
{

public:
    Bloom3(int width, int height, TextureOptions options = {});

    virtual void process(Texture &source, Renderer &target) override;
    virtual ~Bloom3() = default;


    void initMips(int n_levels, int width, int height, TextureOptions option);
    struct TexMip
    {
        TexMip(int width, int height, TextureOptions option)
        : pixels(width, height, option), canvas(pixels), pixels_tmp(width, height, option), canvas_tmp(pixels_tmp)
        {}
        FrameBuffer pixels;
        Renderer canvas;
        FrameBuffer pixels_tmp;
        Renderer canvas_tmp;
    };

private:
    std::vector<TexMip> m_mips;
};

class SmoothLight : public PostEffect
{

public:
    SmoothLight(int width, int height, TextureOptions options = {});

    virtual void process(Texture &source, Renderer &target) override;
    virtual ~SmoothLight() = default;

    struct TexMip
    {
        TexMip(int width, int height)
            : m_pixels(width, height), m_pixels2(width, height), m_canvas(m_pixels), m_canvas2(m_pixels2)
        {
        }

        FrameBuffer m_pixels;
        FrameBuffer m_pixels2;
        Renderer m_canvas;
        Renderer m_canvas2;
    };


private:
    FrameBuffer m_bloom_pass1;
    FrameBuffer m_bloom_pass2;
    FrameBuffer m_downsampled_pixels33;
    FrameBuffer m_downsampled_pixels3;

    std::vector<FrameBuffer> m_mips;

    Renderer m_bloom_renderer1;
    Renderer m_bloom_renderer2;
    Renderer m_downsampler3;
    Renderer m_downsampler33;
};

class EdgeHighLight : public PostEffect
{

public:
    EdgeHighLight(int width, int height);

    virtual void process(Texture &source, Renderer &target) override;
    virtual ~EdgeHighLight() = default;

private:
    Renderer m_bloom_renderer1;
    Renderer m_bloom_renderer2;
    Renderer m_downsampler3;
    Renderer m_downsampler33;
};

class BloomSmoke : public PostEffect
{

public:
    BloomSmoke(int width, int height);

    virtual void process(Texture &source, Renderer &target) override;
    virtual ~BloomSmoke() = default;



private:
    FrameBuffer m_bloom_pass1;
    FrameBuffer m_bloom_pass2;
    FrameBuffer m_downsampled_pixels3;
    FrameBuffer m_downsampled_pixels33;

    std::vector<FrameBuffer> m_mips;

    Renderer m_bloom_renderer1;
    Renderer m_bloom_renderer2;
    Renderer m_downsampler3;
    Renderer m_downsampler33;
};

class EdgeDetect : public PostEffect
{

public:
    EdgeDetect(int width, int height);

    virtual void process(Texture &source, Renderer &target) override;
    virtual ~EdgeDetect() = default;

private:
    FrameBuffer m_vert_pass;
    FrameBuffer m_horiz_pass;

    Renderer m_vert_canvas;
    Renderer m_horiz_canvas;
  
};
class LightCombine : public PostEffect
{

public:
    LightCombine(int width, int height);

    virtual void process(Texture &source, Renderer &target) override;
    virtual ~LightCombine() = default;

private:
    FrameBuffer m_multiply_texture;
    Renderer m_multiply_canvas;

};
