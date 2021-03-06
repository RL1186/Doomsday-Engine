/** @file clientrootwidget.h  GUI root widget tailored for the Doomsday Client.
 *
 * @authors Copyright (c) 2014-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
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

#ifndef DENG_CLIENTROOTWIDGET_H
#define DENG_CLIENTROOTWIDGET_H

#include <de/GuiRootWidget>
#include <de/GLWindow>
#include <de/PackageIconBank>

class ClientWindow;

/**
 * GUI root widget tailored for the Doomsday Client.
 */
class ClientRootWidget : public de::GuiRootWidget
{
public:
    ClientRootWidget(de::GLWindow *window = 0);

    ClientWindow &window();
    de::PackageIconBank &packageIconBank();

    void update() override;
    void addOnTop(de::GuiWidget *widget) override;
    void dispatchLatestMousePosition() override;
    void handleEventAsFallback(de::Event const &event) override;

private:
    DENG2_PRIVATE(d)
};

#endif // DENG_CLIENTROOTWIDGET_H
