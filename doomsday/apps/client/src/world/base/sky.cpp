/** @file sky.cpp  Sky behavior logic for the world system.
 *
 * @authors Copyright © 2003-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
 * @authors Copyright © 2006-2014 Daniel Swanson <danij@dengine.net>
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

#include "world/sky.h"

#include <cmath>
#include <QtAlgorithms>
#include <de/Log>
#include <doomsday/res/TextureManifest>
#include <doomsday/world/Materials>
#include "dd_main.h"

#ifdef __CLIENT__
#  include "gl/gl_main.h"
#  include "gl/gl_tex.h"
#  include "render/rend_main.h"    // rendSkyLightAuto
#  include "render/skydrawable.h"  // SkyDrawable::layerMaterialSpec
#  include "MaterialAnimator"
#  include "ClientTexture"
#endif

#define NUM_LAYERS  2

using namespace de;

namespace world {

DENG2_PIMPL_NOREF(Sky::Layer)
{
    bool active         = false;
    bool masked         = false;
    Material *material = nullptr;
    dfloat offset       = 0;
    dfloat fadeOutLimit = 0;

    Sky &sky;
    Impl(Sky &sky) : sky(sky) {}

    DENG2_PIMPL_AUDIENCE(ActiveChange)
    DENG2_PIMPL_AUDIENCE(MaskedChange)
    DENG2_PIMPL_AUDIENCE(MaterialChange)
};

DENG2_AUDIENCE_METHOD(Sky::Layer, ActiveChange)
DENG2_AUDIENCE_METHOD(Sky::Layer, MaskedChange)
DENG2_AUDIENCE_METHOD(Sky::Layer, MaterialChange)

Sky::Layer::Layer(Sky &sky, Material *material) : d(new Impl(sky))
{
    setMaterial(material);
}

Sky &Sky::Layer::sky() const
{
    return d->sky;
}

bool Sky::Layer::isActive() const
{
    return d->active;
}

void Sky::Layer::setActive(bool yes)
{
    if(d->active != yes)
    {
        d->active = yes;
        DENG2_FOR_AUDIENCE2(ActiveChange, i) i->skyLayerActiveChanged(*this);
    }
}

bool Sky::Layer::isMasked() const
{
    return d->masked;
}

void Sky::Layer::setMasked(bool yes)
{
    if(d->masked != yes)
    {
        d->masked = yes;
        DENG2_FOR_AUDIENCE2(MaskedChange, i) i->skyLayerMaskedChanged(*this);
    }
}

Material *Sky::Layer::material() const
{
    return d->material;
}

void Sky::Layer::setMaterial(Material *newMaterial)
{
    if(d->material != newMaterial)
    {
        d->material = newMaterial;
        DENG2_FOR_AUDIENCE2(MaterialChange, i) i->skyLayerMaterialChanged(*this);
    }
}

dfloat Sky::Layer::offset() const
{
    return d->offset;
}

void Sky::Layer::setOffset(dfloat newOffset)
{
    d->offset = newOffset;
}

dfloat Sky::Layer::fadeOutLimit() const
{
    return d->fadeOutLimit;
}

void Sky::Layer::setFadeoutLimit(dfloat newLimit)
{
    d->fadeOutLimit = newLimit;
}

#ifdef __CLIENT__
static Vector3f const AmbientLightColorDefault(1, 1, 1); // Pure white.
#endif

DENG2_PIMPL(Sky)
#ifdef __CLIENT__
, DENG2_OBSERVES(Layer, ActiveChange)
, DENG2_OBSERVES(Layer, MaterialChange)
, DENG2_OBSERVES(Layer, MaskedChange)
#endif
{
    struct Layers : public QList<Layer *>
    {
        ~Layers() { qDeleteAll(*this); }
    };

    Layers layers;

    Record const *def    = nullptr; ///< Sky definition.
    dfloat height        = 1;
    dfloat horizonOffset = 0;

    Impl(Public *i) : Base(i)
    {
        for(dint i = 0; i < NUM_LAYERS; ++i)
        {
            layers.append(new Layer(self()));
#ifdef __CLIENT__
            Layer *layer = layers.last();
            layer->audienceForActiveChange()   += this;
            layer->audienceForMaskedChange()   += this;
            layer->audienceForMaterialChange() += this;
#endif
        }
    }

    ~Impl()
    {
        DENG2_FOR_PUBLIC_AUDIENCE2(Deletion, i) i->skyBeingDeleted(self());
    }

#ifdef __CLIENT__
    /**
     * Ambient lighting characteristics.
     */
    struct AmbientLight
    {
        bool custom     = false;  /// @c true= defined in a MapInfo def.
        bool needUpdate = true;   /// @c true= update if not custom.
        Vector3f color;

        void setColor(Vector3f const &newColor, bool isCustom = true)
        {
            color  = newColor.min(Vector3f(1, 1, 1)).max(Vector3f(0, 0, 0));
            custom = isCustom;
        }

        void reset()
        {
            custom     = false;
            color      = AmbientLightColorDefault;
            needUpdate = true;
        }
    } ambientLight;

    /**
     * @todo Move to SkyDrawable and have it simply update this component once the
     * ambient color has been calculated.
     *
     * @todo Re-implement me by rendering the sky to a low-quality cubemap and use
     * that to obtain the lighting characteristics.
     */
    void updateAmbientLightIfNeeded()
    {
        // Never update if a custom color is defined.
        if(ambientLight.custom) return;

        // Is it time to update the color?
        if(!ambientLight.needUpdate) return;
        ambientLight.needUpdate = false;

        ambientLight.color = AmbientLightColorDefault;

        // Determine the first active layer.
        dint firstActiveLayer = -1; // -1 denotes 'no active layers'.
        for(dint i = 0; i < layers.count(); ++i)
        {
            if(layers[i]->isActive())
            {
                firstActiveLayer = i;
                break;
            }
        }

        // A sky with no active layer uses the default color.
        if(firstActiveLayer < 0) return;

        Vector3f avgLayerColor;
        Vector3f bottomCapColor;
        Vector3f topCapColor;

        dint avgCount = 0;
        for(dint i = firstActiveLayer; i < layers.count(); ++i)
        {
            Layer &layer = *layers[i];

            // Inactive layers won't be drawn.
            if(!layer.isActive()) continue;

            // A material is required for drawing.
            ClientMaterial *mat = static_cast<ClientMaterial *>(layer.material());
            if(!mat) continue;
            MaterialAnimator &matAnimator = mat->getAnimator(SkyDrawable::layerMaterialSpec(layer.isMasked()));

            // Ensure we've up to date info about the material.
            matAnimator.prepare();

            if(TextureVariant *tex = matAnimator.texUnit(MaterialAnimator::TU_LAYER0).texture)
            {
                auto const *avgColor = reinterpret_cast<averagecolor_analysis_t const *>(tex->base().analysisDataPointer(ClientTexture::AverageColorAnalysis));
                if(!avgColor) throw Error("calculateSkyAmbientColor", "Texture \"" + tex->base().manifest().composeUri().asText() + "\" has no AverageColorAnalysis");

                if(i == firstActiveLayer)
                {
                    auto const *avgLineColor = reinterpret_cast<averagecolor_analysis_t const *>(tex->base().analysisDataPointer(ClientTexture::AverageTopColorAnalysis));
                    if(!avgLineColor) throw Error("calculateSkyAmbientColor", "Texture \"" + tex->base().manifest().composeUri().asText() + "\" has no AverageTopColorAnalysis");

                    topCapColor = Vector3f(avgLineColor->color.rgb);

                    avgLineColor = reinterpret_cast<averagecolor_analysis_t const *>(tex->base().analysisDataPointer(ClientTexture::AverageBottomColorAnalysis));
                    if(!avgLineColor) throw Error("calculateSkyAmbientColor", "Texture \"" +  tex->base().manifest().composeUri().asText() + "\" has no AverageBottomColorAnalysis");

                    bottomCapColor = Vector3f(avgLineColor->color.rgb);
                }

                avgLayerColor += Vector3f(avgColor->color.rgb);
                ++avgCount;
            }
        }

        // The caps cover a large amount of the sky sphere, so factor it in too.
        // Each cap is another unit.
        ambientLight.setColor((avgLayerColor + topCapColor + bottomCapColor) / (avgCount + 2),
                              false /*not a custom color*/);
    }

    /// Observes Layer ActiveChange
    void skyLayerActiveChanged(Layer &)
    {
        ambientLight.needUpdate = true;
    }

    /// Observes Layer MaterialChange
    void skyLayerMaterialChanged(Layer &layer)
    {
        // We may need to recalculate the ambient color of the sky.
        if(!layer.isActive()) return;
        //if(ambientLight.custom) return;

        ambientLight.needUpdate = true;
    }

    /// Observes Layer MaskedChange
    void skyLayerMaskedChanged(Layer &layer)
    {
        // We may need to recalculate the ambient color of the sky.
        if(!layer.isActive()) return;
        //if(ambientLight.custom) return;

        ambientLight.needUpdate = true;
    }

