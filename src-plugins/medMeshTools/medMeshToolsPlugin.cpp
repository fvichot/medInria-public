/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medMeshTools.h"
#include "medMeshToolsPlugin.h"
#include "medMeshToolsToolBox.h"

#include <dtkLog/dtkLog.h>

// /////////////////////////////////////////////////////////////////
// medMeshToolsPluginPrivate
// /////////////////////////////////////////////////////////////////

class medMeshToolsPluginPrivate
{
public:
    // Class variables go here.
    static const char *s_Name;
};

const char * medMeshToolsPluginPrivate::s_Name = "medMeshTools";

// /////////////////////////////////////////////////////////////////
// medMeshToolsPlugin
// /////////////////////////////////////////////////////////////////

medMeshToolsPlugin::medMeshToolsPlugin(QObject *parent) : dtkPlugin(parent), d(new medMeshToolsPluginPrivate)
{
    
}

medMeshToolsPlugin::~medMeshToolsPlugin()
{
    delete d;
    
    d = NULL;
}

bool medMeshToolsPlugin::initialize()
{
    if(!medMeshTools::registered())
        dtkWarn() << "Unable to register medMeshTools type";
    
    if ( !medMeshToolsToolBox::registered() )
        dtkWarn() << "Unable to register medMeshTools toolbox";
    
    return true;
}

bool medMeshToolsPlugin::uninitialize()
{
    return true;
}

QString medMeshToolsPlugin::name() const
{
    return "medMeshToolsPlugin";
}

QString medMeshToolsPlugin::description() const
{
    return tr("");
}

QString medMeshToolsPlugin::version() const
{
    return MEDMESHTOOLSPLUGIN_VERSION;
}

QString medMeshToolsPlugin::contact() const
{
    return "";
}

QStringList medMeshToolsPlugin::authors() const
{
    QStringList list;
    return list;
}

QStringList medMeshToolsPlugin::contributors() const
{
    QStringList list;
    return list;
}

QString medMeshToolsPlugin::identifier() const
{
    return medMeshToolsPluginPrivate::s_Name;
}


QStringList medMeshToolsPlugin::tags() const
{
    return QStringList();
}

QStringList medMeshToolsPlugin::types() const
{
    return QStringList() << "medMeshTools";
}
QStringList medMeshToolsPlugin::dependencies() const
{
    return QStringList();
}
Q_EXPORT_PLUGIN2(medMeshToolsPlugin, medMeshToolsPlugin)
