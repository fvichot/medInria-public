/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medToolBox.h>
#include <medGuiExport.h>
#include <medAbstractRoi.h>
#include <medAbstractData.h>

class dtkAbstractView;
class medRoiToolBoxPrivate;
class medWorkspace;

class MEDGUI_EXPORT medRoiToolBox : public medToolBox
{
    Q_OBJECT;

public:

    medRoiToolBox(QWidget *parent = 0);
    ~medRoiToolBox();
    
    virtual void interpolateRois(QList<medAbstractRoi*>*);
    virtual medAbstractData * convertToBinaryImage(QList<medAbstractRoi*>*); // will be the conversion to brush Roi later on ...
    virtual void applyRoiToImage();
    virtual QString roi_description();

signals:
    
public slots:
    // Override base class
    

private:
    medRoiToolBoxPrivate *d;
};
