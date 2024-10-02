#include "PostEffects.h"

using bf = BlendFactor;

Bloom::Bloom(int width, int height)
    : m_bloom_pass1(width, height),
      m_bloom_pass2(width, height),
      m_downsampled_pixels3(width / 8, height / 8),
      m_downsampled_pixels33(width / 8, height / 8),
      m_bloom_renderer1(m_bloom_pass1),
      m_bloom_renderer2(m_bloom_pass2),
      m_downsampler3(m_downsampled_pixels3),
      m_downsampler33(m_downsampled_pixels33)
{
    m_bloom_renderer1.addShader("gaussVert", "../Resources/basicinstanced.vert", "../Resources/gaussVert.frag");
    m_bloom_renderer2.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_bloom_renderer2.addShader("brightness", "../Resources/basicinstanced.vert", "../Resources/brightness.frag");
    m_downsampler3.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_downsampler3.addShader("brightness", "../Resources/basicinstanced.vert", "../Resources/brightness.frag");
    m_downsampler33.addShader("gaussVert", "../Resources/basicinstanced.vert", "../Resources/gaussVert.frag");

    m_bloom_renderer1.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
    m_bloom_renderer2.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
}

void Bloom::process(Texture &source, Renderer &target)
{
    if (!target.hasShader("combineBloom"))
    {
        target.addShader("combineBloom", "../Resources/basicinstanced.vert", "../Resources/combineBloom.frag");
    }

    auto old_view = target.m_view;
    auto old_blend_factors = target.m_blend_factors;

    auto target_size = target.getTargetSize();
    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size / 2.f);
    screen_sprite.setScale(target_size / 2.f);

    m_bloom_renderer1.m_view.setCenter(screen_sprite.getPosition());
    m_bloom_renderer1.m_view.setSize(target_size);

    m_bloom_renderer2.m_view.setCenter(screen_sprite.getPosition());
    m_bloom_renderer2.m_view.setSize(target_size);

    m_bloom_renderer1.clear({0, 0, 0, 0});
    m_bloom_renderer2.clear({0, 0, 0, 0});

    m_bloom_renderer2.drawSprite(screen_sprite, "brightness", DrawType::Dynamic);
    m_bloom_renderer2.drawAll();

    for (int pass = 0; pass < 4; ++pass)
    {
        m_bloom_renderer1.clear({0, 0, 0, 0});
        screen_sprite.setTexture(m_bloom_pass2.getTexture());
        m_bloom_renderer1.drawSprite(screen_sprite, "gaussVert", DrawType::Dynamic);
        m_bloom_renderer1.drawAll();

        m_bloom_renderer2.clear({0, 0, 0, 0});
        screen_sprite.setTexture(m_bloom_pass1.getTexture());
        m_bloom_renderer2.drawSprite(screen_sprite, "gaussHoriz", DrawType::Dynamic);
        m_bloom_renderer2.drawAll();
    }

    screen_sprite.setTexture(0, source);
    screen_sprite.setTexture(1, m_bloom_pass2.getTexture());
    auto pixels_size = m_bloom_pass2.getSize();
    screen_sprite.setPosition(pixels_size / 2.f);
    screen_sprite.setScale(pixels_size / 2.f);
    target.m_view.setCenter(pixels_size / 2.f);
    target.m_view.setSize(pixels_size);

    target.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
    target.drawSprite(screen_sprite, "combineBloom", DrawType::Dynamic);
    target.drawAll();

    target.m_view = old_view;
    target.m_blend_factors = old_blend_factors;
}

Bloom2::Bloom2(int width, int height, TextureOptions options)
    : m_downsampled_pixels3(width / 4, height / 4, options),
      m_downsampled_pixels33(width / 4, height / 4, options),
      m_downsampler3(m_downsampled_pixels3),
      m_downsampler33(m_downsampled_pixels33)
{
    m_downsampler3.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_downsampler3.addShader("brightness", "../Resources/basicinstanced.vert", "../Resources/brightness.frag");
    m_downsampler33.addShader("gaussVert", "../Resources/basicinstanced.vert", "../Resources/gaussVert.frag");

    m_downsampler3.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
    m_downsampler33.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
}

