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

#include <vtkSphereSource.h>

//
#include <vtkThinPlateSplineTransform.h>
#include <vtkMetaImageWriter.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkImageActorPointPlacer.h>
#include <vtkFocalPlanePointPlacer.h>

#include <vtkBalloonRepresentation.h>
#include <vtkBalloonWidget.h>
#include <vtkPropCollection.h>
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkAxisActor2D.h>
#include <vtkOrientedGlyphFocalPlaneContourRepresentation.h>
#include <vtkContourOverlayRepresentation.h>
#include <vtkContourRepresentation.h>
#include <vtkProperty2D.h>

#include <vtkWidgetEvent.h>
#include <vtkDecimatePolylineFilter.h>

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
    generateBinaryImage_button->hide();
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

    ListOfContours = new QList<vtkSmartPointer<vtkPolyData>>();
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

    // Create balloonWidget
    //vtkSmartPointer<vtkBalloonRepresentation> balloonRep = vtkSmartPointer<vtkBalloonRepresentation>::New();
    //balloonRep->SetBalloonLayoutToImageRight();

    //vtkSmartPointer<vtkBalloonWidget> balloonWidget =vtkSmartPointer<vtkBalloonWidget>::New();
    //balloonWidget->SetInteractor(view2d->GetInteractor());
    //balloonWidget->SetRepresentation(balloonRep);
    //vtkSmartPointer<vtkPropCollection> test = vtkSmartPointer<vtkPropCollection>::New();
    //listOfCurves->at(0)->GetRepresentation()->GetActors(test);
    //    
    //balloonWidget->AddBalloon(test->GetNextProp(),"this is a bezier curve"); //,view2d->GetImageInput(0)
    //balloonWidget->EnabledOn();
    //balloonWidget->Render();
    //view2d->GetRenderWindow()->Render();
    //balloonWidget->On();




}


void bezierCurveToolBox::generateBinaryImage()
{
    /*   if (listOfCurves->isEmpty())
    return;*/
    // Generate binary image
    //vtkSmartPointer<vtkPolyData> polyData = dynamic_cast<vtkOrientedGlyphContourRepresentation * >(listOfCurves->at(0)->GetRepresentation())->GetContourRepresentationAsPolyData();

    //vtkSmartPointer<vtkImageData> whiteImage = vtkSmartPointer<vtkImageData>::New();
    //double bounds[6];
    //polyData->GetBounds(bounds);
    //double spacing[3]; // desired volume spacing
    //whiteImage->SetScalarTypeToUnsignedChar();
    //whiteImage->AllocateScalars();
    //vtkSmartPointer<vtkLinearExtrusionFilter> extruder =
    //vtkSmartPointer<vtkLinearExtrusionFilter>::New();
    //extruder->SetInput(polyData);
    //extruder->SetScaleFactor(1.);
    //extruder->SetExtrusionTypeToNormalExtrusion();
    //extruder->SetVector(0, 0, 1);
    //extruder->Update();
    //// polygonal data --> image stencil:l
    //vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
    //vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    //pol2stenc->SetTolerance(0); // important if extruder->SetVector(0, 0, 1) !!!
    //pol2stenc->SetInputConnection(extruder->GetOutputPort());
    //pol2stenc->SetOutputOrigin( static_cast<vtkImageView2D *>(currentView->getView2D())->GetImageActor()->GetCenter());
    ////pol2stenc->SetOutputSpacing(spacing);
    //pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
    //pol2stenc->Update();

    //// cut the corresponding white image and set the background:
    //vtkSmartPointer<vtkImageStencil> imgstenc =
    //vtkSmartPointer<vtkImageStencil>::New(); 
    //  
    //imgstenc->SetInput(whiteImage);
    //imgstenc->SetStencil(pol2stenc->GetOutput());
    //imgstenc->ReverseStencilOff();
    //imgstenc->SetBackgroundValue(0);
    //imgstenc->Update();
    //vtkSmartPointer<vtkMetaImageWriter> imageWriter =
    //vtkSmartPointer<vtkMetaImageWriter>::New();
    //imageWriter->SetFileName("labelImage.mhd");
    //imageWriter->SetInputConnection(imgstenc->GetOutputPort());
    //imageWriter->Write();
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
}

// For the time these function copy and paste all the contours present on a slice. No selection of a contour is possible.
void bezierCurveToolBox::copyContours()
{
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
    
    currentOrientation = view2d->GetViewOrientation();
    currentSlice = view2d->GetSlice();
    
    if (!view2d->GetRenderWindow())
        return;
    
    listOfPair_CurveSlice * list = getListOfCurrentOrientation();

    for(int i=0;i<ListOfContours->size();i++)
    {
        vtkSmartPointer<vtkContourWidget> contour = vtkSmartPointer<vtkContourWidget>::New();
        
        vtkSmartPointer<vtkContourOverlayRepresentation> contourRep = vtkSmartPointer<vtkContourOverlayRepresentation>::New();
        contourRep->GetLinesProperty()->SetColor(0, 0, 1); 
        contourRep->GetLinesProperty()->SetLineWidth(3);
        
        contour->SetRepresentation(contourRep);
        
        contourRep->SetRenderer(view2d->GetRenderer());
        contour->SetInteractor(view2d->GetInteractor());
        if (penMode)
            contour->ContinuousDrawOn();
        vtkSmartPointer<vtkDecimatePolylineFilter> filter = vtkSmartPointer<vtkDecimatePolylineFilter>::New();
        filter->SetInput(ListOfContours->at(i));
        contour->GetEventTranslator()->SetTranslation(vtkCommand::RightButtonPressEvent,NULL);
        contour->AddObserver(vtkCommand::StartInteractionEvent,observer); 
        contour->Initialize(ListOfContours->at(i));
        qDebug() << "test " << contourRep->GetNumberOfNodes();
        double number = 10;
        qDebug() << "reduction ratio " << (100-(number*(double)(100)/(double)(contourRep->GetNumberOfNodes())))/100;
        filter->SetTargetReduction((100-(number*(double)(100)/(double)(contourRep->GetNumberOfNodes())))/100);
        filter->Update();
        contour->Initialize(filter->GetOutput());
        contourRep->SetClosedLoop(1); // not sure whether it is needed or not
        
        contour->On();
        list->append(QPair<vtkSmartPointer<vtkContourWidget>,unsigned int>(contour,currentSlice));
    }
    currentView->update();
}


//INTERPOLATION ALGORITHM ROI MORPHINGBETWEEN OSIRIX

//2 bezier ROI -> Prendre celle avec le plus de point de controle.
  //  N = maxPoint +1/3maxpoint => N est le nombre de point de controle des nouvelles ROI
// Reordonner le tableau des points de facon a ce que le point superieur gauche soit le premier
// faire une deplacement a ratio a travers les slices.