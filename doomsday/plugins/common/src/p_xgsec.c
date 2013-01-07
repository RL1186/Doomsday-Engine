/**\file p_xgsec.c
 *\section License
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2003-2013 Jaakko Keränen <jaakko.keranen@iki.fi>
 *\author Copyright © 2005-2013 Daniel Swanson <danij@dengine.net>
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

/**
 * Extended Generalized Sector Types.
 */

#if __JDOOM__ || __JHERETIC__ || __JDOOM64__
// HEADER FILES ------------------------------------------------------------

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#if __JDOOM__
#  include "jdoom.h"
#elif __JDOOM64__
#  include "jdoom64.h"
#elif __JHERETIC__
#  include "jheretic.h"
#elif __JSTRIFE__
#  include "jstrife.h"
#endif

#include "dmu_lib.h"
#include "p_mapsetup.h"
#include "p_xgline.h"
#include "p_xgsec.h"
#include "g_common.h"
#include "p_map.h"
#include "p_mapspec.h"
#include "p_sound.h"
#include "p_terraintype.h"
#include "p_tick.h"

// MACROS ------------------------------------------------------------------

#define MAX_VALS        128

#define SIGN(x)         ((x)>0? 1 : (x)<0? -1 : 0)

#define ISFUNC(fn)      (fn->func && fn->func[fn->pos])
#define UPDFUNC(fn)     ((ISFUNC(fn) || fn->link))

#define SPREFTYPESTR(reftype) (reftype == SPREF_NONE? "NONE" \
        : reftype == SPREF_MY_FLOOR? "MY FLOOR" \
        : reftype == SPREF_MY_CEILING? "MY CEILING" \
        : reftype == SPREF_ORIGINAL_FLOOR? "ORIGINAL FLOOR" \
        : reftype == SPREF_ORIGINAL_CEILING? "ORIGINAL CEILING" \
        : reftype == SPREF_CURRENT_FLOOR? "CURRENT FLOOR" \
        : reftype == SPREF_CURRENT_CEILING? "CURRENT CEILING" \
        : reftype == SPREF_HIGHEST_FLOOR? "HIGHEST FLOOR" \
        : reftype == SPREF_HIGHEST_CEILING? "HIGHEST CEILING" \
        : reftype == SPREF_LOWEST_FLOOR? "LOWEST FLOOR" \
        : reftype == SPREF_LOWEST_CEILING? "LOWEST CEILING" \
        : reftype == SPREF_NEXT_HIGHEST_FLOOR? "NEXT HIGHEST FLOOR" \
        : reftype == SPREF_NEXT_HIGHEST_CEILING? "NEXT HIGHEST CEILING" \
        : reftype == SPREF_NEXT_LOWEST_FLOOR? "NEXT LOWEST FLOOR" \
        : reftype == SPREF_NEXT_LOWEST_CEILING? "NEXT LOWEST CEILING" \
        : reftype == SPREF_MIN_BOTTOM_MATERIAL? "MIN BOTTOM MATERIAL" \
        : reftype == SPREF_MIN_MID_MATERIAL? "MIN MIDDLE MATERIAL" \
        : reftype == SPREF_MIN_TOP_MATERIAL? "MIN TOP MATERIAL" \
        : reftype == SPREF_MAX_BOTTOM_MATERIAL? "MAX BOTTOM MATERIAL" \
        : reftype == SPREF_MAX_MID_MATERIAL? "MAX MIDDLE MATERIAL" \
        : reftype == SPREF_MAX_TOP_MATERIAL? "MAX TOP MATERIAL" \
        : reftype == SPREF_SECTOR_TAGGED_FLOOR? "SECTOR TAGGED FLOOR" \
        : reftype == SPREF_LINE_TAGGED_FLOOR? "LINE TAGGED FLOOR" \
        : reftype == SPREF_TAGGED_FLOOR? "TAGGED FLOOR" \
        : reftype == SPREF_ACT_TAGGED_FLOOR? "ACT TAGGED FLOOR" \
        : reftype == SPREF_INDEX_FLOOR? "INDEXED FLOOR" \
        : reftype == SPREF_SECTOR_TAGGED_CEILING? "SECTOR TAGGED CEILING" \
        : reftype == SPREF_LINE_TAGGED_CEILING? "LINE TAGGED CEILING" \
        : reftype == SPREF_TAGGED_CEILING? "TAGGED CEILING" \
        : reftype == SPREF_ACT_TAGGED_CEILING? "ACT TAGGED CEILING" \
        : reftype == SPREF_INDEX_CEILING? "INDEXED CEILING" \
        : reftype == SPREF_BACK_FLOOR? "BACK FLOOR" \
        : reftype == SPREF_BACK_CEILING? "BACK CEILING" \
        : reftype == SPREF_SPECIAL? "SPECIAL" \
        : reftype == SPREF_LINE_ACT_TAGGED_FLOOR? "LINE ACT TAGGED FLOOR" \
        : reftype == SPREF_LINE_ACT_TAGGED_CEILING? "LINE ACT TAGGED CEILING" : "???")

#define TO_DMU_COLOR(x) (x == 0? DMU_COLOR_RED \
        : x == 1? DMU_COLOR_GREEN \
        : DMU_COLOR_BLUE)

#define TO_DMU_CEILING_COLOR(x) (x == 0? DMU_CEILING_COLOR_RED \
        : x == 1? DMU_CEILING_COLOR_GREEN \
        : DMU_CEILING_COLOR_BLUE)

#define TO_DMU_FLOOR_COLOR(x) (x == 0? DMU_FLOOR_COLOR_RED \
        : x == 1? DMU_FLOOR_COLOR_GREEN \
        : DMU_FLOOR_COLOR_BLUE)

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void XS_DoChain(Sector *sec, int ch, int activating, void *actThing);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static sectortype_t sectypebuffer;

// CODE --------------------------------------------------------------------

sectortype_t *XS_GetType(int id)
{
    sectortype_t*       ptr;
    char                buff[6];

    // Try finding it from the DDXGDATA lump.
    ptr = XG_GetLumpSector(id);
    if(ptr)
    {   // Got it!
        memcpy(&sectypebuffer, ptr, sizeof(*ptr));
        return &sectypebuffer;
    }

    dd_snprintf(buff, 6, "%i", id);

    if(Def_Get(DD_DEF_SECTOR_TYPE, buff, &sectypebuffer))
        return &sectypebuffer;  // A definition was found.

    return NULL; // None found.
}

void XF_Init(Sector *sec, function_t *fn, char *func, int min, int max,
             float scale, float offset)
{
    xsector_t          *xsec = P_ToXSector(sec);

    memset(fn, 0, sizeof(*fn));

    if(!func)
        return;

    // Check for links.
    if(func[0] == '=')
    {
        switch(tolower(func[1]))
        {
        case 'r':
            fn->link = &xsec->xg->rgb[0];
            break;

        case 'g':
            fn->link = &xsec->xg->rgb[1];
            break;

        case 'b':
            fn->link = &xsec->xg->rgb[2];
            break;

        case 'f':
            fn->link = &xsec->xg->plane[XGSP_FLOOR];
            break;

        case 'c':
            fn->link = &xsec->xg->plane[XGSP_CEILING];
            break;

        case 'l':
            fn->link = &xsec->xg->light;
            break;

        default:
            Con_Error("XF_Init: Bad linked func (%s).\n", func);
        }
        return;
    }

    // Check for offsets to current values.
    if(func[0] == '+')
    {
        /**
         * @note The original value ranges must be maintained due to the cross linking
         * between sector function types i.e:
         * - RGB = 0 > 254
         * - light = 0 > 254
         * - planeheight = -32768 > 32768
         */
        switch(func[1])
        {
        case 'r':
            offset += 255.f * xsec->origRGB[0];
            break;

        case 'g':
            offset += 255.f * xsec->origRGB[1];
            break;

        case 'b':
            offset += 255.f * xsec->origRGB[2];
            break;

        case 'l':
            offset += 255.0f * xsec->origLight;
            break;

        case 'f':
            offset += xsec->SP_floororigheight;
            break;

        case 'c':
            offset += xsec->SP_ceilorigheight;
            break;

        default:
            Con_Error("XF_Init: Bad preset offset (%s).\n", func);
        }
        fn->func = func + 2;
    }
    else
    {
        fn->func = func;
    }

    fn->timer = -1; // The first step musn't skip the first value.
    fn->maxTimer = XG_RandomInt(min, max);
    fn->minInterval = min;
    fn->maxInterval = max;
    fn->scale = scale;
    fn->offset = offset;
    // Make sure oldvalue is out of range.
    fn->oldValue = -scale + offset;
}

int C_DECL XLTrav_LineAngle(LineDef* line, boolean dummy, void* context,
    void* context2, mobj_t* activator)
{
    Sector* sec = (Sector *) context;
    coord_t d1[2];

    if(P_GetPtrp(line, DMU_FRONT_SECTOR) != sec &&
       P_GetPtrp(line, DMU_BACK_SECTOR)  != sec)
        return true; // Wrong sector, keep looking.

    P_GetDoublepv(line, DMU_DXY, d1);
    *(angle_t*) context2 = M_PointXYToAngle2(0, 0, d1[0], d1[1]);

    return false; // Stop looking after first hit.
}

int findXSThinker(thinker_t* th, void* context)
{
    xsthinker_t* xs = (xsthinker_t*) th;

    if(xs->sector == (Sector*) context)
        return true; // Stop iteration, we've found it.

    return false; // Continue iteration.
}

int destroyXSThinker(thinker_t* th, void* context)
{
    xsthinker_t* xs = (xsthinker_t*) th;

    if(xs->sector == (Sector*) context)
    {
        Thinker_Remove(&xs->thinker);
        return true; // Stop iteration, we're done.
    }

    return false; // Continue iteration.
}

void XS_SetSectorType(struct sector_s* sec, int special)
{
    int                 i;
    xsector_t*          xsec = P_ToXSector(sec);
    xgsector_t*         xg;
    sectortype_t*       info;

    if(XS_GetType(special))
    {
        XG_Dev("XS_SetSectorType: Sector %i, type %i", P_ToIndex(sec),
               special);

        xsec->special = special;

        // All right, do the init.
        if(!xsec->xg)
            xsec->xg = Z_Malloc(sizeof(xgsector_t), PU_MAP, 0);
        memset(xsec->xg, 0, sizeof(*xsec->xg));

        // Get the type info.
        memcpy(&xsec->xg->info, &sectypebuffer, sizeof(sectypebuffer));

        // Init the state.
        xg = xsec->xg;
        info = &xsec->xg->info;

        // Init timer so ambient doesn't play immediately at map start.
        xg->timer =
            XG_RandomInt(FLT2TIC(xg->info.soundInterval[0]),
                         FLT2TIC(xg->info.soundInterval[1]));

        // Light function.
        XF_Init(sec, &xg->light, info->lightFunc, info->lightInterval[0],
                info->lightInterval[1], 255, 0);

        // Color functions.
        for(i = 0; i < 3; ++i)
        {
            XF_Init(sec, &xg->rgb[i], info->colFunc[i],
                    info->colInterval[i][0], info->colInterval[i][1], 255,
                    0);
        }

        // Plane functions / floor.
        XF_Init(sec, &xg->plane[XGSP_FLOOR], info->floorFunc,
                info->floorInterval[0], info->floorInterval[1],
                info->floorMul, info->floorOff);
        XF_Init(sec, &xg->plane[XGSP_CEILING], info->ceilFunc,
                info->ceilInterval[0], info->ceilInterval[1],
                info->ceilMul, info->ceilOff);

        // Derive texmove angle from first act-tagged line?
        if((info->flags & STF_ACT_TAG_MATERIALMOVE) ||
           (info->flags & STF_ACT_TAG_WIND))
        {
            angle_t angle = 0;

            // -1 to support binary XG data with old flag values.
            XL_TraverseLines(0, (xgDataLumps? LREF_TAGGED -1: LREF_TAGGED),
                             info->actTag, sec, &angle,
                             NULL, XLTrav_LineAngle);

            // Convert to degrees.
            if(info->flags & STF_ACT_TAG_MATERIALMOVE)
            {
                info->materialMoveAngle[0] = info->materialMoveAngle[1] =
                    angle / (float) ANGLE_MAX *360;
            }

            if(info->flags & STF_ACT_TAG_WIND)
            {
                info->windAngle = angle / (float) ANGLE_MAX *360;
            }
        }

        // If there is not already an xsthinker for this sector, create one.
        if(!Thinker_Iterate(XS_Thinker, findXSThinker, sec))
        {   // Not created one yet.
            xsthinker_t*    xs = Z_Calloc(sizeof(*xs), PU_MAP, 0);

            xs->thinker.function = XS_Thinker;
            Thinker_Add(&xs->thinker);

            xs->sector = sec;
        }
    }
    else
    {
        XG_Dev("XS_SetSectorType: Sector %i, NORMAL TYPE %i", P_ToIndex(sec),
               special);

        // If there is an xsthinker for this, destroy it.
        Thinker_Iterate(XS_Thinker, destroyXSThinker, sec);

        // Free previously allocated XG data.
        if(xsec->xg)
            Z_Free(xsec->xg);
        xsec->xg = NULL;

        // Just set it, then. Must be a standard sector type...
        // Mind you, we're not going to spawn any standard flash funcs
        // or anything.
        xsec->special = special;
    }
}

