/*=========================================================================

medInria

Copyright (c) INRIA 2013. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.

=========================================================================*/

#pragma once

#include <medRoiToolBox.h>
#include <dtkCore/dtkAbstractView.h>
#include <medAbstractView.h>
#include <PointRoiPluginExport.h>
#include <medAbstractRoi.h>

class pointRoiToolBoxPrivate;

class POINTROIPLUGIN_EXPORT pointRoiToolBox : public medRoiToolBox
{
    Q_OBJECT

public:
    pointRoiToolBox(QWidget *parent = 0); 
    ~pointRoiToolBox();          

    virtual void update(dtkAbstractView * view);

    static bool registered();

    virtual QString roi_description();

    medAbstractView * getCurrentView();

    void emitRoiCreated(medAbstractView*,medAbstractRoi*,QString);

    public slots:
        void seedMode(bool checked);
    
signals:
        void roiCreated(medAbstractView*,medAbstractRoi*,QString);

private:
    pointRoiToolBoxPrivate* d;
};


