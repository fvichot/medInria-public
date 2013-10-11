/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "bezierCurveToolBox.h"

#include <medAbstractData.h>
#include <medAbstractDataImage.h>
#include <medAbstractView.h>
#include <medAbstractViewCoordinates.h>
#include <medDataIndex.h>
#include <medImageMaskAnnotationData.h>
#include <medMetaDataKeys.h>
#include <medMessageController.h>
#include <medSegmentationSelectorToolBox.h>
#include <medSegmentationAbstractToolBox.h>
#include <medToolBox.h>
#include <medViewManager.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkLog/dtkLog.h>
#include <dtkCore/dtkSmartPointer.h>
#include <dtkCore/dtkGlobal.h>

#include <QtCore>
#include <QColorDialog>

#include <vtkSmartPointer.h>
#include <vtkContourWidget.h>
#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkImageActorPointPlacer.h>
#include <vtkBoundedPlanePointPlacer.h>
#include <vtkPlane.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkEvent.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageView2D.h>
#include <vtkImageActor.h>

//
#include <vtkThinPlateSplineTransform.h>
#include <vtkMetaImageWriter.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

#include <vtkBalloonRepresentation.h>
#include <vtkConstantBalloonWidget.h>
#include <vtkOrientedGlyphFocalPlaneContourRepresentation.h>
#include <vtkContourOverlayRepresentation.h>
#include <vtkContourRepresentation.h>
#include <vtkProperty2D.h>

#include <vtkWidgetEvent.h>
#include <vtkDecimatePolylineFilter.h>
#include <vtkParametricSpline.h>
#include <vtkCellArray.h>
#include <vtkPolyLine.h>
#include <vtkPointData.h>

#include <vtkPolygon.h>
#include <msegAlgorithmPaintToolbox.h>
#include <medDataManager.h>

#include <vtkMath.h>
#include <vtkLine.h>
#include <itkVTKPolyDataReader.h>
#include <vtkMatrix4x4.h>
#include <msegAlgorithmPaintToolbox.h>

class bezierObserver : public vtkCommand
{
public:
    typedef QPair<unsigned int,unsigned int> PlaneIndexSlicePair;

    static bezierObserver* New()
    {
        return new bezierObserver;
    }

    void Execute ( vtkObject *caller, unsigned long event, void *callData );

    void setView ( vtkImageView2D *view )
    {
        this->view = view;
        view->AddObserver(vtkImageView2D::SliceChangedEvent,this);
    }

    void setToolBox ( bezierCurveToolBox * toolBox )
    {
        this->toolBox = toolBox;
    }

    inline void lock()
    {
        this->m_lock = 1;
    }
    inline void unlock()
    {
        this->m_lock = 0;
    }

protected:
    bezierObserver();
    ~bezierObserver();

private:
    int m_lock;
    vtkImageView2D *view;
    bezierCurveToolBox * toolBox;
};

bezierObserver::bezierObserver()
{
    this->m_lock = 0;
}

bezierObserver::~bezierObserver(){}

void bezierObserver::Execute ( vtkObject *caller, unsigned long event, void *callData )
{
    if ( this->m_lock )
        return;

    if (!this->view || !toolBox)
        return;

    switch ( event )
    {
    case vtkCommand::StartInteractionEvent:
        {
            vtkContourWidget * contour = dynamic_cast<vtkContourWidget*>(caller);
            switch (view->GetViewOrientation())
            {    
                qDebug() << "view->GetSlice() : " << view->GetSlice(); 
            case 0:
                {
                    toolBox->getSagittalListOfCurves()->append(QPair<vtkSmartPointer<vtkContourWidget>,PlaneIndexSlicePair>(contour,PlaneIndexSlicePair(view->GetSlice(),toolBox->computePlaneIndex())));
                    break;
                }
            case 1:
                {
                    toolBox->getCoronalListOfCurves()->append(QPair<vtkSmartPointer<vtkContourWidget>,PlaneIndexSlicePair>(contour,PlaneIndexSlicePair(view->GetSlice(),toolBox->computePlaneIndex())));
                    break;
                }
            case 2:
                    toolBox->getAxialListOfCurves()->append(QPair<vtkSmartPointer<vtkContourWidget>,PlaneIndexSlicePair>(contour,PlaneIndexSlicePair(view->GetSlice(),toolBox->computePlaneIndex())));
            }
            break;
        }
    case vtkImageView2D::SliceChangedEvent:
        {
            toolBox->hideContour();
            toolBox->showContour();
            break;
        }
    case vtkCommand::EndInteractionEvent:
        {
            //toolBox->onAddNewCurve();
            break;
        }
    }
}