void Bloom2::process(Texture &source, Renderer &target)
{

    if (!target.hasShader("combineBloom"))
    {
        target.addShader("combineBloom", "../Resources/basicinstanced.vert", "../Resources/combineBloom.frag");
    }
    if (!target.hasShader("combineBloomBetter"))
    {
        target.addShader("combineBloomBetter", "../Resources/basicinstanced.vert", "../Resources/combineLightBloom.frag");
    }

    auto old_view = target.m_view;
    auto old_factors = target.m_blend_factors;

    auto target_size = target.getTargetSize();
    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size / 2.f);
    screen_sprite.setScale(target_size / 2.f);
    m_downsampler3.m_view.setCenter(screen_sprite.getPosition().x, screen_sprite.getPosition().y);
    m_downsampler3.m_view.setSize(target_size.x, target_size.y);

    //! BRIGHTNESS PASS
    screen_sprite.setTexture(source);
    m_downsampler3.clear({0, 0, 0, 0});
    m_downsampler3.drawSprite(screen_sprite, "brightness", DrawType::Dynamic);
    m_downsampler3.drawAll();

    auto size = m_downsampled_pixels3.getSize();
    screen_sprite.setTexture(m_downsampled_pixels3.getTexture());
    screen_sprite.setPosition(size / 2.f);
    screen_sprite.setScale(size / 2.f);
    m_downsampler33.m_view.setCenter(size / 2.f);
    m_downsampler33.m_view.setSize(size);
    m_downsampler33.clear({0, 0, 0, 0});
    m_downsampler33.drawSprite(screen_sprite, "gaussVert", DrawType::Dynamic);
    m_downsampler33.drawAll();
    // writeTextureToFile("../", "testfile33.png", m_downsampled_pixels33);
    screen_sprite.setTexture(m_downsampled_pixels33.getTexture());
    m_downsampler3.m_view.setCenter(size / 2.f);
    m_downsampler3.m_view.setSize(size);
    screen_sprite.setPosition(size / 2);
    screen_sprite.setScale(size / 2);
    m_downsampler3.drawSprite(screen_sprite, "gaussHoriz", DrawType::Dynamic);
    m_downsampler3.clear({0, 0, 0, 0});
    m_downsampler3.drawAll();
    // writeTextureToFile("../", "testfile3.png", m_downsampled_pixels3);

    screen_sprite.setTexture(1, m_downsampled_pixels3.getTexture());
    screen_sprite.setTexture(0, source);
    auto pixels_size = m_downsampled_pixels3.getSize();
    screen_sprite.setPosition(pixels_size / 2.f);
    screen_sprite.setScale(pixels_size / 2.f);
    target.m_view.setCenter(pixels_size / 2.f);
    target.m_view.setSize(pixels_size);
    target.drawSprite(screen_sprite, "combineBloomBetter", DrawType::Dynamic);

    target.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha};
    target.drawAll();

    target.m_blend_factors = old_factors;
    target.m_view = old_view;
}

Bloom3::Bloom3(int width, int height, TextureOptions options)

{
    initMips(3, width, height, options);
}

void Bloom3::initMips(int n_levels, int width, int height, TextureOptions options)
{
    m_mips.clear();
    m_mips.reserve(n_levels);
    for (int i = 0; i < n_levels; ++i)
    {
        m_mips.emplace_back(width, height, options);
        m_mips.back().canvas.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
        m_mips.back().canvas_tmp.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
        width /= 2;
        height /= 2;
    }
}

