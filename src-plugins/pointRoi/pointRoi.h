/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medAbstractRoi.h>
#include <vtkImageView2D.h>
#include <pointRoiPluginExport.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>

class pointRoiPrivate;
class vtkSeedCallback;

class POINTROIPLUGIN_EXPORT pointRoi : public medAbstractRoi
{
    Q_OBJECT

public:
    pointRoi(vtkImageView2D * view,vtkSeedWidget * widget, medAbstractRoi * parent = NULL );
    virtual ~pointRoi();

//    /*unsigned int getIdSlice();
//    void setIdSlice(unsigned int idSlice);
//    unsigned char getOrientation();
//    void setOrientation(unsigned char orientation);*/
    vtkImageView2D * getView();
    vtkSeedWidget * getSeedWidget();
//
//    void lockObserver(bool val);
//    
    virtual void Off();
    virtual void On();

    virtual QString info();
    virtual QString type();
    virtual void select();
    virtual void unselect();
    virtual void computeRoiStatistics();

public slots:
    void showOrHide(int orientation, int idSlice);
//
//signals:
    
    
private:
    pointRoiPrivate * d;
    friend class pointRoiPrivate;
};