bezierCurveToolBox::bezierCurveToolBox(QWidget *parent ) :
medSegmentationAbstractToolBox( parent)
{
    QWidget *displayWidget = new QWidget(this);
    this->addWidget(displayWidget);

    this->setTitle(this->s_name(this));

    QVBoxLayout * layout = new QVBoxLayout(displayWidget);

    addNewCurve = new QPushButton(tr("Closed Polygon"),displayWidget);
    addNewCurve->setToolTip(tr("Activate closed polygon mode"));
    connect(addNewCurve,SIGNAL(clicked()),this,SLOT(onAddNewCurve()));

    penMode_CheckBox = new QCheckBox(tr("pen mode"),displayWidget);
    penMode_CheckBox->setToolTip(tr("Activate continuous draw"));
    connect(penMode_CheckBox,SIGNAL(stateChanged(int)),this,SLOT(onPenMode()));

    generateBinaryImage_button = new QPushButton(tr("Generate Binary Image"),displayWidget);
    
    connect(generateBinaryImage_button,SIGNAL(clicked()),this,SLOT(generateBinaryImage()));

    currentView = NULL;
    newCurve = false;
    penMode = false;

    QHBoxLayout * ButtonLayout = new QHBoxLayout();
    layout->addLayout( ButtonLayout );
    layout->addWidget(addNewCurve);
    layout->addWidget(generateBinaryImage_button);
    layout->addWidget(penMode_CheckBox);

    listOfCurvesForSagittal = new QList<QPair<vtkSmartPointer<vtkContourWidget>,PlaneIndexSlicePair> >();
    listOfCurvesForAxial = new QList<QPair<vtkSmartPointer<vtkContourWidget>,PlaneIndexSlicePair> >();
    listOfCurvesForCoronal = new QList<QPair<vtkSmartPointer<vtkContourWidget>,PlaneIndexSlicePair> >();

    observer = bezierObserver::New();
    observer->setToolBox(this);

    currentContour = NULL;
    currentOrientation = -1;
    currentSlice = 0;

    copy_shortcut = new QShortcut(QKeySequence(tr("Ctrl+c","Copy bezier curves")),this);
    paste_shortcut = new QShortcut(QKeySequence(tr("Ctrl+v","Paste bezier curves")),this);
    connect(copy_shortcut,SIGNAL(activated()),this,SLOT(copyContours()));
    connect(paste_shortcut,SIGNAL(activated()),this,SLOT(pasteContours()));

    ListOfContours = new QList<vtkSmartPointer<vtkPolyData> >();

    propagate = new QPushButton("Propagate",this);

    interpolate = new QPushButton("Interpolate",this);
    
    propagateLabel = new QLabel("Curve propagation : Define the interval");
    bound1 = new QSpinBox(this);
    bound2 = new QSpinBox(this);
    bound1->setMaximum(500); // TODO : depends on the currentView see how to update them with the view !! 
    bound2->setMaximum(500);

    layout->addWidget(propagateLabel);
    layout->addWidget(bound1);
    layout->addWidget(bound2);
    layout->addWidget(propagate);
    layout->addWidget(interpolate);
    connect(propagate,SIGNAL(clicked()),this,SLOT(propagateCurve()));
    connect(interpolate,SIGNAL(clicked()),this,SLOT(interpolateCurve()));
}

bezierCurveToolBox::~bezierCurveToolBox(){}

//void bezierCurveToolBox::onClearMaskClicked()
//{
//    if ( m_maskData && m_itkMask ){
//        m_itkMask->FillBuffer( medSegmentationSelectorToolBox::MaskPixelValues::Unset );
//        m_itkMask->Modified();
//        m_itkMask->GetPixelContainer()->Modified();
//        m_itkMask->SetPipelineMTime(m_itkMask->GetMTime());
//
//        m_maskAnnotationData->invokeModified();
//    }
//}

//void bezierCurveToolBox::setData( dtkAbstractData *dtkdata )
//{
//    if (!dtkdata)
//        return;
//
//    // disconnect existing
//    if ( m_imageData ) {
//        // TODO?
//    }
//
//    m_lastVup = QVector3D();
//    m_lastVpn = QVector3D();
//
//    m_imageData = dtkSmartPointer<dtkAbstractData>(dtkdata);
//    
//    // Update values of slider
//
//    GenerateMinMaxValuesFromImage < itk::Image <char,3> > ();
//    GenerateMinMaxValuesFromImage < itk::Image <unsigned char,3> > ();
//    GenerateMinMaxValuesFromImage < itk::Image <short,3> > ();
//    GenerateMinMaxValuesFromImage < itk::Image <unsigned short,3> > ();
//    GenerateMinMaxValuesFromImage < itk::Image <int,3> > ();
//    GenerateMinMaxValuesFromImage < itk::Image <unsigned int,3> > ();
//    GenerateMinMaxValuesFromImage < itk::Image <long,3> > ();
//    GenerateMinMaxValuesFromImage < itk::Image <unsigned long,3> > ();
//    GenerateMinMaxValuesFromImage < itk::Image <float,3> > ();
//    GenerateMinMaxValuesFromImage < itk::Image <double,3> > ();
//
//    if ( m_imageData ) {
//        medImageMaskAnnotationData * existingMaskAnnData = NULL;
//        foreach( medAttachedData * data, m_imageData->attachedData() ) {
//
//            if ( qobject_cast<medImageMaskAnnotationData*>(data) ) {
//                existingMaskAnnData =  qobject_cast<medImageMaskAnnotationData*>(data);
//                break;
//            }
//        }
//
//        if ( existingMaskAnnData ) {
//
//            m_maskAnnotationData = existingMaskAnnData;
//            m_maskData = existingMaskAnnData->maskData();
//
//        } else {
//
//            m_maskData =
//                    dtkAbstractDataFactory::instance()->createSmartPointer( medProcessPaintSegm::MaskImageTypeIdentifier() );
//
//            if ( !m_maskData ) {
//                dtkDebug() << DTK_PRETTY_FUNCTION << "Failed to create " << medProcessPaintSegm::MaskImageTypeIdentifier();
//                return;
//            }
//
//        //    if ( this->m_maskAnnotationData ) {
//        //        m_maskAnnotationData->parentData()->removeAttachedData(m_maskAnnotationData);
//        //    }
//
//            m_maskAnnotationData = new medImageMaskAnnotationData;
//            this->initializeMaskData( m_imageData, m_maskData );
//            m_maskAnnotationData->setMaskData(qobject_cast<medAbstractDataImage*>(m_maskData));
//
//            m_maskAnnotationData->setColorMap( m_labelColorMap );
//
//            m_imageData->addAttachedData(m_maskAnnotationData);
//        }
//    }
//
//    if ( m_imageData ) {
//        m_itkMask = dynamic_cast<MaskType*>( reinterpret_cast<itk::Object*>(m_maskData->data()) );
//        this->showButtons(true);
//    } else {
//        m_itkMask = NULL;
//        this->showButtons(false);
//    }
//}

//static
medSegmentationAbstractToolBox *bezierCurveToolBox::createInstance(QWidget *parent )
{
    return new bezierCurveToolBox( parent );
}

QString bezierCurveToolBox::s_description()
{
    static const QString desc = "Bezier Curve Tool";
    return desc;
}

QString bezierCurveToolBox::s_identifier()
{
    static const QString id = "bezierCurveToolBox";
    return id;
}

QString bezierCurveToolBox::s_name(const QObject * trObj)
{
    if (!trObj)
        trObj = qApp;

    return trObj->tr( "Bezier Curve Segmentation" );
}


