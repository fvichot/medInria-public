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
#include <polygonRoiPluginExport.h>
#include <vtkContourWidget.h>
#include <vtkImageView2D.h>

class polygonRoiPrivate;
class BezierRoiObserver;
/**
 * 
 */
class POLYGONROIPLUGIN_EXPORT polygonRoi : public medAbstractRoi
{
    Q_OBJECT

public:
    polygonRoi(vtkImageView2D * view, medAbstractRoi * parent = NULL );
    virtual ~polygonRoi();

    vtkContourWidget * getContour();
    /*unsigned int getIdSlice();
    void setIdSlice(unsigned int idSlice);
    unsigned char getOrientation();
    void setOrientation(unsigned char orientation);*/
    vtkImageView2D * getView();

    void lockObserver(bool val);
    
    virtual void Off();
    virtual void On();

    virtual QString info();
    virtual QString type();
    virtual void select();
    virtual void unselect();
    virtual void computeRoiStatistics();

public slots:
    
    void showOrHide(int orientation, int idSlice);

signals:
    
    
private:
    polygonRoiPrivate * d;
    friend class polygonRoiObserver;
};


