/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus's Graphical User Interface
// Copyright (C) 2012 Bruno Van de Velde (VDV_B@hotmail.com)
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


#include <TGUI/TGUI.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ComboBox::ComboBox() :
    m_ShowList          (false),
    m_MouseOnListbox    (false),
    m_NrOfItemsToDisplay(1),
    m_TextureNormal     (NULL),
    m_TextureHover      (NULL),
    m_LoadedPathname    ("")
    {
        m_ObjectType = comboBox;

        m_Listbox.m_Size.x = 50;
        m_Listbox.m_ItemHeight = 24;
        m_Listbox.m_BackgroundColor         = sf::Color(255, 255, 255);
        m_Listbox.m_TextColor               = sf::Color(  0,   0,   0);
        m_Listbox.m_SelectedBackgroundColor = sf::Color( 50, 100, 200);
        m_Listbox.m_SelectedTextColor       = sf::Color(255, 255, 255);
        m_Listbox.m_BorderColor             = sf::Color(  0,   0,   0);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ComboBox::ComboBox(const ComboBox& copy) :
    OBJECT              (copy),
    m_ShowList          (copy.m_ShowList),
    m_MouseOnListbox    (copy.m_MouseOnListbox),
    m_NrOfItemsToDisplay(copy.m_NrOfItemsToDisplay),
    m_Listbox           (copy.m_Listbox),
    m_LoadedPathname    (copy.m_LoadedPathname)
    {
        if (TGUI_TextureManager.copyTexture(copy.m_TextureNormal, m_TextureNormal)) m_SpriteNormal.setTexture(*m_TextureNormal);
        if (TGUI_TextureManager.copyTexture(copy.m_TextureHover, m_TextureHover))   m_SpriteHover.setTexture(*m_TextureHover);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ComboBox::~ComboBox()
    {
        if (m_TextureNormal != NULL)     TGUI_TextureManager.removeTexture(m_TextureNormal);
        if (m_TextureHover != NULL) TGUI_TextureManager.removeTexture(m_TextureHover);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ComboBox& ComboBox::operator= (const ComboBox& right)
    {
        if (this != &right)
        {
            ComboBox temp(right);
            this->OBJECT::operator=(right);

            std::swap(m_ShowList,           temp.m_ShowList);
            std::swap(m_MouseOnListbox,     temp.m_MouseOnListbox);
            std::swap(m_NrOfItemsToDisplay, temp.m_NrOfItemsToDisplay);
            std::swap(m_Listbox,            temp.m_Listbox);
            std::swap(m_TextureNormal,      temp.m_TextureNormal);
            std::swap(m_TextureHover,       temp.m_TextureHover);
            std::swap(m_SpriteNormal,       temp.m_SpriteNormal);
            std::swap(m_SpriteHover,        temp.m_SpriteHover);
            std::swap(m_LoadedPathname,     temp.m_LoadedPathname);
        }

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool ComboBox::load(const std::string pathname, unsigned int width, unsigned int nrOfItemsInList, const std::string scrollbarPathname)
    {
        // When everything is loaded successfully, this will become true.
        m_Loaded = false;

        // Make sure that the pathname isn't empty
        if (pathname.empty())
            return false;

        // Store the pathname
        m_LoadedPathname = pathname;

        // When the pathname does not end with a "/" then we will add it
        if (m_LoadedPathname.at(m_LoadedPathname.length()-1) != '/')
            m_LoadedPathname.push_back('/');

        // Open the info file
        InfoFileParser infoFile;
        if (infoFile.openFile(m_LoadedPathname + "info.txt") == false)
        {
            TGUI_OUTPUT((((std::string("TGUI: Failed to open ")).append(m_LoadedPathname)).append("info.txt")).c_str());
            return false;
        }

        std::string property;
        std::string value;

        // Set some default settings
        std::string imageExtension = "png";

        // Read untill the end of the file
        while (infoFile.readProperty(property, value))
        {
            // Check what the property is
            if (property.compare("phases") == 0)
            {
                // Get and store the different phases
                extractPhases(value);
            }
        }

        // Close the info file
        infoFile.closeFile();

        // If the button was loaded before then remove the old textures first
        if (m_TextureNormal != NULL)     TGUI_TextureManager.removeTexture(m_TextureNormal);
        if (m_TextureHover != NULL) TGUI_TextureManager.removeTexture(m_TextureHover);

        // load the required texture
        if (TGUI_TextureManager.getTexture(m_LoadedPathname + "Normal." + imageExtension, m_TextureNormal))
            m_SpriteNormal.setTexture(*m_TextureNormal, true);
        else
            return false;

        // load the optional texture
        if (m_ObjectPhase & objectPhase::hover)
        {
            if (TGUI_TextureManager.getTexture(m_LoadedPathname + "Hover." + imageExtension, m_TextureHover))
                m_SpriteHover.setTexture(*m_TextureHover, true);
            else
                return false;
        }


        // Remove all items (in case this is the second time that the load function was called)
        m_Listbox.removeAllItems();

        // There is a minimum width
        if (width < (50 + (m_LeftBorder + m_RightBorder + m_TextureNormal->getSize().x) * getScale().x))
            width = static_cast<unsigned int>(50 + (m_LeftBorder + m_RightBorder + m_TextureNormal->getSize().x) * getScale().x);

        // At least one item must be shown
        if (nrOfItemsInList < 1)
            nrOfItemsInList = 1;

        // Remember the scrollbar pathname
        m_LoadedScrollbarPathname = scrollbarPathname;

        // Make the changes
        m_NrOfItemsToDisplay = nrOfItemsInList;
        m_Listbox.load(width,
                       static_cast<unsigned int>((m_TextureNormal->getSize().y + m_BottomBorder) * getScale().y),
                       scrollbarPathname,
                       static_cast<unsigned int>((m_TextureNormal->getSize().y + m_TopBorder + m_BottomBorder) * getScale().y));

        // Check if a scrollbar should be loaded
        if (scrollbarPathname.empty() == false)
        {
            // Try to load the scrollbar
            if (m_Listbox.setScrollbar(scrollbarPathname))
            {
                // The scrollbar was loaded successfully
                m_Loaded = true;
                return true;
            }
            else // Loading the scrollbar failed
                return false;
        }
        else // No scrollbar is needed
        {
            m_Loaded = true;
            return true;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::setSize(float width, float height)
    {
        // Don't set the scale when loading failed
        if (m_Loaded == false)
            return;

        // A negative size is not allowed for this object
        if (width  < 0) width  = -width;
        if (height < 0) height = -height;

        // Change the scale factors
        setScale(width / m_Listbox.getSize().x, height / m_TextureNormal->getSize().y);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Vector2u ComboBox::getSize() const
    {
        if (m_Loaded)
            return Vector2u(m_Listbox.getSize().x, m_TextureNormal->getSize().y);
        else
            return Vector2u(0, 0);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Vector2f ComboBox::getScaledSize() const
    {
        if (m_Loaded)
            return Vector2f(m_Listbox.getSize().x * getScale().x, m_TextureNormal->getSize().y * getScale().y);
        else
            return Vector2f(0, 0);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string ComboBox::getLoadedPathname()
    {
        return m_LoadedPathname;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string ComboBox::getLoadedScrollbarPathname()
    {
        return m_LoadedScrollbarPathname;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::setWidth(unsigned int width)
    {
        // Check if the listbox was already loaded
        if (m_Loaded)
        {
            // There is a minimum width
            if (width < (50 + (m_LeftBorder + m_RightBorder + m_TextureNormal->getSize().x) * getScale().x))
                width = static_cast<unsigned int>(50 + (m_LeftBorder + m_RightBorder + m_TextureNormal->getSize().x) * getScale().x);
        }
        else // The combo box wasn't loaded yet
        {
            // There is a minimum width
            if (width < (50 + (m_LeftBorder + m_RightBorder) * getScale().x))
                width = static_cast<unsigned int>(50 + (m_LeftBorder + m_RightBorder + m_TextureNormal->getSize().x) * getScale().x);
        }

        // Set the new size
        m_Listbox.setSize(static_cast<float>(width), static_cast<float>(m_Listbox.m_Size.y));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::setItemsToDisplay(unsigned int nrOfItemsInList)
    {
        // At least one item must be shown
        if (nrOfItemsInList < 1)
            nrOfItemsInList = 1;

        // Make the change
        m_NrOfItemsToDisplay = nrOfItemsInList;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    unsigned int ComboBox::getItemsToDisplay()
    {
        return m_NrOfItemsToDisplay;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::changeColors(const sf::Color& backgroundColor,         const sf::Color& textColor,
                                const sf::Color& selectedBackgroundColor, const sf::Color& selectedTextColor,
                                const sf::Color& borderColor)
    {
        // Store the new colors
        m_Listbox.m_BackgroundColor         = backgroundColor;
        m_Listbox.m_TextColor               = textColor;
        m_Listbox.m_SelectedBackgroundColor = selectedBackgroundColor;
        m_Listbox.m_SelectedTextColor       = selectedTextColor;
        m_Listbox.m_BorderColor             = borderColor;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const sf::Color& ComboBox::getBackgroundColor()
    {
        return m_Listbox.m_BackgroundColor;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const sf::Color& ComboBox::getTextColor()
    {
        return m_Listbox.m_TextColor;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const sf::Color& ComboBox::getSelectedBackgroundColor()
    {
        return m_Listbox.m_SelectedBackgroundColor;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const sf::Color& ComboBox::getSelectedTextColor()
    {
        return m_Listbox.m_SelectedTextColor;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const sf::Color& ComboBox::getBorderColor()
    {
        return m_Listbox.m_BorderColor;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::setTextFont(const sf::Font& font)
    {
        m_Listbox.setTextFont(font);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const sf::Font& ComboBox::getTextFont()
    {
        return m_Listbox.getTextFont();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::setBorders(unsigned int leftBorder, unsigned int topBorder, unsigned int rightBorder, unsigned int bottomBorder)
    {
        // Set the new border size
        m_LeftBorder   = leftBorder;
        m_TopBorder    = topBorder;
        m_RightBorder  = rightBorder;
        m_BottomBorder = bottomBorder;

        // Also change it inside the listbox (there is no top border there)
        m_Listbox.m_LeftBorder   = m_LeftBorder;
        m_Listbox.m_TopBorder    = 0;
        m_Listbox.m_RightBorder  = m_RightBorder;
        m_Listbox.m_BottomBorder = m_BottomBorder;

        // Don't set the width and height when loading failed
        if (m_Loaded == false)
            return;

        // There is a minimum width
        if (m_Listbox.m_Size.x < (50 + (m_LeftBorder + m_RightBorder + m_TextureNormal->getSize().x) * getScale().x))
            m_Listbox.setSize(50 + (m_LeftBorder + m_RightBorder + m_TextureNormal->getSize().x) * getScale().x, static_cast<float>(m_Listbox.m_Size.y));

        // Set the item height again, in case the scale changed by now
        m_Listbox.setItemHeight(m_TextureNormal->getSize().y + m_TopBorder + m_BottomBorder);

    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    unsigned int ComboBox::addItem(const std::string item, bool cropText)
    {
        // An item can only be added when the combo box was loaded correctly
        if (m_Loaded == false)
            return false;

        // Set the item height again, in case the scale changed by now
        m_Listbox.setItemHeight(static_cast<unsigned int>((m_TextureNormal->getSize().y + m_TopBorder + m_BottomBorder) * getScale().y));

        // Make room to add another item, until there are enough items
        if (m_NrOfItemsToDisplay > m_Listbox.m_Items.size())
            m_Listbox.setSize(static_cast<float>(m_Listbox.m_Size.x), ((m_TextureNormal->getSize().y + m_TopBorder + m_BottomBorder) * getScale().y * (m_Listbox.m_Items.size() + 1)) + (m_BottomBorder * getScale().y));

        // Add the item
        return m_Listbox.addItem(item, cropText);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool ComboBox::setSelectedItem(const std::string itemName)
    {
        return m_Listbox.setSelectedItem(itemName);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool ComboBox::setSelectedItem(unsigned int id)
    {
        return m_Listbox.setSelectedItem(id);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::removeItem(unsigned int id)
    {
        m_Listbox.removeItem(id);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::removeItem(const std::string itemName)
    {
        m_Listbox.removeItem(itemName);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::removeAllItems()
    {
        m_Listbox.removeAllItems();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string ComboBox::getItem(unsigned int id)
    {
        return m_Listbox.getItem(id);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    unsigned int ComboBox::getItemID(const std::string itemName)
    {
        return m_Listbox.getItemID(itemName);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<std::string>& ComboBox::getItems()
    {
        return m_Listbox.getItems();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string ComboBox::getSelectedItem() const
    {
        return m_Listbox.getSelectedItem();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    unsigned int ComboBox::getSelectedItemID()
    {
        return m_Listbox.getSelectedItemID();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool ComboBox::setScrollbar(const std::string scrollbarPathname)
    {
        // Remember the scrollbar pathname
        m_LoadedScrollbarPathname = scrollbarPathname;

        // Set the new scrollbar
        return m_Listbox.setScrollbar(scrollbarPathname);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::removeScrollbar()
    {
        // There is no scrollbar loaded so the string should be empty
        m_LoadedScrollbarPathname = "";

        // Remove the scrollbar
        m_Listbox.removeScrollbar();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::setMaximumItems(unsigned int maximumItems)
    {
        m_Listbox.setMaximumItems(maximumItems);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    unsigned int ComboBox::getMaximumItems()
    {
        return m_Listbox.getMaximumItems();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool ComboBox::mouseOnObject(float x, float y)
    {
        // Don't do anything when the combo box wasn't loaded correctly
        if (m_Loaded == false)
            return false;

        // Get the current position and scale
        Vector2f position = getPosition();
        Vector2f curScale = getScale();

        if ((x > position.x) && (x < position.x + (m_Listbox.m_Size.x * curScale.x)) && (y > position.y))
        {
            // Check if the mouse is on top of the combo box
            if (y < position.y + (m_Listbox.m_ItemHeight * curScale.y))
            {
                m_MouseOnListbox = false;
                m_Listbox.mouseNotOnObject();
                return true;
            }

            // Check if the listbox is visible
            if (m_ShowList)
            {
                // Temporarily set a position for the listbox
                m_Listbox.setPosition(position.x, position.y + (m_Listbox.m_ItemHeight * curScale.y));

                // Pass the event to the listbox
                if (m_Listbox.mouseOnObject(x, y))
                {
                    // Reset the position
                    m_Listbox.setPosition(0, 0);

                    m_MouseOnListbox = true;
                    return true;
                }

                // Reset the position
                m_Listbox.setPosition(0, 0);
            }
        }

        // The mouse is not on top of the combo box
        m_MouseHover = false;
        m_Listbox.mouseNotOnObject();
        m_Listbox.mouseNoLongerDown();
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::leftMousePressed(float x, float y)
    {
        if (m_Loaded == false)
            return;

        TGUI_UNUSED_PARAM(x);
        TGUI_UNUSED_PARAM(y);

        m_MouseDown = true;

        // Check if the list is visible and the mouse went down on top of the listbox
        if ((m_ShowList == true) && (m_MouseOnListbox == true))
        {
            // Store the current selected item
            unsigned int oldItem = m_Listbox.getSelectedItemID();

            // Temporarily set a position for the listbox
            m_Listbox.setPosition(getPosition().x, getPosition().y + (m_Listbox.m_ItemHeight * getScale().y));

            // Pass the event to the listbox
            m_Listbox.leftMousePressed(x, y);

            // Reset the position
            m_Listbox.setPosition(0, 0);

            // Check if the callback should be sent to the window
            if ((callbackID > 0) && (oldItem != m_Listbox.getSelectedItemID()))
            {
                Callback callback;
                callback.callbackID = callbackID;
                callback.trigger    = Callback::itemSelected;
                callback.value      = m_Listbox.getSelectedItemID();
                callback.text       = m_Listbox.getSelectedItem();
                m_Parent->addCallback(callback);
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::leftMouseReleased(float x, float y)
    {
        if (m_Loaded == false)
            return;

        // Make sure that the mouse went down on top of the combo box
        if (m_MouseDown == true)
        {
            // Check if the list is visible
            if (m_ShowList == true)
            {
                // Check if the mouse is on top of the listbox
                if (m_MouseOnListbox == true)
                {
                    // Check if the mouse went down on top of the listbox
                    if (m_Listbox.m_MouseDown == true)
                    {
                        // Make sure the mouse didn't went down on the scrollbar
                        if ((m_Listbox.m_Scroll == NULL) || ((m_Listbox.m_Scroll != NULL) && (m_Listbox.m_Scroll->m_MouseDown == false)))
                        {
                            // Stop showing the list
                            m_ShowList = false;
                        }

                        // Temporarily set a position for the listbox
                        m_Listbox.setPosition(getPosition().x, getPosition().y + (m_Listbox.m_ItemHeight * getScale().y));

                        // Pass the event to the listbox
                        m_Listbox.leftMouseReleased(x, y);

                        // Reset the position
                        m_Listbox.setPosition(0, 0);
                    }
                }
                else // The mouse is not on top of the listbox
                    m_ShowList = false;
            }
            else // The list wasn't visible
            {
                // Set the scale factors of the listbox
                m_Listbox.setScale(getScale());

                // Always show the beginning of the list
                if (m_Listbox.m_Scroll != NULL)
                    m_Listbox.m_Scroll->setValue(0);

                // Show the list
                m_ShowList = true;
            }

            // The mouse is no longer down
            m_MouseDown = false;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::mouseMoved(float x, float y)
    {
        // Don't do anything when the combo box wasn't loaded correctly
        if (m_Loaded == false)
            return;

        TGUI_UNUSED_PARAM(x);
        TGUI_UNUSED_PARAM(y);

        m_MouseHover = true;

        // Check if the list is visible and the mouse is on top of the listbox
        if ((m_ShowList == true) && (m_MouseOnListbox == true))
        {
            // Temporarily set a position for the listbox
            m_Listbox.setPosition(getPosition().x, getPosition().y + (m_Listbox.m_ItemHeight * getScale().y));

            // Pass the event to the listbox
            m_Listbox.mouseMoved(x, y);

            // Reset the position
            m_Listbox.setPosition(0, 0);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::mouseNoLongerDown()
    {
        m_MouseDown = false;
        m_Listbox.mouseNotOnObject();
        m_Listbox.mouseNoLongerDown();

        m_ShowList = false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ComboBox::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Don't draw anything when the combo box was not loaded correctly
        if (m_Loaded == false)
            return;

        // Get the current scale
        Vector2f curScale = getScale();

        // Adjust the transformation
        states.transform *= getTransform();

        // Remember the current transformation
        sf::Transform oldTransform = states.transform;

        // Draw the borders
        sf::RectangleShape Back(Vector2f(static_cast<float>(m_Listbox.m_Size.x), static_cast<float>(m_Listbox.m_ItemHeight)));
        Back.setFillColor(m_Listbox.m_BorderColor);
        target.draw(Back, states);

        // Move the front rect a little bit
        states.transform.translate(static_cast<float>(m_LeftBorder), static_cast<float>(m_TopBorder));

        // Draw the combo box
        sf::RectangleShape Front(Vector2f(static_cast<float>(m_Listbox.m_Size.x - m_LeftBorder - m_RightBorder),
                                          static_cast<float>(m_Listbox.m_ItemHeight - m_TopBorder - m_BottomBorder)));
        Front.setFillColor(m_Listbox.m_BackgroundColor);
        target.draw(Front, states);

        // Get the selected item
        std::string selectedItem = m_Listbox.getSelectedItem();

        // Create a text object to draw it
        sf::Text tempText(selectedItem);
        tempText.setCharacterSize(m_Listbox.m_ItemHeight - m_TopBorder - m_BottomBorder);
        tempText.setColor(m_Listbox.m_TextColor);

        // Make sure that the text fits inside the combo box
        {
            // Get the global bounds
            sf::FloatRect bounds = tempText.getGlobalBounds();

            // Calculate the maximum text width
            float maximumTextWidth = ((m_Listbox.m_Size.x - m_LeftBorder - m_RightBorder) * curScale.x) - (m_SpriteNormal.getGlobalBounds().width * curScale.y);

            // Check if the text is too long to fit inside the combo box
            while (((bounds.width * curScale.y) + bounds.left) > maximumTextWidth)
            {
                std::string tempString = tempText.getString().toAnsiString();

                // Make sure that the string isn't empty
                if (tempString.size() == 0)
                    break;

                // Remove the last character
                tempString.erase(tempString.length() -1);
                tempText.setString(tempString);

                // Recalculate the size
                bounds = tempText.getGlobalBounds();
            }
        }

        // Undo the x-scaling
        states.transform.scale(curScale.y/curScale.x, 1);

        // Draw the selected item
        states.transform.translate(0, ((m_Listbox.m_ItemHeight - m_TopBorder - m_BottomBorder) * 0.3333333f) - (tempText.getCharacterSize() / 2.0f));
        target.draw(tempText, states);

        // Reset the transformation
        states.transform = oldTransform.scale(1.0f/curScale.x, 1.0f/curScale.y);

        // Set the arrow like it should (down when listbox is invisible, up when it is visible)
        if (m_ShowList)
        {
            states.transform.scale(curScale.y, -curScale.y);
            states.transform.translate(((m_Listbox.m_Size.x - m_RightBorder) * curScale.x) - (m_SpriteNormal.getGlobalBounds().width * curScale.y), (m_TopBorder + m_TextureNormal->getSize().y) * (-curScale.y));
        }
        else
        {
            states.transform.translate(((m_Listbox.m_Size.x - m_RightBorder) * curScale.x) - (m_SpriteNormal.getGlobalBounds().width * curScale.y), m_TopBorder * curScale.y);
            states.transform.scale(curScale.y, curScale.y);
        }

        // Draw the arrow
        target.draw(m_SpriteNormal, states);

        // If the mouse is on top of the combo box then draw another arrow image (if allowed)
        if ((m_MouseHover) && (m_ObjectPhase & objectPhase::hover) && (m_MouseOnListbox == false))
            target.draw(m_SpriteHover, states);

        // If the listbox should be visible then draw it
        if (m_ShowList)
        {
            states.transform = oldTransform.translate(0, m_Listbox.m_ItemHeight * curScale.y);
            target.draw(m_Listbox, states);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
