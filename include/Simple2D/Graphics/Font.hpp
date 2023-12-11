#pragma once

#include "../Def.hpp"

#include "../Util/Vector.hpp"

namespace S2D::Graphics
{
    struct Texture;

    struct Character
    {
        std::shared_ptr<Texture> texture;
        Math::Vec2u size;
        Math::Vec2i bearing;
        std::size_t advance;
    };

    struct Font
    {
        using Handle = void*;

        Font(const Font&) = delete;

        Font();
        Font(Font&&);
        ~Font();

        operator bool() const;

        bool fromFile(const std::filesystem::path& filepath);

        const Character& getCharacter(char c, uint32_t pixel_height);

    private:
        bool _good;
        Handle _face;

        using CharacterMap = std::unordered_map<char, Character>;
        std::unordered_map<uint32_t, CharacterMap> glyphs;
    };

}