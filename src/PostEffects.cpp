#include "PostEffects.h"

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
}

void Bloom::process(Texture &source, Renderer &target)
{
    if (!target.hasShader("combineBloom"))
    {
        target.addShader("combineBloom", "../Resources/basicinstanced.vert", "../Resources/combineBloom.frag");
    }

    auto old_view = target.m_view;

    auto target_size = target.getTargetSize();
    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size / 2.f);
    screen_sprite.setScale(target_size / 2.f);

    m_bloom_renderer1.m_view.setCenter(screen_sprite.getPosition());
    m_bloom_renderer1.m_view.setSize(target_size);

    m_bloom_renderer2.m_view.setCenter(screen_sprite.getPosition());
    m_bloom_renderer2.m_view.setSize(target_size);

    m_bloom_renderer1.clear({0, 0, 0, 1});
    m_bloom_renderer2.clear({0, 0, 0, 1});

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,);
    m_bloom_renderer2.drawSprite(screen_sprite, "brightness", GL_DYNAMIC_DRAW);
    m_bloom_renderer2.drawAll();

    for (int pass = 0; pass < 4; ++pass)
    {
        // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
        m_bloom_renderer1.clear({0, 0, 0, 1});
        screen_sprite.setTexture(m_bloom_pass2.getTexture());
        m_bloom_renderer1.drawSprite(screen_sprite, "gaussVert", GL_DYNAMIC_DRAW);
        m_bloom_renderer1.drawAll();

        // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
        m_bloom_renderer2.clear({0, 0, 0, 1});
        screen_sprite.setTexture(m_bloom_pass1.getTexture());
        m_bloom_renderer2.drawSprite(screen_sprite, "gaussHoriz", GL_DYNAMIC_DRAW);
        m_bloom_renderer2.drawAll();
    }

    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    screen_sprite.setTexture(0, source);
    screen_sprite.setTexture(1, m_bloom_pass2.getTexture());
    auto pixels_size = m_bloom_pass2.getSize();
    screen_sprite.setPosition(pixels_size / 2.f);
    screen_sprite.setScale(pixels_size / 2.f);
    target.m_view.setCenter(pixels_size / 2.f);
    target.m_view.setSize(pixels_size);
    target.drawSprite(screen_sprite, "combineBloom", GL_DYNAMIC_DRAW);
    target.drawAll();

    target.m_view = old_view;
}

Bloom2::Bloom2(int width, int height)
    : m_bloom_pass1(width, height),
      m_bloom_pass2(width, height),
      m_bloom_renderer1(m_bloom_pass1),
      m_bloom_renderer2(m_bloom_pass2),
      m_downsampled_pixels3(width / 4, height / 4),
      m_downsampled_pixels33(width / 4, height / 4),
      m_downsampler3(m_downsampled_pixels3),
      m_downsampler33(m_downsampled_pixels33)
{
    m_bloom_renderer1.addShader("gaussVert", "../Resources/basicinstanced.vert", "../Resources/gaussVert.frag");
    m_bloom_renderer2.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_bloom_renderer2.addShader("brightness", "../Resources/basicinstanced.vert", "../Resources/brightness.frag");
    m_downsampler3.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_downsampler3.addShader("brightness", "../Resources/basicinstanced.vert", "../Resources/brightness.frag");
    m_downsampler33.addShader("gaussVert", "../Resources/basicinstanced.vert", "../Resources/gaussVert.frag");
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
    m_bloom_renderer1.addShader("gaussVert", "../Resources/basicinstanced.vert", "../Resources/gaussVert.frag");
    m_bloom_renderer2.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_bloom_renderer2.addShader("brightness", "../Resources/basicinstanced.vert", "../Resources/brightness.frag");
    m_downsampler3.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_downsampler3.addShader("brightness", "../Resources/basicinstanced.vert", "../Resources/brightness.frag");
    m_downsampler33.addShader("gaussVert", "../Resources/basicinstanced.vert", "../Resources/gaussVert.frag");
}

void Bloom2::process(Texture &source, Renderer &target)
{
    if (!target.hasShader("combineBloom"))
    {
        target.addShader("combineBloom", "../Resources/basicinstanced.vert", "../Resources/combineBloom.frag");
    }

    auto old_view = target.m_view;

    auto target_size = target.getTargetSize();
    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size / 2.f);
    screen_sprite.setScale(target_size / 2.f);
    m_downsampler3.m_view.setCenter(screen_sprite.getPosition().x, screen_sprite.getPosition().y);
    m_downsampler3.m_view.setSize(target_size.x, target_size.y);

    //! BRIGHTNESS PASS

    screen_sprite.setTexture(source);
    m_downsampler3.clear({0, 0, 0, 0});
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    m_downsampler3.drawSprite(screen_sprite, "brightness", GL_DYNAMIC_DRAW);
    m_downsampler3.drawAll();

    auto size = m_downsampled_pixels3.getSize();
    screen_sprite.setTexture(m_downsampled_pixels3.getTexture());
    screen_sprite.setPosition(size / 2.f);
    screen_sprite.setScale(size / 2.f);
    m_downsampler33.m_view.setCenter(size / 2.f);
    m_downsampler33.m_view.setSize(size);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    m_downsampler33.clear({0, 0, 0, 0});
    m_downsampler33.drawSprite(screen_sprite, "gaussVert", GL_DYNAMIC_DRAW);
    m_downsampler33.drawAll();
    // writeTextureToFile("../", "testfile33.png", m_downsampled_pixels33);
    screen_sprite.setTexture(m_downsampled_pixels33.getTexture());
    m_downsampler3.m_view.setCenter(size / 2.f);
    m_downsampler3.m_view.setSize(size);
    screen_sprite.setPosition(size / 2);
    screen_sprite.setScale(size / 2);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    m_downsampler3.drawSprite(screen_sprite, "gaussHoriz", GL_DYNAMIC_DRAW);
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
    target.drawSprite(screen_sprite, "combineBloomBetter", GL_DYNAMIC_DRAW);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    target.drawAll();

    target.m_view = old_view;
}