void Bloom3::process(Texture &source, Renderer &target)
{

    if (!target.hasShader("combineBloom"))
    {
        target.addShader("combineBloom", "../Resources/basicinstanced.vert", "../Resources/combineBloom.frag");
    }
    if (!target.hasShader("combineLightBloom"))
    {
        target.addShader("combineLightBloom", "../Resources/basicinstanced.vert", "../Resources/combineLightBloom.frag");
    }

    auto old_view = target.m_view;
    auto old_factors = target.m_blend_factors;

    auto target_size = target.getTargetSize();
    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size / 2.f);
    screen_sprite.setScale(target_size / 2.f);
    auto size = target_size;

    //! BRIGHTNESS PASS
    screen_sprite.setTexture(source);
    auto &first_mip = m_mips.front();
    first_mip.canvas.clear({0, 0, 0, 0});
    first_mip.canvas.m_view.setCenter(screen_sprite.getPosition());
    first_mip.canvas.m_view.setSize(screen_sprite.getScale() * 2.);
    first_mip.canvas.drawSprite(screen_sprite, "brightness", DrawType::Dynamic);
    first_mip.canvas.drawAll();

    for (int pass = 0; pass < 1; ++pass)
    {
        // Sprite2 ss1(first_mip.pixels.getTexture());
        screen_sprite.setPosition(first_mip.pixels.getSize() / 2.f);
        screen_sprite.setScale(first_mip.pixels.getSize() / 2.f);
        first_mip.canvas_tmp.clear({0, 0, 0, 0});
        screen_sprite.setTexture(first_mip.pixels.getTexture());
        first_mip.canvas_tmp.m_view.setCenter(screen_sprite.getPosition());
        first_mip.canvas_tmp.m_view.setSize(screen_sprite.getScale() * 2.);
        first_mip.canvas_tmp.drawSprite(screen_sprite, "gaussVert", DrawType::Dynamic);
        first_mip.canvas_tmp.drawAll();

        // Sprite2 ss2(first_mip.pixels_tmp.getTexture());
        first_mip.canvas.clear({0, 0, 0, 0});
        screen_sprite.setTexture(first_mip.pixels_tmp.getTexture());
        first_mip.canvas.m_view.setCenter(screen_sprite.getPosition());
        first_mip.canvas.m_view.setSize(screen_sprite.getScale() * 2.);
        first_mip.canvas.drawSprite(screen_sprite, "gaussHoriz", DrawType::Dynamic);
        first_mip.canvas.drawAll();
    }

    size = target_size / 2;
    for (int mip_id = 1; mip_id < m_mips.size(); ++mip_id)
    {
        auto &prev_mip = m_mips.at(mip_id - 1);
        auto &mip = m_mips.at(mip_id);
        mip.canvas.m_view.setCenter(screen_sprite.getPosition());
        mip.canvas.m_view.setSize(target_size);

        screen_sprite.setTexture(prev_mip.pixels.getTexture());
        screen_sprite.setPosition(screen_sprite.getPosition());
        screen_sprite.setScale(size / 2.f);
        mip.canvas.m_view.setCenter(screen_sprite.getPosition());
        mip.canvas.m_view.setSize(screen_sprite.getScale() * 2.);
        mip.canvas.clear({0, 0, 0, 0});
        mip.canvas.drawSprite(screen_sprite, "gaussHoriz", DrawType::Dynamic);
        mip.canvas.drawAll();

        // auto& c1 = mip
        for (int pass = 0; pass < 3; ++pass)
        {
            mip.canvas_tmp.clear({0, 0, 0, 0});
            screen_sprite.setTexture(mip.pixels.getTexture());
            mip.canvas_tmp.m_view.setCenter(screen_sprite.getPosition());
            mip.canvas_tmp.m_view.setSize(screen_sprite.getScale() * 2.);
            mip.canvas_tmp.drawSprite(screen_sprite, "gaussVert", DrawType::Dynamic);
            mip.canvas_tmp.drawAll();

            mip.canvas.clear({0, 0, 0, 0});
            screen_sprite.setTexture(mip.pixels_tmp.getTexture());

            mip.canvas.drawSprite(screen_sprite, "gaussHoriz", DrawType::Dynamic);
            mip.canvas.drawAll();
        }

        size = size / 2;
    }

    // writeTextureToFile("../", "testfile3.png", m_downsampled_pixels3);

    Sprite2 ss;
    ss.m_color = {255, 255, 255, 255};
    target.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha};
    // for (auto &mip : m_mips)
    {
        auto &mip = m_mips.back();
        screen_sprite.setTexture(1, mip.pixels_tmp.getTexture());
        screen_sprite.setTexture(0, source); //! DO NOT CHANGE ORDER OF SETTEXTURES!!
        auto pixels_size = target_size;      // mip.pixels.getSize();
        screen_sprite.setPosition(pixels_size / 2.f);
        screen_sprite.setScale(pixels_size / 2.f);
        target.m_view.setCenter(pixels_size / 2.f);
        target.m_view.setSize(pixels_size);
        target.drawSprite(screen_sprite, "combineLightBloom", DrawType::Dynamic);

        target.drawAll();
    }

    target.m_blend_factors = old_factors;
    target.m_view = old_view;
}

