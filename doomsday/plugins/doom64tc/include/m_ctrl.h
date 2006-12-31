/**\file
 *\section Copyright and License Summary
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2003-2006 Jaakko Keränen <skyjake@dengine.net>
 *\author Copyright © 2005-2006 Daniel Swanson <danij@dengine.net>
 *\author Copyright © 2003-2005 Samuel Villarreal <svkaiser@gmail.com>
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

/*
 * Controls: Menu definitions
 */

#ifndef __DOOM64TC_CONTROLS__
#define __DOOM64TC_CONTROLS__

#ifndef __DOOM64TC__
#  error "Using Doom64TC headers without __DOOM64TC__"
#endif

#include "doom64tc.h"
#include "g_controls.h"

extern const Control_t *grabbing;

#define NUM_CONTROLS_ITEMS 105

static const MenuItem_t ControlsItems[] = {
    {ITT_EMPTY, 0, "PLAYER ACTIONS", NULL, 0},
    {ITT_EFUNC, 0, "LEFT :", SCControlConfig, A_TURNLEFT},
    {ITT_EFUNC, 0, "RIGHT :", SCControlConfig, A_TURNRIGHT},
    {ITT_EFUNC, 0, "FORWARD :", SCControlConfig, A_FORWARD},
    {ITT_EFUNC, 0, "BACKWARD :", SCControlConfig, A_BACKWARD},
    {ITT_EFUNC, 0, "STRAFE LEFT :", SCControlConfig, A_STRAFELEFT},
    {ITT_EFUNC, 0, "STRAFE RIGHT :", SCControlConfig, A_STRAFERIGHT},
    {ITT_EFUNC, 0, "FIRE :", SCControlConfig, A_FIRE},
    {ITT_EFUNC, 0, "USE :", SCControlConfig, A_USE},
    {ITT_EFUNC, 0, "JUMP : ", SCControlConfig, A_JUMP},
    {ITT_EFUNC, 0, "STRAFE :", SCControlConfig, A_STRAFE},
    {ITT_EFUNC, 0, "SPEED :", SCControlConfig, A_SPEED},
    {ITT_EFUNC, 0, "FLY UP :", SCControlConfig, 87},
    {ITT_EFUNC, 0, "FLY DOWN :", SCControlConfig, 88},
    {ITT_EFUNC, 0, "FALL DOWN :", SCControlConfig, 89},
    {ITT_EFUNC, 0, "LOOK UP :", SCControlConfig, A_LOOKUP},
    {ITT_EFUNC, 0, "LOOK DOWN :", SCControlConfig, A_LOOKDOWN},
    {ITT_EFUNC, 0, "LOOK CENTER :", SCControlConfig, A_LOOKCENTER},
    {ITT_EFUNC, 0, "MOUSE LOOK :", SCControlConfig, A_MLOOK},
    {ITT_EFUNC, 0, "JOYSTICK LOOK :", SCControlConfig, A_JLOOK},
    {ITT_EFUNC, 0, "NEXT WEAPON :", SCControlConfig, A_NEXTWEAPON},
    {ITT_EFUNC, 0, "PREV WEAPON :", SCControlConfig, A_PREVIOUSWEAPON},
    {ITT_EFUNC, 0, "FIST/CHAINSAW :", SCControlConfig, 49},
    {ITT_EFUNC, 0, "FIST :", SCControlConfig, A_WEAPON1},
    {ITT_EFUNC, 0, "CHAINSAW :", SCControlConfig, A_WEAPON8},
    {ITT_EFUNC, 0, "PISTOL :", SCControlConfig, A_WEAPON2},
    {ITT_EFUNC, 0, "SUPER SG/SHOTGUN :", SCControlConfig, 50},
    {ITT_EFUNC, 0, "SHOTGUN :", SCControlConfig, A_WEAPON3},
    {ITT_EFUNC, 0, "SUPER SHOTGUN :", SCControlConfig, A_WEAPON9},
    {ITT_EFUNC, 0, "CHAINGUN :", SCControlConfig, A_WEAPON4},
    {ITT_EFUNC, 0, "ROCKET LAUNCHER :", SCControlConfig, A_WEAPON5},
    {ITT_EFUNC, 0, "PLASMA RIFLE :", SCControlConfig, A_WEAPON6},
    {ITT_EFUNC, 0, "BFG 9000 :", SCControlConfig, A_WEAPON7},
    {ITT_EFUNC, 0, "UNMAKER :", SCControlConfig, 90},
    {ITT_EMPTY, 0, NULL, NULL, 0},
    {ITT_EMPTY, 0, NULL, NULL, 0},
    {ITT_EMPTY, 0, "MENU", NULL, 0},
    {ITT_EFUNC, 0, "OPEN/CLOSE MENU :", SCControlConfig, 70},
    {ITT_EFUNC, 0, "Cursor Up :", SCControlConfig, 64},
    {ITT_EFUNC, 0, "Cursor Down :", SCControlConfig, 65},
    {ITT_EFUNC, 0, "Cursor Left :", SCControlConfig, 66},
    {ITT_EFUNC, 0, "Cursor Right :", SCControlConfig, 67},
    {ITT_EFUNC, 0, "Accept :", SCControlConfig, 68},
    {ITT_EFUNC, 0, "Cancel :", SCControlConfig, 69},
    {ITT_EMPTY, 0, NULL, NULL, 0},
    {ITT_EMPTY, 0, "MENU HOTKEYS", NULL, 0},
    {ITT_EFUNC, 0, "HELP :", SCControlConfig, 28},
    {ITT_EFUNC, 0, "SOUND MENU :", SCControlConfig, 31},
    {ITT_EFUNC, 0, "LOAD GAME :", SCControlConfig, 30},
    {ITT_EFUNC, 0, "SAVE GAME :", SCControlConfig, 29},
    {ITT_EFUNC, 0, "QUICK LOAD :", SCControlConfig, 35},
    {ITT_EFUNC, 0, "QUICK SAVE :", SCControlConfig, 32},
    {ITT_EFUNC, 0, "END GAME :", SCControlConfig, 33},
    {ITT_EFUNC, 0, "QUIT :", SCControlConfig, 36},
    {ITT_EMPTY, 0, NULL, NULL, 0},
    {ITT_EMPTY, 0, "SCREEN", NULL, 0},
    {ITT_EFUNC, 0, "SMALLER VIEW :", SCControlConfig, 39},
    {ITT_EFUNC, 0, "LARGER VIEW :", SCControlConfig, 40},
    {ITT_EFUNC, 0, "SHOW HUD :", SCControlConfig, 91},
    {ITT_EFUNC, 0, "MESSAGES ON/OFF:", SCControlConfig, 34},
    {ITT_EFUNC, 0, "SPY MODE :", SCControlConfig, 38},
    {ITT_EFUNC, 0, "GAMMA CORRECTION :", SCControlConfig, 37},
    {ITT_EMPTY, 0, NULL, NULL, 0},
    {ITT_EMPTY, 0, "AUTOMAP", NULL, 0},
    {ITT_EFUNC, 0, "OPEN/CLOSE MAP :", SCControlConfig, 51},
    {ITT_EFUNC, 0, "PAN UP :", SCControlConfig, 60},
    {ITT_EFUNC, 0, "PAN DOWN :", SCControlConfig, 61},
    {ITT_EFUNC, 0, "PAN LEFT :", SCControlConfig, 62},
    {ITT_EFUNC, 0, "PAN RIGHT :", SCControlConfig, 63},
    {ITT_EFUNC, 0, "FOLLOW MODE :", SCControlConfig, 52},
    {ITT_EFUNC, 0, "ROTATE MODE :", SCControlConfig, 53},
    {ITT_EFUNC, 0, "TOGGLE GRID :", SCControlConfig, 54},
    {ITT_EFUNC, 0, "ZOOM IN :", SCControlConfig, 55},
    {ITT_EFUNC, 0, "ZOOM OUT :", SCControlConfig, 56},
    {ITT_EFUNC, 0, "ZOOM EXTENTS :", SCControlConfig, 57},
    {ITT_EFUNC, 0, "ADD MARK :", SCControlConfig, 58},
    {ITT_EFUNC, 0, "CLEAR MARKS :", SCControlConfig, 59},
    {ITT_EMPTY, 0, NULL, NULL, 0},
    {ITT_EMPTY, 0, "CHATMODE", NULL, 0},
    {ITT_EFUNC, 0, "OPEN CHAT :", SCControlConfig, 43},
    {ITT_EFUNC, 0, "GREEN CHAT :", SCControlConfig, 44},
    {ITT_EFUNC, 0, "INDIGO CHAT :", SCControlConfig, 45},
    {ITT_EFUNC, 0, "BROWN CHAT :", SCControlConfig, 46},
    {ITT_EFUNC, 0, "RED CHAT :", SCControlConfig, 47},
    {ITT_EFUNC, 0, "COMPLETE :", SCControlConfig, 71},
    {ITT_EFUNC, 0, "DELETE :", SCControlConfig, 83},
    {ITT_EFUNC, 0, "CANCEL CHAT :", SCControlConfig, 72},
    {ITT_EFUNC, 0, "MSG REFRESH :", SCControlConfig, 48},
    {ITT_EFUNC, 0, "MACRO 0:", SCControlConfig, 73},
    {ITT_EFUNC, 0, "MACRO 1:", SCControlConfig, 74},
    {ITT_EFUNC, 0, "MACRO 2:", SCControlConfig, 75},
    {ITT_EFUNC, 0, "MACRO 3:", SCControlConfig, 76},
    {ITT_EFUNC, 0, "MACRO 4:", SCControlConfig, 77},
    {ITT_EFUNC, 0, "MACRO 5:", SCControlConfig, 78},
    {ITT_EFUNC, 0, "MACRO 6:", SCControlConfig, 79},
    {ITT_EFUNC, 0, "MACRO 7:", SCControlConfig, 80},
    {ITT_EFUNC, 0, "MACRO 8:", SCControlConfig, 81},
    {ITT_EFUNC, 0, "MACRO 9:", SCControlConfig, 82},
    {ITT_EMPTY, 0, NULL, NULL, 0},
    {ITT_EMPTY, 0, "MISCELLANEOUS", NULL, 0},
    {ITT_EFUNC, 0, "PAUSE :", SCControlConfig, 41},
    {ITT_EFUNC, 0, "SCREENSHOT :", SCControlConfig, 42},
    {ITT_EFUNC, 0, "MESSAGE YES :", SCControlConfig, 84},
    {ITT_EFUNC, 0, "MESSAGE NO :", SCControlConfig, 85},
    {ITT_EFUNC, 0, "MESSAGE CANCEL :", SCControlConfig, 86}
};

void G_DefaultBindings(void);
void M_DrawControlsMenu(void);
void G_BindClassRegistration(void);

#endif
