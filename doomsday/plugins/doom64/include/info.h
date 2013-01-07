/**\file
 *\section License
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2003-2013 Jaakko Keränen <jaakko.keranen@iki.fi>
 *\author Copyright © 2005-2013 Daniel Swanson <danij@dengine.net>
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

/**
 * info.h: Sprite, state, mobjtype and text identifiers.
 */

#ifndef __INFO_CONSTANTS_H__
#define __INFO_CONSTANTS_H__

#ifndef __JDOOM64__
#  error "Using jDoom64 headers without __JDOOM64__"
#endif

// Sprites.
typedef enum {
    //SPR_SPOT,
    SPR_PLAY,
    SPR_SARG,
    SPR_FATT,
    SPR_POSS,
    SPR_TROO,
    SPR_HEAD,
    SPR_BOSS,
    SPR_SKUL,
    SPR_BSPI,
    SPR_CYBR,
    SPR_PAIN,
    SPR_RECT,
    SPR_MISL,
    SPR_PLSS,
    SPR_BFS1,
    //SPR_LASS,
    SPR_BAL1,
    SPR_BAL3,
    //SPR_BAL2,
    //SPR_BAL7,
    //SPR_BAL8,
    SPR_APLS,
    SPR_MANF,
    //SPR_TRCR,
    SPR_DART,
    SPR_FIRE,
    //SPR_RBAL,
    //SPR_PUF2,
    //SPR_PUF3,
    SPR_PUFF,
    SPR_BLUD,
    //SPR_A027,
    SPR_TFOG,
    SPR_BFE2,
    SPR_ARM1,
    SPR_ARM2,
    SPR_BON1,
    SPR_BON2,
    SPR_BKEY,
    SPR_RKEY,
    SPR_YKEY,
    SPR_YSKU,
    SPR_RSKU,
    SPR_BSKU,
    SPR_ART1,
    SPR_ART2,
    SPR_ART3,
    SPR_STIM,
    SPR_MEDI,
    SPR_SOUL,
    SPR_PINV,
    SPR_PSTR,
    SPR_PINS,
    SPR_SUIT,
    SPR_PMAP,
    SPR_PVIS,
    SPR_MEGA,
    SPR_CLIP,
    SPR_AMMO,
    SPR_RCKT,
    SPR_BROK,
    SPR_CELL,
    SPR_CELP,
    SPR_SHEL,
    SPR_SBOX,
    SPR_BPAK,
    SPR_BFUG,
    SPR_CSAW,
    SPR_MGUN,
    SPR_LAUN,
    SPR_PLSM,
    SPR_SHOT,
    SPR_SGN2,
    SPR_LSRG,
    SPR_CAND,
    SPR_BAR1,
    //SPR_LMP1,
    //SPR_LMP2,
    //SPR_A031,
    //SPR_A030,
    //SPR_A032,
    //SPR_A033,
    //SPR_A034,
    //SPR_BFLM,
    //SPR_RFLM,
    //SPR_YFLM,
    //SPR_A006,
    //SPR_A021,
    //SPR_A003,
    //SPR_A020,
    //SPR_A014,
    //SPR_A016,
    //SPR_A008,
    //SPR_A007,
    //SPR_A015,
    //SPR_A001,
    //SPR_A012,
    //SPR_A010,
    //SPR_A018,
    //SPR_A017,
    //SPR_A026,
    //SPR_A022,
    //SPR_A028,
    //SPR_A029,
    //SPR_A035,
    //SPR_A036,
    //SPR_TRE3,
    SPR_TRE2,
    SPR_TRE1,
    //SPR_A013,
    //SPR_A019,
    //SPR_A004,
    //SPR_A005,
    //SPR_A023,
    SPR_SAWG,
    SPR_PUNG,
    SPR_PISG,
    SPR_SHT1,
    SPR_SHT2,
    SPR_CHGG,
    SPR_ROCK,
    SPR_PLAS,
    SPR_BFGG,
    SPR_LASR,

    /**
     * Sprites below here are either unused or need to take indices above.
     */
    SPR_PLSE,
    SPR_BFE1,
    SPR_IFOG,
    SPR_SPOS,
    SPR_FATB,
    SPR_FBXP,
    SPR_SKEL,
    SPR_BAL7,
    SPR_BOS2,
    SPR_BON4,
    SPR_COLU,
    SPR_SMT2,
    SPR_GOR1,
    SPR_POL2,
    SPR_POL5,
    SPR_POL4,
    SPR_POL3,
    SPR_POL1,
    SPR_POL6,
    SPR_GOR2,
    SPR_GOR3,
    SPR_GOR4,
    SPR_GOR5,
    SPR_SMIT,
    SPR_COL1,
    SPR_COL2,
    SPR_COL3,
    SPR_COL4,
    SPR_CBRA,
    SPR_COL6,
    SPR_ELEC,
    SPR_TBLU,
    SPR_TGRN,
    SPR_TRED,
    SPR_SMBT,
    SPR_SMGT,
    SPR_SMRT,
    SPR_HDB1,
    SPR_HDB2,
    SPR_HDB3,
    SPR_HDB4,
    SPR_HDB5,
    SPR_HDB6,
    SPR_POB1,
    SPR_POB2,
    SPR_BRS1,
    SPR_TLMP,
    SPR_TLP2,
    SPR_NTRO,
    SPR_BRNR,
    SPR_NMBL,
    SPR_SMOK,
    SPR_LAZR,
    SPR_LPUF,
    SPR_MBAL,
    SPR_MPUF,
    NUMSPRITES
} spritetype_e;

