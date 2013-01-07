/**\file p_polyobjs.h
 *\section License
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2003-2013 Jaakko Keränen <jaakko.keranen@iki.fi>
 *\author Copyright © 2006-2013 Daniel Swanson <danij@dengine.net>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef LIBDENG_P_POLYOBJS_H
#define LIBDENG_P_POLYOBJS_H

#include "resource/r_data.h"

/**
 * Action the callback if set, otherwise this is no-op.
 */
void P_PolyobjCallback(struct mobj_s* mobj, LineDef* lineDef, Polyobj* polyobj);

void P_PolyobjChanged(Polyobj* po);

/**
 * Lookup a Polyobj on the current map by the base mobj.
 *
 * @param ddMobjBase  Base mobj to look for.
 *
 * @return  Found Polyobj instance, or @c NULL.
 */
Polyobj* P_PolyobjByBase(void* ddMobjBase);

/**
 * Translate the origin of @a polyobj in the map coordinate space.
 */
boolean P_PolyobjMove(Polyobj* polyobj, coord_t xy[]);

#endif /// LIBDENG_P_POLYOBJS_H
