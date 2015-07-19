/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus's Graphical User Interface
// Copyright (C) 2012-2015 Bruno Van de Velde (vdv_b@tgui.eu)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <TGUI/Loading/Serializer.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    std::map<ObjectConverter::Type, Serializer::SerializeFunc> Serializer::m_serializers =
        {
            {ObjectConverter::Type::Font, serializeFont},
            {ObjectConverter::Type::Color, serializeColor},
            {ObjectConverter::Type::Borders, serializeBorders},
            {ObjectConverter::Type::Texture, serializeTexture}
        };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string Serializer::serialize(const ObjectConverter& object)
    {
        return m_serializers[object.getType()](object);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Serializer::setFunction(ObjectConverter::Type type, const SerializeFunc& serializer)
    {
        m_serializers[type] = serializer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string serializeFont(const ObjectConverter& /*value*/)
    {
        return "FONT_PLACEHOLDER";    /// TODO: Support deserializing fonts
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string serializeColor(const ObjectConverter& value)
    {
        sf::Color color = value.getColor();
        if (color.a < 255)
            return "rgba(" + std::to_string(color.r) + ", " + std::to_string(color.g) + ", " + std::to_string(color.b) + ", " + std::to_string(color.a) + ")";
        else
            return "rgb(" + std::to_string(color.r) + ", " + std::to_string(color.g) + ", " + std::to_string(color.b) + ")";
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string serializeBorders(const ObjectConverter& value)
    {
        Borders borders = value.getBorders();
        return "(" + std::to_string(static_cast<unsigned int>(borders.left)) + ", " + std::to_string(static_cast<unsigned int>(borders.top))
             + ", " + std::to_string(static_cast<unsigned int>(borders.right)) + ", " + std::to_string(static_cast<unsigned int>(borders.bottom)) + ")";
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string serializeTexture(const ObjectConverter& value)
    {
        Texture texture = value.getTexture();
        std::string result = "\"" + texture.getId() + "\"";

        if (texture.getData()->rect != sf::IntRect{})
        {
            result += " Part(" + tgui::to_string(texture.getData()->rect.left) + ", " + tgui::to_string(texture.getData()->rect.top)
                        + ", " + tgui::to_string(texture.getData()->rect.width) + ", " + tgui::to_string(texture.getData()->rect.height) + ")";
        }
        if (texture.getMiddleRect() != sf::IntRect{0, 0, static_cast<int>(texture.getData()->texture.getSize().x), static_cast<int>(texture.getData()->texture.getSize().y)})
        {
            result += " Middle(" + tgui::to_string(texture.getMiddleRect().left) + ", " + tgui::to_string(texture.getMiddleRect().top)
                          + ", " + tgui::to_string(texture.getMiddleRect().width) + ", " + tgui::to_string(texture.getMiddleRect().height) + ")";
        }
        if (texture.getData()->texture.isRepeated())
            result += "Repeat";

        return result;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /// TODO: Add serializeString to add quotes around string and to replace stuff like \n in the string
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