// States.
typedef enum {
    S_NULL,
    S_LIGHTDONE,
    S_PLAY,
    S_PLAY_RUN1,
    S_PLAY_RUN2,
    S_PLAY_RUN3,
    S_PLAY_RUN4,
    S_PLAY_ATK1,
    S_PLAY_ATK2,
    S_PLAY_PAIN,
    S_PLAY_PAIN2,
    S_PLAY_DIE1,
    S_PLAY_DIE2,
    S_PLAY_DIE3,
    S_PLAY_DIE4,
    S_PLAY_DIE5,
    S_PLAY_DIE6,
    S_PLAY_XDIE1,
    S_PLAY_XDIE2,
    S_PLAY_XDIE3,
    S_PLAY_XDIE4,
    S_PLAY_XDIE5,
    S_PLAY_XDIE6,
    S_PLAY_XDIE7,
    S_PLAY_XDIE8,
    S_PLAY_XDIE9,
    S_EMARP_STND,
    S_EMARP_RUN1,
    S_EMARP_RUN2,
    S_EMARP_RUN3,
    S_EMARP_RUN4,
    S_EMARP_ATK1,
    S_EMARP_ATK2,
    S_EMARP_ATK3,
    S_EMARP_ATK4,
    S_EMARP_ATK5,
    S_EMARP_ATK6,
    S_EMARP_PAIN,
    S_EMARP_PAIN2,
    S_EMARP_ATKB1,
    S_EMARP_ATKB2,
    S_EMARP_ATKB3,
    S_EMARP_ATKB4,
    S_EMARP_ATKB5,
    S_EMARP_ATKB6,
    S_SARG_STND,
    S_SARG_STND2,
    S_SARG_RUN1,
    S_SARG_RUN2,
    S_SARG_RUN3,
    S_SARG_RUN4,
    S_SARG_RUN5,
    S_SARG_RUN6,
    S_SARG_RUN7,
    S_SARG_RUN8,
    S_SARG_ATK1,
    S_SARG_ATK2,
    S_SARG_ATK3,
    S_SARG_PAIN,
    S_SARG_PAIN2,
    S_SARG_DIE1,
    S_SARG_DIE2,
    S_SARG_DIE3,
    S_SARG_DIE4,
    S_SARG_DIE5,
    S_SARG_DIE6,
    S_SHAD_RUN,
    S_SHAD_ATK,
    S_SHAD_PAIN,
    S_SHAD_DIE,
    S_FATT_STND,
    S_FATT_STND2,
    S_FATT_RUN1,
    S_FATT_RUN2,
    S_FATT_RUN3,
    S_FATT_RUN4,
    S_FATT_RUN5,
    S_FATT_RUN6,
    S_FATT_RUN7,
    S_FATT_RUN8,
    S_FATT_RUN9,
    S_FATT_RUN10,
    S_FATT_RUN11,
    S_FATT_RUN12,
    S_FATT_ATK1,
    S_FATT_ATK2,
    S_FATT_ATK3,
    S_FATT_ATK4,
    S_FATT_ATK5,
    S_FATT_ATK6,
    S_FATT_ATK7,
    S_FATT_PAIN,
    S_FATT_PAIN2,
    S_FATT_DIE1,
    S_FATT_DIE2,
    S_FATT_DIE3,
    S_FATT_DIE4,
    S_FATT_DIE5,
    S_FATT_DIE6,
    S_POSS_STND,
    S_POSS_STND2,
    S_POSS_RUN1,
    S_POSS_RUN2,
    S_POSS_RUN3,
    S_POSS_RUN4,
    S_POSS_RUN5,
    S_POSS_RUN6,
    S_POSS_RUN7,
    S_POSS_RUN8,
    S_POSS_ATK1,
    S_POSS_ATK2,
    S_POSS_ATK3,
    S_POSS_PAIN,
    S_POSS_PAIN2,
    S_POSS_DIE1,
    S_POSS_DIE2,
    S_POSS_DIE3,
    S_POSS_DIE4,
    S_POSS_DIE5,
    S_POSS_XDIE1,
    S_POSS_XDIE2,
    S_POSS_XDIE3,
    S_POSS_XDIE4,
    S_POSS_XDIE5,
    S_POSS_XDIE6,
    S_POSS_XDIE7,
    S_POSS_XDIE8,
    S_POSS_XDIE9,
    S_SPOS_STND,
    S_SPOS_STND2,
    S_SPOS_RUN1,
    S_SPOS_RUN2,
    S_SPOS_RUN3,
    S_SPOS_RUN4,
    S_SPOS_RUN5,
    S_SPOS_RUN6,
    S_SPOS_RUN7,
    S_SPOS_RUN8,
    S_SPOS_ATK1,
    S_SPOS_ATK2,
    S_SPOS_ATK3,
    S_SPOS_PAIN,
    S_SPOS_PAIN2,
    S_SPOS_DIE1,
    S_SPOS_DIE2,
    S_SPOS_DIE3,
    S_SPOS_DIE4,
    S_SPOS_DIE5,
    S_SPOS_XDIE1,
    S_SPOS_XDIE2,
    S_SPOS_XDIE3,
    S_SPOS_XDIE4,
    S_SPOS_XDIE5,
    S_SPOS_XDIE6,
    S_SPOS_XDIE7,
    S_SPOS_XDIE8,
    S_SPOS_XDIE9,
    S_TROO_STND,
    S_TROO_STND2,
    S_TROO_RUN1,
    S_TROO_RUN2,
    S_TROO_RUN3,
    S_TROO_RUN4,
    S_TROO_RUN5,
    S_TROO_RUN6,
    S_TROO_RUN7,
    S_TROO_RUN8,
    S_TROO_MEL1,
    S_TROO_MEL2,
    S_TROO_MEL3,
    S_TROO_ATK1,
    S_TROO_ATK2,
    S_TROO_ATK3,
    S_TROO_PAIN,
    S_TROO_PAIN2,
    S_TROO_DIE1,
    S_TROO_DIE2,
    S_TROO_DIE3,
    S_TROO_DIE4,
    S_TROO_DIE5,
    S_TROO_XDIE1,
    S_TROO_XDIE2,
    S_TROO_XDIE3,
    S_TROO_XDIE4,
    S_TROO_XDIE5,
    S_TROO_XDIE6,
    S_TROO_XDIE7,
    S_TROO_XDIE8,
    S_NTRO_STND,
    S_NTRO_STND2,
    S_NTRO_RUN1,
    S_NTRO_RUN2,
    S_NTRO_RUN3,
    S_NTRO_RUN4,
    S_NTRO_RUN5,
    S_NTRO_RUN6,
    S_NTRO_RUN7,
    S_NTRO_RUN8,
    S_NTRO_MEL1,
    S_NTRO_MEL2,
    S_NTRO_MEL3,
    S_NTRO_ATK1,
    S_NTRO_ATK2,
    S_NTRO_ATK3,
    S_NTRO_PAIN,
    S_NTRO_PAIN2,
    S_NTRO_DIE1,
    S_NTRO_DIE2,
    S_NTRO_DIE3,
    S_NTRO_DIE4,
    S_NTRO_DIE5,
    S_NTRO_XDIE1,
    S_NTRO_XDIE2,
    S_NTRO_XDIE3,
    S_NTRO_XDIE4,
    S_NTRO_XDIE5,
    S_NTRO_XDIE6,
    S_NTRO_XDIE7,
    S_NTRO_XDIE8,
    S_HEAD_STND,
    S_HEAD_STND2,
    S_HEAD_STND3,
    S_HEAD_STND4,
    S_HEAD_RUN1,
    S_HEAD_RUN2,
    S_HEAD_RUN3,
    S_HEAD_RUN4,
    S_HEAD_RUN5,
    S_HEAD_RUN6,
    S_HEAD_RUN7,
    S_HEAD_RUN8,
    S_HEAD_ATK1,
    S_HEAD_ATK2,
    S_HEAD_ATK3,
    S_HEAD_ATK4,
    S_HEAD_PAIN,
    S_HEAD_PAIN2,
    S_HEAD_PAIN3,
    S_HEAD_DIE1,
    S_HEAD_DIE2,
    S_HEAD_DIE3,
    S_HEAD_DIE4,
    S_HEAD_DIE5,
    S_HEAD_DIE6,
    S_BOSS_STND,
    S_BOSS_STND2,
    S_BOSS_RUN1,
    S_BOSS_RUN2,
    S_BOSS_RUN3,
    S_BOSS_RUN4,
    S_BOSS_RUN5,
    S_BOSS_RUN6,
    S_BOSS_RUN7,
    S_BOSS_RUN8,
    S_BOSS_ATK1,
    S_BOSS_ATK2,
    S_BOSS_ATK3,
    S_BOSS_PAIN,
    S_BOSS_PAIN2,
    S_BOSS_DIE1,
    S_BOSS_DIE2,
    S_BOSS_DIE3,
    S_BOSS_DIE4,
    S_BOSS_DIE5,
    S_BOSS_DIE6,
    S_BOS2_STND,
    S_BOS2_STND2,
    S_BOS2_RUN1,
    S_BOS2_RUN2,
    S_BOS2_RUN3,
    S_BOS2_RUN4,
    S_BOS2_RUN5,
    S_BOS2_RUN6,
    S_BOS2_RUN7,
    S_BOS2_RUN8,
    S_BOS2_ATK1,
    S_BOS2_ATK2,
    S_BOS2_ATK3,
    S_BOS2_PAIN,
    S_BOS2_PAIN2,
    S_BOS2_DIE1,
    S_BOS2_DIE2,
    S_BOS2_DIE3,
    S_BOS2_DIE4,
    S_BOS2_DIE5,
    S_BOS2_DIE6,
    S_SKULL_STND,
    S_SKULL_STND2,
    S_SKULL_STND3,
    S_SKULL_RUN1,
    S_SKULL_RUN2,
    S_SKULL_RUN3,
    S_SKULL_ATK1,
    S_SKULL_ATK2,
    S_SKULL_ATK3,
    S_SKULL_ATK4,
    S_SKULL_PAIN,
    S_SKULL_PAIN2,
    S_SKULL_DIE1,
    S_SKULL_DIE2,
    S_SKULL_DIE3,
    S_SKULL_DIE4,
    S_SKULL_DIE5,
    S_SKULL_DIE6,
    S_SKULL_DIE7,
    S_SKULL_DIE8,
    S_SKULL_DIE9,
    S_SKULL_DIE10,
    S_BSPI_STND,
    S_BSPI_STND2,
    S_BSPI_SIGHT,
    S_BSPI_RUN1,
    S_BSPI_RUN2,
    S_BSPI_RUN3,
    S_BSPI_RUN4,
    S_BSPI_RUN5,
    S_BSPI_RUN6,
    S_BSPI_RUN7,
    S_BSPI_RUN8,
    S_BSPI_ATK1,
    S_BSPI_ATK2,
    S_BSPI_ATK3,
    S_BSPI_PAIN,
    S_BSPI_PAIN2,
    S_BSPI_DIE1,
    S_BSPI_DIE2,
    S_BSPI_DIE3,
    S_BSPI_DIE4,
    S_BSPI_DIE5,
    S_BSPI_DIE6,
    S_CYBER_STND,
    S_CYBER_RUN1,
    S_CYBER_RUN2,
    S_CYBER_RUN3,
    S_CYBER_RUN4,
    S_CYBER_RUN5,
    S_CYBER_RUN6,
    S_CYBER_RUN7,
    S_CYBER_RUN8,
    S_CYBER_ATK1,
    S_CYBER_ATK2,
    S_CYBER_ATK3,
    S_CYBER_ATK4,
    S_CYBER_ATK5,
    S_CYBER_ATK6,
    S_CYBER_PAIN,
    S_CYBER_DIE1,
    S_CYBER_DIE2,
    S_CYBER_DIE3,
    S_CYBER_DIE4,
    S_CYBER_DIE5,
    S_CYBER_DIE6,
    S_CYBER_DIE7,
    S_CYBER_DIE8,
    S_CYBER_DIE9,
    S_DCYBER_STND,
    S_DCYBER_ATK1,
    S_DCYBER_ATK2,
    S_PAIN_STND,
    S_PAIN_SEE,
    S_PAIN_ATK1,
    S_PAIN_ATK2,
    S_PAIN_ATK3,
    S_PAIN_ATK4,
    S_PAIN_PAIN,
    S_PAIN_PAIN2,
    S_PAIN_DIE1,
    S_PAIN_DIE2,
    S_PAIN_DIE3,
    S_PAIN_DIE4,
    S_PAIN_DIE5,
    S_PAIN_DIE6,
    S_PAIN_DIE7,
    S_PAIN_DIE8,
    S_RECT_STND,
    S_RECT_STND2,
    S_RECT_STND3,
    S_RECT_STND4,
    S_RECT_RUN1,
    S_RECT_RUN2,
    S_RECT_RUN3,
    S_RECT_RUN4,
    S_RECT_RUN5,
    S_RECT_RUN6,
    S_RECT_RUN7,
    S_RECT_RUN8,
    S_RECT_RUN9,
    S_RECT_RUN10,
    S_RECT_RUN11,
    S_RECT_RUN12,
    S_RECT_ATK1,
    S_RECT_ATK2,
    S_RECT_ATK3,
    S_RECT_ATK4,
    S_RECT_ATK5,
    S_RECT_PAIN,
    S_RECT_DIE1,
    S_RECT_DIE2,
    S_RECT_DIE3,
    S_RECT_DIE4,
    S_RECT_DIE5,
    S_RECT_DIE6,
    S_RECT_DIE7,
    S_BAL3,
    S_BAL32,
    S_BAL33,
    S_BAL3EXP,
    S_BAL3EXP2,
    S_BAL3EXP3,
    S_BAL3EXP4,
    S_BAL3EXP5,
    S_BAL3EXP6,
    S_BRBALL1,
    S_BRBALL2,
    S_BRBALLX1,
    S_BRBALLX2,
    S_BRBALLX3,
    S_BRBALLX4,
    S_BRBALLX5,
    S_BRBALLX6,
    S_ARACH_PLAZ,
    S_ARACH_PLAZ2,
    S_ARACH_PLEX,
    S_ARACH_PLEX2,
    S_ARACH_PLEX3,
    S_ARACH_PLEX4,
    S_ARACH_PLEX5,
    S_ARACH_PLEX6,
    S_FATSHOT1,
    S_FATSHOT2,
    S_FATSHOT3,
    S_FATSHOTX1,
    S_FATSHOTX2,
    S_FATSHOTX3,
    S_FATSHOTX4,
    S_FATSHOTX5,
    S_FATSHOTX6,
    S_TSHOT6_STND,
    S_TSHOT6_STND2,
    S_TSHOT6_RUN1,
    S_TSHOT6_RUN2,
    S_TSHOT6_RUN3,
    S_TSHOT6_RUN4,
    S_TSHOT6_RUN5,
    S_TSHOT6_RUN6,
    S_DART,
    S_MPUFF1,
    S_MPUFF2,
    S_MPUFF3,
    S_MPUFF4,
    S_MPUFF5,
    S_RPUFF1,
    S_RPUFF2,
    S_RPUFF3,
    S_RPUFF4,
    S_RPUFF5,
    S_SMOKE1,
    S_SMOKE2,
    S_SMOKE3,
    S_SMOKE4,
    S_SMOKE5,
    S_PUFF1,
    S_PUFF2,
    S_PUFF3,
    S_PUFF4,
    S_PUFF5,
    S_PUFF6,
    S_BLOOD1,
    S_BLOOD2,
    S_BLOOD3,
    S_BLOOD4,
    S_TBALL1,
    S_TBALL2,
    S_TBALL3,
    S_TBALLX1,
    S_TBALLX2,
    S_TBALLX3,
    S_TBALLX4,
    S_TBALLX5,
    S_TBALLX6,
    S_PLASBALL,
    S_PLASBALL2,
    S_PLASEXP,
    S_PLASEXP2,
    S_PLASEXP3,
    S_PLASEXP4,
    S_PLASEXP5,
    S_PLASEXP6,
    S_ROCKET,
    S_ROCKET2,
    S_BFGSHOT,
    S_BFGSHOT2,
    S_BFGLAND,
    S_BFGLAND2,
    S_BFGLAND3,
    S_BFGLAND4,
    S_BFGLAND5,
    S_BFGLAND6,
    S_BFGEXP,
    S_BFGEXP2,
    S_BFGEXP3,
    S_BFGEXP4,
    S_BFGEXP5,
    S_BFGEXP6,
    S_EXPLODE0,
    S_EXPLODE1,
    S_EXPLODE2,
    S_EXPLODE3,
    S_EXPLODE4,
    S_EXPLODE5,
    S_TFOG,
    S_TFOG2,
    S_TFOG3,
    S_TFOG4,
    S_TFOG5,
    S_TFOG6,
    S_TFOG7,
    S_TFOG8,
    S_TFOG9,
    S_TFOG10,
    S_TFOG11,
    S_TFOG12,
    S_ARM1,
    S_ARM1A,
    S_ARM2,
    S_ARM2A,
    S_BON1,
    S_BON1A,
    S_BON1B,
    S_BON1C,
    S_BON2,
    S_BON2A,
    S_BON2B,
    S_BON2C,
    S_BON2D,
    S_BON2E,
    S_BKEY,
    S_BKEY2,
    S_RKEY,
    S_RKEY2,
    S_YKEY,
    S_YKEY2,
    S_YSKULL,
    S_YSKULL2,
    S_RSKULL,
    S_RSKULL2,
    S_BSKULL,
    S_BSKULL2,
    S_ART1,
    S_ART12,
    S_ART13,
    S_ART14,
    S_ART15,
    S_ART16,
    S_ART17,
    S_ART18,
    S_ART2,
    S_ART22,
    S_ART23,
    S_ART24,
    S_ART25,
    S_ART26,
    S_ART27,
    S_ART28,
    S_ART3,
    S_ART32,
    S_ART33,
    S_ART34,
    S_ART35,
    S_ART36,
    S_ART37,
    S_ART38,
    S_STIM,
    S_MEDI,
    S_SOUL,
    S_SOUL2,
    S_SOUL3,
    S_SOUL4,
    S_SOUL5,
    S_SOUL6,
    S_PINV,
    S_PINV2,
    S_PINV3,
    S_PINV4,
    S_PINV5,
    S_PINV6,
    S_PSTR,
    S_PINS,
    S_PINS2,
    S_PINS3,
    S_PINS4,
    S_PINS5,
    S_PINS6,
    S_SUIT,
    S_SUIT2,
    S_PMAP,
    S_PMAP2,
    S_PMAP3,
    S_PMAP4,
    S_PVIS,
    S_PVIS2,
    S_MEGA,
    S_MEGA2,
    S_MEGA3,
    S_MEGA4,
    S_MEGA5,
    S_MEGA6,
    S_CLIP,
    S_AMMO,
    S_ROCK,
    S_BROK,
    S_CELL,
    S_CELP,
    S_SHEL,
    S_SBOX,
    S_BPAK,
    S_BFUG,
    S_CSAW,
    S_MGUN,
    S_LAUN,
    S_PLSM,
    S_SHOT,
    S_SHOT2,
    S_LSRG,
    S_CANDLE,
    S_CANDLE2,
    S_BAR1,
    S_BAR2,
    S_BAR3,
    S_BAR4,
    S_BAR5,
    S_SAW,
    S_SAWB,
    S_SAWDOWN,
    S_SAWUP,
    S_SAW1,
    S_SAW2,
    S_SAW3,
    S_PUNCH,
    S_PUNCHDOWN,
    S_PUNCHUP,
    S_PUNCH1,
    S_PUNCH2,
    S_PUNCH3,
    S_PUNCH4,
    S_PUNCH5,
    S_PISTOL,
    S_PISTOLDOWN,
    S_PISTOLUP,
    S_PISTOL1,
    S_PISTOL2,
    S_PISTOL3,
    S_PISTOL4,
    S_PISTOL5,
    S_PISTOLFLASH,
    S_SGUN,
    S_SGUNDOWN,
    S_SGUNUP,
    S_SGUN1,
    S_SGUN2,
    S_SGUN3,
    S_SGUN4,
    S_SGUN5,
    S_SGUN6,
    S_SGUNFLASH1,
    S_DSGUN,
    S_DSGUNDOWN,
    S_DSGUNUP,
    S_DSGUN1,
    S_DSGUN2,
    S_DSGUN3,
    S_DSGUN4,
    S_DSGUN5,
    S_DSGUN6,
    S_DSGUN7,
    S_DSGUN8,
    S_DSGUN9,
    S_DSGUN10,
    S_DSGUNFLASH1,
    S_CHAIN,
    S_CHAINDOWN,
    S_CHAINUP,
    S_CHAIN1,
    S_CHAIN2,
    S_CHAIN3,
    S_CHAINFLASH1,
    S_CHAINFLASH2,
    S_MISSILE,
    S_MISSILEDOWN,
    S_MISSILEUP,
    S_MISSILE1,
    S_MISSILE2,
    S_MISSILE3,
    S_MISSILEFLASH1,
    S_MISSILEFLASH2,
    S_MISSILEFLASH3,
    S_MISSILEFLASH4,
    S_PLASMA,
    S_PLASMADOWN,
    S_PLASMAUP,
    S_PLASMAUP2,
    S_PLASMA1,
    S_PLASMA2,
    S_PLASMA3,
    S_PLASMASHOCK1,
    S_PLASMASHOCK2,
    S_PLASMASHOCK3,
    S_BFG,
    S_BFGDOWN,
    S_BFGUP,
    S_BFG1,
    S_BFG2,
    S_BFG3,
    S_BFG4,
    S_BFGFLASH1,
    S_BFGFLASH2,
    S_BFGFLASH3,
    S_UNKF,
    S_UNKFDOWN,
    S_UNKFUP,
    S_UNKF1,
    S_UNKF2,
    S_UNKFLASH1,
    S_COLU,
    S_DARTHIT,
    S_DARTHIT2,
    S_DARTHIT3,
    S_DARTHIT4,
    S_DARTHIT5,
    S_DARTHIT6,
    S_BLOODYTWITCH,
    S_DEADTORSO,
    S_DEADBOTTOM,
    S_HEADSONSTICK,
    S_GIBS,
    S_HEADONASTICK,
    S_HEADCANDLES,
    S_DEADSTICK,
    S_LIVESTICK,
    S_LIVESTICK2,
    S_MEAT2,
    S_MEAT3,
    S_MEAT4,
    S_MEAT5,
    S_STALAGTITE,
    S_TALLGRNCOL,
    S_SHRTGRNCOL,
    S_TALLREDCOL,
    S_SHRTREDCOL,
    S_CANDELABRA,
    S_SKULLCOL,
    S_TORCHTREE,
    S_BIGTREE,
    S_TECHPILLAR,
    S_BLUETORCH,
    S_BLUETORCH2,
    S_BLUETORCH3,
    S_BLUETORCH4,
    S_GREENTORCH,
    S_GREENTORCH2,
    S_GREENTORCH3,
    S_GREENTORCH4,
    S_REDTORCH,
    S_REDTORCH2,
    S_REDTORCH3,
    S_REDTORCH4,
    S_BTORCHSHRT,
    S_BTORCHSHRT2,
    S_BTORCHSHRT3,
    S_BTORCHSHRT4,
    S_BTORCHSHRT5,
    S_GTORCHSHRT,
    S_GTORCHSHRT2,
    S_GTORCHSHRT3,
    S_GTORCHSHRT4,
    S_GTORCHSHRT5,
    S_RTORCHSHRT,
    S_RTORCHSHRT2,
    S_RTORCHSHRT3,
    S_RTORCHSHRT4,
    S_RTORCHSHRT5,
    S_YFIREBALL1,
    S_YFIREBALL2,
    S_YFIREBALL3,
    S_YFIREBALL4,
    S_YFIREBALL5,
    S_BFIREBALL1,
    S_BFIREBALL2,
    S_BFIREBALL3,
    S_BFIREBALL4,
    S_RFIREBALL1,
    S_RFIREBALL2,
    S_RFIREBALL3,
    S_RFIREBALL4,
    S_Y2FIREBALL1,
    S_Y2FIREBALL2,
    S_Y2FIREBALL3,
    S_Y2FIREBALL4,
    S_B2FIREBALL1,
    S_B2FIREBALL2,
    S_B2FIREBALL3,
    S_B2FIREBALL4,
    S_R2FIREBALL1,
    S_R2FIREBALL2,
    S_R2FIREBALL3,
    S_R2FIREBALL4,
    S_HANGNOGUTS,
    S_HANGBNOBRAIN,
    S_HANGTLOOKDN,
    S_HANGTSKULL,
    S_HANGTLOOKUP,
    S_HANGTNOBRAIN,
    S_COLONGIBS,
    S_SMALLPOOL,
    S_BRAINSTEM,
    S_BRNRBALL1,
    S_BRNRBALL2,
    S_BRNRBALLX1,
    S_BRNRBALLX2,
    S_BRNRBALLX3,
    S_BRNRBALLX4,
    S_BRNRBALLX5,
    S_BRNRBALLX6,
    S_NMBL1,
    S_NMBL2,
    S_NMBL3,
    S_NMBLX1,
    S_NMBLX2,
    S_NMBLX3,
    S_NMBLX4,
    S_NMBLX5,
    S_NMBLX6,
    S_LAZR1,
    S_LAZR2,
    S_LAZRDTH,
    S_LAZRDTH2,
    S_LAZRDTH3,
    S_LAZRDTH4,
    S_LAZRDTH5,
    S_LAZRDTH6,
    S_LAZRDTH7,
    S_LAZRDTH8,
    S_LAZRDTH9,
    S_LAZRDTH10,
    S_LAZRDUST,
    S_LAZRW1,
    S_LAZRW2,
    S_RTRACER1,
    S_RTRACER2,
    S_RTRACEREXP,
    S_TECHLAMP,
    S_TECH2LAMP,
    S_TRACER,
    S_TRACER2,
    S_TRACER3,
    S_TRACEEXP1,
    S_TRACEEXP2,
    S_TRACEEXP3,
    S_TRACEEXP4,
    S_TRACEEXP5,
    S_TRACEEXP6,
    S_DISSOUL1,
    S_DISSOUL2,
    S_DISSOUL3,
    S_DISSOUL4,
    S_DISSOUL5,
    S_DISSOUL6,
    S_DISSOUL7,
    S_DISSOUL8,
    S_DISSOUL9,
    S_DBKEY1,
    S_DBKEY2,
    S_DBKEY3,
    S_DBKEY4,
    S_DBKEY5,
    S_DBKEY6,
    S_DBKEY7,
    S_DBKEY8,
    S_DBKEY9,
    S_DBKEY10,
    S_DBKEY11,
    S_DBKEY12,
    S_DBKEY13,
    S_DBKEY14,
    S_DBKEY15,
    S_DBKEY16,
    S_DBKEY17,
    S_DBKEY18,
    S_DBKEY19,
    S_IFOG,
    S_IFOG01,
    S_IFOG02,
    S_IFOG2,
    S_IFOG3,
    S_IFOG4,
    S_IFOG5,
    S_SMALL_WHITE_LIGHT,
    S_TEMPSOUNDORIGIN1,
    NUMSTATES
} statenum_t;

