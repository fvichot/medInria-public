/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "reformatWorkspace.h"
#include "reformatPlugin.h"

#include <reformatToolBox.h>
#include <resampleToolBox.h>
#include <dtkLog/dtkLog.h>

// /////////////////////////////////////////////////////////////////
// reformatPluginPrivate
// /////////////////////////////////////////////////////////////////

class reformatPluginPrivate
{
public:
    // Class variables go here.
    static const char *s_Name;
};

const char * reformatPluginPrivate::s_Name = "reformat";

// /////////////////////////////////////////////////////////////////
// reformatPlugin
// /////////////////////////////////////////////////////////////////

reformatPlugin::reformatPlugin(QObject *parent) : dtkPlugin(parent), d(new reformatPluginPrivate)
{
    
}

reformatPlugin::~reformatPlugin()
{
    delete d;
    
    d = NULL;
}

bool reformatPlugin::initialize()
{
    if(!reformatWorkspace::registered())
        dtkWarn() << "Unable to register reformat type";
    if (!reformatToolBox::registered())
        dtkWarn() << "Unable to register reformat type";
    if (!resampleToolBox::registered())
        dtkWarn() << "Unable to register reformat type";

    return true;
}

bool reformatPlugin::uninitialize()
{
    return true;
}

QString reformatPlugin::name() const
{
    return "reformatPlugin";
}

QString reformatPlugin::description() const
{
    return tr("");
}

QString reformatPlugin::version() const
{
    return REFORMATPLUGIN_VERSION;
}

QString reformatPlugin::contact() const
{
    return "";
}

QStringList reformatPlugin::authors() const
{
    QStringList list;
    return list;
}

QStringList reformatPlugin::contributors() const
{
    QStringList list;
    return list;
}

QString reformatPlugin::identifier() const
{
    return reformatPluginPrivate::s_Name;
}


QStringList reformatPlugin::tags() const
{
    return QStringList();
}

QStringList reformatPlugin::types() const
{
    return QStringList() << "reformat";
}
QStringList reformatPlugin::dependencies() const
{
    return QStringList();
}
Q_EXPORT_PLUGIN2(reformatPlugin, reformatPlugin)
