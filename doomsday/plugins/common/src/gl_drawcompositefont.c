/**\file
 *\section License
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2003-2010 Jaakko Keränen <jaakko.keranen@iki.fi>
 *\author Copyright © 2005-2010 Daniel Swanson <danij@dengine.net>
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

// HEADER FILES ------------------------------------------------------------

#include <assert.h>
#include <string.h>
#include <ctype.h>

// \todo Only included for the config. Refactor away.
#if __JDOOM__
#  include "jdoom.h"
#elif __JDOOM64__
#  include "jdoom64.h"
#elif __JHERETIC__
#  include "jheretic.h"
#elif __JHEXEN__
#  include "jhexen.h"
#endif

#include "gl_drawpatch.h"
#include "gl_drawcompositefont.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef struct {
    struct compositefont_char_s {
        char            lumpname[9];
        patchinfo_t     pInfo;
    } chars[256];
} compositefont_t;

typedef struct {
    gamefontid_t font;
    float scaleX, scaleY;
    float offX, offY;
    float angle;
    float color[4];
    float tracking;
    float leading;
    boolean typeIn;
    boolean caseScale;
    struct {
        float scale, offset;
    } caseMod[2]; // 1=upper, 0=lower

    int numBreaks;
} drawtextstate_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void drawFlash(int x, int y, int w, int h, int bright, float r, float g, float b, float a);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static compositefont_t fonts[NUM_GAME_FONTS];

// CODE --------------------------------------------------------------------

static __inline patchid_t patchForFontChar(gamefontid_t font, unsigned char ch)
{
    assert(font >= GF_FIRST && font < NUM_GAME_FONTS);
    return fonts[font].chars[ch].pInfo.id;
}

static __inline short translateTextToPatchDrawFlags(byte in)
{
    short out = 0;
    if(in & DTF_ALIGN_LEFT)
        out |= DPF_ALIGN_LEFT;
    if(in & DTF_ALIGN_RIGHT)
        out |= DPF_ALIGN_RIGHT;
    if(in & DTF_ALIGN_BOTTOM)
        out |= DPF_ALIGN_BOTTOM;
    if(in & DTF_ALIGN_TOP)
        out |= DPF_ALIGN_TOP;
    return out;
}

/**
 * Expected: <whitespace> = <whitespace> <float>
 */
static float parseFloat(char** str)
{
    float value;
    char* end;

    *str = M_SkipWhite(*str);
    if(**str != '=')
        return 0; // Now I'm confused!

    *str = M_SkipWhite(*str + 1);
    value = strtod(*str, &end);
    *str = end;
    return value;
}

static void parseParamaterBlock(char** strPtr, drawtextstate_t* state)
{
    (*strPtr)++;
    while(*(*strPtr) && *(*strPtr) != '}')
    {
        (*strPtr) = M_SkipWhite((*strPtr));

        // What do we have here?
        if(!strnicmp((*strPtr), "fonta", 5))
        {
            state->font = GF_FONTA;
            (*strPtr) += 5;
        }
        else if(!strnicmp((*strPtr), "fontb", 5))
        {
            state->font = GF_FONTB;
            (*strPtr) += 5;
        }
        else if(!strnicmp((*strPtr), "flash", 5))
        {
            (*strPtr) += 5;
            state->typeIn = true;
        }
        else if(!strnicmp((*strPtr), "noflash", 7))
        {
            (*strPtr) += 7;
            state->typeIn = false;
        }
        else if(!strnicmp((*strPtr), "case", 4))
        {
            (*strPtr) += 4;
            state->caseScale = true;
        }
        else if(!strnicmp((*strPtr), "nocase", 6))
        {
            (*strPtr) += 6;
            state->caseScale = false;
        }
        else if(!strnicmp((*strPtr), "ups", 3))
        {
            (*strPtr) += 3;
            state->caseMod[1].scale = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "upo", 3))
        {
            (*strPtr) += 3;
            state->caseMod[1].offset = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "los", 3))
        {
            (*strPtr) += 3;
            state->caseMod[0].scale = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "loo", 3))
        {
            (*strPtr) += 3;
            state->caseMod[0].offset = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "break", 5))
        {
            (*strPtr) += 5;
            state->numBreaks++;
        }
        else if(!strnicmp((*strPtr), "r", 1))
        {
            (*strPtr)++;
            state->color[CR] = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "g", 1))
        {
            (*strPtr)++;
            state->color[CG] = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "b", 1))
        {
            (*strPtr)++;
            state->color[CB] = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "x", 1))
        {
            (*strPtr)++;
            state->offX = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "y", 1))
        {
            (*strPtr)++;
            state->offY = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "scalex", 6))
        {
            (*strPtr) += 6;
            state->scaleX = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "scaley", 6))
        {
            (*strPtr) += 6;
            state->scaleY = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "scale", 5))
        {
            (*strPtr) += 5;
            state->scaleX = state->scaleY = parseFloat(&(*strPtr));
        }
        else if(!strnicmp((*strPtr), "angle", 5))
        {
            (*strPtr) += 5;
            state->angle = parseFloat(&(*strPtr));
        }
        else
        {
            // Unknown, skip it.
            if(*(*strPtr) != '}')
                (*strPtr)++;
        }
    }

    // Skip over the closing brace.
    if(*(*strPtr))
        (*strPtr)++;
}