SmoothLight::SmoothLight(int width, int height, TextureOptions options)
    : m_bloom_pass1(width, height, options),
      m_bloom_pass2(width, height, options),
      m_bloom_renderer1(m_bloom_pass1),
      m_bloom_renderer2(m_bloom_pass2),
      m_downsampled_pixels3(width / 4, height / 4, options),
      m_downsampled_pixels33(width / 4, height / 4, options),
      m_downsampler3(m_downsampled_pixels3),
      m_downsampler33(m_downsampled_pixels33)
{
    m_bloom_renderer1.addShader("gaussVert", "../Resources/basicinstanced.vert", "../Resources/gaussVert.frag");
    m_bloom_renderer2.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_bloom_renderer2.addShader("brightness", "../Resources/basicinstanced.vert", "../Resources/brightness.frag");
    m_downsampler3.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_downsampler3.addShader("brightness", "../Resources/basicinstanced.vert", "../Resources/brightness.frag");
    m_downsampler33.addShader("gaussVert", "../Resources/basicinstanced.vert", "../Resources/gaussVert.frag");

    m_downsampler3.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
    m_downsampler33.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
}

void SmoothLight::process(Texture &source, Renderer &target)
{
    target.clear({0, 0, 0, 0});

    if (!target.hasShader("combineLightBloom"))
    {
        target.addShader("combineLightBloom", "../Resources/basicinstanced.vert", "../Resources/combineLightBloom.frag");
    }

    auto old_view = target.m_view;
    auto old_blend_factors = target.m_blend_factors;

    auto target_size = target.getTargetSize();
    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size / 2.f);
    screen_sprite.setScale(target_size / 2.f);
    m_downsampler3.m_view.setCenter(screen_sprite.getPosition().x, screen_sprite.getPosition().y);
    m_downsampler3.m_view.setSize(target_size.x, target_size.y);

    //! BRIGHTNESS PASS
    screen_sprite.setTexture(source);
    m_downsampler3.clear({0, 0, 0, 0});
    m_downsampler3.drawSprite(screen_sprite, "brightness", DrawType::Dynamic);
    m_downsampler3.drawAll();

    auto size = m_downsampled_pixels3.getSize();
    screen_sprite.setTexture(m_downsampled_pixels3.getTexture());
    screen_sprite.setPosition(size / 2.f);
    screen_sprite.setScale(size / 2.f);
    m_downsampler33.m_view.setCenter(size / 2.f);
    m_downsampler33.m_view.setSize(size);
    m_downsampler33.clear({0, 0, 0, 0});
    m_downsampler33.drawSprite(screen_sprite, "gaussVert", DrawType::Dynamic);
    m_downsampler33.drawAll();
    // writeTextureToFile("../", "testfile33.png", m_downsampled_pixels33);
    screen_sprite.setTexture(m_downsampled_pixels33.getTexture());
    m_downsampler3.m_view.setCenter(size / 2.f);
    m_downsampler3.m_view.setSize(size);
    screen_sprite.setPosition(size / 2);
    screen_sprite.setScale(size / 2);
    m_downsampler3.drawSprite(screen_sprite, "gaussHoriz", DrawType::Dynamic);
    m_downsampler3.clear({0, 0, 0, 0});
    m_downsampler3.drawAll();
    // writeTextureToFile("../", "testfile3.png", m_downsampled_pixels3);

    screen_sprite.setTexture(0, source);
    screen_sprite.setTexture(1, m_downsampled_pixels3.getTexture());
    auto pixels_size = m_downsampled_pixels3.getSize();
    screen_sprite.setPosition(pixels_size / 2.f);
    screen_sprite.setScale(pixels_size / 2.f);
    target.m_view.setCenter(pixels_size / 2.f);
    target.m_view.setSize(pixels_size);
    target.drawSprite(screen_sprite, "combineLightBloom", DrawType::Dynamic);

    target.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha};
    target.drawAll();

    target.m_view = old_view;
    target.m_blend_factors = old_blend_factors;
}

