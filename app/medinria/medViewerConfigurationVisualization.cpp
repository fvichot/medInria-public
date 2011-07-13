/* medViewerConfigurationVisualization.cpp --- 
 * 
 * Author: Julien Wintz
 * Copyright (C) 2008 - Julien Wintz, Inria.
 * Created: Thu May 13 14:12:19 2010 (+0200)
 * Version: $Id$
 * Last-Updated: Tue Jun 15 11:04:35 2010 (+0200)
 *           By: Julien Wintz
 *     Update #: 7
 */

/* Commentary: 
 * 
 */

/* Change log:
 * 
 */

#include "medViewerConfigurationVisualization.h"

#include <medGui/medViewerToolBoxViewProperties.h>
#include <medGui/medViewContainer.h>
#include <medGui/medStackedViewContainers.h>
#include <medGui/medViewerToolBoxView.h>
#include <medGui/medViewerToolBoxTime.h>

class medViewerConfigurationVisualizationPrivate
{
public:
    medViewerToolBoxTime                *timeToolBox;
    medViewerToolBoxView                *viewToolBox;
    medViewerToolBoxViewProperties      *viewPropertiesToolBox;

};

medViewerConfigurationVisualization::medViewerConfigurationVisualization(QWidget *parent) : medViewerConfiguration(parent), d(new medViewerConfigurationVisualizationPrivate)
{
    // -- Layout toolbox --
    setLayoutToolBoxVisibility(true);
    // Calling showLayoutToolBox causes a widget to be shown before the main window 
    // when the app is starting, which is bad.
    // showLayoutToolBox();

    // d->layoutToolBox = new medViewerToolBoxLayout(parent);
    /*
    connect(d->layoutToolBox, SIGNAL(modeChanged(int)), this, SLOT(switchToContainer(int)));
    connect(d->layoutToolBox, SIGNAL(split(int, int)), this, SLOT(split(int, int)));
    connect(d->layoutToolBox, SIGNAL(presetClicked(int)), this, SLOT(switchToContainerPreset(int)));
    */
    
    // -- View toolbox --
    
    d->viewToolBox = new medViewerToolBoxView(parent);
    d->viewPropertiesToolBox = new medViewerToolBoxViewProperties(parent);
    d->timeToolBox = new medViewerToolBoxTime(parent);
    this->addToolBox( d->viewToolBox );
    this->addToolBox(d->viewPropertiesToolBox);
    this->addToolBox( d->timeToolBox );

    connect(this,SIGNAL(layoutModeChanged(const QString &)),stackedViewContainers(),SLOT(changeCurrentContainerType(const QString &)));

    connect(stackedViewContainers(),SIGNAL(currentChanged(const QString &)),this,SLOT(connectToolboxesToCurrentContainer(const QString &)));
}

void medViewerConfigurationVisualization::setupViewContainerStack()
{
    if (!stackedViewContainers()->count())
    {
        //Default container:
        addMultiContainer("Visualization");
        this->connectToolboxesToCurrentContainer("Visualization");
    }

    this->stackedViewContainers()->unlockTabs();
}

void medViewerConfigurationVisualization::connectToolboxesToCurrentContainer(const QString &name)
{
    connect(stackedViewContainers()->container(name),SIGNAL(viewAdded(dtkAbstractView*)),
            d->timeToolBox,SLOT(onViewAdded(dtkAbstractView*)));
    connect(stackedViewContainers()->container(name),SIGNAL(viewRemoved(dtkAbstractView*)),
            d->timeToolBox,SLOT(onViewRemoved(dtkAbstractView*)));
}

medViewerConfigurationVisualization::~medViewerConfigurationVisualization(void)
{
    delete d;
    d = NULL;
}


QString medViewerConfigurationVisualization::description(void) const
{
    return "Visualization";
}

medViewerConfiguration *createMedViewerConfigurationVisualization(void)
{
    return new medViewerConfigurationVisualization;
}
