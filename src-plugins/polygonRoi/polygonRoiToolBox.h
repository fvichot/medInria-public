/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <polygonRoiPluginExport.h>
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
#include <vtkPolygon.h>

#include <itkImage.h>
#include <medAbstractRoi.h>
#include <polygonRoi.h>
#include <medRoiCreatorToolBox.h>
#include <medRoiToolBox.h>

class medAbstractData;
class medAbstractView;
class medAnnotationData;
class contourWidgetObserver;

class dtkAbstractProcessFactory;


//! Polygon roi toolBox
class POLYGONROIPLUGIN_EXPORT PolygonRoiToolBox : public medRoiToolBox
{
    Q_OBJECT;
public:

    typedef itk::Image<unsigned char, 3> MaskType;
    //typedef itk::Image<unsigned char,2> MaskSliceType;*/
    typedef QPair<unsigned int,unsigned int> PlaneIndexSlicePair;
    typedef QList<QPair<vtkSmartPointer<vtkContourWidget> , PlaneIndexSlicePair> > listOfPair_CurveSlice;
    typedef QList<medAbstractRoi*> * ListRois;
    typedef QHash<medAbstractView*,QList<int> *> MapPlaneIndex;
    
    PolygonRoiToolBox( QWidget *parent );
    ~PolygonRoiToolBox();

    virtual QString roi_description();

    void setCurrentView(medAbstractView * view);
    void update(dtkAbstractView * view);
    static bool registered();

public slots:

    //void onViewClosed();

    void activatePolygonMode();

    //void generateBinaryImage(vtkSmartPointer<vtkPolyData> pd);
    
    //void copyContours(); // For the time these function copy and paste all the contours present on a slice. No selection of a contour is possible.
    //void pasteContours(int slice1,int slice2);
    //void pasteContours();

    //void propagateCurve();
    
    medAbstractData * convertToBinaryImage(QList<medAbstractRoi*>*);
    void interpolateRois(QList<medAbstractRoi*>*);
    
    RoiStatistics ComputeHistogram(QPair<vtkPolygon*,PlaneIndexSlicePair> polygon);

    void computeStatistics();

protected:
    void interpolateRois_inListOrientation(QList<medAbstractRoi*>*);
    medAbstractData* binaryImageFromPolygon(QList<QPair<vtkPolygon*,PlaneIndexSlicePair> > polys);
    void reorderPolygon(vtkPolyData * poly);
    QList<vtkPolyData* > generateIntermediateCurves(vtkSmartPointer<vtkPolyData> curve1,vtkSmartPointer<vtkPolyData> curve2,int nb);
    QList<QPair<vtkPolygon*,PlaneIndexSlicePair> > createImagePolygons(QList<QPair<vtkPolyData*,PlaneIndexSlicePair> > &listPoly);
    void setData( dtkAbstractData *data );

    QList<medSeriesOfRoi*> * getListOfView(medAbstractView * view);
    void resampleCurve(vtkPolyData * poly,int nbPoints);
    void initializeMaskData( medAbstractData * imageData, medAbstractData * maskData ); // copy of a function in painttoolbox
    void setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData);
    int computePlaneIndex();
    int PointInPolygon (double x[3], int numPts, double *pts, 
                                double bounds[6], double *n);

    void emitRoiCreated(medAbstractView*, medAbstractRoi*, QString);
    QList<medAbstractRoi*> *  sort(QList<medAbstractRoi*> *list);
signals:
    void roiCreated(medAbstractView*,medAbstractRoi*,QString);

private:
   
    /*dtkSmartPointer<medImageMaskAnnotationData> m_maskAnnotationData;
*/
   /* dtkSmartPointer<medAbstractData> m_maskData;
    dtkSmartPointer<medAbstractData> m_imageData;*/
    
    //MaskType::Pointer m_itkMask;
    dtkSmartPointer<medAbstractView> currentView;
    
    QPushButton * polygonButton;
    bool polygonModeON;
    QPushButton * generateBinaryImage_button;
    QPushButton * propagate;
    QPushButton * interpolate;

    QLabel * propagateLabel;
    QSpinBox * bound1,* bound2;

    /*QHash<medAbstractView*,ListRois> viewsRoisMap;*/
    MapPlaneIndex viewsPlaneIndex;

    polygonRoi * currentBezierRoi;
    vtkSmartPointer<vtkConstantBalloonWidget> currentBalloon;

    QList<vtkSmartPointer<vtkPolyData> > * ListOfContours; // buffer for copy/paste

    int currentOrientation;
    unsigned int currentSlice;

    contourWidgetObserver * observer;

    QShortcut *copy_shortcut, *paste_shortcut;
    
    /*medRoiManagementToolBox *roiToolBox;
    medHistogramToolBox * histogramToolBox;*/

    friend class contourWidgetObserver;
};