void XS_Init(void)
{
    /*  // Clients rely on the server, they don't do XG themselves.
       if(IS_CLIENT) return; */

    if(numsectors > 0)
    {
        // Allocate stair builder data.
        Sector* sec;
        xsector_t* xsec;
        uint i;

        for(i = 0; i < numsectors; ++i)
        {
            sec = P_ToPtr(DMU_SECTOR, i);
            xsec = P_ToXSector(sec);

            P_GetFloatpv(sec, DMU_COLOR, xsec->origRGB);

            xsec->SP_floororigheight = P_GetDoublep(sec, DMU_FLOOR_HEIGHT);
            xsec->SP_ceilorigheight  = P_GetDoublep(sec, DMU_CEILING_HEIGHT);
            xsec->origLight = P_GetFloatp(sec, DMU_LIGHT_LEVEL);

            // Initialize the XG data for this sector.
            XS_SetSectorType(sec, xsec->special);
        }
    }
}

void XS_SectorSound(Sector* sec, int soundId)
{
    if(!sec || !soundId) return;
    XG_Dev("XS_SectorSound: Play Sound ID (%i) in Sector ID (%i)", soundId, P_ToIndex(sec));
    S_SectorSound(sec, soundId);
}

void XS_PlaneSound(Plane* pln, int soundId)
{
    if(!pln || !soundId) return;
    XG_Dev("XS_PlaneSound: Play Sound ID (%i) in Sector ID (%i)", soundId, P_ToIndex(P_GetPtrp(pln, DMU_SECTOR)));
    S_PlaneSound(pln, soundId);
}

void XS_MoverStopped(xgplanemover_t *mover, boolean done)
{
    xline_t    *origin = P_ToXLine(mover->origin);

    XG_Dev("XS_MoverStopped: Sector %i (done=%i, origin line=%i)",
           P_ToIndex(mover->sector), done,
           mover->origin ? P_ToIndex(mover->origin) : -1);

    if(done)
    {
        if((mover->flags & PMF_ACTIVATE_WHEN_DONE) && mover->origin)
        {
            XL_ActivateLine(true, &origin->xg->info, mover->origin, 0,
                            &dummyThing, XLE_AUTO);
        }

        if((mover->flags & PMF_DEACTIVATE_WHEN_DONE) && mover->origin)
        {
            XL_ActivateLine(false, &origin->xg->info, mover->origin, 0,
                            &dummyThing, XLE_AUTO);
        }

        // Remove this thinker.
        Thinker_Remove((thinker_t *) mover);
    }
    else
    {
        // Normally we just wait, but if...
        if((mover->flags & PMF_ACTIVATE_ON_ABORT) && mover->origin)
        {
            XL_ActivateLine(true, &origin->xg->info, mover->origin, 0,
                            &dummyThing, XLE_AUTO);
        }

        if((mover->flags & PMF_DEACTIVATE_ON_ABORT) && mover->origin)
        {
            XL_ActivateLine(false, &origin->xg->info, mover->origin, 0,
                            &dummyThing, XLE_AUTO);
        }

        if(mover->flags & (PMF_ACTIVATE_ON_ABORT | PMF_DEACTIVATE_ON_ABORT))
        {
            // Destroy this mover.
            Thinker_Remove((thinker_t *) mover);
        }
    }
}

/**
 * A thinker function for plane movers.
 */
void XS_PlaneMover(xgplanemover_t* mover)
{
    int res, res2, dir;
    coord_t ceil  = P_GetDoublep(mover->sector, DMU_CEILING_HEIGHT);
    coord_t floor = P_GetDoublep(mover->sector, DMU_FLOOR_HEIGHT);
    xsector_t* xsec = P_ToXSector(mover->sector);
    boolean docrush = (mover->flags & PMF_CRUSH) != 0;
    boolean follows = (mover->flags & PMF_OTHER_FOLLOWS) != 0;
    boolean setorig = (mover->flags & PMF_SET_ORIGINAL) != 0;

    // Play movesound when timer goes to zero.
    if(mover->timer-- <= 0)
    {
        // Clear the wait flag.
        if(mover->flags & PMF_WAIT)
        {
            mover->flags &= ~PMF_WAIT;
            // Play a sound.
            XS_PlaneSound(P_GetPtrp(mover->sector, mover->ceiling? DMU_CEILING_PLANE : DMU_FLOOR_PLANE),
                          mover->startSound);
        }

        mover->timer = XG_RandomInt(mover->minInterval, mover->maxInterval);
        XS_PlaneSound(P_GetPtrp(mover->sector, mover->ceiling? DMU_CEILING_PLANE : DMU_FLOOR_PLANE),
                      mover->moveSound);
    }

    // Are we waiting?
    if(mover->flags & PMF_WAIT) return;

    // Determine move direction.
    if((mover->destination - (mover->ceiling ? ceil : floor)) > 0)
        dir = 1; // Moving up.
    else
        dir = -1; // Moving down.

    // Do the move.
    res = T_MovePlane(mover->sector, mover->speed, mover->destination,
                      docrush, mover->ceiling, dir);

    // Should we update origheight?
    if(setorig)
    {
        xsec->planes[mover->ceiling? PLN_CEILING:PLN_FLOOR].origHeight =
            P_GetDoublep(mover->sector,
                        mover->ceiling? DMU_CEILING_HEIGHT:DMU_FLOOR_HEIGHT);
    }

    if(follows)
    {
        coord_t off = (mover->ceiling? floor - ceil : ceil - floor);

        res2 = T_MovePlane(mover->sector, mover->speed,
                           mover->destination + off, docrush,
                           !mover->ceiling, dir);

        // Should we update origheight?
        if(setorig)
        {
            xsec->planes[(!mover->ceiling)? PLN_CEILING:PLN_FLOOR].origHeight =
                P_GetDoublep(mover->sector,
                            (!mover->ceiling)? DMU_CEILING_HEIGHT:DMU_FLOOR_HEIGHT);
        }

        if(res2 == crushed)
            res = crushed;
    }

    if(res == pastdest)
    {
        // Move has finished.
        XS_MoverStopped(mover, true);

        // The move is done. Do end stuff.
        if(mover->setMaterial)
        {
            XS_ChangePlaneMaterial(mover->sector, mover->ceiling,
                                   mover->setMaterial, NULL);
        }

        if(mover->setSectorType >= 0)
        {
            XS_SetSectorType(mover->sector, mover->setSectorType);
        }

        // Play sound?
        XS_PlaneSound(P_GetPtrp(mover->sector, mover->ceiling? DMU_CEILING_PLANE : DMU_FLOOR_PLANE),
                      mover->endSound);
    }
    else if(res == crushed)
    {
        if(mover->flags & PMF_CRUSH)
        {
            // We're crushing things.
            mover->speed = mover->crushSpeed;
        }
        else
        {
            // Make sure both the planes are where we started from.
            if((!mover->ceiling || follows) &&
               !FEQUAL(P_GetDoublep(mover->sector, DMU_FLOOR_HEIGHT), floor))
            {
                T_MovePlane(mover->sector, mover->speed, floor, docrush, false, -dir);
            }

            if((mover->ceiling || follows) &&
               !FEQUAL(P_GetDoublep(mover->sector, DMU_CEILING_HEIGHT), ceil))
            {
                T_MovePlane(mover->sector, mover->speed, ceil, docrush, true, -dir);
            }

            XS_MoverStopped(mover, false);
        }
    }
}

typedef struct {
    Sector*             sec;
    boolean             ceiling;
} stopplanemoverparams_t;

static int stopPlaneMover(thinker_t* th, void* context)
{
    stopplanemoverparams_t* params = (stopplanemoverparams_t*) context;
    xgplanemover_t*     mover = (xgplanemover_t *) th;

    if(mover->sector == params->sec &&
       mover->ceiling == params->ceiling)
    {
        XS_MoverStopped(mover, false);
        Thinker_Remove(th); // Remove it.
    }

    return false; // Continue iteration.
}

/**
 * Returns a new thinker for handling the specified plane. Removes any
 * existing thinkers associated with the plane.
 */
xgplanemover_t *XS_GetPlaneMover(Sector *sec, boolean ceiling)
{
    xgplanemover_t*     mover;
    stopplanemoverparams_t params;

    params.sec = sec;
    params.ceiling = ceiling;
    Thinker_Iterate(XS_PlaneMover, stopPlaneMover, &params);

    // Allocate a new thinker.
    mover = Z_Calloc(sizeof(*mover), PU_MAP, 0);
    mover->thinker.function = XS_PlaneMover;
    Thinker_Add(&mover->thinker);

    mover->sector = sec;
    mover->ceiling = ceiling;

    return mover;
}

void XS_ChangePlaneMaterial(Sector *sector, boolean ceiling,
                            material_t* mat, float *rgb)
{
    XG_Dev("XS_ChangePlaneMaterial: Sector %i, %s, texture %i",
           P_ToIndex(sector), (ceiling ? "ceiling" : "floor"), P_ToIndex(mat));
    if(rgb)
        XG_Dev("red %g, green %g, blue %g", rgb[0], rgb[1], rgb[2]);

    if(ceiling)
    {
        if(rgb)
            P_SetFloatpv(sector, DMU_CEILING_COLOR, rgb);

        if(mat)
            P_SetPtrp(sector, DMU_CEILING_MATERIAL, mat);
    }
    else
    {
        if(rgb)
            P_SetFloatpv(sector, DMU_FLOOR_COLOR, rgb);

        if(mat)
            P_SetPtrp(sector, DMU_FLOOR_MATERIAL, mat);
    }
}

uint FindMaxOf(int *list, uint num)
{
    uint            i, idx = 0;
    int             max = list[0];

    for(i = 1; i < num; ++i)
    {
        if(list[i] > max)
        {
            max = list[i];
            idx = i;
        }
    }

    return idx;
}

uint FindMinOf(int *list, uint num)
{
    uint        i, idx = 0;
    int         min = list[0];

    for(i = 1; i < num; ++i)
    {
        if(list[i] < min)
        {
            min = list[i];
            idx = i;
        }
    }

    return idx;
}

int FindNextOf(int *list, int num, int h)
{
    int         i, min = 0, idx = -1;

    for(i = 0; i < num; ++i)
    {
        if(list[i] <= h)
            continue;

        if(idx < 0 || list[i] < min)
        {
            idx = i;
            min = list[i];
        }
    }

    return idx;
}

int FindPrevOf(int *list, int num, int h)
{
    int         i, max = 0, idx = -1;

    for(i = 0; i < num; ++i)
    {
        if(list[i] >= h)
            continue;

        if(idx < 0 || list[i] > max)
        {
            idx = i;
            max = list[i];
        }
    }

    return idx;
}

/**
 * Really an XL_* function!
 *
 * @param part          1=mid, 2=top, 3=bottom.
 *
 * @return              @c MAXINT if not height n/a.
 */
