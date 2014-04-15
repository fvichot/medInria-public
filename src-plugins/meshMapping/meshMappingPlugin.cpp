/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "meshMapping.h"
#include "meshMappingPlugin.h"
#include "meshMappingToolBox.h"

#include <dtkLog/dtkLog.h>

// /////////////////////////////////////////////////////////////////
// meshMappingPluginPrivate
// /////////////////////////////////////////////////////////////////

class meshMappingPluginPrivate
{
public:
    // Class variables go here.
    static const char *s_Name;
};

const char * meshMappingPluginPrivate::s_Name = "meshMapping";

// /////////////////////////////////////////////////////////////////
// meshMappingPlugin
// /////////////////////////////////////////////////////////////////

meshMappingPlugin::meshMappingPlugin(QObject *parent) : dtkPlugin(parent), d(new meshMappingPluginPrivate)
{
    
}

meshMappingPlugin::~meshMappingPlugin()
{
    delete d;
    
    d = NULL;
}

bool meshMappingPlugin::initialize()
{
    if(!meshMapping::registered())
        dtkWarn() << "Unable to register meshMapping type";
    
    if ( !meshMappingToolBox::registered() )
        dtkWarn() << "Unable to register meshMapping toolbox";
    
    return true;
}

bool meshMappingPlugin::uninitialize()
{
    return true;
}

QString meshMappingPlugin::name() const
{
    return "meshMappingPlugin";
}

QString meshMappingPlugin::description() const
{
    return tr("Plugin allowing to sample data values at specified point locations.");
}

QString meshMappingPlugin::version() const
{
    return MESHMAPPINGPLUGIN_VERSION;
}

QString meshMappingPlugin::contact() const
{
    return "";
}

QStringList meshMappingPlugin::authors() const
{
    return QStringList() << "Loïc Cadour <Loic.Cadour@inria.fr>";
}

QStringList meshMappingPlugin::contributors() const
{
    QStringList list;
    return list;
}

QString meshMappingPlugin::identifier() const
{
    return meshMappingPluginPrivate::s_Name;
}


QStringList meshMappingPlugin::tags() const
{
    return QStringList();
}

QStringList meshMappingPlugin::types() const
{
    return QStringList() << "meshMapping";
}
QStringList meshMappingPlugin::dependencies() const
{
    return QStringList();
}
Q_EXPORT_PLUGIN2(meshMappingPlugin, meshMappingPlugin)
