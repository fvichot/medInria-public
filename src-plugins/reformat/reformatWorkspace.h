/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medWorkspace.h>

#include "reformatPluginExport.h"

class reformatWorkspacePrivate;
    
class REFORMATPLUGIN_EXPORT reformatWorkspace : public medWorkspace
{
    Q_OBJECT
    
public:
    reformatWorkspace(QWidget *parent = 0);
    virtual ~reformatWorkspace();
    
    virtual QString description() const;

    virtual QString identifier() const;

    virtual void setupViewContainerStack();

    static bool isUsable();

    static bool registered();

    void showViewPropertiesToolBox(bool val);

    
private:
    reformatWorkspacePrivate *d;
};