LightCombine::LightCombine(int width, int height)
    : m_multiply_texture(width, height), m_multiply_canvas(m_multiply_texture)
{
}
void LightCombine::process(Texture &source, Renderer &target)
{

    if (!target.hasShader("combineLight"))
    {
        target.addShader("combineLight", "../Resources/basicinstanced.vert", "../Resources/combineLight.frag");
    }

    auto old_view = target.m_view;
    auto old_blend_factors = target.m_blend_factors;

    auto target_size = target.getTargetSize();

    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size / 2.f);
    screen_sprite.setScale(target_size / 2.f);

    screen_sprite.setTexture(0, source);
    target.m_view.setCenter(target_size / 2.f);
    target.m_view.setSize(target_size);
    target.drawSprite(screen_sprite, "combineLight", DrawType::Dynamic);

    target.m_blend_factors = {bf::Zero, bf::SrcColor, bf::One, bf::Zero};
    target.drawAll();

    target.m_blend_factors = old_blend_factors;
    target.m_view = old_view;
}

BloomSmoke::BloomSmoke(int width, int height)
    : m_bloom_pass1(width, height),
      m_bloom_pass2(width, height),
      m_bloom_renderer1(m_bloom_pass1),
      m_bloom_renderer2(m_bloom_pass2),
      m_downsampled_pixels3(width / 8, height / 8),
      m_downsampled_pixels33(width / 8, height / 8),
      m_downsampler3(m_downsampled_pixels3),
      m_downsampler33(m_downsampled_pixels33)
{

    m_downsampler3.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_downsampler3.addShader("brightness", "../Resources/basicinstanced.vert", "../Resources/brightness.frag");
    m_downsampler33.addShader("gaussVert", "../Resources/basicinstanced.vert", "../Resources/gaussVert.frag");

    m_downsampler3.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
    m_downsampler33.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
}

void BloomSmoke::process(Texture &source, Renderer &target)
{

    if (!target.hasShader("combineSmoke"))
    {
        target.addShader("combineSmoke", "../Resources/basicinstanced.vert", "../Resources/combineSmoke.frag");
    }

    auto old_view = target.m_view;
    auto old_blend_factors = target.m_blend_factors;

    auto target_size = target.getTargetSize();
    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size.x / 2.f, target_size.y / 2.f);
    screen_sprite.setScale(target_size.x / 2.f, target_size.y / 2.f);

    m_downsampler3.m_view.setCenter(screen_sprite.getPosition().x, screen_sprite.getPosition().y);
    m_downsampler3.m_view.setSize(target_size.x, target_size.y);

    //! BRIGHTNESS PASS
    screen_sprite.setTexture(source);
    m_downsampler3.clear({0, 0, 0, 0});
    m_downsampler3.drawSprite(screen_sprite, "brightness", DrawType::Dynamic);
    m_downsampler3.drawAll();

    auto size = m_downsampled_pixels3.getSize();
    screen_sprite.setTexture(m_downsampled_pixels3.getTexture());
    screen_sprite.setPosition(size / 2.f);
    screen_sprite.setScale(size / 2.f);
    m_downsampler33.m_view.setCenter(size / 2.f);
    m_downsampler33.m_view.setSize(size);
    m_downsampler33.clear({0, 0, 0, 0});
    m_downsampler33.drawSprite(screen_sprite, "gaussVert", DrawType::Dynamic);
    m_downsampler33.drawAll();
    // writeTextureToFile("../", "testfile33.png", m_downsampled_pixels33);
    screen_sprite.setTexture(m_downsampled_pixels33.getTexture());
    m_downsampler3.m_view.setCenter(size / 2.f);
    m_downsampler3.m_view.setSize(size);
    screen_sprite.setPosition(size / 2);
    screen_sprite.setScale(size / 2);
    m_downsampler3.drawSprite(screen_sprite, "gaussHoriz", DrawType::Dynamic);
    m_downsampler3.clear({0, 0, 0, 0});
    m_downsampler3.drawAll();
    // writeTextureToFile("../", "testfile3.png", m_downsampled_pixels3);

    screen_sprite.setTexture(0, source);
    screen_sprite.setTexture(1, m_downsampled_pixels3.getTexture());
    auto pixels_size = m_downsampled_pixels3.getSize();
    screen_sprite.setPosition(pixels_size / 2.f);
    screen_sprite.setScale(pixels_size / 2.f);
    target.m_view.setCenter(pixels_size / 2.f);
    target.m_view.setSize(pixels_size);
    target.drawSprite(screen_sprite, "combineSmoke", DrawType::Dynamic);

    target.m_blend_factors = {bf::SrcAlpha, bf::OneMinusSrcAlpha};
    target.drawAll();

    target.m_blend_factors = old_blend_factors;
    target.m_view = old_view;
}