int XS_TextureHeight(LineDef* line, int part)
{
    SideDef* side;
    int snum = 0;
    int minfloor = 0, maxfloor = 0, maxceil = 0;
    Sector* front = P_GetPtrp(line, DMU_FRONT_SECTOR);
    Sector* back = P_GetPtrp(line, DMU_BACK_SECTOR);
    boolean twosided = front && back;
    material_t* mat;

    if(part != LWS_MID && !twosided)
        return DDMAXINT;

    if(twosided)
    {
        int ffloor = P_GetIntp(front, DMU_FLOOR_HEIGHT);
        int fceil  = P_GetIntp(front, DMU_CEILING_HEIGHT);
        int bfloor = P_GetIntp(back,  DMU_FLOOR_HEIGHT);
        int bceil  = P_GetIntp(back,  DMU_CEILING_HEIGHT);

        minfloor = ffloor;
        maxfloor = bfloor;
        if(part == LWS_LOWER)
            snum = 0;
        if(bfloor < minfloor)
        {
            minfloor = bfloor;
            maxfloor = ffloor;
            if(part == LWS_LOWER)
                snum = 1;
        }
        maxceil = fceil;
        if(part == LWS_UPPER)
            snum = 0;
        if(bceil > maxceil)
        {
            maxceil = bceil;
            if(part == LWS_UPPER)
                snum = 1;
        }
    }
    else
    {
        if(P_GetPtrp(line, DMU_SIDEDEF0))
            snum = 0;
        else
            snum = 1;
    }

    // Which side are we working with?
    if(snum == 0)
        side = P_GetPtrp(line, DMU_SIDEDEF0);
    else
        side = P_GetPtrp(line, DMU_SIDEDEF1);

    // Which section of the wall?
    switch(part)
    {
    case LWS_UPPER:
        if((mat = P_GetPtrp(side, DMU_TOP_MATERIAL)))
            return maxceil - P_GetIntp(mat, DMU_HEIGHT);
        break;

    case LWS_MID:
        if((mat = P_GetPtrp(side, DMU_MIDDLE_MATERIAL)))
            return maxfloor + P_GetIntp(mat, DMU_HEIGHT);
        break;

    case LWS_LOWER:
        if((mat = P_GetPtrp(side, DMU_BOTTOM_MATERIAL)))
            return minfloor + P_GetIntp(mat, DMU_HEIGHT);
        break;

    default:
        Con_Error("XS_TextureHeight: Invalid wall section %d.", part);
    }

    return DDMAXINT;
}

/**
 * Returns a pointer to the first sector with the tag.
 *
 * NOTE: We cannot use the tagged sector lists here as this can be called
 * during an iteration at a higher level. Doing so would change the position
 * of the rover which would affect the other iteration.
 *
 * NOTE2: Re-above, obviously that is bad design and should be addressed.
 */
Sector *XS_FindTagged(int tag)
{
    uint        k;
    uint        foundcount = 0;
    uint        retsectorid = 0;
    Sector     *sec, *retsector;

    retsector = NULL;

    for(k = 0; k < numsectors; ++k)
    {
        sec = P_ToPtr(DMU_SECTOR, k);
        if(P_ToXSector(sec)->tag == tag)
        {
            if(xgDev)
            {
                if(foundcount == 0)
                {
                    retsector = sec;
                    retsectorid = k;
                }
            }
            else
                return sec;

            foundcount++;
        }
    }

    if(xgDev)
    {
        if(foundcount > 1)
        {
            XG_Dev("XS_FindTagged: More than one sector exists with this tag (%i)!",tag);
            XG_Dev("  The sector with the lowest ID (%i) will be used.", retsectorid);
        }

        if(retsector)
            return retsector;
    }

    return NULL;
}

/**
 * Returns a pointer to the first sector with the specified act tag.
 */
Sector *XS_FindActTagged(int tag)
{
    uint        k;
    uint        foundcount = 0;
    uint        retsectorid = 0;
    Sector     *sec, *retsector;
    xsector_t  *xsec;

    retsector = NULL;

    for(k = 0; k < numsectors; ++k)
    {
        sec = P_ToPtr(DMU_SECTOR, k);
        xsec = P_ToXSector(sec);
        if(xsec->xg)
        {
            if(xsec->xg->info.actTag == tag)
            {
                if(xgDev)
                {
                    if(foundcount == 0)
                    {
                        retsector = sec;
                        retsectorid = k;
                    }
                }
                else
                {
                    return sec;
                }

                foundcount++;
            }
        }
    }

    if(xgDev)
    {
        if(foundcount > 1)
        {
            XG_Dev("XS_FindActTagged: More than one sector exists with this ACT tag (%i)!",tag);
            XG_Dev("  The sector with the lowest ID (%i) will be used.", retsectorid);
        }

        if(retsector)
            return retsector;
    }

    return NULL;
}

#define FSETHF_MIN          0x1 // Get min. If not set, get max.

typedef struct findsectorextremaltextureheightparams_s {
    Sector* baseSec;
    byte flags;
    int part;
    coord_t val;
} findsectorextremalmaterialheightparams_t;

int findSectorExtremalMaterialHeight(void* ptr, void* context)
{
    LineDef* li = (LineDef*) ptr;
    findsectorextremalmaterialheightparams_t* params =
        (findsectorextremalmaterialheightparams_t*) context;
    coord_t height;

    // The heights are in real world coordinates.
    height = XS_TextureHeight(li, params->part);
    if(params->flags & FSETHF_MIN)
    {
        if(height < params->val)
            params->val = height;
    }
    else
    {
        if(height > params->val)
            params->val = height;
    }

    return false; // Continue iteration.
}

boolean XS_GetPlane(LineDef* actline, Sector* sector, int ref, uint* refdata,
    coord_t* height, material_t** mat, Sector** planeSector)
{
    material_t* otherMat;
    coord_t otherHeight;
    Sector* otherSec = NULL, *iter;
    xline_t* xline;
    char buff[50];

    if(refdata)
        sprintf(buff, " : %i", *refdata);

    if(xgDev)
    {
        XG_Dev("XS_GetPlane: Line %i, sector %i, ref (%s(%i)%s)",
               actline ? P_ToIndex(actline) : -1, P_ToIndex(sector),
               SPREFTYPESTR(ref), ref, refdata? buff : "" );
    }

    if(ref == SPREF_NONE || ref == SPREF_SPECIAL)
    {
        // No reference to anywhere.
        return false;
    }

    // Init the values to the current sector's floor.
    if(height)
        *height = P_GetDoublep(sector, DMU_FLOOR_HEIGHT);
    if(mat)
        *mat = (material_t*) P_GetPtrp(sector, DMU_FLOOR_MATERIAL);
    if(planeSector)
        *planeSector = sector;

    // First try the non-comparative, iterative sprefs.
    iter = NULL;
    switch(ref)
    {
    case SPREF_SECTOR_TAGGED_FLOOR:
    case SPREF_SECTOR_TAGGED_CEILING:
        iter = XS_FindTagged(P_ToXSector(sector)->tag);
        if(!iter)
            return false;
        break;

    case SPREF_LINE_TAGGED_FLOOR:
    case SPREF_LINE_TAGGED_CEILING:
        if(!actline)
            return false;

        iter = XS_FindTagged(P_ToXLine(actline)->tag);
        if(!iter)
            return false;
        break;

    case SPREF_TAGGED_FLOOR:
    case SPREF_TAGGED_CEILING:
        if(!refdata)
        {
            XG_Dev("  %s IS NOT VALID FOR THIS CLASS PARAMETER!", SPREFTYPESTR(ref));
            return false;
        }

        iter = XS_FindTagged(*refdata);
        if(!iter)
            return false;
        break;

    case SPREF_LINE_ACT_TAGGED_FLOOR:
    case SPREF_LINE_ACT_TAGGED_CEILING:
        xline = P_ToXLine(actline);

        if(!xline)
            return false;

        if(!xline->xg)
        {
            XG_Dev("  ACT LINE IS NOT AN XG LINE!");
            return false;
        }
        if(!xline->xg->info.actTag)
        {
            XG_Dev("  ACT LINE DOES NOT HAVE AN ACT TAG!");
            return false;
        }

        iter = XS_FindActTagged(xline->xg->info.actTag);
        if(!iter)
            return false;
        break;

    case SPREF_ACT_TAGGED_FLOOR:
    case SPREF_ACT_TAGGED_CEILING:
        if(!refdata)
        {
            XG_Dev("  %s IS NOT VALID FOR THIS CLASS PARAMETER!", SPREFTYPESTR(ref));
            return false;
        }

        iter = XS_FindActTagged(*refdata);
        if(!iter)
            return false;
        break;

    case SPREF_INDEX_FLOOR:
    case SPREF_INDEX_CEILING:
        if(!refdata || *refdata >= numsectors)
            return false;
        iter = P_ToPtr(DMU_SECTOR, *refdata);
        break;

    default:
        // No iteration.
        break;
    }

    // Did we find the plane through iteration?
    if(iter)
    {
        if(planeSector)
            *planeSector = iter;
        if((ref >= SPREF_SECTOR_TAGGED_FLOOR && ref <= SPREF_INDEX_FLOOR) ||
            ref == SPREF_LINE_ACT_TAGGED_FLOOR)
        {
            if(height)
                *height = P_GetDoublep(iter, DMU_FLOOR_HEIGHT);
            if(mat)
                *mat = (material_t*) P_GetPtrp(iter, DMU_FLOOR_MATERIAL);
        }
        else
        {
            if(height)
                *height = P_GetDoublep(iter, DMU_CEILING_HEIGHT);
            if(mat)
                *mat = (material_t*) P_GetPtrp(iter, DMU_CEILING_MATERIAL);
        }

        return true;
    }

    if(ref == SPREF_MY_FLOOR)
    {
        Sector* frontsector;

        if(!actline)
            return false;

        frontsector = P_GetPtrp(actline, DMU_FRONT_SECTOR);

        if(!frontsector)
            return false;

        // Actline's front floor.
        if(height)
            *height = P_GetDoublep(frontsector, DMU_FLOOR_HEIGHT);
        if(mat)
            *mat = (material_t*) P_GetPtrp(frontsector, DMU_FLOOR_MATERIAL);
        if(planeSector)
            *planeSector = frontsector;
        return true;
    }

    if(ref == SPREF_BACK_FLOOR)
    {
        Sector* backsector;

        if(!actline)
            return false;

        backsector = P_GetPtrp(actline, DMU_BACK_SECTOR);

        if(!backsector)
            return false;

        // Actline's back floor.
        if(height)
            *height = P_GetDoublep(backsector, DMU_FLOOR_HEIGHT);
        if(mat)
            *mat = (material_t*) P_GetPtrp(backsector, DMU_FLOOR_MATERIAL);
        if(planeSector)
            *planeSector = backsector;

        return true;
    }

    if(ref == SPREF_MY_CEILING)
    {
        Sector* frontsector;

        if(!actline)
            return false;

        frontsector = P_GetPtrp(actline, DMU_FRONT_SECTOR);

        if(!frontsector)
            return false;

        // Actline's front ceiling.
        if(height)
            *height = P_GetDoublep(frontsector, DMU_CEILING_HEIGHT);
        if(mat)
            *mat = (material_t*) P_GetPtrp(frontsector, DMU_CEILING_MATERIAL);
        if(planeSector)
            *planeSector = frontsector;
        return true;
    }

    if(ref == SPREF_BACK_CEILING)
    {
        Sector* backsector;

        if(!actline)
            return false;

        backsector = P_GetPtrp(actline, DMU_BACK_SECTOR);

        if(!backsector)
            return false;

        // Actline's back ceiling.
        if(height)
            *height = P_GetDoublep(backsector, DMU_CEILING_HEIGHT);
        if(mat)
            *mat = (material_t*) P_GetPtrp(backsector, DMU_CEILING_MATERIAL);
        if(planeSector)
            *planeSector = backsector;
        return true;
    }

    if(ref == SPREF_ORIGINAL_FLOOR)
    {
        if(height)
            *height = P_ToXSector(sector)->SP_floororigheight;
        if(mat)
            *mat = (material_t*) P_GetPtrp(sector, DMU_FLOOR_MATERIAL);
        return true;
    }

    if(ref == SPREF_ORIGINAL_CEILING)
    {
        if(height)
            *height = P_ToXSector(sector)->SP_ceilorigheight;
        if(mat)
            *mat = (material_t*) P_GetPtrp(sector, DMU_CEILING_MATERIAL);
        return true;
    }

    if(ref == SPREF_CURRENT_FLOOR)
    {
        if(height)
            *height = P_GetDoublep(sector, DMU_FLOOR_HEIGHT);
        if(mat)
            *mat = (material_t*) P_GetPtrp(sector, DMU_FLOOR_MATERIAL);
        return true;
    }

    if(ref == SPREF_CURRENT_CEILING)
    {
        if(height)
            *height = P_GetDoublep(sector, DMU_CEILING_HEIGHT);
        if(mat)
            *mat = (material_t*) P_GetPtrp(sector, DMU_CEILING_MATERIAL);
        return true;
    }

    // Texture height targets?
    if(ref >= SPREF_MIN_BOTTOM_MATERIAL && ref <= SPREF_MAX_TOP_MATERIAL)
    {
        int part;
        boolean findMin;
        findsectorextremalmaterialheightparams_t params;

        // Which part of the wall are we looking at?
        if(ref == SPREF_MIN_MID_MATERIAL || ref == SPREF_MAX_MID_MATERIAL)
            part = LWS_MID;
        else if(ref == SPREF_MIN_TOP_MATERIAL || ref == SPREF_MAX_TOP_MATERIAL)
            part = LWS_UPPER;
        else // Then it's the bottom.
            part = LWS_LOWER;

        if(ref >= SPREF_MIN_BOTTOM_MATERIAL && ref <= SPREF_MIN_TOP_MATERIAL)
            findMin = true;
        else
            findMin = false;

        params.baseSec = sector;
        params.part = part;
        params.flags = (findMin? FSETHF_MIN : 0);
        params.val = (findMin? DDMAXFLOAT : DDMINFLOAT);
        P_Iteratep(sector, DMU_LINEDEF, &params,
                   findSectorExtremalMaterialHeight);
        if(height)
            *height = params.val;

        return true;
    }

    // Get the right height and pic.
    if(ref == SPREF_HIGHEST_CEILING)
    {
        otherSec =
            P_FindSectorSurroundingHighestCeiling(sector, DDMINFLOAT, &otherHeight);
        if(otherSec)
            otherMat = P_GetPtrp(otherSec, DMU_CEILING_MATERIAL);
    }
    else if(ref == SPREF_HIGHEST_FLOOR)
    {
        otherSec =
            P_FindSectorSurroundingHighestFloor(sector, DDMINFLOAT, &otherHeight);
        if(otherSec)
            otherMat = P_GetPtrp(otherSec, DMU_CEILING_MATERIAL);
    }
    else if(ref == SPREF_LOWEST_CEILING)
    {
        otherSec =
            P_FindSectorSurroundingLowestCeiling(sector, DDMAXFLOAT, &otherHeight);
        if(otherSec)
            otherMat = P_GetPtrp(otherSec, DMU_CEILING_MATERIAL);
    }
    else if(ref == SPREF_LOWEST_FLOOR)
    {
        otherSec =
            P_FindSectorSurroundingLowestFloor(sector, DDMAXFLOAT, &otherHeight);
        if(otherSec)
            otherMat = P_GetPtrp(otherSec, DMU_FLOOR_MATERIAL);
    }
    else if(ref == SPREF_NEXT_HIGHEST_CEILING)
    {
        otherSec =
            P_FindSectorSurroundingNextHighestCeiling(sector,
                P_GetDoublep(sector, DMU_CEILING_HEIGHT), &otherHeight);
        if(otherSec)
            otherMat = P_GetPtrp(otherSec, DMU_CEILING_MATERIAL);
    }
    else if(ref == SPREF_NEXT_HIGHEST_FLOOR)
    {
        otherSec =
            P_FindSectorSurroundingNextHighestFloor(sector,
                P_GetDoublep(sector, DMU_FLOOR_HEIGHT), &otherHeight);
        if(otherSec)
            otherMat = P_GetPtrp(otherSec, DMU_FLOOR_MATERIAL);
    }
    else if(ref == SPREF_NEXT_LOWEST_CEILING)
    {
        otherSec =
            P_FindSectorSurroundingNextLowestCeiling(sector,
                P_GetDoublep(sector, DMU_CEILING_HEIGHT), &otherHeight);
        if(otherSec)
            otherMat = P_GetPtrp(otherSec, DMU_CEILING_MATERIAL);
    }
    else if(ref == SPREF_NEXT_LOWEST_FLOOR)
    {
        otherSec =
            P_FindSectorSurroundingNextLowestFloor(sector,
                P_GetDoublep(sector, DMU_FLOOR_HEIGHT), &otherHeight);
        if(otherSec)
            otherMat = P_GetPtrp(otherSec, DMU_FLOOR_MATERIAL);
    }

    // The requested plane was not found.
    if(!otherSec)
        return false;

    // Set the values.
    if(height)
        *height = otherHeight;
    if(mat)
        *mat = otherMat;
    if(planeSector)
        *planeSector = otherSec;

    return true;
}

