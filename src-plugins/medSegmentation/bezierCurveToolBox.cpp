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
#include <vtkBalloonWidget.h>
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

class bezierObserver : public vtkCommand
{
public:
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
                    toolBox->getSagittalListOfCurves()->append(QPair<vtkSmartPointer<vtkContourWidget>,unsigned int>(contour,view->GetSlice()));
                    break;
                }
            case 1:
                {
                    toolBox->getCoronalListOfCurves()->append(QPair<vtkSmartPointer<vtkContourWidget>,unsigned int>(contour,view->GetSlice()));
                    break;
                }
            case 2:
                toolBox->getAxialListOfCurves()->append(QPair<vtkSmartPointer<vtkContourWidget>,unsigned int>(contour,view->GetSlice()));
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

    listOfCurvesForSagittal = new QList<QPair<vtkSmartPointer<vtkContourWidget>,unsigned int> >();
    listOfCurvesForAxial = new QList<QPair<vtkSmartPointer<vtkContourWidget>,unsigned int> >();
    listOfCurvesForCoronal = new QList<QPair<vtkSmartPointer<vtkContourWidget>,unsigned int> >();

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


//void bezierCurveToolBox::initializeMaskData( medAbstractData * imageData, medAbstractData * maskData )
//{
//    MaskType::Pointer mask = MaskType::New();
//
//    Q_ASSERT(mask->GetImageDimension() == 3);
//
//    medAbstractDataImage * mImage = qobject_cast<medAbstractDataImage*>(imageData);
//    Q_ASSERT(mImage);
//    //Q_ASSERT(mask->GetImageDimension() >= mImage->Dimension());
//
//    MaskType::RegionType region;
//    region.SetSize(0, ( mImage->Dimension() > 0 ? mImage->xDimension() : 1 ) );
//    region.SetSize(1, ( mImage->Dimension() > 1 ? mImage->yDimension() : 1 ) );
//    region.SetSize(2, ( mImage->Dimension() > 2 ? mImage->zDimension() : 1 ) );
//
//    MaskType::DirectionType direction;
//    MaskType::SpacingType spacing;
//    MaskType::PointType origin;
//
//    direction.Fill(0);
//    spacing.Fill(0);
//    origin.Fill(0);
//    for (unsigned int i = 0;i < mask->GetImageDimension();++i)
//        direction(i,i) = 1;
//
//    unsigned int maxIndex = std::min<unsigned int>(mask->GetImageDimension(),mImage->Dimension());
//
//    switch (mImage->Dimension())
//    {
//        case 2:
//        {
//            itk::ImageBase <2> * imageDataOb = dynamic_cast<itk::ImageBase <2> *>( reinterpret_cast<itk::Object*>(imageData->data()) );
//
//            for (unsigned int i = 0;i < maxIndex;++i)
//            {
//                for (unsigned int j = 0;j < maxIndex;++j)
//                    direction(i,j) = imageDataOb->GetDirection()(i,j);
//
//                spacing[i] = imageDataOb->GetSpacing()[i];
//                origin[i] = imageDataOb->GetOrigin()[i];
//            }
//
//            break;
//        }
//
//        case 4:
//        {
//            itk::ImageBase <4> * imageDataOb = dynamic_cast<itk::ImageBase <4> *>( reinterpret_cast<itk::Object*>(imageData->data()) );
//
//            for (unsigned int i = 0;i < maxIndex;++i)
//            {
//                for (unsigned int j = 0;j < maxIndex;++j)
//                    direction(i,j) = imageDataOb->GetDirection()(i,j);
//
//                spacing[i] = imageDataOb->GetSpacing()[i];
//                origin[i] = imageDataOb->GetOrigin()[i];
//            }
//
//            break;
//        }
//
//        case 3:
//        default:
//        {
//            itk::ImageBase <3> * imageDataOb = dynamic_cast<itk::ImageBase <3> *>( reinterpret_cast<itk::Object*>(imageData->data()) );
//
//            for (unsigned int i = 0;i < maxIndex;++i)
//            {
//                for (unsigned int j = 0;j < maxIndex;++j)
//                    direction(i,j) = imageDataOb->GetDirection()(i,j);
//
//                spacing[i] = imageDataOb->GetSpacing()[i];
//                origin[i] = imageDataOb->GetOrigin()[i];
//            }
//
//            break;
//        }
//    }
//
//    mask->SetOrigin(origin);
//    mask->SetDirection(direction);
//    mask->SetSpacing(spacing);
//    mask->SetLargestPossibleRegion(region);
//    mask->SetBufferedRegion(region);
//    mask->Allocate();
//    mask->FillBuffer( medSegmentationSelectorToolBox::MaskPixelValues::Unset );
//
//    maskData->setData((QObject*)(mask.GetPointer()));
//}
//




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
    contourRep->GetLinesProperty()->SetLineWidth(3);
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
    balloonRep->SetBalloonLayoutToImageRight();

    currentBalloon = vtkSmartPointer<vtkBalloonWidget>::New();
    currentBalloon->SetInteractor(view2d->GetInteractor());
    currentBalloon->SetRepresentation(balloonRep);
    currentBalloon->AddBalloon(contourRep,"Area : ??\nMean : ?? SDev : ?? Sum : ??\nMin : ?? Max : ?? \nLength : ??"); //,view2d->GetImageInput(0)
    currentBalloon->On();
    currentBalloon->SetTimerDuration(0);
    //currentBalloon->InvokeEvent(
        //balloonRep->Pick();
    balloonRep->VisibilityOn();
}