//void bezierCurveToolBox::onViewClosed()
//{
//    medAbstractView *viewClosed = qobject_cast<medAbstractView*>(QObject::sender());
//    m_undoStacks->value(viewClosed)->clear();
//    m_redoStacks->value(viewClosed)->clear();
//    m_undoStacks->remove(viewClosed);
//    m_redoStacks->remove(viewClosed);
//    if (viewClosed==currentView)
//        currentView = NULL;
//}


void bezierCurveToolBox::update(dtkAbstractView *view)
{
    medToolBox::update(view);
    if(!view)
    {
        clear();
        return;
    }

    /* if ((currentView) && (currentView != medView) )
    {
    currentView->disconnect(this,0);
    clear();
    }*/

    medAbstractView * cast = dynamic_cast<medAbstractView *> (view);

    if (currentView==cast)
        return;
    
    currentView=cast;
    observer->setView(static_cast<vtkImageView2D *>(currentView->getView2D()));
    
    //QObject::connect(d->currentView, SIGNAL(dataAdded(dtkAbstractData*, int)),
    //                 this, SLOT(addData(dtkAbstractData*, int)),
    //                 Qt::UniqueConnection);

    //QObject::connect(d->currentView, SIGNAL(dataRemoved(dtkAbstractData*,int)),
    //                 this, SLOT(removeData(dtkAbstractData*, int)),
    //                 Qt::UniqueConnection);
}

//void bezierCurveToolBox::setCurrentView(medAbstractView * view)
//{
//    currentView = view;
//    
//    if (!m_undoStacks->contains(currentView)){
//        m_redoStacks->insert(currentView,new QStack<PairListSlicePlaneId>());
//        m_undoStacks->insert(currentView,new QStack<PairListSlicePlaneId>());
//        connect(view,SIGNAL(closed()),this,SLOT(onViewClosed()));
//    }
//}
//

void bezierCurveToolBox::onAddNewCurve()
{
    if (!currentView)
        return;

    newCurve = true;
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());

    vtkSmartPointer<vtkContourOverlayRepresentation> contourRep = vtkSmartPointer<vtkContourOverlayRepresentation>::New();
    contourRep->GetLinesProperty()->SetColor(0, 0, 1); 
    contourRep->GetLinesProperty()->SetLineWidth(1);
    contourRep->GetProperty()->SetPointSize(4);

    currentContour = vtkSmartPointer<vtkContourWidget>::New();
    currentContour->SetRepresentation(contourRep);
    currentContour->SetInteractor(view2d->GetInteractor());
    
    if (penMode)
        currentContour->ContinuousDrawOn();

    currentContour->GetEventTranslator()->SetTranslation(vtkCommand::RightButtonPressEvent,NULL);
    currentContour->AddObserver(vtkCommand::StartInteractionEvent,observer); 
    currentContour->AddObserver(vtkCommand::EndInteractionEvent,observer);
    
    currentContour->On();
    currentOrientation = view2d->GetViewOrientation(); 
    currentSlice = view2d->GetSlice();

     //Create balloonWidget
    vtkSmartPointer<vtkBalloonRepresentation> balloonRep = vtkSmartPointer<vtkBalloonRepresentation>::New();
    balloonRep->SetBalloonLayoutToTextRight();

    currentBalloon = vtkSmartPointer<vtkConstantBalloonWidget>::New();
    currentBalloon->SetInteractor(view2d->GetInteractor());
    currentBalloon->SetRepresentation(balloonRep);
    currentBalloon->AddBalloon(contourRep,"Area : ??\nMean : ?? SDev : ?? Sum : ??\nMin : ?? Max : ?? \nLength : ??"); //,view2d->GetImageInput(0)
    currentBalloon->On();
    currentBalloon->SetTimerDuration(0);
    currentBalloon->AttachToRightNode(contourRep);
    //balloonRep->VisibilityOn();
}

void bezierCurveToolBox::onPenMode()
{
    penMode = penMode_CheckBox->isChecked();
}

bezierCurveToolBox::listOfPair_CurveSlice * bezierCurveToolBox::getSagittalListOfCurves()
{
    return listOfCurvesForSagittal;
}

bezierCurveToolBox::listOfPair_CurveSlice * bezierCurveToolBox::getCoronalListOfCurves()
{
    return listOfCurvesForCoronal;
}

bezierCurveToolBox::listOfPair_CurveSlice * bezierCurveToolBox::getAxialListOfCurves()
{
    return listOfCurvesForAxial;
}

// this method shows the contours present on the currentOrientation and currentSlice. This method updates the currentOrientation and currentSlice variables 
// before showing the contours

void bezierCurveToolBox::showContour()
{
    if (!currentView)
        return;

    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());

    if (!view2d->GetRenderWindow())
        return;
    
    currentOrientation = view2d->GetViewOrientation();
    currentSlice = view2d->GetSlice();

    listOfPair_CurveSlice * list = getListOfCurrentOrientation();

    if (!list)
        return;

    for(int i=0;i<list->size();i++)
        if (list->at(i).second.first==currentSlice)
            list->at(i).first->On();
}

// this method hides the contours present in the currentOrientation and currentSlice. This method updates the currentOrientation and currentSlice only 
// if they were not initialized after construction of the instance of the class
void bezierCurveToolBox::hideContour()
{
    if (!currentView)
        return;

    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());

    if (currentOrientation == -1 && !currentSlice)
        return;
    
    if (!view2d->GetRenderWindow())
        return;
    
    listOfPair_CurveSlice * list = getListOfCurrentOrientation();

    if (!list)
        return;

    for(int i=0;i<list->size();i++)
        if (list->at(i).second.first==currentSlice)
            list->at(i).first->Off();
}

bezierCurveToolBox::listOfPair_CurveSlice * bezierCurveToolBox::getListOfCurrentOrientation()
{
    switch (currentOrientation)
    {    
    case 0:
        return listOfCurvesForSagittal;
    case 1:
        return listOfCurvesForCoronal;
    case 2:
        return listOfCurvesForAxial;
    }
    return NULL;
}