/**
 * DJS - Why find the highest??? Surely unlogical to mod authors.
 * IMO if a user references multiple sectors, the one with the lowest ID
 * should be chosen (the same way it works for FIND(act)TAGGED). If that
 * happens to be zero - so be it.
 */
int C_DECL XSTrav_HighestSectorType(Sector *sec, boolean ceiling,
                                    void *context, void *context2,
                                    mobj_t *activator)
{
    int        *type = context2;
    xsector_t  *xsec = P_ToXSector(sec);

    if(xsec->special > *type)
        *type = xsec->special;

    return true; // Keep looking...
}

void XS_InitMovePlane(LineDef *line)
{
    xline_t *xline = P_ToXLine(line);

    // fdata keeps track of wait time.
    xline->xg->fdata = xline->xg->info.fparm[5];
    xline->xg->idata = true; // Play sound.
};

int C_DECL XSTrav_MovePlane(Sector *sector, boolean ceiling, void *context,
                            void *context2, mobj_t *activator)
{
    LineDef*        line = (LineDef *) context;
    linetype_t*     info = (linetype_t *) context2;
    xgplanemover_t* mover;
    int             st;
    material_t*     mat;
    xline_t*        xline = P_ToXLine(line);
    boolean         playsound;

    playsound = xline->xg->idata;

    XG_Dev("XSTrav_MovePlane: Sector %i (by line %i of type %i)",
           P_ToIndex(sector), P_ToIndex(line), info->id);

    // i2: destination type (zero, relative to current, surrounding
    //     highest/lowest floor/ceiling)
    // i3: flags (PMF_*)
    // i4: start sound
    // i5: end sound
    // i6: move sound
    // i7: start material origin (uses same ids as i2)
    // i8: start material index (used with PMD_ZERO).
    // i9: end material origin (uses same ids as i2)
    // i10: end material (used with PMD_ZERO)
    // i11 + i12: (plane ref) start sector type
    // i13 + i14: (plane ref) end sector type
    // f0: move speed (units per tic).
    // f1: crush speed (units per tic).
    // f2: destination offset
    // f3: move sound min interval (seconds)
    // f4: move sound max interval (seconds)
    // f5: time to wait before starting the move
    // f6: wait increment for each plane that gets moved

    mover = XS_GetPlaneMover(sector, ceiling);
    mover->origin = line;

    // Setup the thinker and add it to the list.
    {
    coord_t temp = mover->destination;
    XS_GetPlane(line, sector, info->iparm[2], NULL, &temp, 0, 0);
    mover->destination = temp + info->fparm[2];
    }
    mover->speed = info->fparm[0];
    mover->crushSpeed = info->fparm[1];
    mover->minInterval = FLT2TIC(info->fparm[3]);
    mover->maxInterval = FLT2TIC(info->fparm[4]);
    mover->flags = info->iparm[3];
    mover->endSound = playsound ? info->iparm[5] : 0;
    mover->moveSound = playsound ? info->iparm[6] : 0;

    // Change texture at end?
    if(info->iparm[9] == SPREF_NONE || info->iparm[9] == SPREF_SPECIAL)
        mover->setMaterial = P_ToPtr(DMU_MATERIAL, info->iparm[10]);
    else
    {
        if(!XS_GetPlane(line, sector, info->iparm[9], NULL, 0, &mover->setMaterial, 0))
            XG_Dev("  Couldn't find suitable material to set when move ends!");
    }

    // Init timer.
    mover->timer = XG_RandomInt(mover->minInterval, mover->maxInterval);

    // Do we need to wait before starting the move?
    if(xline->xg->fdata > 0)
    {
        mover->timer = FLT2TIC(xline->xg->fdata);
        mover->flags |= PMF_WAIT;
    }

    // Increment wait time.
    xline->xg->fdata += info->fparm[6];

    // Do start stuff. Play sound?
    if(playsound)
        XS_PlaneSound(P_GetPtrp(sector, ceiling? DMU_CEILING_PLANE : DMU_FLOOR_PLANE),
                      info->iparm[4]);

    // Change texture at start?
    if(info->iparm[7] == SPREF_NONE || info->iparm[7] == SPREF_SPECIAL)
        mat = P_ToPtr(DMU_MATERIAL, info->iparm[8]);
    else
    {
        if(!XS_GetPlane(line, sector, info->iparm[7], NULL, 0, &mat, 0))
            XG_Dev("  Couldn't find suitable material to set when move starts!");
    }
    if(mat)
        XS_ChangePlaneMaterial(sector, ceiling, mat, NULL);

    // Should we play no more sounds?
    if(info->iparm[3] & PMF_ONE_SOUND_ONLY)
    {
        // Sound was played only for the first plane.
        xline->xg->idata = false;
    }

    // Change sector type right now?
    st = info->iparm[12];
    if(info->iparm[11] != LPREF_NONE)
    {
        if(XL_TraversePlanes
           (line, info->iparm[11], info->iparm[12], 0, &st, false, 0,
            XSTrav_HighestSectorType))
        {
            XS_SetSectorType(sector, st);
        }
        else
            XG_Dev("  SECTOR TYPE NOT SET (nothing referenced)");
    }

    // Change sector type in the end of move?
    st = info->iparm[14];
    if(info->iparm[13] != LPREF_NONE)
    {
        if(XL_TraversePlanes
           (line, info->iparm[13], info->iparm[14], 0, &st, false, 0,
            XSTrav_HighestSectorType))
        {   // OK, found one or more.
            mover->setSectorType = st;
        }
        else
        {
            XG_Dev("  SECTOR TYPE WON'T BE CHANGED AT END (nothing referenced)");
            mover->setSectorType = -1;
        }
    }
    else
        mover->setSectorType = -1;

    return true; // Keep looking...
}

void XS_InitStairBuilder(LineDef* line)
{
    uint i;
    for(i = 0; i < numsectors; ++i)
    {
        P_GetXSector(i)->blFlags = 0;
    }
}

boolean XS_DoBuild(Sector* sector, boolean ceiling, LineDef* origin,
                   linetype_t* info, uint stepcount)
{
    static coord_t firstheight;

    float waittime;
    xsector_t* xsec;
    xgplanemover_t* mover;

    if(!sector)
        return false;

    xsec = P_ToXSector(sector);

    // Make sure each sector is only processed once.
    if(xsec->blFlags & BL_BUILT)
        return false; // Already built this one!
    xsec->blFlags |= BL_WAS_BUILT;

    // Create a new mover for the plane.
    mover = XS_GetPlaneMover(sector, ceiling);
    mover->origin = origin;

    // Setup the mover.
    if(stepcount != 0)
        firstheight = P_GetDoublep(sector, (ceiling? DMU_CEILING_HEIGHT:DMU_FLOOR_HEIGHT));

    mover->destination =
        firstheight + (stepcount + 1) * info->fparm[1];

    mover->speed = (info->fparm[0] + stepcount * info->fparm[6]);
    if(mover->speed <= 0)
        mover->speed = 1 / 1000;

    mover->minInterval = FLT2TIC(info->fparm[4]);
    mover->maxInterval = FLT2TIC(info->fparm[5]);

    if(info->iparm[8])
        mover->flags = PMF_CRUSH;

    mover->endSound = info->iparm[6];
    mover->moveSound = info->iparm[7];

    // Wait before starting?
    waittime = info->fparm[2] + info->fparm[3] * stepcount;
    if(waittime > 0)
    {
        mover->timer = FLT2TIC(waittime);
        mover->flags |= PMF_WAIT;
        // Play start sound when waiting ends.
        mover->startSound = info->iparm[5];
    }
    else
    {
        mover->timer = XG_RandomInt(mover->minInterval, mover->maxInterval);
        // Play step start sound immediately.
        XS_PlaneSound(P_GetPtrp(sector, ceiling? DMU_CEILING_PLANE : DMU_FLOOR_PLANE),
                      info->iparm[5]);
    }

    // Do start stuff. Play sound?
    if(stepcount != 0)
    {
        // Start building start sound.
        XS_PlaneSound(P_GetPtrp(sector, ceiling? DMU_CEILING_PLANE : DMU_FLOOR_PLANE),
                      info->iparm[4]);
    }

    return true; // Building has begun!
}

