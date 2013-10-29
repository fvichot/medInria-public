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
#include <vtkConstantBalloonWidget.h>

#include <itkImage.h>
#include <medAbstractRoi.h>
#include <bezierPolygonRoi.h>
#include <medRoiManagementToolBox.h>
#include <medHistogramToolBox.h>

class medAbstractData;
class medAbstractView;
class medAnnotationData;
class contourWidgetObserver;

class dtkAbstractProcessFactory;


//! Segmentation toolbox to allow manual painting of pixels
class MEDVIEWSEGMENTATIONPLUGIN_EXPORT bezierCurveToolBox : public medSegmentationAbstractToolBox
{
    Q_OBJECT;
public:

    typedef itk::Image<unsigned char, 3> MaskType;
    //typedef itk::Image<unsigned char,2> MaskSliceType;*/
    typedef QPair<unsigned int,unsigned int> PlaneIndexSlicePair;
    typedef QList<QPair<vtkSmartPointer<vtkContourWidget> , PlaneIndexSlicePair> > listOfPair_CurveSlice;
    typedef QList<medAbstractRoi*> * ListRois;
    typedef QHash<medAbstractView*,QList<int> *> MapPlaneIndex;
    

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

public slots:

    //void onViewClosed();

    //void activateBezierCurve(bool);
    void onAddNewCurve();

    //void generateBinaryImage(vtkSmartPointer<vtkPolyData> pd);
    void generateBinaryImage();
    
    //void copyContours(); // For the time these function copy and paste all the contours present on a slice. No selection of a contour is possible.
    //void pasteContours(int slice1,int slice2);
    //void pasteContours();

    //void propagateCurve();
    
    void interpolateCurve();
    
    RoiStatistics ComputeHistogram(QPair<vtkPolygon*,PlaneIndexSlicePair> polygon);

    void computeStatistics();

protected:
    void binaryImageFromPolygon(QList<QPair<vtkPolygon*,PlaneIndexSlicePair> > polys);
    void reorderPolygon(vtkPolyData * poly);
    QList<vtkPolyData* > generateIntermediateCurves(vtkSmartPointer<vtkPolyData> curve1,vtkSmartPointer<vtkPolyData> curve2,int nb);
    QList<QPair<vtkPolygon*,PlaneIndexSlicePair> > createImagePolygons(QList<QPair<vtkPolyData*,PlaneIndexSlicePair> > &listPoly);
    void setData( dtkAbstractData *data );

    QList<medAbstractRoi*> * getListOfView(medAbstractView * view);
    void resampleCurve(vtkPolyData * poly,int nbPoints);
    void initializeMaskData( medAbstractData * imageData, medAbstractData * maskData ); // copy of a function in painttoolbox
    void setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData);
    int computePlaneIndex();
    int PointInPolygon (double x[3], int numPts, double *pts, 
                                double bounds[6], double *n);
private:
   
    /*dtkSmartPointer<medImageMaskAnnotationData> m_maskAnnotationData;
*/
   /* dtkSmartPointer<medAbstractData> m_maskData;
    dtkSmartPointer<medAbstractData> m_imageData;*/
    
    //MaskType::Pointer m_itkMask;
    dtkSmartPointer<medAbstractView> currentView;
    
    QPushButton * addNewCurve;
    QPushButton * generateBinaryImage_button;
    QPushButton * propagate;
    QPushButton * interpolate;

    QLabel * propagateLabel;
    QSpinBox * bound1,* bound2;

    /*QHash<medAbstractView*,ListRois> viewsRoisMap;*/
    MapPlaneIndex viewsPlaneIndex;

    bezierPolygonRoi * currentBezierRoi;
    vtkSmartPointer<vtkConstantBalloonWidget> currentBalloon;

    QList<vtkSmartPointer<vtkPolyData> > * ListOfContours; // buffer for copy/paste

    int currentOrientation;
    unsigned int currentSlice;

    contourWidgetObserver * observer;

    QShortcut *copy_shortcut, *paste_shortcut;
    
    medRoiManagementToolBox *roiToolBox;
    medHistogramToolBox * histogramToolBox;

    friend class contourWidgetObserver;
};