// For the time these function copy and paste all the contours present on a slice. No selection of a contour is possible.
void bezierCurveToolBox::copyContours()
{
    if (!currentView)
        return;

    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());

    currentOrientation = view2d->GetViewOrientation();
    currentSlice = view2d->GetSlice();

    if (!view2d->GetRenderWindow())
        return;

    ListOfContours->clear();
    listOfPair_CurveSlice * list = getListOfCurrentOrientation();
    
    for(int i=0;i<list->size();i++)
        if (list->at(i).second.first==currentSlice)
        {
            vtkSmartPointer<vtkPolyData> polydata = list->at(i).first->GetContourRepresentation()->GetContourRepresentationAsPolyData();
            ListOfContours->append(polydata);    
        }
}

void bezierCurveToolBox::pasteContours()
{
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
    currentSlice = view2d->GetSlice();
    pasteContours(currentSlice,currentSlice);
}

void bezierCurveToolBox::pasteContours(int slice1,int slice2)
{
    if (!currentView)
        return;

    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
    
    currentOrientation = view2d->GetViewOrientation();
    currentSlice = view2d->GetSlice();
    
    if (!view2d->GetRenderWindow())
        return;
    
    listOfPair_CurveSlice * list = getListOfCurrentOrientation();

    int min = slice1;
    int max = slice2;
    if (slice1>slice2)
    {
        min = slice2;
        max = slice1;
    }
    
    for (int j=min;j<=max;j++)
    {
        for(int i=0;i<ListOfContours->size();i++)
        {
            vtkSmartPointer<vtkContourWidget> contour = vtkSmartPointer<vtkContourWidget>::New();
        
            vtkSmartPointer<vtkContourOverlayRepresentation> contourRep = vtkSmartPointer<vtkContourOverlayRepresentation>::New();
            contourRep->GetLinesProperty()->SetColor(0, 0, 1); 
            contourRep->GetLinesProperty()->SetLineWidth(1);
            contourRep->GetProperty()->SetPointSize(4);
        
            contour->SetRepresentation(contourRep);
        
            contourRep->SetRenderer(view2d->GetRenderer());
            contour->SetInteractor(view2d->GetInteractor());
            if (penMode)
                contour->ContinuousDrawOn();

            vtkSmartPointer<vtkDecimatePolylineFilter> filter = vtkSmartPointer<vtkDecimatePolylineFilter>::New();
            filter->SetInput(ListOfContours->at(i));
            contour->GetEventTranslator()->SetTranslation(vtkCommand::RightButtonPressEvent,NULL);

            qDebug() << "Number of lines in the copied polyData : " << ListOfContours->at(i)->GetNumberOfLines();
            
            filter->Update();
            contour->Initialize(filter->GetOutput());
            contourRep->SetClosedLoop(1); 
            if (currentSlice==j)
                contour->On();

            list->append(QPair<vtkSmartPointer<vtkContourWidget>,PlaneIndexSlicePair>(contour,PlaneIndexSlicePair(j,computePlaneIndex())));
        }
    }
    currentView->update();
}

void bezierCurveToolBox::propagateCurve()
{
    if (!currentView)
        return;

    copyContours();
    int slice1 = bound1->value()-1;
    int slice2 = bound2->value()-1;
    pasteContours(slice1,slice2);
}

void bezierCurveToolBox::reorderPolygon(vtkPolyData * poly)
{
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());

    double displayPoint[3];
    double * worldPoint;
    double xmin=VTK_DOUBLE_MAX;
    int xminIndex;
    double ymin=VTK_DOUBLE_MAX;
    int yminIndex;

    for(int i=0;i<poly->GetNumberOfPoints();i++)
    {
        worldPoint = poly->GetPoint(i);
        vtkInteractorObserver::ComputeWorldToDisplay(view2d->GetRenderer(),worldPoint[0], worldPoint[1], worldPoint[2], displayPoint); 
        
        if (displayPoint[0]<xmin)
        {
            xmin =  displayPoint[0];
            xminIndex= i;
        }

        if (displayPoint[1]<ymin)
        {
            ymin =  displayPoint[1];
            yminIndex= i;
        }
    }

    int dist = xminIndex-yminIndex;
    bool reverse =((abs(dist)>poly->GetNumberOfPoints()/2)!=(dist<0));
    vtkSmartPointer<vtkPoints> reorderedPoints = vtkSmartPointer<vtkPoints>::New();
    vtkPoints * points = poly->GetPoints();
    if (reverse)
    {
        for(int i=0;i<poly->GetNumberOfPoints();i++)
        {
            double p[3];
            points->GetPoint(xminIndex,p);        
            xminIndex--;
            if (xminIndex<0)
                xminIndex = poly->GetNumberOfPoints()-1;
            reorderedPoints->InsertNextPoint(p);
        }
    }
    else
    {
        for(int i=0;i<poly->GetNumberOfPoints();i++)
        {
            double p[3];
            points->GetPoint(xminIndex,p);        
            xminIndex = (xminIndex+1)%poly->GetNumberOfPoints();
            reorderedPoints->InsertNextPoint(p);
        }
    }
    poly->SetPoints(reorderedPoints);
}

void bezierCurveToolBox::resampleCurve(vtkPolyData * poly,int nbPoints)
{
    vtkSmartPointer<vtkParametricSpline> spline =vtkSmartPointer<vtkParametricSpline>::New();
    poly->GetPoints()->InsertNextPoint(poly->GetPoints()->GetPoint(0));
    spline->SetPoints(poly->GetPoints());
    
    vtkPoints * points = vtkPoints::New();
    double p[3];
    double u[3];
    for(int j=0;j<nbPoints+1;j++)
    {
        u[0]=u[1]=u[2]=j/(double)(nbPoints+1);
        spline->Evaluate(u,p,NULL);
        points->InsertNextPoint(p);
    }
    
    poly->SetPoints(points);
}