#define F_MATERIALSTOP          0x1
#define F_CEILING               0x2

typedef struct spreadbuildparams_s {
    Sector*             baseSec;
    material_t*         baseMat;
    byte                flags;
    LineDef*            origin;
    linetype_t*         info;
    int                 stepCount;
    size_t              spreaded;
} spreadbuildparams_t;

int spreadBuild(void *ptr, void *context)
{
    LineDef             *li = (LineDef*) ptr;
    spreadbuildparams_t *params = (spreadbuildparams_t*) context;
    Sector              *frontSec, *backSec;

    frontSec = P_GetPtrp(li, DMU_FRONT_SECTOR);
    if(!frontSec || frontSec != params->baseSec)
        return false;

    backSec = P_GetPtrp(li, DMU_BACK_SECTOR);
    if(!backSec)
        return false;

    if(params->flags & F_MATERIALSTOP)
    {   // Planepic must match.
        if(params->flags & F_CEILING)
        {
            if(P_GetPtrp(params->baseSec, DMU_CEILING_MATERIAL) !=
               params->baseMat)
                return false;
        }
        else
        {
            if(P_GetPtrp(params->baseSec, DMU_FLOOR_MATERIAL) !=
               params->baseMat)
                return false;
        }
    }

    // Don't spread to sectors which have already spreaded.
    if(P_ToXSector(backSec)->blFlags & BL_SPREADED)
        return false;

    // Build backsector.
    XS_DoBuild(backSec, ((params->flags & F_CEILING)? true : false),
               params->origin, params->info, params->stepCount);
    params->spreaded++;

    return false; // Continue iteration.
}

static void markBuiltSectors(void)
{
    uint                i;

    // Mark the sectors of the last step as processed.
    for(i = 0; i < numsectors; ++i)
    {
        xsector_t*          xsec = P_GetXSector(i);

        if(xsec->blFlags & BL_WAS_BUILT)
        {
            xsec->blFlags &= ~BL_WAS_BUILT;
            xsec->blFlags |= BL_BUILT;
        }
    }
}

static boolean spreadBuildToNeighborAll(LineDef* origin, linetype_t* info,
                                        boolean picstop, boolean ceiling,
                                        material_t* myMat, int stepCount)
{
    uint                i;
    boolean             result = false;
    spreadbuildparams_t params;

    params.baseMat = myMat;
    params.info = info;
    params.origin = origin;
    params.stepCount = stepCount;
    params.flags = 0;
    if(picstop)
        params.flags |= F_MATERIALSTOP;
    if(ceiling)
        params.flags |= F_CEILING;

    for(i = 0; i < numsectors; ++i)
    {
        Sector*             sec;
        xsector_t*          xsec = P_GetXSector(i);

        // Only spread from built sectors (spread only once!).
        if(!(xsec->blFlags & BL_BUILT) || xsec->blFlags & BL_SPREADED)
            continue;

        xsec->blFlags |= BL_SPREADED;

        // Any 2-sided lines facing the right way?
        sec = P_ToPtr(DMU_SECTOR, i);

        params.baseSec = sec;
        params.spreaded = 0;

        P_Iteratep(sec, DMU_LINEDEF, &params, spreadBuild);
        if(params.spreaded > 0)
            result = true;
    }

    return result;
}

#define F_MATERIALSTOP          0x1
#define F_CEILING               0x2

typedef struct findbuildneighborparams_s {
    Sector*             baseSec;
    material_t*         baseMat;
    byte                flags;
    LineDef*            origin;
    linetype_t*         info;
    int                 stepCount;
    uint                foundIDX;
    Sector*             foundSec;
} findbuildneighborparams_t;

int findBuildNeighbor(void* ptr, void* context)
{
    LineDef*            li = (LineDef*) ptr;
    findbuildneighborparams_t *params =
        (findbuildneighborparams_t*) context;
    Sector*             frontSec, *backSec;
    uint                idx;

    frontSec = P_GetPtrp(li, DMU_FRONT_SECTOR);
    if(!frontSec || frontSec != params->baseSec)
        return false;

    backSec = P_GetPtrp(li, DMU_BACK_SECTOR);
    if(!backSec)
        return false;

    if(params->flags & F_MATERIALSTOP)
    {   // Planepic must match.
        if(params->flags & F_CEILING)
        {
            if(P_GetPtrp(params->baseSec, DMU_CEILING_MATERIAL) !=
               params->baseMat)
                return false;
        }
        else
        {
            if(P_GetPtrp(params->baseSec, DMU_FLOOR_MATERIAL) !=
               params->baseMat)
                return false;
        }
    }

    // Don't spread to sectors which have already spreaded.
    if(P_ToXSector(backSec)->blFlags & BL_SPREADED)
        return false;

    // We need the lowest line number.
    idx = P_ToIndex(li);
    if(idx < params->foundIDX)
    {
        params->foundSec = backSec;
        params->foundIDX = idx;
    }

    return false; // Continue iteration.
}

boolean spreadBuildToNeighborLowestIDX(LineDef* origin, linetype_t* info,
                                       boolean picstop, boolean ceiling,
                                       material_t* myMat, int stepcount,
                                       Sector* foundSec)
{
    uint                i;
    boolean             result = false;
    findbuildneighborparams_t params;

    params.baseMat = myMat;
    params.info = info;
    params.origin = origin;
    params.stepCount = stepcount;
    params.flags = 0;
    if(picstop)
        params.flags |= F_MATERIALSTOP;
    if(ceiling)
        params.flags |= F_CEILING;

    for(i = 0; i < numsectors; ++i)
    {
        Sector*             sec;
        xsector_t*          xsec = P_GetXSector(i);

        // Only spread from built sectors (spread only once!).
        if(!(xsec->blFlags & BL_BUILT) || xsec->blFlags & BL_SPREADED)
            continue;

        xsec->blFlags |= BL_SPREADED;

        // Any 2-sided lines facing the right way?
        sec = P_ToPtr(DMU_SECTOR, i);

        params.baseSec = sec;
        params.foundIDX = numlines;
        params.foundSec = NULL;

        P_Iteratep(sec, DMU_LINEDEF, &params, findBuildNeighbor);

        if(params.foundSec)
        {
            result = true;
            foundSec = params.foundSec;
        }
    }

    return result;
}

int C_DECL XSTrav_BuildStairs(Sector* sector, boolean ceiling,
                              void* context, void* context2,
                              mobj_t* activator)
{
    uint                stepCount = 0;
    LineDef*            origin = (LineDef *) context;
    linetype_t*         info = context2;
    Sector*             foundSec = NULL;
    boolean             picstop = info->iparm[2] != 0;
    boolean             spread = info->iparm[3] != 0;
    material_t*         myMat;

    XG_Dev("XSTrav_BuildStairs: Sector %i, %s", P_ToIndex(sector),
           ceiling ? "ceiling" : "floor");

    // i2: (true/false) stop when texture changes
    // i3: (true/false) spread build?

    myMat = ceiling ? P_GetPtrp(sector, DMU_CEILING_MATERIAL) :
                      P_GetPtrp(sector, DMU_FLOOR_MATERIAL);

    // Apply to first step.
    XS_DoBuild(sector, ceiling, origin, info, 0);
    stepCount++;

    if(spread)
    {
        boolean             found;

        do
        {
            markBuiltSectors();

            // Scan the sectors for the next ones to spread to.
            found = spreadBuildToNeighborAll(origin, info, picstop, ceiling,
                                             myMat, stepCount);
            stepCount++;
        } while(found);
    }
    else
    {
        boolean             found;

        do
        {
            found = false;
            markBuiltSectors();

            // Scan the sectors for the next ones to spread to.
            if(spreadBuildToNeighborLowestIDX(origin, info, picstop,
                                              ceiling, myMat, stepCount,
                                              foundSec))
            {
                XS_DoBuild(foundSec, ceiling, origin, info, stepCount);
                found = true;
            }

            stepCount++;
        } while(found);
    }

    return true; // Continue searching for planes...
}

int C_DECL XSTrav_SectorSound(Sector* sec, boolean ceiling, void* context,
    void* context2, mobj_t* activator)
{
    linetype_t* info = context2;

    /// @c 0= sector
    /// @c 1= floor plane
    /// @c 2= ceiling plane
    if(!info->iparm[3])
    {
        XS_SectorSound(sec, info->iparm[2]);
    }
    else
    {
        Plane* plane = P_GetPtrp(sec, info->iparm[3] == 2? DMU_CEILING_PLANE : DMU_FLOOR_PLANE);
        XS_PlaneSound(plane, info->iparm[2]);
    }

    return true;
}

int C_DECL XSTrav_PlaneMaterial(struct sector_s *sec, boolean ceiling,
                                void *context, void *context2,
                                mobj_t *activator)
{
    LineDef*        line = (LineDef *) context;
    linetype_t*     info = context2;
    material_t*     mat;
    float           rgb[3];

    // i2: (spref) material origin
    // i3: texture number (flat), used with SPREF_NONE
    // i4: red
    // i5: green
    // i6: blue
    if(info->iparm[2] == SPREF_NONE)
    {
        mat = P_ToPtr(DMU_MATERIAL, info->iparm[3]);
    }
    else
    {
        if(!XS_GetPlane(line, sec, info->iparm[2], NULL, 0, &mat, 0))
            XG_Dev("XSTrav_PlaneMaterial: Sector %i, couldn't find suitable material!",
                   P_ToIndex(sec));
    }

    rgb[0] = MINMAX_OF(0.f, info->iparm[4] / 255.f, 1.f);
    rgb[1] = MINMAX_OF(0.f, info->iparm[5] / 255.f, 1.f);
    rgb[2] = MINMAX_OF(0.f, info->iparm[6] / 255.f, 1.f);

    // Set the texture.
    XS_ChangePlaneMaterial(sec, ceiling, mat, rgb);

    return true;
}

int C_DECL XSTrav_SectorType(struct sector_s* sec, boolean ceiling,
                             void* context, void* context2,
                             mobj_t* activator)
{
    linetype_t*         info = context2;

    XS_SetSectorType(sec, info->iparm[2]);
    return true;
}