#endif  // __CLIENT__

    DENG2_PIMPL_AUDIENCE(Deletion)
    DENG2_PIMPL_AUDIENCE(HeightChange)
    DENG2_PIMPL_AUDIENCE(HorizonOffsetChange)
};

DENG2_AUDIENCE_METHOD(Sky, Deletion)
DENG2_AUDIENCE_METHOD(Sky, HeightChange)
DENG2_AUDIENCE_METHOD(Sky, HorizonOffsetChange)

Sky::Sky(defn::Sky const *definition)
    : MapElement(DMU_SKY)
    , d(new Impl(this))
{
    configure(definition);
}

void Sky::configure(defn::Sky const *def)
{
    LOG_AS("Sky");

    // Remember the definition for this configuration (if any).
    d->def = def? def->accessedRecordPtr() : 0;

    setHeight(def? def->getf("height") : DEFAULT_SKY_HEIGHT);
    setHorizonOffset(def? def->getf("horizonOffset") : DEFAULT_SKY_HORIZON_OFFSET);

    for(dint i = 0; i < d->layers.count(); ++i)
    {
        Record const *lyrDef = def? &def->layer(i) : 0;
        Layer &lyr = *d->layers[i];

        lyr.setMasked(lyrDef? (lyrDef->geti("flags") & SLF_MASK) : false);
        lyr.setOffset(lyrDef? lyrDef->getf("offset") : DEFAULT_SKY_SPHERE_XOFFSET);
        lyr.setFadeoutLimit(lyrDef? lyrDef->getf("colorLimit") : DEFAULT_SKY_SPHERE_FADEOUT_LIMIT);

        de::Uri const matUri = lyrDef? de::makeUri(lyrDef->gets("material")) : de::makeUri(DEFAULT_SKY_SPHERE_MATERIAL);
        world::Material *mat = 0;
        try
        {
            mat = world::Materials::get().materialPtr(matUri);
        }
        catch(world::MaterialManifest::MissingMaterialError const &er)
        {
            // Log if a material is specified but otherwise ignore this error.
            if(lyrDef)
            {
                LOG_RES_WARNING(er.asText() + ". Unknown material \"%s\" in definition layer %i, using default")
                        << matUri << i;
            }
        }
        lyr.setMaterial(mat);

        lyr.setActive(lyrDef? (lyrDef->geti("flags") & SLF_ENABLE) : i == 0);
    }

#ifdef __CLIENT__
    if(def)
    {
        Vector3f color(def->get("color"));
        if(color != Vector3f(0, 0, 0))
        {
            d->ambientLight.setColor(color);
        }
    }
    else
    {
        d->ambientLight.reset();
    }
#endif
}