QList<vtkPolyData* > bezierCurveToolBox::generateIntermediateCurves(vtkSmartPointer<vtkPolyData> curve1,vtkSmartPointer<vtkPolyData> curve2,int nb)
{
    int min = curve1->GetNumberOfPoints();
    int max = curve2->GetNumberOfPoints();
    
    vtkSmartPointer<vtkPolyData> startCurve = curve1, endCurve = curve2;
    bool curve2ToCurve1 = false;

    if (curve1->GetNumberOfPoints()>=curve2->GetNumberOfPoints())
    {
        min = curve2->GetNumberOfPoints(); 
        max = curve1->GetNumberOfPoints();
        startCurve = curve2;
        endCurve = curve1;
        curve2ToCurve1 = true;
    }
    
    resampleCurve(startCurve,max);
    resampleCurve(endCurve,max);

    reorderPolygon(startCurve);
    reorderPolygon(endCurve);
    
    vtkSmartPointer<vtkPoints>  bufferPoints = vtkPoints::New(); 
    QList<vtkPolyData*> list;
    for(int i=1;i<=nb;i++)
    {
        vtkPolyData * poly = vtkPolyData::New();
        vtkCellArray * cells = vtkCellArray::New();
        vtkPolyLine * polyLine = vtkPolyLine::New();
        
        polyLine->GetPointIds()->SetNumberOfIds(startCurve->GetNumberOfPoints());
        
        bufferPoints->Reset();
        for(int k=0;k<startCurve->GetNumberOfPoints();k++)
        {
            double p1[3],p2[3],p3[3];
            startCurve->GetPoint(k,p1);
            endCurve->GetPoint(k,p2);
            if (curve2ToCurve1)
            {
                p3[0]= p1[0] +(p2[0]-p1[0])*((i)/(double)(nb+1));
                p3[1]= p1[1] +(p2[1]-p1[1])*((i)/(double)(nb+1));
                p3[2]= p1[2] +(p2[2]-p1[2])*((i)/(double)(nb+1));
            }
            else
            {
                p3[0]= p2[0] +(p1[0]-p2[0])*((i)/(double)(nb+1));
                p3[1]= p2[1] +(p1[1]-p2[1])*((i)/(double)(nb+1));
                p3[2]= p2[2] +(p1[2]-p2[2])*((i)/(double)(nb+1));
            }
            bufferPoints->InsertNextPoint(p3);
            polyLine->GetPointIds()->SetId(k,k);
        }
        cells->InsertNextCell(polyLine);

        vtkPoints * polyPoints =vtkPoints::New();
        polyPoints->DeepCopy(bufferPoints);
        poly->SetPoints(polyPoints);
        poly->SetLines(cells);
        
        list.append(poly);
    }
    return list;
}

void bezierCurveToolBox::interpolateCurve()
{
    if (!currentView)
        return;

    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
    listOfPair_CurveSlice * list = getListOfCurrentOrientation();
    
    int maxSlice = 0;
    int minSlice = 999999;
    
    for(int i=0;i<list->size();i++)
    {
        if (list->at(i).second.first>maxSlice)
            maxSlice = list->at(i).second.first;
        if (list->at(i).second.first<minSlice)
            minSlice = list->at(i).second.first;
    }
    
    vtkSmartPointer<vtkPolyData> curve1;
    vtkSmartPointer<vtkPolyData> curve2;
    int curve1NbNode, curve2NbNode;

    for(int i=0;i<list->size();i++)
    {
        if (list->at(i).second.first==maxSlice)
        {
            curve1 = list->at(i).first->GetContourRepresentation()->GetContourRepresentationAsPolyData();
            curve1NbNode = list->at(i).first->GetContourRepresentation()->GetNumberOfNodes();
        }
        if (list->at(i).second.first==minSlice)
        {
            curve2 = list->at(i).first->GetContourRepresentation()->GetContourRepresentationAsPolyData();
            curve2NbNode = list->at(i).first->GetContourRepresentation()->GetNumberOfNodes();
        }
    }

    QList<vtkPolyData *> listPolyData = generateIntermediateCurves(curve1,curve2,maxSlice-minSlice-1);

    int number = ceil(4/3.0 * (double)(curve2NbNode));
    if (curve1NbNode>curve2NbNode)
        number = ceil(4/3.0 * (double)(curve1NbNode));

    for (int i = minSlice+1;i<maxSlice;i++)
    {
        vtkSmartPointer<vtkContourOverlayRepresentation> contourRep = vtkSmartPointer<vtkContourOverlayRepresentation>::New();
        contourRep->GetLinesProperty()->SetColor(0, 0, 1); 
        contourRep->GetLinesProperty()->SetLineWidth(1);
        contourRep->GetProperty()->SetPointSize(4);
        
        vtkSmartPointer<vtkContourWidget> contour = vtkSmartPointer<vtkContourWidget>::New();
        
        contour->SetRepresentation(contourRep);
        
        contour->GetEventTranslator()->SetTranslation(vtkCommand::RightButtonPressEvent,NULL);
        contourRep->SetRenderer(view2d->GetRenderer());
        contour->SetInteractor(view2d->GetInteractor());
        
        contour->Initialize(listPolyData.at(i-(minSlice+1))); 
        
        int nbPoints = contourRep->GetNumberOfNodes();
        int div = floor(nbPoints/(double)number);
        bool first = true;
        for (int k = nbPoints-1;k>=0;k--)
        {
            if (k%div!=0)
                contourRep->DeleteNthNode(k);
            else
                if (first) // delete the last node, no need for that normally
                {
                    contourRep->DeleteNthNode(k);
                    first=false;
                }
        }
        contourRep->SetClosedLoop(1); 
        if (currentSlice==i)
            contour->On();
        list->append(QPair<vtkSmartPointer<vtkContourWidget>,PlaneIndexSlicePair>(contour,PlaneIndexSlicePair(i,computePlaneIndex())));
    }
    currentView->update();
}