int C_DECL XSTrav_SectorLight(Sector* sector, boolean ceiling,
                              void* context, void* context2,
                              mobj_t* activator)
{
    LineDef*            line = (LineDef *) context;
    linetype_t*         info = context2;
    int                 num;
    float               usergb[3];
    float               lightLevel;

    // i2: (true/false) set level
    // i3: (true/false) set RGB
    // i4: source of light level (SSREF*)
    // i5: offset
    // i6: source of RGB (none, my, original)
    // i7: red offset
    // i8: green offset
    // i9: blue offset

    if(info->iparm[2])
    {
        switch(info->iparm[4])
        {
        case LIGHTREF_NONE:
            lightLevel = 0;
            break;

        case LIGHTREF_MY:
            {
            Sector             *frontSec =
                P_GetPtrp(line, DMU_FRONT_SECTOR);
            lightLevel = P_GetFloatp(frontSec, DMU_LIGHT_LEVEL);
            }
            break;

        case LIGHTREF_BACK:
            {
            Sector             *backSec = P_GetPtrp(line, DMU_BACK_SECTOR);
            if(backSec)
                lightLevel = P_GetFloatp(backSec, DMU_LIGHT_LEVEL);
            }
            break;

        case LIGHTREF_ORIGINAL:
            lightLevel = P_ToXSector(sector)->origLight;
            break;

        case LIGHTREF_CURRENT:
            lightLevel = P_GetFloatp(sector, DMU_LIGHT_LEVEL);
            break;

        case LIGHTREF_HIGHEST:
            P_FindSectorSurroundingHighestLight(sector, &lightLevel);
            break;

        case LIGHTREF_LOWEST:
            P_FindSectorSurroundingLowestLight(sector, &lightLevel);
            break;

        case LIGHTREF_NEXT_HIGHEST:
            {
            float               currentLevel =
                P_GetFloatp(sector, DMU_LIGHT_LEVEL);
            P_FindSectorSurroundingNextHighestLight(sector, currentLevel, &lightLevel);
            if(lightLevel < currentLevel)
                lightLevel = currentLevel;
            }
            break;

        case LIGHTREF_NEXT_LOWEST:
            {
            float               currentLevel =
                P_GetFloatp(sector, DMU_LIGHT_LEVEL);
            P_FindSectorSurroundingNextLowestLight(sector, currentLevel, &lightLevel);
            if(lightLevel > currentLevel)
                lightLevel = currentLevel;
            }
            break;

        default:
            break;
        }

        // Add the offset.
        lightLevel += (float) info->iparm[5] / 255.f;

        // Clamp the result.
        if(lightLevel < 0)
            lightLevel = 0;
        if(lightLevel > 1)
            lightLevel = 1;

        // Set the value.
        P_SetFloatp(sector, DMU_LIGHT_LEVEL, lightLevel);
    }

    if(info->iparm[3])
    {
        switch(info->iparm[6])
        {
        case LIGHTREF_MY:
            {
            Sector* sector = P_GetPtrp(line, DMU_FRONT_SECTOR);

            P_GetFloatpv(sector, DMU_COLOR, usergb);
            break;
            }
        case LIGHTREF_BACK:
            {
            Sector* sector = P_GetPtrp(line, DMU_BACK_SECTOR);

            if(sector)
                P_GetFloatpv(sector, DMU_COLOR, usergb);
            else
            {
                XG_Dev("XSTrav_SectorLight: Warning, the referenced LineDef has no back sector. Using default color.");
                memset(usergb, 0, sizeof(usergb));
            }
            break;
            }
        case LIGHTREF_ORIGINAL:
            memcpy(usergb, P_ToXSector(sector)->origRGB, sizeof(float) * 3);
            break;

        default:
            memset(usergb, 0, sizeof(usergb));
            break;
        }

        for(num = 0; num < 3; ++num)
        {
            float f = usergb[num] + (float) info->iparm[7 + num] / 255.f;
            if(f < 0)
                f = 0;
            if(f > 1)
                f = 1;
            P_SetFloatp(sector, TO_DMU_COLOR(num), f);
        }
    }

    return true;
}

int C_DECL XSTrav_MimicSector(Sector *sector, boolean ceiling,
                              void *context, void *context2,
                              mobj_t *activator)
{
    LineDef    *line = (LineDef *) context;
    linetype_t *info = context2;
    Sector     *from = NULL;
    uint        refdata;

    // Set the spref data parameter (tag or index).
    switch(info->iparm[2])
    {
    case SPREF_TAGGED_FLOOR:
    case SPREF_TAGGED_CEILING:
    case SPREF_INDEX_FLOOR:
    case SPREF_INDEX_CEILING:
    case SPREF_ACT_TAGGED_FLOOR:
    case SPREF_ACT_TAGGED_CEILING:
        if(info->iparm[3] >= 0)
            refdata = info->iparm[3];
        break;

    case SPREF_LINE_ACT_TAGGED_FLOOR:
    case SPREF_LINE_ACT_TAGGED_CEILING:
        if(info->actTag >= 0)
            refdata = info->actTag;
        break;

    default:
        refdata = 0;
        break;
    }

    // If can't apply to a sector, just skip it.
    if(!XS_GetPlane(line, sector, info->iparm[2], &refdata, 0, 0, &from))
    {
        XG_Dev("XSTrav_MimicSector: No suitable neighbor " "for %i.\n",
               P_ToIndex(sector));
        return true;
    }

    // Mimicing itself is pointless.
    if(from == sector)
        return true;

    XG_Dev("XSTrav_MimicSector: Sector %i mimicking sector %i",
           P_ToIndex(sector), P_ToIndex(from));

    // Copy the properties of the target sector.
    P_CopySector(sector, from);

    P_ChangeSector(sector, false);

    // Copy type as well.
    XS_SetSectorType(sector, P_ToXSector(from)->special);

    if(P_ToXSector(from)->xg)
        memcpy(P_ToXSector(sector)->xg, P_ToXSector(from)->xg, sizeof(xgsector_t));

    return true;
}

int C_DECL XSTrav_Teleport(Sector* sector, boolean ceiling, void* context,
                           void* context2, mobj_t* thing)
{
    mobj_t*         mo = NULL;
    boolean         ok = false;
    linetype_t*     info = context2;

    // Don't teleport things marked noteleport!
    if(thing->flags2 & MF2_NOTELEPORT)
    {
        XG_Dev("XSTrav_Teleport: Activator is unteleportable (THING type %i)",
                thing->type);
        return false;
    }

    for(mo = P_GetPtrp(sector, DMT_MOBJS); mo; mo = mo->sNext)
    {
        thinker_t *th = (thinker_t*) mo;

        // Not a mobj.
        if(th->function != P_MobjThinker)
            continue;

        // Not a teleportman.
        if(mo->type != MT_TELEPORTMAN)
            continue;

        ok = true;
        break;
    }

    if(ok)
    {   // We can teleport.
        mobj_t* flash;
        unsigned an;
        coord_t oldpos[3];
        coord_t thfloorz, thceilz;
        coord_t aboveFloor, fogDelta = 0;
        angle_t oldAngle;

        XG_Dev("XSTrav_Teleport: Sector %i, %s, %s%s", P_ToIndex(sector),
                info->iparm[2]? "No Flash":"", info->iparm[3]? "Play Sound":"Silent",
                info->iparm[4]? " Stomp" : "");

        if(!P_TeleportMove(thing, mo->origin[VX], mo->origin[VY], (info->iparm[4] > 0? 1 : 0)))
        {
            XG_Dev("XSTrav_Teleport: No free space at teleport exit. Aborting teleport...");
            return false;
        }

        memcpy(oldpos, thing->origin, sizeof(thing->origin));
        oldAngle = thing->angle;
        thfloorz = P_GetDoublep(thing->bspLeaf, DMU_FLOOR_HEIGHT);
        thceilz  = P_GetDoublep(thing->bspLeaf, DMU_CEILING_HEIGHT);
        aboveFloor = thing->origin[VZ] - thfloorz;

        // Players get special consideration
        if(thing->player)
        {
            if((thing->player->plr->mo->flags2 & MF2_FLY) && aboveFloor)
            {
                thing->origin[VZ] = thfloorz + aboveFloor;
                if(thing->origin[VZ] + thing->height > thceilz)
                {
                    thing->origin[VZ] = thceilz - thing->height;
                }
                thing->player->viewZ =
                    thing->origin[VZ] + thing->player->viewHeight;
            }
            else
            {
                thing->origin[VZ] = thfloorz;
                thing->player->viewZ =
                    thing->origin[VZ] + thing->player->viewHeight;
                thing->dPlayer->lookDir = 0; /* $unifiedangles */
            }
#if __JHERETIC__
            if(!thing->player->powers[PT_WEAPONLEVEL2])
#endif
            {
                // Freeze player for about .5 sec
                thing->reactionTime = 18;
            }

            //thing->dPlayer->clAngle = thing->angle; /* $unifiedangles */
            thing->dPlayer->flags |= DDPF_FIXANGLES | DDPF_FIXORIGIN | DDPF_FIXMOM;
        }
#if __JHERETIC__
        else if(thing->flags & MF_MISSILE)
        {
            thing->origin[VZ] = thfloorz + aboveFloor;
            if(thing->origin[VZ] + thing->height > thceilz)
            {
                thing->origin[VZ] = thceilz - thing->height;
            }
        }
#endif
        else
        {
            thing->origin[VZ] = thfloorz;
        }

        // Spawn flash at the old position?
        if(!info->iparm[2])
        {
            // Old position
#if __JHERETIC__
            fogDelta = ((thing->flags & MF_MISSILE)? 0 : TELEFOGHEIGHT);
#endif
            if((flash = P_SpawnMobjXYZ(MT_TFOG, oldpos[VX], oldpos[VY],
                                      oldpos[VZ] + fogDelta,
                                      oldAngle + ANG180, 0)))
            {
                // Play a sound?
                if(info->iparm[3])
                    S_StartSound(info->iparm[3], flash);
            }
        }

        an = mo->angle >> ANGLETOFINESHIFT;

        // Spawn flash at the new position?
        if(!info->iparm[2])
        {
            // New position
            if((flash = P_SpawnMobjXYZ(MT_TFOG,
                                      mo->origin[VX] + 20 * FIX2FLT(finecosine[an]),
                                      mo->origin[VY] + 20 * FIX2FLT(finesine[an]),
                                      mo->origin[VZ] + fogDelta, mo->angle, 0)))
            {
                // Play a sound?
                if(info->iparm[3])
                    S_StartSound(info->iparm[3], flash);
            }
        }

        // Adjust the angle to match that of the teleporter exit
        thing->angle = mo->angle;

        // Have we teleported from/to a sector with a non-solid floor?
        if(thing->flags2 & MF2_FLOORCLIP)
        {
            thing->floorClip = 0;

            if(FEQUAL(thing->origin[VZ], P_GetDoublep(thing->bspLeaf, DMU_FLOOR_HEIGHT)))
            {
                const terraintype_t* tt = P_MobjGetFloorTerrainType(thing);
                if(tt->flags & TTF_FLOORCLIP)
                {
                    thing->floorClip = 10;
                }
            }
        }

        if(thing->flags & MF_MISSILE)
        {
            an >>= ANGLETOFINESHIFT;
            thing->mom[MX] = thing->info->speed * FIX2FLT(finecosine[an]);
            thing->mom[MY] = thing->info->speed * FIX2FLT(finesine[an]);
        }
        else
        {
            thing->mom[MX] = thing->mom[MY] = thing->mom[MZ] = 0;
        }
    }
    else
    {   // Keep looking, there may be another referenced sector we could
        // teleport to...
        XG_Dev("XSTrav_Teleport: No teleport exit in referenced sector (ID %i)."
               " Continuing search...", P_ToIndex(sector));
        return true;
    }

    return false; // Only do this once.
}

int XF_FindRewindMarker(char *func, int pos)
{
    while(pos > 0 && func[pos] != '>')
        pos--;
    if(func[pos] == '>')
        pos++;

    return pos;
}

int XF_GetCount(function_t * fn, int *pos)
{
    char       *end;
    int         count;

    count = strtol(fn->func + *pos, &end, 10);
    *pos = end - fn->func;

    return count;
}

float XF_GetValue(function_t * fn, int pos)
{
    int         ch;

    if(fn->func[pos] == '/' || fn->func[pos] == '%')
    {
        // Exact value.
        return strtod(fn->func + pos + 1, 0);
    }

    ch = tolower(fn->func[pos]);
    // A=0, Z=25.
    return (ch - 'a') / 25.0f;
}

/**
 * Returns the position of the next value.
 * Repeat counting is handled here.
 * Poke should be true only if fn->pos is really about to move.
 */
int XF_FindNextPos(function_t *fn, int pos, boolean poke, Sector *sec)
{
    int         startpos = pos;
    int         c;
    char       *ptr;
    double      dvalue;

    if(fn->repeat > 0)
    {
        if(poke)
            fn->repeat--;
        return pos;
    }

    // Skip current.
    if(fn->func[pos] == '/' || fn->func[pos] == '%')
    {
        dvalue = strtod(fn->func + pos + 1, &ptr); // returned value ignored
        pos = ptr - fn->func; // Go to the end.
    }
    else
    {   // It's just a normal character [a-z,A-Z].
        pos++;
    }

    while(pos != startpos && fn->func[pos])
    {
        // Check for various special characters.
        if(isdigit(fn->func[pos]))
        {   // A repeat!
            // Move pos to the value to be repeated and set repeat counter.
            c = XF_GetCount(fn, &pos) - 1;
            if(poke)
                fn->repeat = c;
            return pos;
        }

        if(fn->func[pos] == '!')
        {   // Chain event.
            pos++;
            c = XF_GetCount(fn, &pos);
            if(poke)
            {
                // Sector funcs don't have activators.
                XS_DoChain(sec, XSCE_FUNCTION, c, NULL);
            }
            continue;
        }

        if(fn->func[pos] == '#')
        {   // Set timer.
            pos++;
            c = XF_GetCount(fn, &pos);
            if(poke)
            {
                fn->timer = 0;
                fn->maxTimer = c;
            }
            continue;
        }

        if(fn->func[pos] == '?')
        {   // Random timer.
            pos++;
            c = XF_GetCount(fn, &pos);
            if(poke)
            {
                fn->timer = 0;
                fn->maxTimer = XG_RandomInt(0, c);
            }
            continue;
        }

        if(fn->func[pos] == '<')
        {   // Rewind.
            pos = XF_FindRewindMarker(fn->func, pos);
            continue;
        }

        if(poke)
        {
            if(islower(fn->func[pos]) || fn->func[pos] == '/')
            {
                int     next = XF_FindNextPos(fn, pos, false, sec);

                if(fn->func[next] == '.')
                {
                    pos++;
                    continue;
                }
                break;
            }
        }
        else if(fn->func[pos] == '.')
        {
            break;
        }

        // Is it a value, then?
        if(isalpha(fn->func[pos]) || fn->func[pos] == '/' ||
           fn->func[pos] == '%')
            break;

        // A bad character, skip it.
        pos++;
    }

    return pos;
}