EdgeDetect::EdgeDetect(int width, int height)
    : m_vert_pass(width, height),
      m_horiz_pass(width, height),
      m_vert_canvas(m_vert_pass),
      m_horiz_canvas(m_horiz_pass)
{
    m_vert_canvas.addShader("edgeDetectVert", "../Resources/basicinstanced.vert", "../Resources/edgeDetectVert.frag");
    m_horiz_canvas.addShader("edgeDetectHoriz", "../Resources/basicinstanced.vert", "../Resources/edgeDetectHoriz.frag");

    m_vert_canvas.m_blend_factors = {bf::SrcAlpha, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
    m_horiz_canvas.m_blend_factors = {bf::SrcAlpha, bf::OneMinusSrcAlpha, bf::One, bf::Zero};
}
void EdgeDetect::process(Texture &source, Renderer &target)
{
    if (!target.hasShader("combineEdges"))
    {
        target.addShader("combineEdges", "../Resources/basicinstanced.vert", "../Resources/combineEdges.frag");
    }

    auto old_view = target.m_view;
    auto old_blend_factors = target.m_blend_factors;

    auto target_size = target.getTargetSize();
    m_vert_canvas.m_view.setCenter(target_size / 2.f);
    m_vert_canvas.m_view.setSize(target_size);
    m_horiz_canvas.m_view.setCenter(target_size / 2.f);
    m_horiz_canvas.m_view.setSize(target_size);

    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size / 2.f);
    screen_sprite.setScale(target_size / 2.f);

    //! BRIGHTNESS PASS
    screen_sprite.setTexture(source);
    m_vert_canvas.clear({0, 0, 0, 0});
    m_vert_canvas.drawSprite(screen_sprite, "edgeDetectVert", DrawType::Dynamic);
    m_vert_canvas.drawAll();

    m_horiz_canvas.clear({0, 0, 0, 0});
    screen_sprite.setTexture(m_vert_pass.getTexture());
    m_horiz_canvas.drawSprite(screen_sprite, "edgeDetectHoriz", DrawType::Dynamic);
    m_horiz_canvas.drawAll();

    screen_sprite.setTexture(0, source);
    screen_sprite.setTexture(1, m_horiz_pass.getTexture());
    target.m_view.setCenter(target_size / 2.f);
    target.m_view.setSize(target_size);
    target.drawSprite(screen_sprite, "combineEdges", DrawType::Dynamic);

    target.m_blend_factors = {bf::One, bf::OneMinusSrcAlpha};
    target.drawAll();

    target.m_view = old_view;
    target.m_blend_factors = old_blend_factors;
}