void R_SetFontCharacter(gamefontid_t fontid, byte ch, const char* lumpname)
{
    compositefont_t* font;

    if(!(fontid >= GF_FIRST && fontid < NUM_GAME_FONTS))
    {
        Con_Message("R_SetFontCharacter: Warning, unknown font id %i.\n", (int) fontid);
        return;
    }

    font = &fonts[fontid];
    memset(font->chars[ch].lumpname, 0, sizeof(font->chars[ch].lumpname));
    strncpy(font->chars[ch].lumpname, lumpname, 8);

    // Instruct Doomsday to load the patch in monochrome mode.
    // (2 = weighted average).
    DD_SetInteger(DD_MONOCHROME_PATCHES, 2);
    DD_SetInteger(DD_UPSCALE_AND_SHARPEN_PATCHES, true);

    R_PrecachePatch(font->chars[ch].lumpname, &font->chars[ch].pInfo);

    DD_SetInteger(DD_UPSCALE_AND_SHARPEN_PATCHES, false);
    DD_SetInteger(DD_MONOCHROME_PATCHES, 0);
}

void R_InitFont(gamefontid_t fontid, const fontpatch_t* patches, size_t num)
{
    compositefont_t* font;
    size_t i;

    if(!(fontid >= GF_FIRST && fontid < NUM_GAME_FONTS))
    {
        Con_Message("R_InitFont: Warning, unknown font id %i.\n", (int) fontid);
        return;
    }

    font = &fonts[fontid];
    memset(font, 0, sizeof(*font));
    for(i = 0; i < 256; ++i)
        font->chars[i].pInfo.id = -1;

    for(i = 0; i < num; ++i)
    {
        const fontpatch_t* p = &patches[i];
        R_SetFontCharacter(fontid, p->ch, p->lumpName);
    }
}

static void initDrawTextState(drawtextstate_t* state)
{
    state->font = GF_FONTA;
    state->scaleX = state->scaleY = 1;
    state->offX = state->offY = 0;
    state->angle = 0;
    state->color[CR] = state->color[CG] = state->color[CB] = state->color[CA] = 1;
    state->tracking = 0;
    state->leading = .25f;
    state->typeIn = true;
    state->caseScale = false;
    state->caseMod[0].scale = 1;
    state->caseMod[0].offset = 3;
    state->caseMod[1].scale = 1.25f;
    state->caseMod[1].offset = 0;
    state->numBreaks = 0;
}

/**
 * Draw a string of text controlled by parameter blocks.
 */
