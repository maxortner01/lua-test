#pragma once

#include "Font.hpp"
#include "Drawable.hpp"

#include "../Util/Transform.hpp"

namespace S2D::Graphics
{

    struct Text : Drawable
    {
        Text(Font* font, const std::string& text = "");
        ~Text() = default;

        constexpr uint32_t getPixelHeight() const { return _pixel_height; }
        void setPixelHeight(uint32_t pixel_height);

        void setText(const std::string& text);
        Math::Vec2f getSize() const;

        void draw(Graphics::Surface* surface) const override;

    private:
        Font* _font;
        std::string _text;
        uint32_t _pixel_height;

        Math::Transform _transform;
    };

}