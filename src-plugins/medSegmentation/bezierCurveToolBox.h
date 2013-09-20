/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medSegmentationAbstractToolBox.h>

#include "msegPluginExport.h"

#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medDataIndex.h>
#include <medViewEventFilter.h>
#include <medImageMaskAnnotationData.h>

#include <QVector3D>
#include <QTextEdit>

#include <vtkSmartPointer.h>
#include <vtkContourWidget.h>
#include <vtkRenderWindowInteractor.h> 
#include <vtkOrientedGlyphFocalPlaneContourRepresentation.h>
#include <vector>

class medAbstractData;
class medAbstractView;
class medAnnotationData;
class bezierObserver;

class dtkAbstractProcessFactory;


//! Segmentation toolbox to allow manual painting of pixels
class MEDVIEWSEGMENTATIONPLUGIN_EXPORT bezierCurveToolBox : public medSegmentationAbstractToolBox
{
    Q_OBJECT;
public:

    /*typedef itk::Image<unsigned char, 3> MaskType;
    typedef itk::Image<unsigned char,2> MaskSliceType;*/
    
    typedef QList<QPair<vtkSmartPointer<vtkContourWidget> , unsigned int> > listOfPair_CurveSlice;

    bezierCurveToolBox( QWidget *parent );
    ~bezierCurveToolBox();

     //! Override dtkAbstractObject
    QString description() const { return s_description(); }
    QString identifier() const { return s_identifier(); }

    static medSegmentationAbstractToolBox * createInstance( QWidget *parent );

    static QString s_description();

    /** Get name to use for this when registering with a factory.*/
    static QString s_identifier();

    //! Get a human readable name for this widget.
    /** \param trObj : Provide an object for the tr() function. If NULL qApp will be used. */
    static QString s_name(const QObject * trObj =  NULL);

    void setCurrentView(medAbstractView * view);
    void update(dtkAbstractView * view);

    listOfPair_CurveSlice * getCoronalListOfCurves();
    listOfPair_CurveSlice * getSagittalListOfCurves();
    listOfPair_CurveSlice * getAxialListOfCurves();
       

public slots:

    //void onViewClosed();

    //void activateBezierCurve(bool);
    void onAddNewCurve();
    void onPenMode();
    void generateBinaryImage();
protected:
    
    //void setData( dtkAbstractData *data );

     //void initializeMaskData( medAbstractData * imageData, medAbstractData * maskData );
    
private:
   
    /*dtkSmartPointer<medImageMaskAnnotationData> m_maskAnnotationData;
*/
   /* dtkSmartPointer<medAbstractData> m_maskData;
    dtkSmartPointer<medAbstractData> m_imageData;*/
    
    //MaskType::Pointer m_itkMask;
    medAbstractView * currentView;
    
    //QPushButton * bezierCurve,
    QPushButton * addNewCurve;
    QPushButton * generateBinaryImage_button;
    QCheckBox * penMode_CheckBox;
    bool newCurve;
    bool penMode;
    vtkSmartPointer<vtkRenderWindowInteractor> curveInteractor;
    listOfPair_CurveSlice  * listOfCurvesForAxial; 
    listOfPair_CurveSlice  * listOfCurvesForSagittal;
    listOfPair_CurveSlice  * listOfCurvesForCoronal;

    bezierObserver * observer;
};