// Map objects.
typedef enum {
    MT_NONE = -1,
    MT_FIRST = 0,
    MT_PLAYER = MT_FIRST,
    MT_EMARINEL,
    MT_EMARINEP,
    MT_EMARINES,
    MT_POSSESSED,
    MT_SHOTGUY,
    MT_TRACER,
    MT_SMOKE,
    MT_FATSO,
    MT_FATSHOT,
    MT_TROOP,
    MT_SERGEANT,
    MT_SHADOWS,
    MT_HEAD,
    MT_BRUISER,
    MT_BRUISERSHOT,
    MT_KNIGHT,
    MT_SKULL,
    MT_BABY,
    MT_CYBORG,
    MT_DCYBORG,
    MT_PAIN,
    MT_EMARINER,
    MT_BARREL,
    MT_TROOPSHOT,
    MT_HEADSHOT,
    MT_ROCKET,
    MT_CYBERROCKET,
    MT_PLASMA,
    MT_BFG,
    MT_ARACHPLAZ,
    MT_PUFF,
    MT_BLOOD,
    MT_TFOG,
    MT_IFOG,
    MT_TELEPORTMAN,
    MT_EXTRABFG,
    MT_MISC0,
    MT_MISC1,
    MT_MISC2,
    MT_MISC3,
    MT_MISC4,
    MT_MISC5,
    MT_MISC6,
    MT_MISC7,
    MT_MISC8,
    MT_MISC9,
    MT_MISC10,
    MT_MISC11,
    MT_MISC12,
    MT_INV,
    MT_MISC13,
    MT_INS,
    MT_MISC14,
    MT_MISC15,
    MT_MISC16,
    MT_MEGA,
    MT_CLIP,
    MT_MISC17,
    MT_MISC18,
    MT_MISC19,
    MT_MISC20,
    MT_MISC21,
    MT_MISC22,
    MT_MISC23,
    MT_MISC24,
    MT_MISC25,
    MT_CHAINGUN,
    MT_MISC26,
    MT_MISC27,
    MT_MISC28,
    MT_SHOTGUN,
    MT_SUPERSHOTGUN,
    MT_MISC29,
    MT_MISC30,
    MT_MISC31,
    MT_MISC32,
    MT_MISC33,
    MT_MISC34,
    MT_MISC35,
    MT_MISC36,
    MT_MISC40,
    MT_MISC41,
    MT_MISC42,
    MT_MISC43,
    MT_MISC44,
    MT_MISC45,
    MT_MISC46,
    MT_MISC47,
    MT_MISC48,
    MT_MISC49,
    MT_MISC50,
    MT_MISC51,
    MT_MISC52,
    MT_MISC53,
    MT_MISC54,
    MT_MISC55,
    MT_MISC56,
    MT_MISC57,
    MT_MISC58,
    MT_MISC59,
    MT_MISC60,
    MT_MISC68,
    MT_MISC69,
    MT_MISC70,
    MT_MISC71,
    MT_MISC72,
    MT_MISC73,
    MT_MISC74,
    MT_MISC75,
    MT_MISC76,
    MT_MISC78,
    MT_MISC79,
    MT_MISC80,
    MT_MISC81,
    MT_MISC82,
    MT_MISC83,
    MT_MISC84,
    MT_MISC85,
    MT_MISC86,
    MT_BRUISERSHOTRED,
    MT_NTROSHOT,
    MT_ROCKETPUFF,
    MT_LASERGUN,
    MT_LASERSHOT,
    MT_LASERDUST,
    MT_LPOWERUP1,
    MT_LPOWERUP2,
    MT_LPOWERUP3,
    MT_LASERSHOTWEAK,
    MT_BITCH,
    MT_BITCHBALL,
    MT_MOTHERPUFF,
    MT_DART,

    //LIST OF SPAWN THINGS - SAMUEL
    MT_TELEPORTSHOT,
    MT_TELEPORTCHAIN,
    MT_TELEPORTSSHOT,
    MT_TELEPORTROCKET,
    MT_TELEPORTPLASMA,
    MT_TELEPORTBFG,
    MT_TELEPORTMEDKIT,
    MT_TELEPORTSTIM,
    MT_TELEPORTARMOR1,
    MT_TELEPORTARMOR2,
    MT_TELEPORTLASER,
    MT_TELEPORTLKEY1,
    MT_TELEPORTLKEY2,
    MT_TELEPORTLKEY3,
    MT_TELEPORTMEGA,
    MT_TELEPORTSOUL,
    MT_TELEPORTBLUR,
    MT_TELEPORTINVUL,
    MT_TELEPORTBERSERK,
    MT_TELEPORTPOTION,
    MT_TELEPORTHELMET,
    MT_TELEPORTMAP,
    MT_TELEPORTLIGHT,
    MT_TELEPORTSUIT,
    MT_TELEPORTSHELL,
    MT_TELEPORTSBOX,
    MT_TELEPORTCLIP,
    MT_TELEPORTBULLETS,
    MT_TELEPORTRROCKET,
    MT_TELEPORTRBOX,
    MT_TELEPORTCELL,
    MT_TELEPORTCBOX,
    MT_TELEPORTBACKPACK,
    MT_TELEPORTPOSS,
    MT_TELEPORTSPOS,
    MT_TELEPORTTROOP,
    MT_TELEPORTNTROP,
    MT_TELEPORTSARG,
    MT_TELEPORTSARG2,
    MT_TELEPORTNSARG,
    MT_TELEPORTHEAD,
    MT_TELEPORTHEAD2,
    MT_TELEPORTLOSTSOUL,
    MT_TELEPORTPAIN,
    MT_TELEPORTFATSO,
    MT_TELEPORTBABY,
    MT_TELEPORTCYBORG,
    MT_TELEPORTBITCH,
    MT_TELEPORTKNIGHT,
    MT_TELEPORTBARON,
    MT_TELEPORTRKEY,
    MT_TELEPORTRKEY2,
    MT_TELEPORTBKEY,
    MT_TELEPORTBKEY2,
    MT_TELEPORTYKEY,
    MT_TELEPORTYKEY2,
    MT_NTROOP,
    MT_KABOOM,
    MT_LIGHTSOURCE,
    MT_TEMPSOUNDORIGIN,

    MT_FAKEKNIGHT,
    MT_FAKETROOP,
    MT_FAKESHADOWS,
    MT_FAKESKULL,
    MT_F_BRUISER,
    MT_TRACER6,
    MT_YELLOWFIREBALL,
    MT_BLUEFIREBALL,
    MT_REDFIREBALL,
    MT_YELLOWFIREBALL2,
    MT_BLUEFIREBALL2,
    MT_REDFIREBALL2,
    MT_FSHOTGUY,
    MT_FDEMON,
    MT_FPAIN,
    MT_DISSOUL,
    MT_DISBKEY,
    MT_FBABY,
    MT_FFATSO,
    NUMMOBJTYPES
} mobjtype_t;

