/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <polygonRoiPlugin.h>
#include <polygonRoiToolBox.h>

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
    if(!polygonRoiToolBox::registered()) qDebug() << "Unable to register polygonRoiToolBox"; 
   return true;
}

bool Plugin::uninitialize()
{
    return true;
}

QString Plugin::name() const
{
    return "polygonRoiPlugin";
}

QString Plugin::description() const
{
    return tr("polygon roi plugin\n<br/>"
              "define polygon in image as roi" // TODO : write a new description
              "<br/>");
}

QString Plugin::version() const
{
    return POLYGONROIPLUGIN_VERSION;
}

QString Plugin::contact() const
{
    return "hakim.fadil@inria.fr";
}

QStringList Plugin::authors() const
{
    QStringList list;
    list << "Hakim Fadil";
    list << "Florian Vichot";
    return list;
}

QStringList Plugin::contributors() const
{
    QStringList list;
    list <<  QString::fromUtf8("Hakim Fadil")
             << "Florian Vichot";
    return list;
}

QString Plugin::identifier() const
{
    return "polygonRoiPlugin";
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

