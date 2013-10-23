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
#include <msegPluginExport.h>
#include <vtkContourWidget.h>
#include <vtkImageView2D.h>

class bezierPolygonRoiPrivate;
class BezierRoiObserver;
/**
 * 
 */
class MEDVIEWSEGMENTATIONPLUGIN_EXPORT bezierPolygonRoi : public medAbstractRoi
{
    Q_OBJECT

public:
    bezierPolygonRoi(vtkImageView2D * view, medAbstractRoi * parent = NULL );
    virtual ~bezierPolygonRoi();

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


public slots:
    
    void showOrHide(int orientation, int idSlice);

signals:
    
    
private:
    bezierPolygonRoiPrivate * d;
    friend class BezierRoiObserver;
};