//
//void bezierCurveToolBox::generateBinaryImage(vtkPolyData * pd)
//{
//    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
//
//    vtkSmartPointer<vtkImageData> whiteImage = vtkSmartPointer<vtkImageData>::New();    
//    whiteImage->SetSpacing(view2d->GetImageInput(0)->GetSpacing());
//    whiteImage->SetDimensions(view2d->GetImageInput(0)->GetDimensions());
//    whiteImage->SetExtent(view2d->GetImageInput(0)->GetExtent());
//    whiteImage->SetOrigin(view2d->GetImageInput(0)->GetOrigin());
//    
//    whiteImage->SetScalarTypeToUnsignedChar();
//    whiteImage->AllocateScalars();
//
//    // fill the image with foreground voxels:
//    unsigned char inval = 255;
//    unsigned char outval = 0;
//    vtkIdType count = whiteImage->GetNumberOfPoints();
//    for (vtkIdType i = 0; i < count; ++i)
//    {
//        whiteImage->GetPointData()->GetScalars()->SetTuple1(i, inval);
//    }
//
//    // polygonal data --> image stencil:
//    vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
//
//    pol2stenc->SetInput(pd);
//
//    pol2stenc->SetOutputOrigin(whiteImage->GetOrigin());
//    pol2stenc->SetOutputSpacing(whiteImage->GetSpacing());
//    pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
//    
//    pol2stenc->Update();
//
//    // cut the corresponding white image and set the background:
//    vtkSmartPointer<vtkImageStencil> imgstenc = vtkSmartPointer<vtkImageStencil>::New();
//
//    imgstenc->SetInput(whiteImage);
//    imgstenc->SetStencil(pol2stenc->GetOutput());
//    
//    imgstenc->ReverseStencilOff();
//    imgstenc->SetBackgroundValue(outval);
//    imgstenc->Update();
//
//    vtkSmartPointer<vtkMetaImageWriter> writer = 
//        vtkSmartPointer<vtkMetaImageWriter>::New();
//    writer->SetFileName("GenerateBinaryImage.mhd");
//
//    writer->SetInput(imgstenc->GetOutput());
//
//    writer->Write();  
//    
//}


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
        if (list->at(i).second==currentSlice)
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
        if (list->at(i).second==currentSlice)
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
        if (list->at(i).second==currentSlice)
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
            contourRep->GetLinesProperty()->SetLineWidth(3);
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

            list->append(QPair<vtkSmartPointer<vtkContourWidget>,unsigned int>(contour,j));
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



