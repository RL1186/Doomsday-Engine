/** @file worldsystem.h  World subsystem.
 *
 * @authors Copyright © 2003-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
 * @authors Copyright © 2006-2015 Daniel Swanson <danij@dengine.net>
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
 * General Public License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA</small>
 */

#ifndef WORLDSYSTEM_H
#define WORLDSYSTEM_H

#include <de/liblegacy.h>
#include <de/Error>
#include <de/Observers>
#include <de/Scheduler>
#include <de/Vector>
#include <doomsday/world/world.h>
#include <doomsday/uri.h>

namespace world {
class Map;
#ifdef __CLIENT__
//class Hand;
#endif
}

/**
 * Ideas for improvement:
 *
 * "background loading" - it would be very cool if map loading happened in
 * another thread. This way we could be keeping busy while players watch the
 * intermission animations.
 *
 * "seamless world" - multiple concurrent maps with no perceivable delay when
 * players move between them.
 *
 * @ingroup world
 */
class ClientServerWorld : public World
{
public:
    /// No map is currently loaded. @ingroup errors
    DENG2_ERROR(MapError);

#ifdef __CLIENT__
    /// Notified when a new frame begins.
    DENG2_DEFINE_AUDIENCE2(FrameBegin, void worldSystemFrameBegins(bool resetNextViewer))

    /// Notified when the "current" frame ends.
    DENG2_DEFINE_AUDIENCE2(FrameEnd, void worldSystemFrameEnds())
#endif

public:
    /**
     * Construct a new world system (no map is loaded by default).
     */
    ClientServerWorld();

    /**
     * To be called to reset the world back to the initial state. Any currently
     * loaded map will be unloaded and player states are re-initialized.
     *
     * @todo World should observe GameChange.
     */
    void reset();

    /**
     * To be called following an engine reset to update the world state.
     */
    void update();

    de::Scheduler &scheduler();

    /**
     * Provides access to the currently loaded map.
     */
    world::Map &map() const;

    /**
     * Returns a pointer to the currently loaded map, if any.
     */
    inline world::Map *mapPtr() const { return hasMap() ? &map() : nullptr; }

    /**
     * @param uri  Universal resource identifier (URI) for the map to change to.
     *             If an empty URI is specified the current map will be unloaded.
     *
     * @return  @c true= the map change completed successfully.
     */
    bool changeMap(de::Uri const &uri);

    /**
     * Unload the currently loaded map (if any).
     *
     * @see changeMap()
     */
    inline void unloadMap() { changeMap(de::Uri()); }

    /**
     * Returns the effective map-info definition Record associated with the given
     * @a mapUri (which may be the default definition, if invalid/unknown).
     *
     * @param mapUri  Unique identifier for the map to lookup map-info data for.
     */
    de::Record const &mapInfoForMapUri(de::Uri const &mapUri) const;

    /**
     * Advance time in the world.
     *
     * @param delta  Time delta to apply.
     */
    void advanceTime(timespan_t delta);

    /**
     * Returns the current world time.
     */
    timespan_t time() const;

    void tick(timespan_t elapsed);

#ifdef __CLIENT__
    /**
     * To be called at the beginning of a render frame, so that we can prepare for
     * drawing view(s) of the current map.
     */
    void beginFrame(bool resetNextViewer = false);

    /**
     * To be called at the end of a render frame, so that we can finish up any tasks
     * that must be completed after view(s) have been drawn.
     */
    void endFrame();

#if 0
    /**
     * Returns the hand of the "user" in the world. Used for manipulating elements
     * for the purposes of runtime map editing.
     *
     * @param distance  The current distance of the hand from the viewer will be
     *                  written here if not @c nullptr.
     */
    world::Hand &hand(de::ddouble *distance = nullptr) const;
#endif

#endif  // __CLIENT__

public:
    /**
     * To be called to register the commands and variables of this module.
     */
    static void consoleRegister();

    /// Scripting helper: get pointer to current instance mobj_t based on the script callstack.
    static mobj_t &contextMobj(const de::Context &);

private:
    DENG2_PRIVATE(d)
};

DENG_EXTERN_C dd_bool ddMapSetup;
DENG_EXTERN_C int validCount;

#endif  // WORLDSYSTEM_H