/**
 * Tick the function, update value.
 */
void XF_Ticker(function_t* fn, Sector* sec)
{
    int                 next;
    float               inter;

    // Store the previous value of the function.
    fn->oldValue = fn->value;

    // Is there a function?
    if(!ISFUNC(fn) || fn->link)
        return;

    // Increment time.
    if(fn->timer++ >= fn->maxTimer)
    {
        fn->timer = 0;
        fn->maxTimer = XG_RandomInt(fn->minInterval, fn->maxInterval);

        // Advance to next pos.
        fn->pos = XF_FindNextPos(fn, fn->pos, true, sec);
    }

    /**
     * Syntax:
     *
     * abcdefghijlkmnopqrstuvwxyz (26)
     *
     * az.< (fade from 0 to 1, break interpolation and repeat)
     * [note that AZ.AZ is the same as AZAZ]
     * [also note that a.z is the same as z]
     * az.>mz< (fade from 0 to 1, break, repeat fade from 0.5 to 1 to 0.5)
     * 10a10z is the same as aaaaaaaaaazzzzzzzzzz
     * aB or Ba do not interpolate
     * zaN (interpolate from 1 to 0, wait at 0, stay at N)
     * za.N (interpolate from 1 to 0, skip to N)
     * 1A is the same as A
     */

    // Stop?
    if(!fn->func[fn->pos])
        return;

    if(isupper(fn->func[fn->pos]) || fn->func[fn->pos] == '%')
    {   // No interpolation.
        fn->value = XF_GetValue(fn, fn->pos);
    }
    else
    {
        inter = 0;
        next = XF_FindNextPos(fn, fn->pos, false, sec);
        if(islower(fn->func[next]) || fn->func[next] == '/')
        {
            if(fn->maxTimer)
                inter = fn->timer / (float) fn->maxTimer;
        }

        fn->value = (1 - inter) * XF_GetValue(fn, fn->pos) +
            inter * XF_GetValue(fn, next);
    }

    // Scale and offset.
    fn->value = fn->value * fn->scale + fn->offset;
}

void XS_UpdatePlanes(Sector* sec)
{
    int i;
    xgsector_t* xg;
    function_t* fn;
    boolean docrush;

    xg = P_ToXSector(sec)->xg;
    docrush = (xg->info.flags & STF_CRUSH) != 0;

    // Update floor.
    fn = &xg->plane[XGSP_FLOOR];
    if(UPDFUNC(fn))
    {
        // Changed; How different?
        i = fn->value - P_GetFloatp(sec, DMU_FLOOR_HEIGHT);
        if(i)
        {
            // Move the floor plane accordingly.
            T_MovePlane(sec, abs(i), fn->value, docrush, 0, SIGN(i));
        }
    }

    // Update celing.
    fn = &xg->plane[XGSP_CEILING];
    if(UPDFUNC(fn))
    {
        // Changed; How different?
        i = fn->value - P_GetFloatp(sec, DMU_CEILING_HEIGHT);
        if(i)
        {
            // Move the ceiling accordingly.
            T_MovePlane(sec, abs(i), fn->value, docrush, 1, SIGN(i));
        }
    }
}

void XS_UpdateLight(Sector* sec)
{
    int                 i;
    float               c, lightlevel;
    xgsector_t*         xg;
    function_t*         fn;

    xg = P_ToXSector(sec)->xg;

    // Light intensity.
    fn = &xg->light;
    if(UPDFUNC(fn))
    {   // Changed.
        lightlevel = MINMAX_OF(0, fn->value / 255.f, 1);
        P_SetFloatp(sec, DMU_LIGHT_LEVEL, lightlevel);
    }

    // Red, green and blue.
    for(i = 0; i < 3; ++i)
    {
        fn = &xg->rgb[i];
        if(!UPDFUNC(fn))
            continue;

        // Changed.
        c = MINMAX_OF(0, fn->value / 255.f, 1);

        P_SetFloatp(sec, TO_DMU_COLOR(i), c);
    }
}

void XS_DoChain(Sector* sec, int ch, int activating, void* act_thing)
{
    xgsector_t* xg;
    sectortype_t* info;
    float flevtime = TIC2FLT(mapTime);
    LineDef* dummyLine;
    xline_t* xdummyLine;
    linetype_t* ltype;
    SideDef* dummySideDef;

    xg = P_ToXSector(sec)->xg;
    info = &xg->info;

    if(ch < XSCE_NUM_CHAINS)
    {
        // How's the counter?
        if(!info->count[ch])
            return;

        // How's the time?
        if(flevtime < info->start[ch] ||
           (info->end[ch] > 0 && flevtime > info->end[ch]))
            return; // Not operating at this time.

        // Time to try the chain. Reset timer.
        xg->chainTimer[ch] =
            XG_RandomInt(FLT2TIC(info->interval[ch][0]),
                         FLT2TIC(info->interval[ch][1]));
    }

    // Prepare the dummies to use for the event.
    dummyLine = P_AllocDummyLine();
    xdummyLine = P_ToXLine(dummyLine);
    xdummyLine->xg = Z_Calloc(sizeof(xgline_t), PU_MAP, 0);
    dummySideDef = P_AllocDummySideDef();

    P_SetPtrp(dummyLine, DMU_SIDEDEF0, dummySideDef);
    P_SetPtrp(dummyLine, DMU_FRONT_SECTOR, sec);
    P_SetPtrp(dummySideDef, DMU_LINEDEF, dummyLine);

    xdummyLine->special = (ch == XSCE_FUNCTION ? activating : info->chain[ch]);

    xdummyLine->tag = P_ToXSector(sec)->tag;

    ltype = XL_GetType(xdummyLine->special);
    if(!ltype)
    {
        // What is this? There is no such XG line type.
        XG_Dev("XS_DoChain: Unknown XG line type %i", xdummyLine->special);
        // We're done, free the dummy.
        Z_Free(xdummyLine->xg);
        P_FreeDummyLine(dummyLine);
        return;
    }

    memcpy(&xdummyLine->xg->info, ltype, sizeof(*ltype));

    if(act_thing)
        xdummyLine->xg->activator = act_thing;
    else
        xdummyLine->xg->activator = NULL;

    xdummyLine->xg->active = (ch == XSCE_FUNCTION ? false : !activating);

    XG_Dev("XS_DoChain: Dummy line will show up as %i", P_ToIndex(dummyLine));

    // Send the event.
    if(XL_LineEvent((ch == XSCE_FUNCTION ? XLE_FUNC : XLE_CHAIN), 0,
                    dummyLine, 0, act_thing))
    {   // Success!
        if(ch < XSCE_NUM_CHAINS)
        {
            // Decrease counter.
            if(info->count[ch] > 0)
            {
                info->count[ch]--;

                XG_Dev
                    ("XS_DoChain: %s, sector %i (activating=%i): Counter now at %i",
                     ch == XSCE_FLOOR ? "FLOOR" : ch ==
                     XSCE_CEILING ? "CEILING" : ch ==
                     XSCE_INSIDE ? "INSIDE" : ch ==
                     XSCE_TICKER ? "TICKER" : ch ==
                     XSCE_FUNCTION ? "FUNCTION" : "???", P_ToIndex(sec),
                     activating, info->count[ch]);
            }
        }
    }

    // We're done, free the dummies.
    Z_Free(xdummyLine->xg);
    P_FreeDummyLine(dummyLine);
    P_FreeDummySideDef(dummySideDef);
}

static boolean checkChainRequirements(Sector* sec, mobj_t* mo, int ch,
                                      boolean* activating)
{
    xgsector_t*         xg;
    sectortype_t*       info;
    player_t*           player = mo->player;
    int                 flags;
    boolean             typePasses = false;

    xg = P_ToXSector(sec)->xg;
    info = &xg->info;
    flags = info->chainFlags[ch];

    // Check mobj type.
    if((flags & (SCEF_ANY_A | SCEF_ANY_D | SCEF_TICKER_A | SCEF_TICKER_D)) ||
       ((flags & (SCEF_PLAYER_A | SCEF_PLAYER_D)) && player) ||
       ((flags & (SCEF_OTHER_A | SCEF_OTHER_D)) && !player) ||
       ((flags & (SCEF_MONSTER_A | SCEF_MONSTER_D)) && (mo->flags & MF_COUNTKILL)) ||
       ((flags & (SCEF_MISSILE_A | SCEF_MISSILE_D)) && (mo->flags & MF_MISSILE)))
        typePasses = true;

    if(!typePasses)
        return false; // Wrong type.

    // Are we looking for an activation effect?
    if(player)
        *activating = !(flags & SCEF_PLAYER_D);
    else if(mo->flags & MF_COUNTKILL)
        *activating = !(flags & SCEF_MONSTER_D);
    else if(mo->flags & MF_MISSILE)
        *activating = !(flags & SCEF_MISSILE_D);
    else if(flags & (SCEF_ANY_A | SCEF_ANY_D))
        *activating = !(flags & SCEF_ANY_D);
    else
        *activating = !(flags & SCEF_OTHER_D);

    // Check for extra requirements (touching).
    switch(ch)
    {
    case XSCE_FLOOR:
        // Is it touching the floor?
        if(mo->origin[VZ] > P_GetDoublep(sec, DMU_FLOOR_HEIGHT))
            return false;

    case XSCE_CEILING:
        // Is it touching the ceiling?
        if(mo->origin[VZ] + mo->height < P_GetDoublep(sec, DMU_CEILING_HEIGHT))
            return false;

    default:
        break;
    }

    return true;
}

typedef struct {
    Sector* sec;
    int data;
} xstrav_sectorchainparams_t;

int XSTrav_SectorChain(thinker_t* th, void* context)
{
    xstrav_sectorchainparams_t* params =
        (xstrav_sectorchainparams_t*) context;
    mobj_t* mo = (mobj_t *) th;

    if(params->sec == P_GetPtrp(mo->bspLeaf, DMU_SECTOR))
    {
        boolean activating;

        if(checkChainRequirements(params->sec, mo, params->data, &activating))
            XS_DoChain(params->sec, params->data, activating, mo);
    }

    return false; // Continue iteration.
}

void P_ApplyWind(mobj_t* mo, Sector* sec)
{
    sectortype_t* info;
    float ang;

    if(mo->player && (mo->player->plr->flags & DDPF_CAMERA))
        return; // Wind does not affect cameras.

    info = &(P_ToXSector(sec)->xg->info);
    ang = PI * info->windAngle / 180;

    if(IS_CLIENT)
    {
        // Clientside wind only affects the local player.
        if(!mo->player || mo->player != &players[CONSOLEPLAYER])
            return;
    }

    // Does wind affect this sort of things?
    if(((info->flags & STF_PLAYER_WIND) && mo->player) ||
       ((info->flags & STF_OTHER_WIND) && !mo->player) ||
       ((info->flags & STF_MONSTER_WIND) && (mo->flags & MF_COUNTKILL)) ||
       ((info->flags & STF_MISSILE_WIND) && (mo->flags & MF_MISSILE)))
    {
        coord_t thfloorz = P_GetDoublep(mo->bspLeaf, DMU_FLOOR_HEIGHT);
        coord_t thceilz  = P_GetDoublep(mo->bspLeaf, DMU_CEILING_HEIGHT);

        if(!(info->flags & (STF_FLOOR_WIND | STF_CEILING_WIND)) ||
           ((info->flags & STF_FLOOR_WIND) && mo->origin[VZ] <= thfloorz) ||
           ((info->flags & STF_CEILING_WIND) &&
            mo->origin[VZ] + mo->height >= thceilz))
        {
            // Apply vertical wind.
            mo->mom[MZ] += info->verticalWind;

            // Horizontal wind.
            mo->mom[MX] += cos(ang) * info->windSpeed;
            mo->mom[MY] += sin(ang) * info->windSpeed;
        }
    }
}

