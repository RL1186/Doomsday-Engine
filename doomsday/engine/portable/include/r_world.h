/* DE1: $Id$
 * Copyright (C) 2003, 2004 Jaakko Ker�nen <jaakko.keranen@iki.fi>
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
 * along with this program; if not: http://www.opensource.org/
 */

/*
 * r_world.h: World Setup and Refresh
 */

#ifndef __DOOMSDAY_REFRESH_WORLD_H__
#define __DOOMSDAY_REFRESH_WORLD_H__

#include "r_data.h"

extern int      rendSkyLight;      // cvar

// Map Info flags.
#define MIF_FOG             0x1    // Fog is used in the level.
#define MIF_DRAW_SPHERE     0x2    // Always draw the sky sphere.

const char     *R_GetCurrentLevelID(void);
const char     *R_GetUniqueLevelID(void);
const byte     *R_GetSectorLightColor(sector_t *sector);
boolean         R_IsSkySurface(surface_t *surface);
void            R_SetupLevel(char *level_id, int flags);
void            R_InitLinks(void);
void            R_SetupFog(void);
void            R_SetupSky(void);
void            R_SetSectorLinks(sector_t *sec);
sector_t       *R_GetLinkedSector(sector_t *startsec, int plane);
void            R_UpdatePlanes(void);
void            R_ClearSectorFlags(void);
void            R_SkyFix(boolean fixFloors, boolean fixCeilings);
void            R_OrderVertices(line_t *line, const sector_t *sector,
                                vertex_t * verts[2]);
void            R_GetMapSize(vertex_t *min, vertex_t *max);
#endif