void GL_DrawText(const char* inString, int x, int y, gamefontid_t defFont,
    short flags, int defTracking, float defRed, float defGreen, float defBlue, float defAlpha,
    boolean defCase)
{
#define SMALLBUFF_SIZE  80

    char smallBuff[SMALLBUFF_SIZE+1], *bigBuff = NULL;
    char temp[256], *str, *string, *end;
    int charCount = 0, curCase = -1;
    float cx = x, cy = y, width = 0, extraScale;
    drawtextstate_t state;
    size_t len;

    if(!inString || !inString[0])
        return;

    len = strlen(inString);
    if(len < SMALLBUFF_SIZE)
    {
        memcpy(smallBuff, inString, len);
        smallBuff[len] = '\0';

        str = smallBuff;
    }
    else
    {
        bigBuff = malloc(len+1);
        memcpy(bigBuff, inString, len);
        bigBuff[len] = '\0';

        str = bigBuff;
    }

    initDrawTextState(&state);
    // Apply defaults:
    state.font = defFont;
    state.typeIn = (flags & DTF_NO_TYPEIN) == 0;
    state.color[CR] = defRed;
    state.color[CG] = defGreen;
    state.color[CB] = defBlue;
    state.color[CA] = defAlpha;
    state.tracking = defTracking;
    state.caseScale = defCase;

    string = str;
    while(*string)
    {
        // Parse and draw the replacement string.
        if(*string == '{') // Parameters included?
        {
            gamefontid_t oldFont = state.font;
            float oldScaleY = state.scaleY;
            float oldLeading = state.leading;

            parseParamaterBlock(&string, &state);

            if(state.numBreaks > 0)
            {
                do
                {
                    cx = x;
                    cy += GL_CharHeight(0, oldFont) * (1+oldLeading) * oldScaleY;
                } while(--state.numBreaks > 0);
            }
        }

        for(end = string; *end && *end != '{';)
        {
            boolean newline = false;
            short fragmentFlags;
            int alignx = 0;

            // Find the end of the next fragment.
            if(state.caseScale)
            {
                curCase = -1;
                // Select a substring with characters of the same case (or whitespace).
                for(; *end && *end != '{' && *end != '\n'; end++)
                {
                    // We can other skip whitespace.
                    if(isspace(*end))
                        continue;

                    if(curCase < 0)
                        curCase = (isupper(*end) != 0);
                    else if(curCase != (isupper(*end) != 0))
                        break;
                }
            }
            else
            {
                curCase = 0;
                for(; *end && *end != '{' && *end != '\n'; end++);
            }

            strncpy(temp, string, end - string);
            temp[end - string] = 0;

            if(end && *end == '\n')
            {
                newline = true;
                ++end;
            }

            // Continue from here.
            string = end;

            if(!(flags & (DTF_ALIGN_LEFT|DTF_ALIGN_RIGHT)))
            {
                fragmentFlags = flags;
            }
            else
            {
                // We'll take care of horizontal positioning of the fragment so align left.
                fragmentFlags = (flags & ~(DTF_ALIGN_RIGHT)) | DTF_ALIGN_LEFT;
                if(flags & DTF_ALIGN_RIGHT)
                    alignx = -GL_TextFragmentWidth2(temp, state.font, state.tracking) * state.scaleX;
            }

            // Setup the scaling.
            DGL_MatrixMode(DGL_MODELVIEW);
            DGL_PushMatrix();

            // Rotate.
            if(state.angle != 0)
            {
                // The origin is the specified (x,y) for the patch.
                // We'll undo the VGA aspect ratio (otherwise the result would
                // be skewed).
                DGL_Translatef(x, y, 0);
                DGL_Scalef(1, 200.0f / 240.0f, 1);
                DGL_Rotatef(state.angle, 0, 0, 1);
                DGL_Scalef(1, 240.0f / 200.0f, 1);
                DGL_Translatef(-x, -y, 0);
            }

            DGL_Translatef(cx + state.offX + alignx, cy + state.offY + (state.caseScale ? state.caseMod[curCase].offset : 0), 0);
            extraScale = (state.caseScale ? state.caseMod[curCase].scale : 1);
            DGL_Scalef(state.scaleX, state.scaleY * extraScale, 1);

            // Draw it.
            DGL_Color4fv(state.color);
            GL_DrawTextFragment5(temp, 0, 0, state.font, fragmentFlags, state.tracking, state.typeIn ? charCount : 0);
            charCount += strlen(temp);

            // Advance the current position?
            if(!newline)
            {
                cx += (float) GL_TextFragmentWidth2(temp, state.font, state.tracking) * state.scaleX;
            }
            else
            {
                cx = x;
                cy += (float) GL_TextFragmentHeight(temp, state.font) * (1+state.leading) * state.scaleY;
            }

            DGL_MatrixMode(DGL_MODELVIEW);
            DGL_PopMatrix();
        }
    }

    // Free temporary storage.
    if(bigBuff)
        free(bigBuff);

#undef SMALLBUFF_SIZE
}

int GL_CharWidth(unsigned char ch, gamefontid_t font)
{
    assert(font >= GF_FIRST && font < NUM_GAME_FONTS);
    return fonts[font].chars[ch].pInfo.width;
}

int GL_CharHeight(unsigned char ch, gamefontid_t font)
{
    assert(font >= GF_FIRST && font < NUM_GAME_FONTS);
    return fonts[font].chars[ch].pInfo.height;
}

/**
 * Find the visible width of the text string fragment.
 * \note Does NOT skip paramater blocks - assumes the caller has dealt
 * with them and not passed them.
 */
int GL_TextFragmentWidth2(const char* string, gamefontid_t font, int tracking)
{
    assert(string);
    {
    const char* ch;
    unsigned char c;
    size_t len;
    int width;
    uint i;

    if(!string[0])
        return 0;

    i = 0;
    width = 0;
    len = strlen(string);
    ch = string;
    while(i++ < len && (c = *ch++) != 0 && c != '\n')
        width += GL_CharWidth(c, font);

    return width + tracking * (len-1);
    }
}