Record const *Sky::def() const
{
    return d->def;
}

dint Sky::layerCount() const
{
    return d->layers.count();
}

bool Sky::hasLayer(dint layerIndex) const
{
    return !d->layers.isEmpty() && layerIndex < d->layers.count();
}

Sky::Layer *Sky::layerPtr(dint layerIndex) const
{
    if (hasLayer(layerIndex)) return d->layers.at(layerIndex);
    return nullptr;
}

Sky::Layer &Sky::layer(dint layerIndex)
{
    if (Layer *layer = layerPtr(layerIndex)) return *layer;
    /// @throw MissingLayerError Unknown layerIndex specified,
    throw MissingLayerError("Sky::layer", "Unknown layer #" + QString::number(layerIndex));
}

Sky::Layer const &Sky::layer(dint layerIndex) const
{
    if (Layer *layer = layerPtr(layerIndex)) return *layer;
    /// @throw MissingLayerError Unknown layerIndex specified,
    throw MissingLayerError("Sky::layer", "Unknown layer #" + QString::number(layerIndex));
}

LoopResult Sky::forAllLayers(std::function<LoopResult (Layer &)> func)
{
    for (Layer *layer : d->layers)
    {
        if (auto result = func(*layer)) return result;
    }
    return LoopContinue;
}