void BloomSmoke::process(Texture &source, Renderer &target)
{
    if (!target.hasShader("combineSmoke"))
    {
        target.addShader("combineSmoke", "../Resources/basicinstanced.vert", "../Resources/combineSmoke.frag");
    }

    auto old_view = target.m_view;

    auto target_size = target.getTargetSize();
    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size.x / 2.f, target_size.y / 2.f);
    screen_sprite.setScale(target_size.x / 2.f, target_size.y / 2.f);

    m_downsampler3.m_view.setCenter(screen_sprite.getPosition().x, screen_sprite.getPosition().y);
    m_downsampler3.m_view.setSize(target_size.x, target_size.y);

    //! BRIGHTNESS PASS
    screen_sprite.setTexture(source);
    m_downsampler3.clear({0, 0, 0, 0});
    m_downsampler3.drawSprite(screen_sprite, "brightness", GL_DYNAMIC_DRAW);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    m_downsampler3.drawAll();

    auto size = m_downsampled_pixels3.getSize();
    screen_sprite.setTexture(m_downsampled_pixels3.getTexture());
    screen_sprite.setPosition(size / 2.f);
    screen_sprite.setScale(size / 2.f);
    m_downsampler33.m_view.setCenter(size / 2.f);
    m_downsampler33.m_view.setSize(size);
    m_downsampler33.clear({0, 0, 0, 0});
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    m_downsampler33.drawSprite(screen_sprite, "gaussVert", GL_DYNAMIC_DRAW);
    m_downsampler33.drawAll();
    // writeTextureToFile("../", "testfile33.png", m_downsampled_pixels33);
    screen_sprite.setTexture(m_downsampled_pixels33.getTexture());
    m_downsampler3.m_view.setCenter(size / 2.f);
    m_downsampler3.m_view.setSize(size);
    screen_sprite.setPosition(size / 2);
    screen_sprite.setScale(size / 2);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    m_downsampler3.drawSprite(screen_sprite, "gaussHoriz", GL_DYNAMIC_DRAW);
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
    target.drawSprite(screen_sprite, "combineSmoke", GL_DYNAMIC_DRAW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    target.drawAll();

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
}
void EdgeDetect::process(Texture &source, Renderer &target)
{

    if(!target.hasShader("combineEdges"))
    {
        target.addShader("combineEdges", "../Resources/basicinstanced.vert", "../Resources/combineEdges.frag");
    }

    auto old_view = target.m_view;

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
    m_vert_canvas.drawSprite(screen_sprite, "edgeDetectVert", GL_DYNAMIC_DRAW);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    m_vert_canvas.drawAll();

    m_horiz_canvas.clear({0, 0, 0, 0});
    screen_sprite.setTexture(m_vert_pass.getTexture());
    m_horiz_canvas.drawSprite(screen_sprite, "edgeDetectHoriz", GL_DYNAMIC_DRAW);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    m_horiz_canvas.drawAll();

    screen_sprite.setTexture(0, source);
    screen_sprite.setTexture(1, m_horiz_pass.getTexture());
    target.m_view.setCenter(target_size / 2.f);
    target.m_view.setSize(target_size);
    target.drawSprite(screen_sprite, "combineEdges", GL_DYNAMIC_DRAW);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    target.drawAll();

    target.m_view = old_view;
}

LightCombine::LightCombine(int width, int height)
: m_multiply_texture(width, height), m_multiply_canvas(m_multiply_texture)
{
}
void LightCombine::process(Texture &source, Renderer &target)
{

    if(!target.hasShader("combineLight"))
    {
        target.addShader("combineLight", "../Resources/basicinstanced.vert", "../Resources/combineLight.frag");
    }

    auto old_view = target.m_view;

    auto target_size = target.getTargetSize();

    Sprite2 screen_sprite(source);
    screen_sprite.setPosition(target_size / 2.f);
    screen_sprite.setScale(target_size / 2.f);

    screen_sprite.setTexture(0, source);
    target.m_view.setCenter(target_size / 2.f);
    target.m_view.setSize(target_size);
    target.drawSprite(screen_sprite, "combineLight", GL_DYNAMIC_DRAW);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_SRC_COLOR, GL_ZERO, GL_ONE);
    target.drawAll();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    target.m_view = old_view;
}