int GL_TextFragmentWidth(const char* string, gamefontid_t font)
{
    return GL_TextFragmentWidth2(string, font, 0);
}

/**
 * Find the visible height of the text string fragment.
 * \note Does NOT skip paramater blocks - assumes the caller has dealt
 * with them and not passed them.
 */
int GL_TextFragmentHeight(const char* string, gamefontid_t font)
{
    assert(string);
    {
    const char* ch;
    unsigned char c;
    size_t len;
    int height;
    uint i;

    if(!string[0])
        return 0;

    i = 0;
    height = 0;
    len = strlen(string);
    ch = string;
    while(i++ < len && (c = *ch++) != 0 && c != '\n')
    {
        int charHeight = GL_CharHeight(c, font);
        if(charHeight > height)
            height = charHeight;
    }

    return height;
    }
}

/**
 * Find the visible width of the whole formatted text string.
 * Skips parameter blocks eg "{param}Text" = 4 chars
 */
int GL_TextWidth(const char* string, gamefontid_t font)
{
    assert(string);
    {
    int w, maxWidth = -1;
    boolean skip = false;
    const char* ch;
    size_t i, len;

    if(!string[0])
        return 0;
    if(font < 0 || font >= NUM_GAME_FONTS)
        return 0;

    w = 0;
    len = strlen(string);
    ch = string;
    for(i = 0; i < len; ++i, ch++)
    {
        unsigned char c = *ch;

        if(c == '{')
            skip = true;
        else if(c == '}')
            skip = false;
        if(skip)
            continue;

        if(c == '\n')
        {
            if(w > maxWidth)
                maxWidth = w;
            w = 0;
            continue;
        }

        w += GL_CharWidth(c, font);

        if(i == len - 1 && maxWidth == -1)
        {
            maxWidth = w;
        }
    }

    return maxWidth;
    }
}

/**
 * Find the visible height of the whole formatted text string.
 * Skips parameter blocks eg "{param}Text" = 4 chars
 */
int GL_TextHeight(const char* string, gamefontid_t font)
{
    assert(string);
    {
    int h, currentLineHeight;
    boolean skip = false;
    const char* ch;
    size_t i, len;

    if(!string[0])
        return 0;
    if(font < 0 || font >= NUM_GAME_FONTS)
        return 0;
    
    currentLineHeight = 0;
    len = strlen(string);
    h = 0;
    ch = string;
    for(i = 0; i < len; ++i, ch++)
    {
        unsigned char c = *ch;
        int charHeight;

        if(c == '{')
            skip = true;
        else if(c == '}')
            skip = false;
        if(skip)
            continue;

        if(c == '\n')
        {
            h += currentLineHeight == 0? GL_CharHeight('A', font) : currentLineHeight;
            currentLineHeight = 0;
            continue;
        }

        charHeight = GL_CharHeight(c, font);
        if(charHeight > currentLineHeight)
            currentLineHeight = charHeight;
    }
    h += currentLineHeight;

    return h;
    }
}

/**
 * Write a string using a colored, custom font and do a type-in effect.
 */
