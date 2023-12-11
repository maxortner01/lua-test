#include <Simple2D/Graphics/Text.hpp>
#include <Simple2D/Graphics/VertexArray.hpp>

namespace S2D::Graphics
{

Text::Text(Font* font, const std::string& text) : 
    _font(font),
    _pixel_height(32)
{
    setText(text);
}

void Text::setPixelHeight(uint32_t pixel_height)
{
    _pixel_height = pixel_height;
}

void Text::setText(const std::string& text)
{
    _text = text;

    // When texture atlas is used, generate vertices here
}

Math::Vec2f Text::getSize() const
{
    Math::Vec2f size;
    if (!_text.size()) return size;

    std::vector<Math::Vec2f> points;
    points.reserve(4 * _text.size());

    Math::Vec2f pos;
    for (const auto& c : _text)
    {
        const auto& glyph = _font->getCharacter(c, _pixel_height);

        const auto bottom_left = Math::Vec2f(
            pos.x + glyph.bearing.x / (float)_pixel_height * _transform.getScale().x,
            pos.y - (glyph.size.y - glyph.bearing.y) / (float)_pixel_height * _transform.getScale().y * -1.f
        );

        const auto glyph_size = Math::Vec2f(
            glyph.size.x / (float)_pixel_height * _transform.getScale().x, 
            glyph.size.y / (float)_pixel_height * _transform.getScale().y * -1.f
        );

        points.push_back(bottom_left);
        points.push_back(bottom_left + glyph_size);
        points.push_back(Math::Vec2f(bottom_left.x, bottom_left.y + glyph_size.y));
        points.push_back(Math::Vec2f(bottom_left.x + glyph_size.x, bottom_left.y));
    }

    float top{0.f}, bottom{0.f}, left{0.f}, right{0.f};
    for (const auto& p : points)
    {
        if (p.x < left)   left = p.x;
        if (p.x > right)  right = p.x;
        if (p.y < bottom) bottom = p.y;
        if (p.y > top)    top = p.y;
    }

    return Math::Vec2f(right - left, top - bottom);
}

void Text::draw(Graphics::Surface* surface) const
{
    Graphics::VertexArray vao; // Generate VAO and position
}


}