/**\file
 *\section License
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2003-2010 Jaakko Keränen <jaakko.keranen@iki.fi>
 *\author Copyright © 2006-2010 Daniel Swanson <danij@dengine.net>
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

#ifndef LIBDENG_INFINE_MAIN_H
#define LIBDENG_INFINE_MAIN_H

#include "finaleinterpreter.h"

/**
 * @defgroup playsimServerFinaleFlags Play-simulation Server-side Finale Flags.
 *
 * Packet: PSV_FINALE Finale flags. Used with GPT_FINALE and GPT_FINALE2
 */
/*@{*/
#define FINF_BEGIN          0x01
#define FINF_END            0x02
#define FINF_SCRIPT         0x04   // Script included.
#define FINF_AFTER          0x08   // Otherwise before.
#define FINF_SKIP           0x10
#define FINF_OVERLAY        0x20   // Otherwise before (or after).
/*@}*/

#define FINALE_SCRIPT_EXTRADATA_SIZE      gx.finaleConditionsSize

void                FI_Register(void);
void                FI_Init(void);
void                FI_Shutdown(void);

void                FI_Ticker(timespan_t time);
int                 FI_Responder(ddevent_t* ev);
void                FI_Drawer(void);

int                 FI_SkipRequest(void);
boolean             FI_CmdExecuted(void);
finaleinterpreter_t* FI_ScriptInterpreter(void);
void*               FI_ScriptExtraData(void);

// We'll use the base template directly as our object.
typedef struct fi_object_s {
    FIOBJECT_BASE_ELEMENTS()
} fi_object_t;
fi_object_t*        FI_NewObject(fi_obtype_e type, const char* name);
void                FI_DeleteObject(fi_object_t* obj);

/// \todo Should be private.
typedef struct fi_namespace_s {
    uint            num;
    fi_object_t**   vector;
} fi_namespace_t;

typedef struct fi_page_s {
    finale_mode_t   mode;
    uint            timer;

    // Interactive script interpreter.
    finaleinterpreter_t _interpreter;

    // Known symbols (to this script).
    fi_namespace_t  _namespace;

    struct material_s* bgMaterial;
    animatorvector4_t bgColor;
    animatorvector2_t imgOffset;
    animatorvector4_t filter;
    animatorvector3_t textColor[9];

    int             initialGameState; // Game state before the script began.
    int             overlayGameState; // Overlay scripts run only in one gameMode.
    void*           extraData;
} fi_page_t;

fi_object_t*        FIPage_AddObject(fi_page_t* page, fi_object_t* obj);
fi_object_t*        FIPage_RemoveObject(fi_page_t* page, fi_object_t* obj);
boolean             FIPage_HasObject(fi_page_t* page, fi_object_t* obj);
fi_objectid_t       FIPage_ObjectIdForName(fi_page_t* page, const char* name, fi_obtype_e type);

void                FIPage_SetBackground(fi_page_t* page, struct material_s* mat);
void                FIPage_SetBackgroundColor(fi_page_t* page, float red, float green, float blue, int steps);
void                FIPage_SetBackgroundColorAndAlpha(fi_page_t* page, float red, float green, float blue, float alpha, int steps);
void                FIPage_SetImageOffsetX(fi_page_t* page, float x, int steps);
void                FIPage_SetImageOffsetY(fi_page_t* page, float y, int steps);
void                FIPage_SetImageOffsetXY(fi_page_t* page, float x, float y, int steps);
void                FIPage_SetFilterColorAndAlpha(fi_page_t* page, float red, float green, float blue, float alpha, int steps);
void                FIPage_SetPredefinedColor(fi_page_t* page, uint idx, float red, float green, float blue, int steps);

fi_page_t*          FI_PageStackTop(void);

#endif /* LIBDENG_INFINE_MAIN_H */