void bezierCurveToolBox::interpolateCurve()
{
    if (!currentView)
        return;

    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
    listOfPair_CurveSlice * list = getListOfCurrentOrientation();
    QList<QPair<vtkPolyData *,unsigned int> > listTest = QList<QPair<vtkPolyData *,unsigned int> >();

    int maxSlice = 0;
    int minSlice = 999999;
    
    for(int i=0;i<list->size();i++)
    {
        if (list->at(i).second>maxSlice)
            maxSlice = list->at(i).second;
        if (list->at(i).second<minSlice)
            minSlice = list->at(i).second;
    }
    
    vtkSmartPointer<vtkPolyData> curve1;
    vtkSmartPointer<vtkPolyData> curve2;
    int curve1NbNode, curve2NbNode;

    for(int i=0;i<list->size();i++)
    {
        if (list->at(i).second==maxSlice)
        {
            curve1 = list->at(i).first->GetContourRepresentation()->GetContourRepresentationAsPolyData();
            curve1NbNode = list->at(i).first->GetContourRepresentation()->GetNumberOfNodes();
        }
        if (list->at(i).second==minSlice)
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
        contourRep->GetLinesProperty()->SetLineWidth(3);
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
        list->append(QPair<vtkSmartPointer<vtkContourWidget>,unsigned int>(contour,i));
        listTest.append(QPair<vtkPolyData*,unsigned int>(contourRep->GetContourRepresentationAsPolyData(),i));
    }
    currentView->update();
    QList<QPair<vtkPolygon*,unsigned int> > listTest2 = createImagePolygons(listTest);
    generateBinaryImage(listTest2);
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
    //int nbpoints = max;
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

QList<QPair<vtkPolygon*,unsigned int> > bezierCurveToolBox::createImagePolygons(QList<QPair<vtkPolyData*,unsigned int> > &listPoly)
{
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
    
    //vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New();
    /*vtkCellArray * polygons = vtkCellArray::New();*/
    
    QList<QPair<vtkPolygon*,unsigned int> > listPolygon = QList<QPair<vtkPolygon*,unsigned int> >();

    /*int nb = 0;*/
    int ind=0;
    /*for(int i=0;i<listPoly.size();i++)
        nb+=listPoly.at(i).first->GetNumberOfPoints();
    points->SetNumberOfPoints(nb); */
    
    for(int i=0;i<listPoly.size();i++)
    {
        vtkPolygon *polygon = vtkPolygon::New();
        vtkPoints * points = vtkPoints::New();
        
        const int nb = listPoly.at(i).first->GetNumberOfPoints();
        
        points->SetNumberOfPoints(nb);
        vtkIdType ids[1000];

        //polygon->GetPointIds()->SetNumberOfIds(nb);
            
        for(int j=0;j<nb;j++)
        {
            double * point = listPoly.at(i).first->GetPoint(j);
            int imagePoint[3];
            double imagePointDouble[3];

            view2d->GetImageCoordinatesFromWorldCoordinates(point,imagePoint);
            //qDebug() << "number of points of polygon : " << polygon->GetNumberOfPoints();
            //qDebug() << "number of points of points of polygon : " << polygon->GetPoints()->GetNumberOfPoints();
            //qDebug() << "imagePoint[0] : "  << imagePoint[0];
            //qDebug() << "imagePoint[1] : "  << imagePoint[1];
            //qDebug() << "imagePoint[2] : "  << imagePoint[2];

            imagePointDouble[0]= (double)imagePoint[0];
            imagePointDouble[1]= (double)imagePoint[1];
            //qDebug() << "SLICE NUMBER : "  << listPoly.at(i).second;
            imagePointDouble[2]= (double)listPoly.at(i).second;
            //qDebug() << "imagePointDouble[0] : "  << imagePointDouble[0];
            //qDebug() << "imagePointDouble[1] : "  << imagePointDouble[1];
            //qDebug() << "imagePointDouble[2] : "  << imagePointDouble[2];
            points->InsertPoint(j,imagePointDouble);
            ids[j]=j;
            //polygon->GetPointIds()->SetId(j,j);
        }
        
        
        polygon->Initialize(nb,ids,points);
        //polygon->GetPointIds()->SetNumberOfIds(nb);
        

    /*qDebug() <<" NUMBER OF CELLS " <<poly->GetNumberOfCells();
    qDebug() <<" NUMBER OF POINTS " <<poly->GetNumberOfPoints();*/
        listPolygon.append(QPair<vtkPolygon*,unsigned int>(polygon,listPoly.at(i).second));
        //polygons->InsertNextCell(polygon);
        //ind+=nb;
    }
    //poly->SetPolys(polygons);
    //poly->SetPoints(points);
    //   qDebug() << "point = " << point[0] << " " << point[1] << " " << point[2] << " imagePoint = " << imagePoint[0] << " " << imagePoint[1] << " " << imagePoint[2];   
   
   

    return listPolygon;
}


void bezierCurveToolBox::generateBinaryImage(QList<QPair<vtkPolygon*,unsigned int> > polys)
{
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());
    int *dims = view2d->GetImageInput(0)->GetDimensions();
    typedef itk::Image<unsigned char,3> MaskType;
    dtkSmartPointer<medAbstractData> m_maskData = dtkAbstractDataFactory::instance()->createSmartPointer( medProcessPaintSegm::MaskImageTypeIdentifier() );
    initializeMaskData(dynamic_cast<medAbstractData*>(this->segmentationToolBox()->viewData(currentView)),m_maskData);
    MaskType::Pointer m_itkMask = dynamic_cast<MaskType*>( reinterpret_cast<itk::Object*>(m_maskData->data()) );
    bool firstime = true;
    QList<double*> boundsList;
    
    QList<QList<double>> normalList;
    
    double bigBounds[6]={99999,-99999,99999,-99999,99999,-99999};
    for(int c=0;c<polys.size();c++)
    {
        double bounds[6];
        polys[c].first->GetBounds(bounds);
        if (bounds[0]<bigBounds[0])
            bigBounds[0]=bounds[0];
        if (bounds[1]>bigBounds[1])
            bigBounds[1]=bounds[1];
        if (bounds[2]<bigBounds[2])
            bigBounds[2]=bounds[2];
        if (bounds[3]>bigBounds[3])
            bigBounds[3]=bounds[3];
        if (bounds[4]<bigBounds[4])
            bigBounds[4]=bounds[4];
        if (bounds[5]>bigBounds[5])
            bigBounds[5]=bounds[5];

        boundsList.append(polys[c].first->GetBounds());
// PROBLEME HERE !! NORMAL AND BOUNDS IN LIST !! 
        double n[3];
        polys[c].first->ComputeNormal(polys[c].first->GetPoints()->GetNumberOfPoints(),static_cast<double*>(polys[c].first->GetPoints()->GetData()->GetVoidPointer(0)), n);
        //normalList.append(n);
        QList<double> normal;
        normal.append(n[0]);
        normal.append(n[1]);
        normal.append(n[2]);
        normalList.append(normal);
        
    }
    
    for(int i=bigBounds[0];i<=bigBounds[1];i++)
    {
        for(int j=bigBounds[2];j<=bigBounds[3];j++)
        {
            for(int k=0;k<polys.size();k++)
            {
                    double pointTest[3];
                    double n[3];

                    n[0]=normalList[k][0];
                    n[1]=normalList[k][1];
                    n[2]=normalList[k][2];


                    pointTest[0]=i;
                    pointTest[1]=j;
                    pointTest[2]=polys[k].second;
                    
                    int val =PointInPolygon(pointTest,polys[k].first->GetPoints()->GetNumberOfPoints(),static_cast<double*>(polys[k].first->GetPoints()->GetData()->GetVoidPointer(0)),
                        boundsList[k],n);
                    if (i==400 && j == 300)
                    {
                        qDebug() << boundsList[k][0] <<" "<< boundsList[k][1] << " "<< boundsList[k][2] << " "<< boundsList[k][3] << " "<< boundsList[k][4] << " "<< boundsList[k][5];
                        qDebug() << normalList[k][0] << " "<< normalList[k][1] << " "<< normalList[k][2];
                        qDebug() << k << " YOU GOT TO BE KIDDING ME " << val; 
                    }
                    
                    if (val)
                    {
                        MaskType::IndexType index;
                        index[0]=i;index[1]=j;index[2]=polys[k].second;
                        m_itkMask->SetPixel(index,1);
                    }
            }
        }
    }
    this->setOutputMetadata(dynamic_cast<medAbstractData*>(this->segmentationToolBox()->viewData(currentView)), m_maskData);
    medDataManager::instance()->importNonPersistent( m_maskData.data());

    //vtkSmartPointer<vtkImageData> whiteImage = vtkSmartPointer<vtkImageData>::New();    
    //whiteImage->SetSpacing(view2d->GetImageInput(0)->GetSpacing());
    //whiteImage->SetDimensions(view2d->GetImageInput(0)->GetDimensions());
    //whiteImage->SetExtent(view2d->GetImageInput(0)->GetExtent());
    //whiteImage->SetOrigin(view2d->GetImageInput(0)->GetOrigin());
    //
    //whiteImage->SetScalarTypeToUnsignedChar();
    //whiteImage->AllocateScalars();

    //// fill the image with foreground voxels:
    //unsigned char inval = 255;
    //unsigned char outval = 0;
    //vtkIdType count = whiteImage->GetNumberOfPoints();
    //for (vtkIdType i = 0; i < count; ++i)
    //{
    //    whiteImage->GetPointData()->GetScalars()->SetTuple1(i, inval);
    //}

    //// polygonal data --> image stencil:
    //vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc = vtkSmartPointer<vtkPolyDataToImageStencil>::New();

    //pol2stenc->SetInput(pd);

    //pol2stenc->SetOutputOrigin(whiteImage->GetOrigin());
    //pol2stenc->SetOutputSpacing(whiteImage->GetSpacing());
    //pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
    //
    //pol2stenc->Update();

    //// cut the corresponding white image and set the background:
    //vtkSmartPointer<vtkImageStencil> imgstenc = vtkSmartPointer<vtkImageStencil>::New();

    //imgstenc->SetInput(whiteImage);
    //imgstenc->SetStencil(pol2stenc->GetOutput());
    //
    //imgstenc->ReverseStencilOff();
    //imgstenc->SetBackgroundValue(outval);
    //imgstenc->Update();

    //vtkSmartPointer<vtkMetaImageWriter> writer = 
    //    vtkSmartPointer<vtkMetaImageWriter>::New();
    //writer->SetFileName("GenerateBinaryImage.mhd");

    //writer->SetInput(imgstenc->GetOutput());

    //writer->Write();  


    
    
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
      {
          //qDebug() << "Same point";
          continue;
      }

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
    //qDebug()<<"deltaVotes "<<deltaVotes;
    return VTK_POLYGON_INSIDE;
    }
}