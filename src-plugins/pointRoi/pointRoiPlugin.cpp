/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <pointRoiPlugin.h>
#include <pointRoiToolBox.h>

#include <dtkLog/dtkLog.h>

// /////////////////////////////////////////////////////////////////
// PluginPrivate
// /////////////////////////////////////////////////////////////////

class PluginPrivate
{
public:
    // Class variables go here.
    
};

// /////////////////////////////////////////////////////////////////
// Plugin
// /////////////////////////////////////////////////////////////////

Plugin::Plugin(QObject *parent) : dtkPlugin(parent), d(new PluginPrivate)
{

}

Plugin::~Plugin()
{
    delete d;
    d = NULL;
}

bool Plugin::initialize()
{
   if(!pointRoiToolBox::registered()) qDebug() << "Unable to register "; // TODO : register pointRoi, and pointRoiGui
   return true;
}

bool Plugin::uninitialize()
{
    return true;
}

QString Plugin::name() const
{
    return "pointRoiPlugin";
}

QString Plugin::description() const
{
    return tr("point roi plugin\n<br/>"
              "defined point in image as roi" // TODO : write a new description
              "<br/>");
}

QString Plugin::version() const
{
    return POINTROIPLUGIN_VERSION;
}

QString Plugin::contact() const
{
    return "loic.cadour@inria.fr";
}

QStringList Plugin::authors() const
{
    QStringList list;
    list << "Loic Cadour";
    list << "Hakim Fadil";
    return list;
}

QStringList Plugin::contributors() const
{
    QStringList list;
    list <<  QString::fromUtf8("Loic Cadour")
             << "Hakim Fadil";
    return list;
}

QString Plugin::identifier() const
{
    return "pointRoiPlugin";
}

QStringList Plugin::tags() const
{
    return QStringList();
}

QStringList Plugin::types() const
{
    return QStringList();
}

Q_EXPORT_PLUGIN2(Plugin, Plugin)

