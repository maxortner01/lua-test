#include <Simple2D/Graphics/Font.hpp>
#include <Simple2D/Graphics/Texture.hpp>

#include <Simple2D/Log/Log.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H 
#include FT_BITMAP_H

namespace S2D::Graphics
{
    inline static FT_Library ft = nullptr;

    bool check_and_init()
    {
        if (ft) return true;

        if (FT_Init_FreeType(&ft))
        {
            Log::Logger::instance("engine")->error("Error initializing Freetype");
            return false;
        }

        return true;
    }

    Font::Font() :
        _good(false),
        _face(nullptr)
    {

    }

    Font::Font(Font&& font) :
        _good(font._good),
        _face(font._face)
    {
        font._good = false;
        font._face = nullptr;
    }

    Font::~Font()
    {
        if (_face)
            FT_Done_Face(static_cast<FT_Face>(_face));
        _face = nullptr;
    }

    Font::operator bool() const
    {
        return _good;
    }

    bool Font::fromFile(const std::filesystem::path& filepath)
    {
        S2D_ASSERT(check_and_init(), "Error initializing freetype");
        S2D_ASSERT(!_face, "Face already loaded");

        FT_Face face;
        if (FT_New_Face(ft, filepath.c_str(), 0, &face))
        {
            Log::Logger::instance("engine")->error("Error loading font face \"{}\"", filepath.filename().c_str());
            return false;
        }

        _face = face;
        // For some reason I have to do this, otherwise the first character doesn't work
        getCharacter('C', 16);

        return true;
    }

    const Character& Font::getCharacter(char c, uint32_t pixel_height)
    {
        S2D_ASSERT(_face, "Face not loaded");

        if (!glyphs.count(pixel_height)) 
            glyphs.insert(std::pair(pixel_height, CharacterMap()));

        auto& map = glyphs.at(pixel_height);
        if (!map.count(c))
        {
            Character character;

            bool mono = true;
            
            const auto face = static_cast<FT_Face>(_face);
            FT_Set_Pixel_Sizes(face, 0, pixel_height); 
            S2D_ASSERT_ARGS(!FT_Load_Char(face, c, (mono ? FT_LOAD_RENDER | FT_LOAD_TARGET_MONO : FT_LOAD_RENDER)), "Error loading glyph %c", c);

            character.size    = Math::Vec2u(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            character.bearing = Math::Vec2i(face->glyph->bitmap_left,  face->glyph->bitmap_top );
            character.texture = std::make_shared<Texture>();
            character.advance = face->glyph->advance.x;

            const auto convert = [&](const uint8_t* source, uint8_t* destination)
            {
                for (uint32_t y = 0; y < character.size.y; y++)
                    for (uint32_t byte_index = 0; byte_index < face->glyph->bitmap.pitch; byte_index++)
                    {
                        const auto byte_value = source[y * face->glyph->bitmap.pitch + byte_index];
                        const auto num_bits_done = byte_index * 8;
                        const auto rowstart = y * character.size.x + num_bits_done;

                        const auto min = [](auto a, auto b) { return (a < b ? a : b); };

                        for (uint32_t bit_index = 0; bit_index < min(8, character.size.x - num_bits_done); bit_index++)
                        {
                            const auto bit = byte_value & (1 << (7 - bit_index));
                            if (bit) destination[rowstart + bit_index] = 255;
                        }
                    }
            };

            uint8_t* data = face->glyph->bitmap.buffer;
            if (mono)
            {
                data = (uint8_t*)std::calloc(sizeof(uint8_t), character.size.x * character.size.y);
                convert(face->glyph->bitmap.buffer, data);
            }

            S2D_ASSERT(character.texture->fromMemory(
                character.size, 
                data, 
                Texture::Format::Red), "Failure to create glyph texture");

            if (mono)
                std::free(data);

            map.insert(std::pair(c, character));
        }

        return map.at(c);
    }
}