void bezierCurveToolBox::generateBinaryImage()
{
    if (!currentView)
        return;

    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
    listOfPair_CurveSlice * listAxial = getAxialListOfCurves();
    listOfPair_CurveSlice * listSagittal = getSagittalListOfCurves();
    listOfPair_CurveSlice * listCoronal = getCoronalListOfCurves();
    QList<QPair<vtkPolyData *,PlaneIndexSlicePair> > list1 = QList<QPair<vtkPolyData *,PlaneIndexSlicePair> >();
    
    int currentOrientation = view2d->GetViewOrientation();
    view2d->SetViewOrientation(2); // we set the view Orientation to the orientation of the next bezierCurve list to retreive the polyData with world coordinates based on the display from the orientation.
    for (int i =0;i<listAxial->size();i++)
    {
        vtkContourRepresentation * contourRep = listAxial->at(i).first->GetContourRepresentation();
        
        list1.append(QPair<vtkPolyData*,PlaneIndexSlicePair>(contourRep->GetContourRepresentationAsPolyData(),listAxial->at(i).second));
    }
    view2d->SetViewOrientation(0);
    for (int i =0;i<listSagittal->size();i++)
    {
        
        vtkContourRepresentation * contourRep = listSagittal->at(i).first->GetContourRepresentation();
        list1.append(QPair<vtkPolyData*,PlaneIndexSlicePair>(contourRep->GetContourRepresentationAsPolyData(),listSagittal->at(i).second));
    }
    view2d->SetViewOrientation(1);
    for (int i =0;i<listCoronal->size();i++)
    {
        vtkContourRepresentation * contourRep = listCoronal->at(i).first->GetContourRepresentation();
        list1.append(QPair<vtkPolyData*,PlaneIndexSlicePair>(contourRep->GetContourRepresentationAsPolyData(),listCoronal->at(i).second));
    }
    view2d->SetViewOrientation(currentOrientation);
    QList<QPair<vtkPolygon*,PlaneIndexSlicePair> > list2 = createImagePolygons(list1);
    binaryImageFromPolygon(list2);
}


QList<QPair<vtkPolygon*,bezierCurveToolBox::PlaneIndexSlicePair> > bezierCurveToolBox::createImagePolygons(QList<QPair<vtkPolyData*,PlaneIndexSlicePair> > &listPoly)
{
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
    QList<QPair<vtkPolygon*,PlaneIndexSlicePair> > listPolygon = QList<QPair<vtkPolygon*,PlaneIndexSlicePair> >();

    int ind=0;

    for(int i=0;i<listPoly.size();i++)
    {
        vtkPolygon *polygon = vtkPolygon::New();
        vtkPoints * points = vtkPoints::New();
        
        const int nb = listPoly.at(i).first->GetNumberOfPoints();
        
        vtkIdType ids[1000];
        double imagePreviousPoint[3] ={0,0,0};
        unsigned int nbPoint = 0;
        
        unsigned int x1,y1,z1;
        switch (listPoly.at(i).second.second)
        {
        case 0 :
            {
                x1=1;
                y1=2;
                z1=0;
                break;
            }
        case 1 :
            {
                x1=0;
                y1=2;
                z1=1;
                break;
            }
        case 2 :
            {
                x1=0;
                y1=1;
                z1=2;
                break;
            }
        }

        for(int j=0;j<nb;j++)
        {
            double * point = listPoly.at(i).first->GetPoint(j);
            
            int imagePoint[3];
            double imagePointDouble[3];
            
            view2d->GetImageCoordinatesFromWorldCoordinates(point,imagePoint); 
            
            imagePointDouble[x1]= (double)imagePoint[x1];
            imagePointDouble[y1]= (double)imagePoint[y1];

            imagePointDouble[z1]= (double)listPoly.at(i).second.first;

            if (imagePointDouble[x1]==imagePreviousPoint[x1] && imagePointDouble[y1]==imagePreviousPoint[y1] && imagePointDouble[z1]==imagePreviousPoint[z1])
                continue;
            
            points->InsertNextPoint(imagePointDouble);
            ids[nbPoint]=nbPoint;
            nbPoint++;

            imagePreviousPoint[x1] = imagePointDouble[x1];
            imagePreviousPoint[y1] = imagePointDouble[y1];
            imagePreviousPoint[z1] = imagePointDouble[z1];
        }
        
        polygon->Initialize(points->GetNumberOfPoints(),ids,points);

        listPolygon.append(QPair<vtkPolygon*,PlaneIndexSlicePair>(polygon,listPoly.at(i).second));
    }

    return listPolygon;
}


void bezierCurveToolBox::binaryImageFromPolygon(QList<QPair<vtkPolygon*,PlaneIndexSlicePair> > polys)
{
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
    
    typedef itk::Image<unsigned char,3> MaskType;
    dtkSmartPointer<medAbstractData> m_maskData = dtkAbstractDataFactory::instance()->createSmartPointer( medProcessPaintSegm::MaskImageTypeIdentifier() );
    initializeMaskData(dynamic_cast<medAbstractData*>(this->segmentationToolBox()->viewData(currentView)),m_maskData);
    MaskType::Pointer m_itkMask = dynamic_cast<MaskType*>( reinterpret_cast<itk::Object*>(m_maskData->data()) );

    for(int k=0;k<polys.size();k++)
    {
        double n[3];
        double bounds[6];
        polys[k].first->GetBounds(bounds);
        polys[k].first->ComputeNormal(polys[k].first->GetPoints()->GetNumberOfPoints(),static_cast<double*>(polys[k].first->GetPoints()->GetData()->GetVoidPointer(0)), n);
        unsigned int x,y,z;
        unsigned int bx,by;
        switch (polys[k].second.second)
        {
        case 0 :
            {
                x=1;
                y=2;
                z=0;
                bx = 2;
                by = 4;
                break;
            }
        case 1 :
            {
                x=0;
                y=2;
                z=1;
                bx = 0;
                by = 4;
                break;
            }
        case 2 :
            {
                x=0;
                y=1;
                z=2;
                bx = 0;
                by = 2;
                break;
            }
        }

        for(int i=bounds[bx];i<=bounds[bx+1];i++)
        {
            for(int j=bounds[by];j<=bounds[by+1];j++)
            {
                double pointTest[3];

                pointTest[x]=i;
                pointTest[y]=j;
                pointTest[z]=polys[k].second.first;

                int val =PointInPolygon(pointTest,polys[k].first->GetPoints()->GetNumberOfPoints(),static_cast<double*>(polys[k].first->GetPoints()->GetData()->GetVoidPointer(0)),
                    bounds,n);

                if (val)
                {
                    MaskType::IndexType index;
                    index[x]=i;index[y]=j;index[z]=polys[k].second.first;
                    m_itkMask->SetPixel(index,1);
                }
            }
        }
    }
    this->setOutputMetadata(dynamic_cast<medAbstractData*>(this->segmentationToolBox()->viewData(currentView)), m_maskData);
    medDataManager::instance()->importNonPersistent( m_maskData.data());
}

