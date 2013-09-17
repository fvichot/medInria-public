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

#include <vtkBalloonRepresentation.h>
#include <vtkBalloonWidget.h>
#include <vtkPropCollection.h>


bezierCurveToolBox::bezierCurveToolBox(QWidget *parent ) :
    medSegmentationAbstractToolBox( parent)
{
    QWidget *displayWidget = new QWidget(this);
    this->addWidget(displayWidget);

    this->setTitle(this->s_name(this));

    QVBoxLayout * layout = new QVBoxLayout(displayWidget);
   
    addNewCurve = new QPushButton(tr("Add new Curve"),displayWidget);
    addNewCurve->setToolTip(tr("Start a new curve"));
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

    listOfCurves = new QList<vtkSmartPointer<vtkContourWidget> >;
    curveInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
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

    medAbstractView * medView = dynamic_cast<medAbstractView *> (view);
    
    if ( !medView )
        return;

   /* if ((currentView) && (currentView != medView) )
    {
        currentView->disconnect(this,0);
        clear();
    }*/
    
    currentView = medView;

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

void bezierCurveToolBox::onAddNewCurve()
{
    if (!currentView)
        return;
        
    newCurve = true;
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(currentView->getView2D());


    //if (!listOfCurves->isEmpty())
    //{
    //    // Create the widget
    //    vtkSmartPointer<vtkBalloonRepresentation> balloonRep = vtkSmartPointer<vtkBalloonRepresentation>::New();
    //    balloonRep->SetBalloonLayoutToImageRight();
 
    //    vtkSmartPointer<vtkBalloonWidget> balloonWidget =vtkSmartPointer<vtkBalloonWidget>::New();
    //    balloonWidget->SetInteractor(curveInteractor);
    //    balloonWidget->SetRepresentation(balloonRep);
    //    vtkSmartPointer<vtkPropCollection> test = vtkSmartPointer<vtkPropCollection>::New();
    //    listOfCurves->at(0)->GetRepresentation()->GetActors(test);
    //    
    //    balloonWidget->AddBalloon(test->GetNextProp(),"this is a bezier curve"); //,view2d->GetImageInput(0)
    //    balloonWidget->EnabledOn();
    //    balloonWidget->Render();
    //    view2d->SetRenderWindow(/*static_cast<vtkRenderWindow*>*/(view2d->GetRenderWindow()));
    //    view2d->GetRenderWindow()->Render();
    //    curveInteractor->Start();
    //}
        
    vtkSmartPointer<vtkOrientedGlyphContourRepresentation> contourRep = vtkSmartPointer<vtkOrientedGlyphContourRepresentation>::New();
    contourRep->GetLinesProperty()->SetColor(0, 0, 1); //set color to purple
    contourRep->GetLinesProperty()->SetLineWidth(3);//set color to purple
        
    //contourRep->AlwaysOnTopOn();
    vtkSmartPointer<vtkContourWidget> contourWidget = vtkSmartPointer<vtkContourWidget>::New();
        

    /* vtkSmartPointer< vtkSphereSource > ss =
	vtkSmartPointer< vtkSphereSource >::New();
	ss->SetRadius(0.5);*/
        
    curveInteractor->SetRenderWindow(view2d->GetRenderWindow());
    contourWidget->SetInteractor(/*d->view2d->GetInteractor()*/curveInteractor);
    contourWidget->SetRepresentation(contourRep);
    contourWidget->On();

    if (penMode)
        contourWidget->ContinuousDrawOn();
    //contourWidget->AllowNodePickingOn();
    //contourWidget->FollowCursorOn();
    vtkSmartPointer<vtkBoundedPlanePointPlacer> placer = vtkSmartPointer<vtkBoundedPlanePointPlacer>::New();
        
    placer->RemoveAllBoundingPlanes();
    qDebug() << "view2d->GetImageActor()->GetCenter() = " << view2d->GetImageActor()->GetCenter()[0] << " " << view2d->GetImageActor()->GetCenter()[1] << " " << view2d->GetImageActor()->GetCenter()[2] ;
    if (currentView->property("Orientation")=="Axial")
    {
        placer->SetProjectionNormalToZAxis();
        placer->SetProjectionPosition(view2d->GetImageActor()->GetCenter()[2]);
    }
    if (currentView->property("Orientation")=="Coronal")
    {
        placer->SetProjectionNormalToYAxis();
        placer->SetProjectionPosition(view2d->GetImageActor()->GetCenter()[1]);
    }
    if (currentView->property("Orientation")=="Sagittal")
    {
        placer->SetProjectionNormalToXAxis();
        placer->SetProjectionPosition(view2d->GetImageActor()->GetCenter()[0]);
    }

    contourWidget->GetEventTranslator()->SetTranslation(vtkCommand::RightButtonPressEvent,NULL);
    contourRep->SetPointPlacer(placer);

    //contourWidget->EnabledOn();
    //contourWidget->ProcessEventsOn();
  
    contourWidget->Render();
    view2d->SetRenderWindow(/*static_cast<vtkRenderWindow*>*/(view2d->GetRenderWindow()));
    listOfCurves->append(contourWidget);

    curveInteractor->Initialize();
    curveInteractor->Start();
}

void bezierCurveToolBox::generateBinaryImage()
{
    if (listOfCurves->isEmpty())
        return;
    // Generate binary image
    vtkSmartPointer<vtkPolyData> polyData = dynamic_cast<vtkOrientedGlyphContourRepresentation * >(listOfCurves->at(0)->GetRepresentation())->GetContourRepresentationAsPolyData();

    vtkSmartPointer<vtkImageData> whiteImage = vtkSmartPointer<vtkImageData>::New();
    double bounds[6];
    polyData->GetBounds(bounds);
    double spacing[3]; // desired volume spacing
    whiteImage->SetScalarTypeToUnsignedChar();
    whiteImage->AllocateScalars();
    vtkSmartPointer<vtkLinearExtrusionFilter> extruder =
    vtkSmartPointer<vtkLinearExtrusionFilter>::New();
    extruder->SetInput(polyData);
    extruder->SetScaleFactor(1.);
    extruder->SetExtrusionTypeToNormalExtrusion();
    extruder->SetVector(0, 0, 1);
    extruder->Update();
    // polygonal data --> image stencil:l
    vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
    vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    pol2stenc->SetTolerance(0); // important if extruder->SetVector(0, 0, 1) !!!
    pol2stenc->SetInputConnection(extruder->GetOutputPort());
    pol2stenc->SetOutputOrigin( static_cast<vtkImageView2D *>(currentView->getView2D())->GetImageActor()->GetCenter());
    //pol2stenc->SetOutputSpacing(spacing);
    pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
    pol2stenc->Update();
 
    // cut the corresponding white image and set the background:
    vtkSmartPointer<vtkImageStencil> imgstenc =
    vtkSmartPointer<vtkImageStencil>::New(); 
      
    imgstenc->SetInput(whiteImage);
    imgstenc->SetStencil(pol2stenc->GetOutput());
    imgstenc->ReverseStencilOff();
    imgstenc->SetBackgroundValue(0);
    imgstenc->Update();
    vtkSmartPointer<vtkMetaImageWriter> imageWriter =
    vtkSmartPointer<vtkMetaImageWriter>::New();
    imageWriter->SetFileName("labelImage.mhd");
    imageWriter->SetInputConnection(imgstenc->GetOutputPort());
    imageWriter->Write();
}


void bezierCurveToolBox::onPenMode()
{
    penMode = penMode_CheckBox->isChecked();
}