// Text.
typedef enum {
    TXT_PRESSKEY,
    TXT_PRESSYN,
    TXT_QUITMSG,
    TXT_LOADNET,
    TXT_QLOADNET,
    TXT_QSAVESPOT,
    TXT_SAVEDEAD,
    TXT_QSPROMPT,
    TXT_QLPROMPT,
    TXT_NEWGAME,
    TXT_MSGOFF,
    TXT_MSGON,
    TXT_NETEND,
    TXT_ENDGAME,
    TXT_GAMMALVL0,
    TXT_GAMMALVL1,
    TXT_GAMMALVL2,
    TXT_GAMMALVL3,
    TXT_GAMMALVL4,
    TXT_EMPTYSTRING,
    TXT_GOTARMOR,
    TXT_GOTMEGA,
    TXT_GOTHTHBONUS,
    TXT_GOTARMBONUS,
    TXT_GOTSTIM,
    TXT_GOTMEDINEED,
    TXT_GOTMEDIKIT,
    TXT_GOTSUPER,
    TXT_GOTBLUECARD,
    TXT_GOTYELWCARD,
    TXT_GOTREDCARD,
    TXT_GOTBLUESKUL,
    TXT_GOTYELWSKUL,
    TXT_GOTREDSKULL,
    TXT_GOTINVUL,
    TXT_GOTBERSERK,
    TXT_GOTINVIS,
    TXT_GOTSUIT,
    TXT_GOTMAP,
    TXT_GOTVISOR,
    TXT_GOTMSPHERE,
    TXT_GOTCLIP,
    TXT_GOTCLIPBOX,
    TXT_GOTROCKET,
    TXT_GOTROCKBOX,
    TXT_GOTCELL,
    TXT_GOTCELLBOX,
    TXT_GOTSHELLS,
    TXT_GOTSHELLBOX,
    TXT_GOTBACKPACK,
    TXT_GOTBFG9000,
    TXT_GOTCHAINGUN,
    TXT_GOTCHAINSAW,
    TXT_GOTLAUNCHER,
    TXT_GOTPLASMA,
    TXT_GOTSHOTGUN,
    TXT_GOTSHOTGUN2,
    TXT_GOTUNMAKER,
    TXT_NGOTUNMAKER,
    TXT_UNMAKERCHARGE,
    TXT_GOTPOWERUP1,
    TXT_NGOTPOWERUP1,
    TXT_GOTPOWERUP2,
    TXT_NGOTPOWERUP2,
    TXT_GOTPOWERUP3,
    TXT_NGOTPOWERUP3,
    TXT_PD_OPNPOWERUP,
    TXT_PD_BLUEO,
    TXT_PD_REDO,
    TXT_PD_YELLOWO,
    TXT_PD_BLUEK,
    TXT_PD_REDK,
    TXT_PD_YELLOWK,
    TXT_GGSAVED,
    TXT_HUSTR_MSGU,
    TXT_HUSTR_MAP01,
    TXT_HUSTR_MAP02,
    TXT_HUSTR_MAP03,
    TXT_HUSTR_MAP04,
    TXT_HUSTR_MAP05,
    TXT_HUSTR_MAP06,
    TXT_HUSTR_MAP07,
    TXT_HUSTR_MAP08,
    TXT_HUSTR_MAP09,
    TXT_HUSTR_MAP10,
    TXT_HUSTR_MAP11,
    TXT_HUSTR_MAP12,
    TXT_HUSTR_MAP13,
    TXT_HUSTR_MAP14,
    TXT_HUSTR_MAP15,
    TXT_HUSTR_MAP16,
    TXT_HUSTR_MAP17,
    TXT_HUSTR_MAP18,
    TXT_HUSTR_MAP19,
    TXT_HUSTR_MAP20,
    TXT_HUSTR_MAP21,
    TXT_HUSTR_MAP22,
    TXT_HUSTR_MAP23,
    TXT_HUSTR_MAP24,
    TXT_HUSTR_MAP25,
    TXT_HUSTR_MAP26,
    TXT_HUSTR_MAP27,
    TXT_HUSTR_MAP28,
    TXT_HUSTR_MAP29,
    TXT_HUSTR_MAP30,
    TXT_HUSTR_MAP31,
    TXT_HUSTR_MAP32,
    TXT_HUSTR_MAP33,
    TXT_HUSTR_CHATMACRO0,
    TXT_HUSTR_CHATMACRO1,
    TXT_HUSTR_CHATMACRO2,
    TXT_HUSTR_CHATMACRO3,
    TXT_HUSTR_CHATMACRO4,
    TXT_HUSTR_CHATMACRO5,
    TXT_HUSTR_CHATMACRO6,
    TXT_HUSTR_CHATMACRO7,
    TXT_HUSTR_CHATMACRO8,
    TXT_HUSTR_CHATMACRO9,
    TXT_HUSTR_TALKTOSELF1,
    TXT_HUSTR_TALKTOSELF2,
    TXT_HUSTR_TALKTOSELF3,
    TXT_HUSTR_TALKTOSELF4,
    TXT_HUSTR_TALKTOSELF5,
    TXT_HUSTR_MESSAGESENT,
    TXT_HUSTR_PLRGREEN,
    TXT_HUSTR_PLRINDIGO,
    TXT_HUSTR_PLRBROWN,
    TXT_HUSTR_PLRRED,
    TXT_AMSTR_FOLLOWON,
    TXT_AMSTR_FOLLOWOFF,
    TXT_AMSTR_GRIDON,
    TXT_AMSTR_GRIDOFF,
    TXT_AMSTR_MARKEDSPOT,
    TXT_AMSTR_MARKSCLEARED,
    TXT_STSTR_MUS,
    TXT_STSTR_NOMUS,
    TXT_STSTR_DQDON,
    TXT_STSTR_DQDOFF,
    TXT_STSTR_KFAADDED,
    TXT_STSTR_FAADDED,
    TXT_STSTR_NCON,
    TXT_STSTR_NCOFF,
    TXT_STSTR_BEHOLD,
    TXT_STSTR_BEHOLDX,
    TXT_STSTR_CHOPPERS,
    TXT_STSTR_CLEV,
    TXT_C1TEXT,
    TXT_C2TEXT,
    TXT_C3TEXT,
    TXT_C4TEXT,
    TXT_CC_ZOMBIE,
    TXT_CC_SHOTGUN,
    TXT_CC_IMP,
    TXT_CC_DEMON,
    TXT_CC_LOST,
    TXT_CC_CACO,
    TXT_CC_HELL,
    TXT_CC_BARON,
    TXT_CC_ARACH,
    TXT_CC_PAIN,
    TXT_CC_MANCU,
    TXT_CC_CYBER,
    TXT_CC_NTROOP,
    TXT_CC_BITCH,
    TXT_CC_HERO,
    TXT_QUITMESSAGE1,
    TXT_QUITMESSAGE2,
    TXT_QUITMESSAGE3,
    TXT_QUITMESSAGE4,
    TXT_QUITMESSAGE5,
    TXT_QUITMESSAGE6,
    TXT_QUITMESSAGE7,
    TXT_QUITMESSAGE8,
    TXT_QUITMESSAGE9,
    TXT_QUITMESSAGE10,
    TXT_QUITMESSAGE11,
    TXT_QUITMESSAGE12,
    TXT_QUITMESSAGE13,
    TXT_QUITMESSAGE14,
    TXT_QUITMESSAGE15,
    TXT_QUITMESSAGE16,
    TXT_QUITMESSAGE17,
    TXT_QUITMESSAGE18,
    TXT_QUITMESSAGE19,
    TXT_QUITMESSAGE20,
    TXT_QUITMESSAGE21,
    TXT_QUITMESSAGE22,
    TXT_JOINNET,
    TXT_SAVENET,
    TXT_CLNETLOAD,
    TXT_LOADMISSING,
    TXT_FINALEFLAT_C2,
    TXT_FINALEFLAT_C1,
    TXT_FINALEFLAT_C3,
    TXT_FINALEFLAT_C4,
    TXT_FINALEFLAT_C5,
    TXT_FINALEFLAT_C6,
    TXT_FINALEFLAT_C7,
    TXT_FINALEFLAT_C8,
    TXT_FINALEFLAT_C9,
    TXT_KILLMSG_SUICIDE,
    TXT_KILLMSG_WEAPON0,
    TXT_KILLMSG_PISTOL,
    TXT_KILLMSG_SHOTGUN,
    TXT_KILLMSG_CHAINGUN,
    TXT_KILLMSG_MISSILE,
    TXT_KILLMSG_PLASMA,
    TXT_KILLMSG_BFG,
    TXT_KILLMSG_CHAINSAW,
    TXT_KILLMSG_SUPERSHOTGUN,
    TXT_KILLMSG_UNMAKER,
    TXT_KILLMSG_STOMP,
    TXT_AMSTR_ROTATEON,
    TXT_AMSTR_ROTATEOFF,
    TXT_WEAPON1,
    TXT_WEAPON2,
    TXT_WEAPON3,
    TXT_WEAPON4,
    TXT_WEAPON5,
    TXT_WEAPON6,
    TXT_WEAPON7,
    TXT_WEAPON8,
    TXT_WEAPON9,
    TXT_WEAPON10,
    TXT_SKILL1,
    TXT_SKILL2,
    TXT_SKILL3,
    TXT_SKILL4,
    TXT_KEY1,
    TXT_KEY2,
    TXT_KEY3,
    TXT_KEY4,
    TXT_KEY5,
    TXT_KEY6,
    TXT_DEMONKEY1,
    TXT_DEMONKEY2,
    TXT_DEMONKEY3,
    TXT_SAVEOUTMAP,
    TXT_ENDNOGAME,
    TXT_SUICIDEOUTMAP,
    TXT_SUICIDEASK,
    TXT_PICKGAMETYPE,
    TXT_SINGLEPLAYER,
    TXT_MULTIPLAYER,
    TXT_NOTDESIGNEDFOR,
    TXT_GAMESETUP,
    TXT_PLAYERSETUP,
    TXT_DISCONNECT,
    TXT_DELETESAVEGAME_CONFIRM,
    TXT_REBORNLOAD_CONFIRM,
    NUMTEXT
} textenum_t;

