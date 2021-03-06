/** @file signalaction.h
 *
 * @authors Copyright (c) 2013-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * @par License
 * LGPL: http://www.gnu.org/licenses/lgpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details. You should have received a copy of
 * the GNU Lesser General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#ifndef LIBAPPFW_SIGNALACTION_H
#define LIBAPPFW_SIGNALACTION_H

#include "../libappfw.h"
#include <de/Action>
#include <QObject>

namespace de {

/**
 * Action that triggers a Qt signal.
 *
 * @ingroup appfw
 */
class LIBAPPFW_PUBLIC SignalAction : public QObject, public Action
{
    Q_OBJECT

public:
    SignalAction(QObject *target, char const *slot);

    void trigger();

signals:
    void triggered();

private:
    QObject *_target;
    char const *_slot;
};

} // namespace de

#endif // LIBAPPFW_SIGNALACTION_H
