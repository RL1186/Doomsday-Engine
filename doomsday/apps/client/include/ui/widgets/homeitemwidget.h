/** @file homeitemwidget.h  Label with action buttons and an icon.
 *
 * @authors Copyright (c) 2016-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * @par License
 * GPL: http://www.gnu.org/licenses/gpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details. You should have received a copy of the GNU
 * General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#ifndef DENG_CLIENT_UI_HOME_HOMEITEMWIDGET_H
#define DENG_CLIENT_UI_HOME_HOMEITEMWIDGET_H

#include <de/Asset>
#include <de/ButtonWidget>

class Game;
namespace res { class LumpCatalog; }
class HomeMenuWidget;

/**
 * Label with action buttons and an icon. The item can be in either selected
 * or non-selected state.
 */
class HomeItemWidget : public de::GuiWidget, public de::IAssetGroup
{
    Q_OBJECT

public:
    enum Flag
    {
        NonAnimatedHeight = 0,
        AnimatedHeight    = 0x1,
        WithoutIcon       = 0x2,
    };
    Q_DECLARE_FLAGS(Flags, Flag)

public:
    HomeItemWidget(Flags flags = AnimatedHeight, de::String const &name = de::String());

    de::AssetGroup &assets() override;

    de::LabelWidget &       icon();
    de::LabelWidget &       label();
    de::LabelWidget const & label() const;

    void addButton(de::GuiWidget *button);
    de::GuiWidget &buttonWidget(int index) const;
    void setKeepButtonsVisible(bool yes);
    void setLabelMinimumRightMargin(de::Rule const &rule);

    virtual void setSelected(bool selected);
    bool isSelected() const;

    void useNormalStyle();
    void useInvertedStyle();
    void useColorTheme(ColorTheme style);
    void useColorTheme(ColorTheme unselected, ColorTheme selected);
    de::DotPath const &textColorId() const;

    void acquireFocus();

    HomeMenuWidget *parentMenu();

    // Events.
    bool handleEvent(de::Event const &event) override;
    void focusGained() override;
    void focusLost() override;

    virtual void itemRightClicked();

protected:
    void updateButtonLayout();

signals:
    void mouseActivity();
    void doubleClicked();
    void openContextMenu();
    void selected();
    //void deselected();

private:
    DENG2_PRIVATE(d)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HomeItemWidget::Flags)

#endif // DENG_CLIENT_UI_HOME_HOMEITEMWIDGET_H