void GL_DrawTextFragment5(const char* string, int x, int y, gamefontid_t font, short flags,
    int tracking, int initialCount)
{
    boolean noTypein = (flags & DTF_NO_TYPEIN) != 0;
    boolean noShadow = (flags & DTF_NO_SHADOW) != 0;
    int i, pass, w, h, cx, cy, count, yoff;
    float flash, origColor[4], flashColor[4];
    unsigned char c;
    const char* ch;

    if(!string || !string[0])
        return;

    if(flags & DTF_ALIGN_RIGHT)
        x -= GL_TextFragmentWidth2(string, font, tracking);
    else if(!(flags & DTF_ALIGN_LEFT))
        x -= GL_TextFragmentWidth2(string, font, tracking)/2;

    if(flags & DTF_ALIGN_BOTTOM)
        y -= GL_TextFragmentHeight(string, font);
    else if(!(flags & DTF_ALIGN_TOP))
        y -= GL_TextFragmentHeight(string, font)/2;

    if(!noTypein || !noShadow)
        DGL_GetFloatv(DGL_CURRENT_COLOR_RGBA, origColor);

    if(!noTypein)
    {
        for(i = 0; i < 3; ++i)
            flashColor[i] = (1 + 2 * origColor[i]) / 3;
        flashColor[CA] = cfg.menuGlitter * origColor[CA];
    }

    for(pass = (noShadow? 1 : 0); pass < 2; ++pass)
    {
        count = initialCount;
        ch = string;
        cx = x;
        cy = y;

        if(!noTypein || !noShadow)
            DGL_Color4fv(origColor);

        for(;;)
        {
            c = *ch++;
            yoff = 0;
            flash = 0;
            // Do the type-in effect?
            if(!noTypein)
            {
                int maxCount = (typeInTime > 0? typeInTime * 2 : 0);

                if(count == maxCount)
                {
                    flash = 1;
                    DGL_Color4f(1, 1, 1, origColor[CA]);
                }
                else if(count + 1 == maxCount)
                {
                    flash = 0.5f;
                    DGL_Color4f((1 + origColor[CR]) / 2, (1 + origColor[CG]) / 2, (1 + origColor[CB]) / 2, origColor[CA]);
                }
                else if(count + 2 == maxCount)
                {
                    flash = 0.25f;
                    DGL_Color4fv(origColor);
                }
                else if(count + 3 == maxCount)
                {
                    flash = 0.12f;
                    DGL_Color4fv(origColor);
                }
                else if(count > maxCount)
                {
                    break;
                }
            }
            count++;

            if(!c || c == '\n')
                break;

            w = GL_CharWidth(c, font);
            h = GL_CharHeight(c, font);

            if(patchForFontChar(font, c) != -1 && c != ' ')
            {
                // A character we have a patch for that is not white space.
                if(pass)
                {
                    // The character itself.
                    GL_DrawChar2(c, cx, cy + yoff, font);

                    if(flash > 0)
                    {   // Do something flashy.
                        drawFlash(cx, cy + yoff, w, h, true, flashColor[CR], flashColor[CG], flashColor[CB], flashColor[CA] * flash);
                    }
                }
                else if(!noShadow)
                {   // Shadow.
                    drawFlash(cx, cy + yoff, w, h, false, 1, 1, 1, origColor[CA] * cfg.menuShadow);
                }
            }

            cx += w + tracking;
        }
    }

    if(!noTypein || !noShadow)
    {   // Ensure we restore the original color.
        DGL_Color4fv(origColor);
    }
}

void GL_DrawTextFragment4(const char* string, int x, int y, gamefontid_t font, short flags, int tracking)
{
    GL_DrawTextFragment5(string, x, y, font, flags, tracking, 0);
}

void GL_DrawTextFragment3(const char* string, int x, int y, gamefontid_t font, short flags)
{
    GL_DrawTextFragment4(string, x, y, font, flags, 0);
}

void GL_DrawTextFragment2(const char* string, int x, int y, gamefontid_t font)
{
    GL_DrawTextFragment3(string, x, y, font, DTF_ALIGN_TOPLEFT|DTF_NO_TYPEIN);
}

void GL_DrawTextFragment(const char* string, int x, int y)
{
    GL_DrawTextFragment2(string, x, y, GF_FONTA);
}

void GL_DrawChar3(unsigned char ch, int x, int y, gamefontid_t font, short flags)
{
    GL_DrawPatch2(patchForFontChar(font, ch), x, y, translateTextToPatchDrawFlags(flags));
}

void GL_DrawChar2(unsigned char ch, int x, int y, gamefontid_t font)
{
    GL_DrawChar3(ch, x, y, font, DTF_ALIGN_TOPLEFT);
}

void GL_DrawChar(unsigned char ch, int x, int y)
{
    GL_DrawChar2(ch, x, y, GF_FONTA);
}

static void drawFlash(int x, int y, int w, int h, int bright, float r,
    float g, float b, float a)
{
    float fsize = 4 + bright, red, green, blue, alpha;
    float fw = fsize * w / 2.0f, fh = fsize * h / 2.0f;

    // Don't draw anything for very small letters.
    if(h <= 4)
        return;

    // Don't bother with hidden letters.
    if(!(a > 0.f))
        return;

    red   = MINMAX_OF(0.f, r, 1.f);
    green = MINMAX_OF(0.f, g, 1.f);
    blue  = MINMAX_OF(0.f, b, 1.f);
    alpha = MINMAX_OF(0.f, a, 1.f);

    DGL_Bind(Get(DD_DYNLIGHT_TEXTURE));

    if(bright)
        DGL_BlendMode(BM_ADD);
    else
        DGL_BlendFunc(DGL_ZERO, DGL_ONE_MINUS_SRC_ALPHA);

    DGL_DrawRect(x + w / 2.0f - fw / 2, y + h / 2.0f - fh / 2, fw, fh, red, green, blue, alpha);

    DGL_BlendMode(BM_NORMAL);
}
