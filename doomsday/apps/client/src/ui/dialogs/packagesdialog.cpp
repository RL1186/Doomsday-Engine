/** @file packagesdialog.cpp
 *
 * @authors Copyright (c) 2015 Jaakko Keränen <jaakko.keranen@iki.fi>
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

#include "ui/dialogs/packagesdialog.h"
#include "clientapp.h"

#include <de/FileSystem>

using namespace de;

DENG_GUI_PIMPL(PackagesDialog)
{
    Instance(Public *i) : Base(i)
    {}

    void populate()
    {
        StringList packages = App::packageLoader().findAllPackages();
        qSort(packages);
        qDebug() << "Found packages:" << packages;
    }
};

PackagesDialog::PackagesDialog()
    : DialogWidget("packages", WithHeading)
    , d(new Instance(this))
{
    heading().setText(tr("Packages"));
    buttons() << new DialogButtonItem(Default | Accept, tr("Close"));

    d->populate();
}