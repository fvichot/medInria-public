/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once


#include <QtCore>

#include <medAbstractWorkspace.h>
#include <medDiffusionSelectorToolBox.h>

class medAbstractData;
class medDiffusionWorkspacePrivate;

class medDiffusionWorkspace : public medAbstractWorkspace
{
    Q_OBJECT
    MED_WORKSPACE_INTERFACE("Diffusion",
                            "Diffusion Tensor Images.")

public:
     medDiffusionWorkspace(QWidget *parent = 0);
    virtual ~medDiffusionWorkspace();

    static bool isUsable();

    void setupTabbedViewContainer() { return; } // TODO UNFUCK
    
protected slots:
    void setupProcess(QString);
    void startProcess();
    void enableSelectorToolBox();


private:
    medDiffusionWorkspacePrivate *d;
};