// Sounds.
typedef enum {
    SFX_NONE,
    SFX_PISTOL,
    SFX_SHOTGN,
    SFX_SGCOCK,
    SFX_DSHTGN,
    SFX_DBOPN,
    SFX_DBCLS,
    SFX_DBLOAD,
    SFX_PLASMA,
    SFX_BFG,
    SFX_SAWUP,
    SFX_SAWIDL,
    SFX_SAWFUL,
    SFX_SAWHIT,
    SFX_RLAUNC,
    SFX_RXPLOD,
    SFX_FIRSHT,
    SFX_FIRXPL,
    SFX_PSTART,
    SFX_PSTOP,
    SFX_DOROPN,
    SFX_DORCLS,
    SFX_STNMOV,
    SFX_SWTCHN,
    SFX_SWTCHX,
    SFX_PLPAIN,
    SFX_DMPAIN,
    SFX_POPAIN,
    SFX_VIPAIN,
    SFX_MNPAIN,
    SFX_PEPAIN,
    SFX_SLOP,
    SFX_ITEMUP,
    SFX_WPNUP,
    SFX_OOF,
    SFX_TELEPT,
    SFX_POSIT1,
    SFX_POSIT2,
    SFX_POSIT3,
    SFX_BGSIT1,
    SFX_BGSIT2,
    SFX_SGTSIT,
    SFX_CACSIT,
    SFX_BRSSIT,
    SFX_CYBSIT,
    SFX_BSPSIT,
    SFX_KNTSIT,
    SFX_VILSIT,
    SFX_MANSIT,
    SFX_PESIT,
    SFX_SKLATK,
    SFX_SGTATK,
    SFX_SKEPCH,
    SFX_VILATK,
    SFX_CLAW,
    SFX_SKESWG,
    SFX_PLDETH,
    SFX_PDIEHI,
    SFX_PODTH1,
    SFX_PODTH2,
    SFX_PODTH3,
    SFX_BGDTH1,
    SFX_BGDTH2,
    SFX_SGTDTH,
    SFX_CACDTH,
    SFX_SKLDTH,
    SFX_BRSDTH,
    SFX_CYBDTH,
    SFX_BSPDTH,
    SFX_VILDTH,
    SFX_KNTDTH,
    SFX_PEDTH,
    SFX_SKEDTH,
    SFX_POSACT,
    SFX_BGACT,
    SFX_DMACT,
    SFX_BSPACT,
    SFX_BSPWLK,
    SFX_VILACT,
    SFX_NOWAY,
    SFX_BAREXP,
    SFX_PUNCH,
    SFX_HOOF,
    SFX_MEAL,
    SFX_CHGUN,
    SFX_TINK,
    SFX_BDOPN,
    SFX_BDCLS,
    SFX_ITMBK,
    SFX_FLAME,
    SFX_FLAMST,
    SFX_GETPOW,
    SFX_MANATK,
    SFX_MANDTH,
    SFX_SSSIT,
    SFX_SSDTH,
    SFX_SKEACT,
    SFX_SKESIT,
    SFX_SKEATK,
    SFX_RADIO,
    // jd64 >
    SFX_PSIDL,
    SFX_LASER,
    SFX_MTHATK,
    SFX_MTHSIT,
    SFX_MTHPAI,
    SFX_MTHACT,
    SFX_MTHDTH,
    SFX_HTIME,
    // < D64TC
    SFX_SECRET,
    NUMSFX
} sfxenum_t;

/**
 * Music.
 * These ids are no longer used. All tracks are played by ident.
typedef enum {
    MUS_NONE,
    MUS_RUNNIN,
    MUS_STALKS,
    MUS_COUNTD,
    MUS_BETWEE,
    MUS_DOOM,
    MUS_THE_DA,
    MUS_DDTBLU,
    MUS_DEAD,
    MUS_STLKS2,
    MUS_THEDA2,
    MUS_DOOM2,
    MUS_DDTBL2,
    MUS_RUNNI2,
    MUS_STLKS3,
    MUS_SHAWN2,
    MUS_COUNT2,
    MUS_DDTBL3,
    MUS_AMPIE,
    MUS_EVIL,
    MUS_READ_M,
    MUS_DM2TTL,
    MUS_DM2INT,
    NUMMUSIC
} musicenum_t;*/

#endif