void bezierCurveToolBox::initializeMaskData( medAbstractData * imageData, medAbstractData * maskData )
{
    typedef itk::Image<unsigned char,3> MaskType; 
    MaskType::Pointer mask = MaskType::New();

    Q_ASSERT(mask->GetImageDimension() == 3);

    medAbstractDataImage * mImage = qobject_cast<medAbstractDataImage*>(imageData);
    Q_ASSERT(mImage);
    //Q_ASSERT(mask->GetImageDimension() >= mImage->Dimension());

    MaskType::RegionType region;
    region.SetSize(0, ( mImage->Dimension() > 0 ? mImage->xDimension() : 1 ) );
    region.SetSize(1, ( mImage->Dimension() > 1 ? mImage->yDimension() : 1 ) );
    region.SetSize(2, ( mImage->Dimension() > 2 ? mImage->zDimension() : 1 ) );

    MaskType::DirectionType direction;
    MaskType::SpacingType spacing;
    MaskType::PointType origin;

    direction.Fill(0);
    spacing.Fill(0);
    origin.Fill(0);
    for (unsigned int i = 0;i < mask->GetImageDimension();++i)
        direction(i,i) = 1;

    unsigned int maxIndex = std::min<unsigned int>(mask->GetImageDimension(),mImage->Dimension());

    switch (mImage->Dimension())
    {
        case 2:
        {
            itk::ImageBase <2> * imageDataOb = dynamic_cast<itk::ImageBase <2> *>( reinterpret_cast<itk::Object*>(imageData->data()) );

            for (unsigned int i = 0;i < maxIndex;++i)
            {
                for (unsigned int j = 0;j < maxIndex;++j)
                    direction(i,j) = imageDataOb->GetDirection()(i,j);

                spacing[i] = imageDataOb->GetSpacing()[i];
                origin[i] = imageDataOb->GetOrigin()[i];
            }

            break;
        }

        case 4:
        {
            itk::ImageBase <4> * imageDataOb = dynamic_cast<itk::ImageBase <4> *>( reinterpret_cast<itk::Object*>(imageData->data()) );

            for (unsigned int i = 0;i < maxIndex;++i)
            {
                for (unsigned int j = 0;j < maxIndex;++j)
                    direction(i,j) = imageDataOb->GetDirection()(i,j);

                spacing[i] = imageDataOb->GetSpacing()[i];
                origin[i] = imageDataOb->GetOrigin()[i];
            }

            break;
        }

        case 3:
        default:
        {
            itk::ImageBase <3> * imageDataOb = dynamic_cast<itk::ImageBase <3> *>( reinterpret_cast<itk::Object*>(imageData->data()) );

            for (unsigned int i = 0;i < maxIndex;++i)
            {
                for (unsigned int j = 0;j < maxIndex;++j)
                    direction(i,j) = imageDataOb->GetDirection()(i,j);

                spacing[i] = imageDataOb->GetSpacing()[i];
                origin[i] = imageDataOb->GetOrigin()[i];
            }

            break;
        }
    }

    mask->SetOrigin(origin);
    mask->SetDirection(direction);
    mask->SetSpacing(spacing);
    mask->SetLargestPossibleRegion(region);
    mask->SetBufferedRegion(region);
    mask->Allocate();
    mask->FillBuffer( medSegmentationSelectorToolBox::MaskPixelValues::Unset );

    maskData->setData((QObject*)(mask.GetPointer()));
}

void bezierCurveToolBox::setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData)
{
    Q_ASSERT(outputData && inputData);

    QStringList metaDataToCopy;
    metaDataToCopy 
        << medMetaDataKeys::PatientName.key()
        << medMetaDataKeys::StudyDescription.key();

    foreach( const QString & key, metaDataToCopy ) {
        outputData->setMetaData(key, inputData->metadatas(key));
    }

    QString seriesDesc;
    seriesDesc = tr("Segmented from ") + medMetaDataKeys::SeriesDescription.getFirstValue( inputData );

    medMetaDataKeys::SeriesDescription.set(outputData,seriesDesc);
}

int bezierCurveToolBox::computePlaneIndex()
{
    typedef  MaskType::DirectionType::InternalMatrixType::element_type ElemType;
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
    int planeIndex=-1;
    
    medAbstractViewCoordinates * coords = currentView->coordinates();
    if (coords->is2D()){
        const QVector3D vpn = coords->viewPlaneNormal();
    //    
    //    const MaskType::DirectionType & direction = m_itkMask->GetDirection();
        vnl_vector_fixed<ElemType, 3> vecVpn(vpn.x(), vpn.y(), vpn.z() );
        vtkMatrix4x4 * direction = view2d->GetOrientationMatrix();
        double absDotProductMax = 0;
        planeIndex = 0;
        for (unsigned int i = 0;i < 3;++i)
        {
            double dotProduct = 0;
            for (unsigned int j = 0;j < 3;++j)
                dotProduct += direction->GetElement(j,i)* vecVpn[j];

            if (fabs(dotProduct) > absDotProductMax)
            {
                planeIndex = i;
                absDotProductMax = fabs(dotProduct);
            }
        }
    }
    return planeIndex;
}


#define VTK_POLYGON_CERTAIN 1
#define VTK_POLYGON_UNCERTAIN 0
#define VTK_POLYGON_RAY_TOL 1.e-03 //Tolerance for ray firing
#define VTK_POLYGON_MAX_ITER 10    //Maximum iterations for ray-firing
#define VTK_POLYGON_VOTE_THRESHOLD 2
#define VTK_POLYGON_FAILURE -1
#define VTK_POLYGON_OUTSIDE 0
#define VTK_POLYGON_INSIDE 1
#define VTK_POLYGON_INTERSECTION 2
#define VTK_POLYGON_ON_LINE 3


