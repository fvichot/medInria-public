/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medMeshingWorkspace.h"

#include <medViewContainer.h>
#include <medTabbedViewContainers.h>
#include <medVisualizationLayoutToolBox.h>
#include <medSettingsManager.h>
#include <medToolBoxFactory.h>

class medMeshingWorkspacePrivate
{
public:
    medVisualizationLayoutToolBox * layoutToolBox;
};

medMeshingWorkspace::medMeshingWorkspace(QWidget *parent) : medWorkspace(parent), d(new medMeshingWorkspacePrivate)
{
    // -- Layout toolbox --
    d->layoutToolBox = new medVisualizationLayoutToolBox(parent);

    connect (d->layoutToolBox, SIGNAL(modeChanged(const QString&)),
             this,             SIGNAL(layoutModeChanged(const QString&)));
    connect (d->layoutToolBox, SIGNAL(presetClicked(int)),
             this,             SIGNAL(layoutPresetClicked(int)));
    connect (d->layoutToolBox, SIGNAL(split(int,int)),
             this,             SIGNAL(layoutSplit(int,int)));

    connect(this,SIGNAL(setLayoutTab(const QString &)), d->layoutToolBox, SLOT(setTab(const QString &)));

    this->addToolBox( d->layoutToolBox );

    // -- View toolboxes --
    /*medFilteringAbstractToolBox *binaryOpToolBox = qobject_cast<medFilteringAbstractToolBox*>(medToolBoxFactory::instance()->createToolBox("medBinaryOperationToolBox", parent));
    medFilteringAbstractToolBox *maskApplicatioToolBox = qobject_cast<medFilteringAbstractToolBox*>(medToolBoxFactory::instance()->createToolBox("medMaskApplicationToolBox", parent));*/
    QList<QString> toolboxNames = medToolBoxFactory::instance()->toolBoxesFromCategory("mesh");
    toolboxNames.append("medViewPropertiesToolBox");

    if(toolboxNames.contains("medViewPropertiesToolBox"))
    {
        // we want the medViewPropertiesToolBox to be the first "view" toolbox
        toolboxNames.move(toolboxNames.indexOf("medViewPropertiesToolBox"),0);
    }
    foreach(QString toolbox, toolboxNames)
    {
       addToolBox( medToolBoxFactory::instance()->createToolBox(toolbox, parent) );
    }	

    //this->addToolBox( maskApplicatioToolBox );
    //this->addToolBox( binaryOpToolBox );
    connect ( this, SIGNAL(layoutModeChanged(const QString &)),
              stackedViewContainers(), SLOT(changeCurrentContainerType(const QString &)));
}

void medMeshingWorkspace::setupViewContainerStack()
{
    if (!stackedViewContainers()->count())
    {
        const QString description = this->description();
        QString createdTab = addDefaultTypeContainer(description);
    }
    this->stackedViewContainers()->unlockTabs();
}

medMeshingWorkspace::~medMeshingWorkspace(void)
{
    delete d;
    d = NULL;
}

QString medMeshingWorkspace::identifier() const {
    return "Meshing";
}

QString medMeshingWorkspace::description() const {
    return tr("Meshing");
}

bool medMeshingWorkspace::isUsable(){
    return true; // for the time being, no test is defined.
}
