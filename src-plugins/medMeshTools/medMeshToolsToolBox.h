/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medFilteringAbstractToolBox.h>
#include "medMeshToolsPluginExport.h"

class medMeshToolsToolBoxPrivate;

class MEDMESHTOOLSPLUGIN_EXPORT medMeshToolsToolBox : public medFilteringAbstractToolBox
{
    Q_OBJECT
    
public:
    medMeshToolsToolBox(QWidget *parent = 0);
    ~medMeshToolsToolBox();
    
    dtkAbstractData *processOutput();
    
    static bool registered();
    dtkPlugin * plugin();
    
signals:
    void success();
    void failure();
    
    public slots:
    void run();
    
private:
    medMeshToolsToolBoxPrivate *d;
};


