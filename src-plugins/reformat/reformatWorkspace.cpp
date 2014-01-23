/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "reformatWorkspace.h"

#include <medWorkspaceFactory.h>
#include <medToolBoxFactory.h>
#include <medTabbedViewContainers.h>
#include <reformatToolBox.h>
#include <resampleToolBox.h>


static QString s_identifier()
{
   return "Reformat/Resample";
}

static QString s_description()
{
   return "Reformat/Resample";
}

// /////////////////////////////////////////////////////////////////
// reformatWorkspacePrivate
// /////////////////////////////////////////////////////////////////

class reformatWorkspacePrivate
{
public:
};

// /////////////////////////////////////////////////////////////////
// reformat
// /////////////////////////////////////////////////////////////////

reformatWorkspace::reformatWorkspace(QWidget *parent) : medWorkspace(parent), d(new reformatWorkspacePrivate)
{
    // -- View toolboxes --
    QList<QString> toolboxNames = medToolBoxFactory::instance()->toolBoxesFromCategory("view");
    if(toolboxNames.contains("medViewPropertiesToolBox"))
    {
        // we want the medViewPropertiesToolBox to be the first "view" toolbox
        toolboxNames.move(toolboxNames.indexOf("medViewPropertiesToolBox"),0);
    }

    //toolboxNames.append("reformatToolBox");
    toolboxNames.append("resampleToolBox");
    foreach(QString toolbox, toolboxNames)
    {
       addToolBox( medToolBoxFactory::instance()->createToolBox(toolbox, parent) );
    }

    reformatToolBox * reformatTb = new reformatToolBox();
    reformatTb->setWorkspace(this);
    addToolBox(reformatTb);
}

reformatWorkspace::~reformatWorkspace()
{
    
}

QString reformatWorkspace::identifier() const
{
    return s_identifier();
}

QString reformatWorkspace::description() const
{
    return s_description();
}

void reformatWorkspace::setupViewContainerStack()
{
    if (!stackedViewContainers()->count())
    {
        const QString description = this->description();
        QString createdTab = addDefaultTypeContainer(description);
    }
    this->stackedViewContainers()->unlockTabs();
}

bool reformatWorkspace::isUsable()
{
    // TODO: you can add some conditions here to check if your workspace is ready to be used
    // (successfully initialized, ...)
    return true;
}

bool reformatWorkspace::registered()
{
    return medWorkspaceFactory::instance()->registerWorkspace <reformatWorkspace>
            (s_identifier(), "Reformat/Resample", s_description());
}


