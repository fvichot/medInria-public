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
#include <msegPluginExport.h>
#include <vtkSeedWidget.h>

class seedPointRoiPrivate;
class vtkSeedCallback;


class MEDVIEWSEGMENTATIONPLUGIN_EXPORT seedPointRoi : public medAbstractRoi
{
    Q_OBJECT

public:
    seedPointRoi(vtkImageView2D * view,vtkSeedWidget * widget, medAbstractRoi * parent = NULL );
    virtual ~seedPointRoi();

//    vtkContourWidget * getContour();
//    /*unsigned int getIdSlice();
//    void setIdSlice(unsigned int idSlice);
//    unsigned char getOrientation();
//    void setOrientation(unsigned char orientation);*/
    vtkImageView2D * getView();
    vtkSmartPointer<vtkSeedWidget> getSeedWidget();
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
    seedPointRoiPrivate * d;
    friend class seedPointRoiPrivate;
};