#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

//----------------------------------------------------------------------------
// Determine whether point is inside polygon. Function uses ray-casting
// to determine if point is inside polygon. Works for arbitrary polygon shape
// (e.g., non-convex). Returns 0 if point is not in polygon; 1 if it is.
// Can also return -1 to indicate degenerate polygon. Note: a point in
// bounding box check is NOT performed prior to in/out check. You may want
// to do this to improve performance.
int bezierCurveToolBox::PointInPolygon (double x[3], int numPts, double *pts, 
                                double bounds[6], double *n)
{
  double *x1, *x2, xray[3], u, v;
  double rayMag, mag=1, ray[3];
  int testResult, rayOK, status, numInts, i;
  int iterNumber;
  int maxComp, comps[2];
  int deltaVotes;

  // do a quick bounds check
  if ( x[0] < bounds[0] || x[0] > bounds[1] ||
       x[1] < bounds[2] || x[1] > bounds[3] ||
       x[2] < bounds[4] || x[2] > bounds[5])
    {
    return VTK_POLYGON_OUTSIDE;
    }
  
  //
  //  Define a ray to fire.  The ray is a random ray normal to the
  //  normal of the face.  The length of the ray is a function of the
  //  size of the face bounding box.
  //
  for (i=0; i<3; i++)
    {
    ray[i] = ( bounds[2*i+1] - bounds[2*i] )*1.1 +
      fabs((bounds[2*i+1] + bounds[2*i])/2.0 - x[i]);
    }

  if ( (rayMag = vtkMath::Norm(ray)) == 0.0 )
    {
    return VTK_POLYGON_OUTSIDE;
    }

  //  Get the maximum component of the normal.
  //
  if ( fabs(n[0]) > fabs(n[1]) )
    {
    if ( fabs(n[0]) > fabs(n[2]) ) 
      {
      maxComp = 0;
      comps[0] = 1;
      comps[1] = 2;
      } 
    else 
      {
      maxComp = 2;
      comps[0] = 0;
      comps[1] = 1;
      }
    }
  else
    {
    if ( fabs(n[1]) > fabs(n[2]) ) 
      {
      maxComp = 1;
      comps[0] = 0;
      comps[1] = 2;
      } 
    else 
      {
      maxComp = 2;
      comps[0] = 0;
      comps[1] = 1;
      }
    }

  //  Check that max component is non-zero
  //
  if ( n[maxComp] == 0.0 )
    {
    return VTK_POLYGON_FAILURE;
    }

  //  Enough information has been acquired to determine the random ray.
  //  Random rays are generated until one is satisfactory (i.e.,
  //  produces a ray of non-zero magnitude).  Also, since more than one
  //  ray may need to be fired, the ray-firing occurs in a large loop.
  //
  //  The variable iterNumber counts the number of iterations and is
  //  limited by the defined variable VTK_POLYGON_MAX_ITER.
  //
  //  The variable deltaVotes keeps track of the number of votes for
  //  "in" versus "out" of the face.  When delta_vote > 0, more votes
  //  have counted for "in" than "out".  When delta_vote < 0, more votes
  //  have counted for "out" than "in".  When the delta_vote exceeds or
  //  equals the defined variable VTK_POLYGON_VOTE_THRESHOLD, than the
  //  appropriate "in" or "out" status is returned.
  //
  for (deltaVotes = 0, iterNumber = 1;
       (iterNumber < VTK_POLYGON_MAX_ITER)
         && (abs(deltaVotes) < VTK_POLYGON_VOTE_THRESHOLD);
       iterNumber++) 
    {
    //
    //  Generate ray
    //
    for (rayOK = FALSE; rayOK == FALSE; ) 
      {
      ray[comps[0]] = vtkMath::Random(-rayMag, rayMag);
      ray[comps[1]] = vtkMath::Random(-rayMag, rayMag);
      ray[maxComp] = -(n[comps[0]]*ray[comps[0]] + 
                        n[comps[1]]*ray[comps[1]]) / n[maxComp];
      if ( (mag = vtkMath::Norm(ray)) > rayMag*VTK_TOL )
        {
        rayOK = TRUE;
        }
      }

    //  The ray must be appropriately sized.
    //
    for (i=0; i<3; i++)
      {
      xray[i] = x[i] + (rayMag/mag)*ray[i];
      }

    //  The ray may now be fired against all the edges
    //
    for (numInts=0, testResult=VTK_POLYGON_CERTAIN, i=0; i<numPts; i++) 
      {
      x1 = pts + 3*i;
      x2 = pts + 3*((i+1)%numPts);
      if (x1[0]==x2[0] && x1[1]==x2[1] && x1[2]==x2[2])
          continue;

      //   Fire the ray and compute the number of intersections.  Be careful
      //   of degenerate cases (e.g., ray intersects at vertex).
      //
      if ((status=vtkLine::Intersection(x,xray,x1,x2,u,v)) == VTK_POLYGON_INTERSECTION) 
        {
        if ( (VTK_POLYGON_RAY_TOL < v) && (v < 1.0-VTK_POLYGON_RAY_TOL) )
          {
          numInts++;
          }
        else
          {
          testResult = VTK_POLYGON_UNCERTAIN;
          }
        } 
      else if ( status == VTK_POLYGON_ON_LINE )
        {
        testResult = VTK_POLYGON_UNCERTAIN;
        }
      }
    if ( testResult == VTK_POLYGON_CERTAIN ) 
      {
      if ( (numInts % 2) == 0)
          {
          --deltaVotes;
          }
      else
        {
        ++deltaVotes;
        }
      }
    } //try another ray

  //   If the number of intersections is odd, the point is in the polygon.
  //
  if ( deltaVotes < 0 )
    {
    return VTK_POLYGON_OUTSIDE;
    }
  else
    {
    return VTK_POLYGON_INSIDE;
    }
}