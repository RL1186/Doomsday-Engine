/** @file packagescolumnwidget.cpp
 *
 * @authors Copyright (c) 2016-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
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

#include "ui/home/packagescolumnwidget.h"
#include "ui/widgets/packageswidget.h"
#include "ui/dialogs/packageinfodialog.h"
#include "ui/dialogs/datafilesettingsdialog.h"
#include "ui/widgets/homeitemwidget.h"
#include "ui/widgets/homemenuwidget.h"

#include <de/CallbackAction>
#include <de/Config>
#include <de/DirectoryListDialog>
#include <de/FileSystem>
#include <de/Loop>
#include <de/Package>
#include <de/PopupMenuWidget>
#include <de/ui/ActionItem>
#include <de/ui/SubwidgetItem>

#include <doomsday/DoomsdayApp>
#include <doomsday/Games>
#include <doomsday/resource/databundle.h>

using namespace de;

#if 0
static PopupWidget *makePackageFoldersDialog()
{
    Variable &pkgFolders = Config::get("resource.packageFolder");

    auto *dlg = new DirectoryListDialog;
    //dlg->title().setFont("heading");
    //dlg->title().setText(QObject::tr("Add-on and Package Folders"));
    //dlg->message().setText(QObject::tr("The following folders are searched for resource packs and other add-ons:"));
    Id group = dlg->addGroup(QObject::tr("Add-on and Package Folders"),
                             QObject::tr("The following folders are searched for resource packs and other add-ons:"));
    dlg->setValue(group, pkgFolders.value());
    dlg->setAcceptanceAction(new CallbackAction([dlg, &pkgFolders, group] ()
    {
        pkgFolders.set(dlg->value(group));

        // Reload packages and recheck for game availability.
        DoomsdayApp::app().initPackageFolders();
    }));
    return dlg;
}
#endif

DENG_GUI_PIMPL(PackagesColumnWidget)
{
    PackagesWidget *packages;
    LabelWidget *countLabel;
    ButtonWidget *folderOptionsButton;
    ui::ListData actions;
    LoopCallback mainCall;

    Impl(Public *i) : Base(i)
    {
        actions << new ui::SubwidgetItem(tr("..."), ui::Left, [this] () -> PopupWidget *
        {
            return new PackageInfoDialog(packages->actionPackage());
        });

        countLabel = new LabelWidget;

        ScrollAreaWidget &area = self().scrollArea();
        area.add(packages = new PackagesWidget(PackagesWidget::PopulationEnabled, "home-packages"));
        packages->setActionItems(actions);
        packages->setRightClickToOpenContextMenu(true);
        packages->rule()
                .setInput(Rule::Width, area.contentRule().width())
                .setInput(Rule::Top,   self().header().rule().bottom() + rule("gap"))
                .setInput(Rule::Left,  area.contentRule().left());

        QObject::connect(packages, &PackagesWidget::itemCountChanged,
                         [this] (duint shown, duint total)
        {
            if (shown == total)
            {
                countLabel->setText(tr("%1 available").arg(total));
            }
            else
            {
                countLabel->setText(tr("%1 shown out of %2 available").arg(shown).arg(total));
            }
        });

        area.add(folderOptionsButton = new ButtonWidget);
        folderOptionsButton->setStyleImage("gear", "default");
        folderOptionsButton->setText(tr("Data Files"));
        folderOptionsButton->setTextAlignment(ui::AlignRight);
        folderOptionsButton->setSizePolicy(ui::Fixed, ui::Expand);
        folderOptionsButton->rule()
                .setInput(Rule::Width, area.contentRule().width())
                .setInput(Rule::Left,  area.contentRule().left())
                .setInput(Rule::Top,   packages->rule().bottom());
        folderOptionsButton->setAction(new CallbackAction([this] ()
        {
            auto *dlg = new DataFileSettingsDialog;
            dlg->setDeleteAfterDismissed(true);
            dlg->setAnchorAndOpeningDirection(folderOptionsButton->rule(), ui::Left);
            self().root().addOnTop(dlg);
            dlg->open();
        }));

        // Column actions menu.
        self().header().menuButton().setPopup([this] (PopupButtonWidget const &) -> PopupWidget * {
            auto *menu = new PopupMenuWidget;
            menu->items()
                    << new ui::ActionItem(/*style().images().image("refresh"), */tr("Refresh"),
                                          new CallbackAction([this] () { packages->refreshPackages(); }));
                    /*<< new ui::SubwidgetItem(tr("Folders"), ui::Left, [menu] () -> PopupWidget *
                    {
                        auto *pop = makePackageFoldersDialog();
                        QObject::connect(pop, SIGNAL(closed()), menu, SLOT(close()));
                        return pop;
                    });*/
                return menu;
        }, ui::Down);
    }
};

PackagesColumnWidget::PackagesColumnWidget()
    : ColumnWidget("packages-column")
    , d(new Impl(this))
{
    header().title().setText(_E(s) "\n" _E(.) + tr("Packages"));
    header().info().setText(tr("Browse available packages and install new ones."));
    header().infoPanel().close(0);

    // Total number of packages listed.
    d->countLabel->setFont("small");
    d->countLabel->setSizePolicy(ui::Expand, ui::Fixed);
    d->countLabel->rule()
            .setInput(Rule::Left,   header().menuButton().rule().right())
            .setInput(Rule::Height, header().menuButton().rule().height())
            .setInput(Rule::Top,    header().menuButton().rule().top());
    header().add(d->countLabel);

    scrollArea().setContentSize(maximumContentWidth(),
                                header().rule().height() +
                                rule("gap") +
                                d->packages->rule().height() +
                                d->folderOptionsButton->rule().height()*2);

    // Additional layout for the packages list.
    d->packages->setFilterEditorMinimumY(scrollArea().margins().top());
    d->packages->progress().rule().setRect(scrollArea().rule());
}

String PackagesColumnWidget::tabHeading() const
{
    return tr("Packages");
}

void PackagesColumnWidget::setHighlighted(bool highlighted)
{
    ColumnWidget::setHighlighted(highlighted);
    if (highlighted)
    {
        root().setFocus(&d->packages->searchTermsEditor());
    }
}
