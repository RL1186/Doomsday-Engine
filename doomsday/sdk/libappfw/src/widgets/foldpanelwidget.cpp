/** @file foldpanelwidget.cpp  Folding panel.
 *
 * @authors Copyright (c) 2013-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * @par License
 * LGPL: http://www.gnu.org/licenses/lgpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details. You should have received a copy of
 * the GNU Lesser General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#include "de/FoldPanelWidget"
#include "de/ProceduralImage"
#include "de/DialogContentStylist"
#include "de/SignalAction"

namespace de {

using namespace ui;

static TimeSpan const INDICATOR_ANIM_SPAN = 0.4;

DENG2_PIMPL_NOREF(FoldPanelWidget)
{
    /**
     * Indicator that shows whether a fold panel is open or closed.
     */
    struct FoldImage : public ProceduralImage
    {
        FoldPanelWidget &fold;
        bool needSize;
        bool animating;
        Animation angle;

        FoldImage(FoldPanelWidget &owner)
            : fold(owner)
            , needSize(true)
            , animating(false)
            , angle(0, Animation::EaseBoth)
        {}

        /// We'll report the status as changed if the image was animating or its
        /// size was updated.
        bool update() override
        {
            bool changed = animating;

            float target = (fold.isOpen()? 0 : 90);
            if (!fequal(target, angle.target()))
            {
                angle.setValue(target, INDICATOR_ANIM_SPAN);
                animating = true;
                changed = true;
            }

            if (needSize)
            {
                needSize = false;
                changed = true;

                const float h = pixelsToPoints(fold.title().font().height().value());
                setPointSize({h, h});
            }

            // Stop animating?
            if (animating && angle.done())
            {
                animating = false;
            }

            return changed;
        }

        void glMakeGeometry(GuiVertexBuilder &verts, Rectanglef const &rect) override
        {
            GuiRootWidget &root = fold.root();
            Atlas &atlas = root.atlas();
            ColorBank::Colorf const &textColor = fold.title().textColorf();

            // Frame.
            /*verts.makeFlexibleFrame(rect.toRectanglei(), 5, textColor,
                                    atlas.imageRectf(root.roundCorners()));*/

            Rectanglef uv = atlas.imageRectf(root.styleTexture("fold"));
            Matrix4f const turn = Matrix4f::rotateAround(rect.middle(), angle);
            verts.makeQuad(rect, textColor * Vector4f(1, 1, 1, .5f), uv, &turn);
        }
    };

    SafeWidgetPtr<ButtonWidget> title;
    GuiWidget *container = nullptr; ///< Held here while not part of the widget tree.
    DialogContentStylist stylist;

    ~Impl()
    {
        stylist.clear(); // References the container.

        // We have ownership of the content when the fold is closed.
        delete container;
    }
};

FoldPanelWidget::FoldPanelWidget(String const &name) : PanelWidget(name), d(new Impl)
{}

ButtonWidget *FoldPanelWidget::makeTitle(String const &text)
{
    d->title.reset(new ButtonWidget("fold-title"));

    d->title->setSizePolicy(Expand, Expand);
    d->title->setText(text);
    d->title->setTextColor("accent");
    d->title->setHoverTextColor("text", ButtonWidget::ReplaceColor);
    d->title->setFont("heading");
    d->title->setAlignment(ui::AlignLeft);
    d->title->setTextLineAlignment(ui::AlignLeft);
    d->title->set(Background()); // no frame or background
    d->title->setAction(new SignalAction(this, SLOT(toggleFold())));
    d->title->setOpacity(.8f);

    // Fold indicator.
    d->title->setOverlayImage(new Impl::FoldImage(*this), ui::AlignRight);

    return d->title;
}

ButtonWidget &FoldPanelWidget::title()
{
    DENG2_ASSERT(d->title != nullptr);
    return *d->title;
}

void FoldPanelWidget::setContent(GuiWidget *content)
{
    d->stylist.setContainer(*content);

    if (!isOpen())
    {
        // We'll just take it and do nothing else yet.
        if (d->container)
        {
            d->container->guiDeleteLater();
        }
        d->container = content;
        return;
    }

    PanelWidget::setContent(content);
}

GuiWidget &FoldPanelWidget::content() const
{
    if (d->container)
    {
        return *d->container;
    }
    return PanelWidget::content();
}

FoldPanelWidget *FoldPanelWidget::makeOptionsGroup(const String &name, const String &heading,
                                                   GuiWidget *parent)
{
    auto *fold = new FoldPanelWidget(name);
    parent->add(fold->makeTitle(heading));
    parent->add(fold);
    fold->title().setSizePolicy(ui::Fixed, ui::Expand);
    fold->title().setFont("separator.label");
    fold->title().margins().setTop("gap");
//    fold->title().set(Background(Vector4f(1, 0, 1, .5f)));
    fold->title().setImageAlignment(ui::AlignRight);
    fold->title().rule()
        .setInput(Rule::Left, fold->rule().left())
        .setInput(Rule::Bottom, fold->rule().top())
        .setInput(Rule::Width, fold->rule().width());
    return fold;
}

void FoldPanelWidget::toggleFold()
{
    if (!isOpen())
    {
        open();
    }
    else
    {
        close(0);
    }
}

void FoldPanelWidget::preparePanelForOpening()
{
    if (d->container)
    {
        // Insert the content back into the panel.
        PanelWidget::setContent(d->container);
        d->container = nullptr;
    }

    if (d->title)
    {
        d->title->setOpacity(1);
    }

    PanelWidget::preparePanelForOpening();
}

void FoldPanelWidget::panelDismissed()
{
    PanelWidget::panelDismissed();

    if (d->title)
    {
        d->title->setOpacity(.8f, .5f);
    }

    content().notifySelfAndTree(&Widget::deinitialize);

    DENG2_ASSERT(d->container == 0);
    d->container = takeContent();
}

} // namespace de
