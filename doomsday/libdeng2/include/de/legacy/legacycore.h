/*
 * The Doomsday Engine Project -- libdeng2
 *
 * Copyright (c) 2011 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBDENG2_LEGACYCORE_H
#define LIBDENG2_LEGACYCORE_H

#include "../libdeng2.h"
#include <QObject>

namespace de {

class LegacyNetwork;

/**
 * Transitional kernel for supporting legacy Dooomsday C code in accessing
 * libdeng2 functionality. The legacy engine needs to construct one of these
 * via the deng2 C API and make sure it gets destroyed at shutdown. The C API
 * can be used to access functionality in LegacyCore.
 */
class LegacyCore : public QObject
{
    Q_OBJECT

public:
    /**
     * Initializes the legacy core.
     *
     * @param argc  Reference to the application's command line argument
     *              count. Note that this may be modified in this method.
     * @param argv  Command line arguments of the application. Note that
     *              this may be modified in the this method.
     */
    LegacyCore(int& argc, char** argv);

    ~LegacyCore();

    /**
     * Starts the deng2 event loop in the current thread. Does not return until
     * the loop is stopped.
     *
     * @param callback  Function to call periodically from the loop, as
     *                  often as possible.
     *
     * @return  Exit code.
     */
    int runEventLoop(void (*callback)(void));

    /**
     * Stops the event loop. This is automatically called when the core is
     * destroyed.
     */
    void stop(int exitCode = 0);

    /**
     * Returns the LegacyCore singleton instance.
     */
    static LegacyCore& instance();

    /**
     * Returns the legacy network subsystem interface.
     */
    LegacyNetwork& network();

public slots:
    void callback();

private:
    // Private instance data.
    struct Instance;
    Instance* d;

    /// Globally available.
    static LegacyCore* _appCore;
};

} // namespace de

#endif // LIBDENG2_LEGACYCORE_H