typedef struct {
    Sector* sec;
} xstrav_windparams_t;

int XSTrav_Wind(thinker_t* th, void* context)
{
    xstrav_windparams_t* params = (xstrav_windparams_t*) context;
    mobj_t* mo = (mobj_t *) th;

    if(params->sec == P_GetPtrp(mo->bspLeaf, DMU_SECTOR))
    {
        P_ApplyWind(mo, params->sec);
    }

    return false; // Continue iteration.
}

/**
 * Makes sure the offset is in the range 0..64.
 */
void XS_ConstrainPlaneOffset(float *offset)
{
    if(*offset > 64)
        *offset -= 64;
    if(*offset < 0)
        *offset += 64;
}

/**
 * XG sectors get to think.
 */
void XS_Thinker(xsthinker_t* xs)
{
    Sector* sector = xs->sector;
    xsector_t* xsector = P_ToXSector(sector);
    xgsector_t* xg;
    sectortype_t* info;
    int i;

    if(!xsector) return; // Not an xsector? Most perculiar...

    xg = xsector->xg;
    if(!xg) return; // Not an extended sector.

    if(xg->disabled) return; // This sector is disabled.

    info = &xg->info;

    if(!IS_CLIENT)
    {
        // Function tickers.
        for(i = 0; i < 2; ++i)
            XF_Ticker(&xg->plane[i], sector);
        XF_Ticker(&xg->light, sector);
        for(i = 0; i < 3; ++i)
            XF_Ticker(&xg->rgb[i], sector);

        // Update linked functions.
        for(i = 0; i < 3; ++i)
        {
            if(i < 2 && xg->plane[i].link)
                xg->plane[i].value = xg->plane[i].link->value;

            if(xg->rgb[i].link)
                xg->rgb[i].value = xg->rgb[i].link->value;
        }

        if(xg->light.link)
            xg->light.value = xg->light.link->value;

        // Update planes.
        XS_UpdatePlanes(sector);

        // Update sector light.
        XS_UpdateLight(sector);

        // Decrement chain timers.
        for(i = 0; i < XSCE_NUM_CHAINS; ++i)
            xg->chainTimer[i]--;

        // Floor chain. Check any mobjs that are touching the floor of the
        // sector.
        if(info->chain[XSCE_FLOOR] && xg->chainTimer[XSCE_FLOOR] <= 0)
        {
            xstrav_sectorchainparams_t params;

            params.sec = sector;
            params.data = XSCE_FLOOR;
            Thinker_Iterate(P_MobjThinker, XSTrav_SectorChain, &params);
        }

        // Ceiling chain. Check any mobjs that are touching the ceiling.
        if(info->chain[XSCE_CEILING] && xg->chainTimer[XSCE_CEILING] <= 0)
        {
            xstrav_sectorchainparams_t params;

            params.sec = sector;
            params.data = XSCE_CEILING;
            Thinker_Iterate(P_MobjThinker, XSTrav_SectorChain, &params);
        }

        // Inside chain. Check any sectorlinked mobjs.
        if(info->chain[XSCE_INSIDE] && xg->chainTimer[XSCE_INSIDE] <= 0)
        {
            xstrav_sectorchainparams_t params;

            params.sec = sector;
            params.data = XSCE_INSIDE;
            Thinker_Iterate(P_MobjThinker, XSTrav_SectorChain, &params);
        }

        // Ticker chain. Send an activate event if TICKER_D flag is not set.
        if(info->chain[XSCE_TICKER] && xg->chainTimer[XSCE_TICKER] <= 0)
        {
            XS_DoChain(sector, XSCE_TICKER,
                       !(info->chainFlags[XSCE_TICKER] & SCEF_TICKER_D),
                       &dummyThing);
        }

        // Play ambient sounds.
        if(xg->info.ambientSound)
        {
            if(xg->timer-- < 0)
            {
                xg->timer = XG_RandomInt(FLT2TIC(xg->info.soundInterval[0]),
                                         FLT2TIC(xg->info.soundInterval[1]));
                S_SectorSound(sector, xg->info.ambientSound);
            }
        }
    }

    // Floor Texture movement
    if(xg->info.materialMoveSpeed[0] != 0)
    {
        coord_t floorOffset[2];
        double ang = PI * xg->info.materialMoveAngle[0] / 180;

        P_GetDoublepv(sector, DMU_FLOOR_MATERIAL_OFFSET_XY, floorOffset);
        floorOffset[VX] -= cos(ang) * xg->info.materialMoveSpeed[0];
        floorOffset[VY] -= sin(ang) * xg->info.materialMoveSpeed[0];

        // Set the results
        P_SetDoublepv(sector, DMU_FLOOR_MATERIAL_OFFSET_XY, floorOffset);
    }

    // Ceiling Texture movement
    if(xg->info.materialMoveSpeed[1] != 0)
    {
        coord_t ceilOffset[2];
        double ang = PI * xg->info.materialMoveAngle[1] / 180;

        P_GetDoublepv(sector, DMU_CEILING_MATERIAL_OFFSET_XY, ceilOffset);
        ceilOffset[VX] -= cos(ang) * xg->info.materialMoveSpeed[1];
        ceilOffset[VY] -= sin(ang) * xg->info.materialMoveSpeed[1];

        // Set the results
        P_SetDoublepv(sector, DMU_CEILING_MATERIAL_OFFSET_XY, ceilOffset);
    }

    // Wind for all sectorlinked mobjs.
    if(xg->info.windSpeed || xg->info.verticalWind)
    {
        xstrav_windparams_t params;

        params.sec = sector;
        Thinker_Iterate(P_MobjThinker, XSTrav_Wind, &params);
    }
}

coord_t XS_Gravity(struct sector_s* sec)
{
    xsector_t* xsec;

    if(!sec) return P_GetGravity(); // World gravity.

    xsec = P_ToXSector(sec);
    if(!xsec->xg || !(xsec->xg->info.flags & STF_GRAVITY))
    {
        return P_GetGravity(); // World gravity.
    }
    else
    {   // Sector-specific gravity.
        coord_t gravity = xsec->xg->info.gravity;

        // Apply gravity modifier.
        if(cfg.netGravity != -1)
            gravity *= (coord_t) cfg.netGravity / 100;

        return gravity;
    }
}

coord_t XS_Friction(struct sector_s* sector)
{
    if(!P_ToXSector(sector)->xg || !(P_ToXSector(sector)->xg->info.flags & STF_FRICTION))
        return FRICTION_NORMAL; // Normal friction.

    return P_ToXSector(sector)->xg->info.friction;
}

/**
 * @return              The thrust multiplier to emulate by friction.
 */
coord_t XS_ThrustMul(struct sector_s *sector)
{
    coord_t x = XS_Friction(sector);

    if(x <= FRICTION_NORMAL)
        return 1; // Normal friction.

    if(x > 1)
        return 0; // There's nothing to thrust from!

    // {c = -93.31092643, b = 208.0448223, a = -114.7338958}
    return (-114.7338958 * x * x + 208.0448223 * x - 93.31092643);
}

/**
 * During update, definitions are re-read, so the pointers need to be
 * updated. However, this is a bit messy operation, prone to errors.
 * Instead, we just disable XG...
 */
void XS_Update(void)
{
    uint        i;
    xsector_t  *xsec;

    // It's all PU_MAP memory, so we can just lose it.
    for(i = 0; i < numsectors; ++i)
    {
        xsec = P_ToXSector(P_ToPtr(DMU_SECTOR, i));
        if(xsec->xg)
        {
            xsec->xg = NULL;
            xsec->special = 0;
        }
    }
}

#if 0 // no longer supported in 1.8.7
/*
 * Write XG types into a binary file.
 */
D_CMD(DumpXG)
{
    FILE   *file;

    if(argc != 2)
    {
        Con_Printf("Usage: %s (file)\n", argv[0]);
        Con_Printf("Writes XG line and sector types to the file.\n");
        return true;
    }

    file = fopen(argv[1], "wb");
    if(!file)
    {
        Con_Printf("Can't open \"%s\" for writing.\n", argv[1]);
        return false;
    }
    XG_WriteTypes(file);
    fclose(file);
    return true;
}
#endif

/**
 * $moveplane: Command line interface to the plane mover.
 */
D_CMD(MovePlane)
{
    boolean isCeiling = !stricmp(argv[0], "moveceil");
    boolean isBoth = !stricmp(argv[0], "movesec");
    boolean isOffset = false, isCrusher = false;
    Sector* sector = NULL;
    coord_t units = 0;
    float speed = FRACUNIT;
    int p = 0;
    coord_t floorheight, ceilingheight;
    xgplanemover_t* mover;

    if(argc < 2)
    {
        Con_Printf("Usage: %s (opts)\n", argv[0]);
        Con_Printf("Opts can be:\n");
        Con_Printf("  here [crush] [off] (z/units) [speed]\n");
        Con_Printf("  at (x) (y) [crush] [off] (z/units) [speed]\n");
        Con_Printf("  tag (sector-tag) [crush] [off] (z/units) [speed]\n");
        return true;
    }

    if(IS_CLIENT)
    {
        Con_Printf("Clients can't move planes.\n");
        return false;
    }

    // Which mode?
    if(!stricmp(argv[1], "here"))
    {
        p = 2;
        if(!players[CONSOLEPLAYER].plr->mo)
            return false;
        sector = P_GetPtrp(players[CONSOLEPLAYER].plr->mo->bspLeaf, DMU_SECTOR);
    }
    else if(!stricmp(argv[1], "at") && argc >= 4)
    {
        coord_t point[2];
        point[VX] = (coord_t)strtol(argv[2], 0, 0);
        point[VY] = (coord_t)strtol(argv[3], 0, 0);
        sector = P_GetPtrp(P_BspLeafAtPoint(point), DMU_SECTOR);

        p = 4;
    }
    else if(!stricmp(argv[1], "tag") && argc >= 3)
    {
        int tag = (short) strtol(argv[2], 0, 0);
        Sector* sec = NULL;
        iterlist_t* list;

        p = 3;
        list = P_GetSectorIterListForTag(tag, false);
        if(list)
        {   // Find the first sector with the tag.
            IterList_SetIteratorDirection(list, ITERLIST_FORWARD);
            IterList_RewindIterator(list);
            while((sec = IterList_MoveIterator(list)) != NULL)
            {
                sector = sec;
                break;
            }
        }
    }
    else
    {   // Unknown mode.
        Con_Printf("Unknown mode.\n");
        return false;
    }

    floorheight   = P_GetDoublep(sector, DMU_FLOOR_HEIGHT);
    ceilingheight = P_GetDoublep(sector, DMU_CEILING_HEIGHT);

    // No more arguments?
    if(argc == p)
    {
        Con_Printf("Ceiling = %g\nFloor = %g\n", ceilingheight, floorheight);
        return true;
    }

    // Check for the optional 'crush' parameter.
    if(argc >= p + 1 && !stricmp(argv[p], "crush"))
    {
        isCrusher = true;
        ++p;
    }

    // Check for the optional 'off' parameter.
    if(argc >= p + 1 && !stricmp(argv[p], "off"))
    {
        isOffset = true;
        ++p;
    }

    // The amount to move.
    if(argc >= p + 1)
    {
        units = strtod(argv[p++], 0);
    }
    else
    {
        Con_Printf("You must specify Z-units.\n");
        return false; // Required parameter missing.
    }

    // The optional speed parameter.
    if(argc >= p + 1)
    {
        speed = strtod(argv[p++], 0);
        // The speed is always positive.
        if(speed < 0)
            speed = -speed;
    }

    // We must now have found the sector to operate on.
    if(!sector)
        return false;

    mover = XS_GetPlaneMover(sector, isCeiling);

    // Setup the thinker and add it to the list.
    mover->destination = units + (isOffset ? (isCeiling ? ceilingheight : floorheight) : 0);

    mover->speed = speed;
    if(isCrusher)
    {
        mover->crushSpeed = speed * .5;  // Crush at half speed.
        mover->flags |= PMF_CRUSH;
    }
    if(isBoth)
        mover->flags |= PMF_OTHER_FOLLOWS;

    return true;
}

#endif
