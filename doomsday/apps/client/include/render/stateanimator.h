/** @file stateanimator.h
 *
 * @authors Copyright (c) 2014-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
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
 * General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#ifndef DENG_CLIENT_RENDER_STATEANIMATOR_H
#define DENG_CLIENT_RENDER_STATEANIMATOR_H

#include "render/modelrenderer.h"

#include <doomsday/world/mobj.h>
#include <de/ModelDrawable>
#include <de/GLProgram>
#include <de/IObject>
#include <de/Scheduler>

namespace render {

/**
 * State-based object animator for `ModelDrawable`s. Triggers animation sequences
 * based on state changes and damage points received by the owner.
 *
 * Used for both mobjs and psprites. The state and movement of the object
 * determine which animation sequences are started.
 *
 * @par Scripting
 *
 * The script object has the following constants:
 * - `ID` (Text): asset identifier.
 * - `__asset__` (Record): asset metadata.
 */
class StateAnimator : public de::ModelDrawable::Animator,
                      public de::IObject
{
public:
    DENG2_ERROR(DefinitionError);

public:
    StateAnimator();
    StateAnimator(de::DotPath const &id, Model const &model);

    Model const &model() const;

    /**
     * Returns the script scheduler specific to this animator. A new scheduler will
     * be created the first time this is called.
     */
    de::Scheduler &scheduler();

    /**
     * Sets the namespace of the animator's owner. Available as a variable in
     * animation scripts.
     *
     * @param names    Owner's namespace.
     * @param varName  Name of the variable that points to @a names.
     */
    void setOwnerNamespace(de::Record &names, de::String const &varName);

    /**
     * Returns the name of the variable pointing to the owning object's namespace.
     */
    de::String ownerNamespaceName() const;

    void triggerByState(de::String const &stateName);

    void triggerDamage(int points, struct mobj_s const *inflictor);

    void startAnimation(int animationId, int priority, bool looping,
                        de::String const &node = "");

    int animationId(de::String const &name) const;

    de::ModelDrawable::Appearance const &appearance() const;

    // ModelDrawable::Animator
    void advanceTime(de::TimeSpan const &elapsed) override;
    de::ddouble currentTime(int index) const override;
    de::Vector4f extraRotationForNode(de::String const &nodeName) const override;

    // Implements IObject.
    de::Record &       objectNamespace()       override;
    de::Record const & objectNamespace() const override;

    // ISerializable.
    void operator >> (de::Writer &to) const override;
    void operator << (de::Reader &from) override;

private:
    DENG2_PRIVATE(d)
};

} // namespace render

#endif // DENG_CLIENT_RENDER_STATEANIMATOR_H
