/** @file texturemateriallayer.h  Logical material, texture layer.
 *
 * @authors Copyright © 2011-2014 Daniel Swanson <danij@dengine.net>
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

#ifndef LIBDOOMSDAY_WORLD_TEXTUREMATERIALLAYER_H
#define LIBDOOMSDAY_WORLD_TEXTUREMATERIALLAYER_H

#include <de/String>
#include "../defs/dedtypes.h"
#include "Material"

namespace world {

/**
 * Specialized MaterialLayer for describing an animated texture layer.
 *
 * @ingroup resource
 */
class LIBDOOMSDAY_PUBLIC TextureMaterialLayer : public MaterialLayer
{
public:
    /**
     * Stages describe texture change animations.
     */
    class LIBDOOMSDAY_PUBLIC AnimationStage : public Stage
    {
    public:
        AnimationStage(de::Uri const &texture, int tics,
                       float variance                     = 0,
                       float glowStrength                 = 0,
                       float glowStrengthVariance         = 0,
                       de::Vector2f const origin          = de::Vector2f(),
                       de::Uri const &maskTexture         = de::Uri(),
                       de::Vector2f const &maskDimensions = de::Vector2f(1, 1),
                       blendmode_t blendMode              = BM_NORMAL,
                       float opacity                      = 1);
        AnimationStage(AnimationStage const &other);
        virtual ~AnimationStage();

        virtual void resetToDefaults();

        /**
         * Construct a new AnimationStage from the given @a stageDef.
         */
        static AnimationStage *fromDef(de::Record const &stageDef);

        de::String description() const;

    public:
        de::Uri      texture;
        float        glowStrength;
        float        glowStrengthVariance;
        de::Vector2f origin;
        de::Uri      maskTexture;
        de::Vector2f maskDimensions;
        blendmode_t  blendMode;
        float        opacity;
    };

public:
    virtual ~TextureMaterialLayer() {}

    /**
     * Construct a new TextureLayer from the given @a layerDef.
     */
    static TextureMaterialLayer *fromDef(de::Record const &layerDef);

    /**
     * Returns @c true if glow is enabled for one or more animation stages.
     */
    bool hasGlow() const;

    /**
     * Add a new animation stage to the texture layer.
     *
     * @param stage  New stage to add (a copy is made).
     *
     * @return  Index of the newly added stage (0 based).
     */
    int addStage(AnimationStage const &stage);

    /**
     * Lookup an AnimationStage by it's unique @a index.
     *
     * @param index  Index of the AnimationStage to lookup. Will be cycled into valid range.
     */
    AnimationStage const &stage(int index) const;

    /**
     * Lookup an AnimationStage by it's unique @a index.
     *
     * @param index  Index of the AnimationStage to lookup. Will be cycled into valid range.
     */
    AnimationStage &stage(int index);

    de::String describe() const;
};

} // namespace world

#endif  // LIBDOOMSDAY_WORLD_TEXTUREMATERIALLAYER_H
