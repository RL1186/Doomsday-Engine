/**
 * @file p_sound.h
 * id tech 1 sound playback functionality for the play simulation.
 *
 * @ingroup play
 *
 * @authors Copyright &copy; 2003-2013 Jaakko Keränen <jaakko.keranen@iki.fi>
 * @authors Copyright &copy; 2005-2013 Daniel Swanson <danij@dengine.net>
 * @authors Copyright &copy; 1993-1996 by id Software, Inc.
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

#ifndef LIBCOMMON_PLAY_SOUND_H
#define LIBCOMMON_PLAY_SOUND_H

#include "doomsday.h"

/**
 * Start the song for the specified map.
 */
void S_MapMusic(uint episode, uint map);

/**
 * Doom-like sector sounds: when a new sound starts, stop any existing
 * sounds from other origins in this Sector.
 *
 * @param sec           Sector to use as the origin of the sound.
 * @param id            ID number of the sound to be played.
 */
void S_SectorSound(Sector* sec, int id);

/**
 * @param sec           Sector in which to stop sounds.
 */
void S_SectorStopSounds(Sector* sec);

/**
 * Doom-like sector sounds: when a new sound starts, stop any existing
 * sounds from other origins in the same Sector.
 *
 * @param plane         Plane to use as the origin of the sound.
 * @param id            ID number of the sound to be played.
 */
void S_PlaneSound(Plane* pln, int id);

#ifdef __JHEXEN__
int S_GetSoundID(const char* name);

void S_ParseSndInfoLump(void);
#endif

#endif /// LIBCOMMON_PLAY_SOUND_H