LoopResult Sky::forAllLayers(std::function<LoopResult (Layer const &)> func) const
{
    for (Layer const *layer : d->layers)
    {
        if (auto result = func(*layer)) return result;
    }
    return LoopContinue;
}

dfloat Sky::height() const
{
    return d->height;
}

void Sky::setHeight(dfloat newHeight)
{
    newHeight = de::clamp(0.f, newHeight, 1.f);
    if(!de::fequal(d->height, newHeight))
    {
        d->height = newHeight;
        DENG2_FOR_AUDIENCE2(HeightChange, i) i->skyHeightChanged(*this);
    }
}

dfloat Sky::horizonOffset() const
{
    return d->horizonOffset;
}

void Sky::setHorizonOffset(dfloat newOffset)
{
    if(!de::fequal(d->horizonOffset, newOffset))
    {
        d->horizonOffset = newOffset;
        DENG2_FOR_AUDIENCE2(HorizonOffsetChange, i) i->skyHorizonOffsetChanged(*this);
    }
}

dint Sky::property(DmuArgs &args) const
{
    LOG_AS("Sky");

    switch(args.prop)
    {
    case DMU_FLAGS: {
        dint flags = 0;
        if(layer(0).isActive()) flags |= SKYF_LAYER0_ENABLED;
        if(layer(1).isActive()) flags |= SKYF_LAYER1_ENABLED;

        args.setValue(DDVT_INT, &flags, 0);
        break; }

    case DMU_HEIGHT:
        args.setValue(DDVT_FLOAT, &d->height, 0);
        break;

    /*case DMU_HORIZONOFFSET:
        args.setValue(DDVT_FLOAT, &d->horizonOffset, 0);
        break;*/

    default: return MapElement::property(args);
    }

    return false; // Continue iteration.
}

dint Sky::setProperty(DmuArgs const &args)
{
    LOG_AS("Sky");

    switch(args.prop)
    {
    case DMU_FLAGS: {
        dint flags = 0;
        if(layer(0).isActive()) flags |= SKYF_LAYER0_ENABLED;
        if(layer(1).isActive()) flags |= SKYF_LAYER1_ENABLED;

        args.value(DDVT_INT, &flags, 0);

        layer(0).setActive(flags & SKYF_LAYER0_ENABLED);
        layer(1).setActive(flags & SKYF_LAYER1_ENABLED);
        break; }

    case DMU_HEIGHT: {
        dfloat newHeight = d->height;
        args.value(DDVT_FLOAT, &newHeight, 0);

        setHeight(newHeight);
        break; }

    /*case DMU_HORIZONOFFSET: {
        float newOffset = d->horizonOffset;
        args.value(DDVT_FLOAT, &d->horizonOffset, 0);

        setHorizonOffset(newOffset);
        break; }*/

    default: return MapElement::setProperty(args);
    }

    return false; // Continue iteration.
}

#ifdef __CLIENT__

Vector3f const &Sky::ambientColor() const
{
    if(d->ambientLight.custom || rendSkyLightAuto)
    {
        d->updateAmbientLightIfNeeded();
        return d->ambientLight.color;
    }
    return AmbientLightColorDefault;
}

void Sky::setAmbientColor(Vector3f const &newColor)
{
    d->ambientLight.setColor(newColor);
}

#endif // __CLIENT__

}  // namespace world
