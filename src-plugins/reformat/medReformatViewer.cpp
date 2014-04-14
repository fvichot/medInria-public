#include "medReformatViewer.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkResliceImageViewer.h"
#include "vtkResliceCursorLineRepresentation.h"
#include "vtkResliceCursorThickLineRepresentation.h"
#include "vtkResliceCursorWidget.h"
#include "vtkResliceCursorActor.h"
#include "vtkResliceCursorPolyDataAlgorithm.h"
#include "vtkResliceCursor.h"
#include "vtkDICOMImageReader.h"
#include "vtkCellPicker.h"
#include "vtkProperty.h"
#include "vtkPlane.h"
#include "vtkImageData.h"
#include "vtkCommand.h"
#include "vtkPlaneSource.h"
#include "vtkLookupTable.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkInteractorStyleImage.h"
#include "vtkImageSlabReslice.h"
#include "vtkBoundedPlanePointPlacer.h"
//#include "vtkDistanceWidget.h"
//#include "vtkDistanceRepresentation.h"
#include "vtkHandleRepresentation.h"
#include "vtkResliceImageViewerMeasurements.h"
//#include "vtkDistanceRepresentation2D.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkPointHandleRepresentation2D.h"
#include <QVTKWidget.h>
#include <QGridLayout.h>
#include <medVtkViewBackend.h>
#include <vtkImageView2D.h>
#include <medWorkspace.h>
#include <itkVtkImageToImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <dtkCore/dtkAbstractData.h>
#include <medMetaDataKeys.h>
#include <medDataManager.h>
#include <dtkCore/dtkAbstractDataFactory.h>
#include <QDoubleSpinBox>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractView.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkCamera.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageFlip.h>
#include <vtkTransform.h>
#include <itkResampleImageFilter.h>
#include <vtkTransformFilter.h>

class QVTKFrame;
//----------------------------------------------------------------------------
class vtkResliceCursorCallback : public vtkCommand
{
public:
    static vtkResliceCursorCallback *New()
    { return new vtkResliceCursorCallback; }

    void Execute( vtkObject *caller, unsigned long ev,
        void *callData )
    {

        if (ev == vtkResliceCursorWidget::WindowLevelEvent ||
            ev == vtkCommand::WindowLevelEvent ||
            ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
        {
            // Render everything
            for (int i = 0; i < 3; i++)
            {
                this->RCW[i]->Render();
            }
            this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
            return;
        }

        vtkImagePlaneWidget* ipw =
            dynamic_cast< vtkImagePlaneWidget* >( caller );
        if (ipw)
        {
            double* wl = static_cast<double*>( callData );

            if ( ipw == this->IPW[0] )
            {
                this->IPW[1]->SetWindowLevel(wl[0],wl[1],1);
                this->IPW[2]->SetWindowLevel(wl[0],wl[1],1);
            }
            else if( ipw == this->IPW[1] )
            {
                this->IPW[0]->SetWindowLevel(wl[0],wl[1],1);
                this->IPW[2]->SetWindowLevel(wl[0],wl[1],1);
            }
            else if (ipw == this->IPW[2])
            {
                this->IPW[0]->SetWindowLevel(wl[0],wl[1],1);
                this->IPW[1]->SetWindowLevel(wl[0],wl[1],1);
            }
        }

        vtkResliceCursorWidget *rcw = dynamic_cast<
            vtkResliceCursorWidget * >(caller);
        if (rcw)
        {
            vtkResliceCursorLineRepresentation *rep = dynamic_cast<
                vtkResliceCursorLineRepresentation * >(rcw->GetRepresentation());
            // Although the return value is not used, we keep the get calls
            // in case they had side-effects
            rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();

            for (int i = 0; i < 3; i++)
            {
                vtkPlaneSource *ps = static_cast< vtkPlaneSource * >(
                    this->IPW[i]->GetPolyDataAlgorithm());
                ps->SetOrigin(this->RCW[i]->GetResliceCursorRepresentation()->
                    GetPlaneSource()->GetOrigin());
                ps->SetPoint1(this->RCW[i]->GetResliceCursorRepresentation()->
                    GetPlaneSource()->GetPoint1());
                ps->SetPoint2(this->RCW[i]->GetResliceCursorRepresentation()->
                    GetPlaneSource()->GetPoint2());

                // If the reslice plane has modified, update it on the 3D widget
                this->IPW[i]->UpdatePlacement();
            }
        }

        //if (ev == vtkInteractorStyleImage::keypress)

        // Render everything
        for (int i = 0; i < 3; i++)
        {
            this->RCW[i]->Render();
        }
        this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
    }

    vtkResliceCursorCallback() {}
    vtkImagePlaneWidget* IPW[3];
    vtkResliceCursorWidget *RCW[3];
};

/* TODO : 
- The images do not appear in a similar way as our v3dview plugin . Need to check what is the problem ?
- Save image does not seem to work perfectly especially for the thick mode. Need to find the cause ! 
- give the possibility to the user to change slice via keyboard up and down 
*/

medReformatViewer::medReformatViewer(medAbstractView * view,QWidget * parent): medCustomViewContainer(parent)
{
    int * imageDims;
    vtkImageView3D * view3d;
    if (!view)
        return;
       
    _view = view;

    //dtkAbstractData * data = _view->dataInList(0);
    //typedef itk::Image<short,3> ImageType;
    //typedef itk::ImageToVTKImageFilter<ImageType>       ConnectorType;
    //ImageType::Pointer image = dynamic_cast<ImageType*>((itk::Object*)(data->data()));
    ///*ConnectorType::Pointer */converter = ConnectorType::New();
    //converter->SetInput(image);
    //converter->Update();
    //vtkViewData = converter->GetOutput();
    
     // ----- Hack to keep the itkImages info (origin and orientation)
    //vtkMatrix4x4* matrix = vtkMatrix4x4::New();
    //matrix->Identity();
    //for (unsigned int x=0; x<3; x++) {
    //    for (unsigned int y=0; y<3; y++) 
    //    {
    //        matrix->SetElement(x,y,image->GetDirection()[x][y]);
    //    }
    //}

    //itk::ImageBase<3>::PointType origin = image->GetOrigin();
    //double v_origin[4], v_origin2[4];
    //for (int i=0; i<3; i++)
    //    v_origin[i] = origin[i];
    //v_origin[3] = 1.0;
    //matrix->MultiplyPoint (v_origin, v_origin2);
    //for (int i=0; i<3; i++)
    //    matrix->SetElement (i, 3, v_origin[i]-v_origin2[i]);

    //vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
    //t->SetMatrix(matrix);
    
    /*vtkImageReslice * reslicerFlip = vtkImageReslice::New();
    reslicerFlip->SetResliceTransform(t);
    reslicerFlip->SetInterpolationModeToLinear();
    reslicerFlip->SetInput(converter->GetOutput());
    reslicerFlip->Update();
    vtkViewData = reslicerFlip->GetOutput();*/
    
 //------------------------------------------------------
 
    view3d = static_cast<medVtkViewBackend*>(view->backend())->view3D;
    //vtkViewData = view3d->GetInput();
    vtkSmartPointer<vtkImageFlip> flipper = vtkImageFlip::New();
    flipper->SetInput(view3d->GetInput());
    flipper->SetFilteredAxis(1);
    flipper->Update();
    vtkViewData = flipper->GetOutput();

    //vtkViewData = reader->GetOutput();
    imageDims = vtkViewData->GetDimensions();

    QWidget * widgetbody = new QWidget(this);
    for (int i = 0; i < 3; i++)
    {
        riw[i] = vtkSmartPointer< vtkResliceImageViewer >::New();
        frames[i] = new QVTKFrame(widgetbody);
        views[i] = frames[i]->getView();
        views[i]->setSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Minimum );
        if (i==0)
            frames[i]->setStyleSheet("* {border : 1px solid #FF0000;}");
        else if (i==1)
            frames[i]->setStyleSheet("* {border : 1px solid #00FF00;}");
        else if (i==2)
            frames[i]->setStyleSheet("* {border : 1px solid #0000FF;}");
        views[i]->installEventFilter(this);
    }
    frames[3] = new QVTKFrame(widgetbody);
    views[3] = frames[3]->getView();
    views[3]->setSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Minimum );
    QGridLayout * gridLayout = new QGridLayout(this);
    gridLayout->addWidget(frames[2],0,0);
    gridLayout->addWidget(frames[3],0,1);
    gridLayout->addWidget(frames[1],1,0);
    gridLayout->addWidget(frames[0],1,1);

    gridLayout->setColumnStretch ( 0, 0 );
    gridLayout->setColumnStretch ( 1, 0 );
    gridLayout->setRowStretch ( 0, 0 );
    gridLayout->setRowStretch ( 1, 0 );

    widgetbody->setLayout(gridLayout);
    this->layout()->addWidget(widgetbody);

    views[0]->SetRenderWindow(riw[0]->GetRenderWindow()); 
    riw[0]->SetupInteractor(views[0]->GetRenderWindow()->GetInteractor());
    
    
    views[1]->SetRenderWindow(riw[1]->GetRenderWindow());
    riw[1]->SetupInteractor(views[1]->GetRenderWindow()->GetInteractor());

    views[2]->SetRenderWindow(riw[2]->GetRenderWindow());
    riw[2]->SetupInteractor(views[2]->GetRenderWindow()->GetInteractor());

    for (int i = 0; i < 3; i++)
    {
        // make them all share the same reslice cursor object.
        vtkResliceCursorLineRepresentation *rep =
            vtkResliceCursorLineRepresentation::SafeDownCast(
            riw[i]->GetResliceCursorWidget()->GetRepresentation());
        riw[i]->SetResliceCursor(riw[2]->GetResliceCursor());
        
        rep->GetResliceCursorActor()->
            GetCursorAlgorithm()->SetReslicePlaneNormal(i);

        riw[i]->SetInput(vtkViewData); 
        riw[i]->SetSliceOrientation(i);
        //riw[i]->SetResliceModeToAxisAligned();
        riw[i]->SetResliceModeToOblique();
    }

    vtkViewData->GetSpacing(outputSpacing);

    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);

    vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

    vtkSmartPointer< vtkRenderer > ren = vtkSmartPointer< vtkRenderer >::New();

    views[3]->GetRenderWindow()->AddRenderer(ren);
    vtkRenderWindowInteractor *iren = views[3]->GetInteractor();

    for (int i = 0; i < 3; i++)
    {
        planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        planeWidget[i]->SetInteractor( iren );
        planeWidget[i]->SetPicker(picker);
        planeWidget[i]->RestrictPlaneToVolumeOn();
        double color[3] = {0, 0, 0};
        color[i] = 1;
        planeWidget[i]->GetPlaneProperty()->SetColor(color);

        color[0] /= 4.0;
        color[1] /= 4.0;
        color[2] /= 4.0;
        riw[i]->GetRenderer()->SetBackground( 0,0,0 );

        planeWidget[i]->SetTexturePlaneProperty(ipwProp);
        planeWidget[i]->TextureInterpolateOff();
        planeWidget[i]->SetResliceInterpolateToLinear();
        planeWidget[i]->SetInput(vtkViewData);
        planeWidget[i]->SetPlaneOrientation(i);
        planeWidget[i]->SetSliceIndex(imageDims[i]/2);
        planeWidget[i]->DisplayTextOn();
        planeWidget[i]->SetDefaultRenderer(ren);
        planeWidget[i]->SetWindowLevel(1358, -27);
        planeWidget[i]->On();
        planeWidget[i]->InteractionOn();
    }

    vtkSmartPointer<vtkResliceCursorCallback> cbk =
        vtkSmartPointer<vtkResliceCursorCallback>::New();

    for (int i = 0; i < 3; i++)
    {
        cbk->IPW[i] = planeWidget[i];
        cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
        riw[i]->GetResliceCursorWidget()->AddObserver(
            vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk );
        riw[i]->GetResliceCursorWidget()->AddObserver(
            vtkResliceCursorWidget::WindowLevelEvent, cbk );
        riw[i]->GetResliceCursorWidget()->AddObserver(
            vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk );
        riw[i]->GetResliceCursorWidget()->AddObserver(
            vtkResliceCursorWidget::ResetCursorEvent, cbk );
        riw[i]->GetInteractorStyle()->AddObserver(
            vtkCommand::WindowLevelEvent, cbk );

        // Make them all share the same color map.
        riw[i]->SetLookupTable(riw[2]->GetLookupTable());
        planeWidget[i]->GetColorMap()->SetLookupTable(riw[2]->GetLookupTable());
        //planeWidget[i]->GetColorMap()->SetInput(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
        planeWidget[i]->SetColorMap(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());

    }
    //this->ConventionMatrix    = vtkMatrix4x4::New();
    //this->SetViewConvention(0);
    
    riw[0]->GetRenderer()->ResetCamera();
    riw[1]->GetRenderer()->ResetCamera();
    riw[2]->GetRenderer()->ResetCamera();
    //this->SetCameraFromOrientation(riw[0],0);
    //this->SetCameraFromOrientation(riw[1],1);
    //this->SetCameraFromOrientation(riw[2],2);

    views[0]->show();
    views[1]->show();
    views[2]->show();

    // Set up action signals and slots // TODO : Set up all the interaction widget and the connections in the toolbox.
    /*connect(this->ui->resliceModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(resliceMode(int)));
    connect(this->ui->thickModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(thickMode(int)));
    this->ui->thickModeCheckBox->setEnabled(0);

    connect(this->ui->radioButton_Max, SIGNAL(pressed()), this, SLOT(SetBlendModeToMaxIP()));
    connect(this->ui->radioButton_Min, SIGNAL(pressed()), this, SLOT(SetBlendModeToMinIP()));
    connect(this->ui->radioButton_Mean, SIGNAL(pressed()), this, SLOT(SetBlendModeToMeanIP()));
    this->ui->blendModeGroupBox->setEnabled(0);

    connect(this->ui->resetButton, SIGNAL(pressed()), this, SLOT(ResetViews()));*/
    //connect(this->ui->AddDistance1Button, SIGNAL(pressed()), this, SLOT(AddDistanceMeasurementToView1()));
    this->show();


};


//void medReformatViewer::resliceMode(int mode)
//{
//    //this->ui->thickModeCheckBox->setEnabled(mode ? 1 : 0);
//    //this->ui->blendModeGroupBox->setEnabled(mode ? 1 : 0);
//
//    for (int i = 0; i < 3; i++)
//    {
//        riw[i]->SetResliceMode(mode ? 1 : 0);
//        riw[i]->GetRenderer()->ResetCamera();
//        riw[i]->Render();
//    }
//}

void medReformatViewer::thickMode(int val)
{
    for (int i = 0; i < 3; i++)
    {
        //TODO : set the axes to the reslicecursorwidget vtkimageReslice .... this will propably solve the problem of disappearance.
        riw[i]->SetThickMode(val);
        riw[i]->GetRenderer()->ResetCamera();
        riw[i]->Render();
    }
}
void medReformatViewer::blendMode(int val)
{
    if (val)
        SetBlendModeToMinIP();
}

void medReformatViewer::SetBlendMode(int m)
{
    for (int i = 0; i < 3; i++)
    {
        vtkImageSlabReslice *thickSlabReslice = vtkImageSlabReslice::SafeDownCast(
            vtkResliceCursorThickLineRepresentation::SafeDownCast(
            riw[i]->GetResliceCursorWidget()->GetRepresentation())->GetReslice());
        thickSlabReslice->SetBlendMode(m);
        riw[i]->Render();
    }
}

void medReformatViewer::SetBlendModeToMaxIP()
{
    this->SetBlendMode(VTK_IMAGE_SLAB_MAX);
}

void medReformatViewer::SetBlendModeToMinIP()
{
    this->SetBlendMode(VTK_IMAGE_SLAB_MIN);
}

void medReformatViewer::SetBlendModeToMeanIP()
{
    this->SetBlendMode(VTK_IMAGE_SLAB_MEAN);
}

void medReformatViewer::ResetViews()
{
    // Reset the reslice image views
    for (int i = 0; i < 3; i++)
    {
        riw[i]->Reset();
    }

    // Also sync the Image plane widget on the 3D top right view with any
    // changes to the reslice cursor.
    for (int i = 0; i < 3; i++)
    {
        vtkPlaneSource *ps = static_cast< vtkPlaneSource * >(
            planeWidget[i]->GetPolyDataAlgorithm());
        ps->SetNormal(riw[2]->GetResliceCursor()->GetPlane(i)->GetNormal());
        ps->SetCenter(riw[2]->GetResliceCursor()->GetPlane(i)->GetOrigin());

        // If the reslice plane has modified, update it on the 3D widget
        this->planeWidget[i]->UpdatePlacement();
    }

    // Render in response to changes.
    this->Render();
}

void medReformatViewer::Render()
{
    for (int i = 0; i < 3; i++)
    {
        riw[i]->Render();
        views[i]->GetRenderWindow()->Render();
    }
}

//void medReformatViewer::AddDistanceMeasurementToView1()
//{
//    this->AddDistanceMeasurementToView(1);
//}
//
//void medReformatViewer::AddDistanceMeasurementToView(int i)
//{
//    // remove existing widgets.
//    if (this->DistanceWidget[i])
//    {
//        this->DistanceWidget[i]->SetEnabled(0);
//        this->DistanceWidget[i] = NULL;
//    }
//
//    // add new widget
//    this->DistanceWidget[i] = vtkSmartPointer< vtkDistanceWidget >::New();
//    this->DistanceWidget[i]->SetInteractor(
//        this->riw[i]->GetResliceCursorWidget()->GetInteractor());
//
//    // Set a priority higher than our reslice cursor widget
//    this->DistanceWidget[i]->SetPriority(
//        this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);
//
//    vtkSmartPointer< vtkPointHandleRepresentation2D > handleRep =
//        vtkSmartPointer< vtkPointHandleRepresentation2D >::New();
//    vtkSmartPointer< vtkDistanceRepresentation2D > distanceRep =
//        vtkSmartPointer< vtkDistanceRepresentation2D >::New();
//    distanceRep->SetHandleRepresentation(handleRep);
//    this->DistanceWidget[i]->SetRepresentation(distanceRep);
//    distanceRep->InstantiateHandleRepresentation();
//    distanceRep->GetPoint1Representation()->SetPointPlacer(riw[i]->GetPointPlacer());
//    distanceRep->GetPoint2Representation()->SetPointPlacer(riw[i]->GetPointPlacer());
//
//    // Add the distance to the list of widgets whose visibility is managed based
//    // on the reslice plane by the ResliceImageViewerMeasurements class
//    this->riw[i]->GetMeasurements()->AddItem(this->DistanceWidget[i]);
//
//    this->DistanceWidget[i]->CreateDefaultRepresentation();
//    this->DistanceWidget[i]->EnabledOn();
//}

void medReformatViewer::orthogonalAxisModeEnabled(bool)
{
    // For the time being this mode is activated via CTRL (with CMD I suppose on MAC)
}

void medReformatViewer::saveImage()
{
    vtkImageData * output;
    vtkImageData * outputToFlip;
    //typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;

    if (riw[2]->GetThickMode())
    {
        vtkImageSlabReslice *reslicerTop = vtkImageSlabReslice::New();
        vtkImageSlabReslice *thickSlabReslice = vtkImageSlabReslice::SafeDownCast(
            vtkResliceCursorThickLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->GetReslice());
        reslicerTop->SetInput(thickSlabReslice->GetInput());
        reslicerTop->SetResliceAxes(thickSlabReslice->GetResliceAxes());
        reslicerTop->SetSlabThickness(thickSlabReslice->GetSlabThickness());
        reslicerTop->SetBlendMode(thickSlabReslice->GetBlendMode());
        reslicerTop->Update();
        outputToFlip = reslicerTop->GetOutput();
    }
    else
    {
        vtkImageReslice *reslicer0 = vtkImageReslice::SafeDownCast(
            vtkResliceCursorRepresentation::SafeDownCast(riw[0]->GetResliceCursorWidget()->GetRepresentation())->GetReslice());
        vtkImageReslice *reslicer1 = vtkImageReslice::SafeDownCast(
            vtkResliceCursorRepresentation::SafeDownCast(riw[1]->GetResliceCursorWidget()->GetRepresentation())->GetReslice());
        vtkImageReslice *reslicer2 = vtkImageReslice::SafeDownCast(
            vtkResliceCursorRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->GetReslice());


        vtkMatrix4x4 * matrixAxes = vtkMatrix4x4::New();
        matrixAxes->SetElement(0,0,reslicer0->GetResliceAxes()->GetElement(0,0));
        matrixAxes->SetElement(1,0,reslicer0->GetResliceAxes()->GetElement(1,0));
        matrixAxes->SetElement(2,0,reslicer0->GetResliceAxes()->GetElement(2,0));
        matrixAxes->SetElement(3,0,reslicer0->GetResliceAxes()->GetElement(3,0));

        matrixAxes->SetElement(0,1,reslicer1->GetResliceAxes()->GetElement(0,1));
        matrixAxes->SetElement(1,1,reslicer1->GetResliceAxes()->GetElement(1,1));
        matrixAxes->SetElement(2,1,reslicer1->GetResliceAxes()->GetElement(2,1));
        matrixAxes->SetElement(3,1,reslicer1->GetResliceAxes()->GetElement(3,1));

        matrixAxes->SetElement(0,2,reslicer2->GetResliceAxes()->GetElement(0,2));
        matrixAxes->SetElement(1,2,reslicer2->GetResliceAxes()->GetElement(1,2));
        matrixAxes->SetElement(2,2,reslicer2->GetResliceAxes()->GetElement(2,2));
        matrixAxes->SetElement(3,2,reslicer2->GetResliceAxes()->GetElement(3,2));

        matrixAxes->SetElement(0,3,reslicer2->GetResliceAxes()->GetElement(0,3));
        matrixAxes->SetElement(1,3,reslicer2->GetResliceAxes()->GetElement(1,3));
        matrixAxes->SetElement(2,3,reslicer2->GetResliceAxes()->GetElement(2,3));
        matrixAxes->SetElement(3,3,reslicer2->GetResliceAxes()->GetElement(3,3));

        for (int i = 0;i<4;i++)
        {
            qDebug() << "reslicer0 " << i ;
            QString lignei="";
            
            for(int j = 0;j<4;j++)
                lignei+= " " + QString::number(reslicer0->GetResliceAxes()->GetElement(i,j));
            qDebug () << lignei;
        }

        for (int i = 0;i<4;i++)
        {
            qDebug() << "reslicer1 " << i ;
            QString lignei="";
            for(int j = 0;j<4;j++)
                lignei+=  " " + QString::number(reslicer1->GetResliceAxes()->GetElement(i,j));
            qDebug () << lignei;
        }
        
        for (int i = 0;i<4;i++)
        {
            qDebug() << "reslicer2 " << i ;
            QString lignei="";
            for(int j = 0;j<4;j++)
                lignei+= " " + QString::number(reslicer2->GetResliceAxes()->GetElement(i,j));
            qDebug () << lignei;
        }

        for (int i = 0;i<4;i++)
        {
            qDebug() << "matrixAxes " << i ;
            QString lignei="";
            for(int j = 0;j<4;j++)
                lignei+= " " + QString::number(matrixAxes->GetElement(i,j));
            qDebug () << lignei;
        }

        vtkImageReslice *reslicerTop0 = vtkImageReslice::New();
        vtkImageReslice *reslicerTop1 = vtkImageReslice::New();
        vtkImageReslice *reslicerTop2 = vtkImageReslice::New();

        reslicerTop0->SetInput(riw[2]->GetInput());
        reslicerTop0->SetResliceAxes(reslicer0->GetResliceAxes());
        reslicerTop0->SetBackgroundLevel(riw[2]->GetInput()->GetScalarRange()[0]); // todo: set the background value in an automatic way.
        reslicerTop0->SetOutputSpacing(outputSpacing);
        reslicerTop0->SetInterpolationModeToLinear();
        reslicerTop0->Update();

        reslicerTop1->SetInput(reslicerTop0->GetOutput());
        reslicerTop1->SetResliceAxes(reslicer1->GetResliceAxes());
        reslicerTop1->SetBackgroundLevel(riw[2]->GetInput()->GetScalarRange()[0]); // todo: set the background value in an automatic way.
        reslicerTop1->SetOutputSpacing(outputSpacing);
        reslicerTop1->SetInterpolationModeToLinear();
        reslicerTop1->Update();
        
        reslicerTop2->SetInput(reslicerTop1->GetOutput());
        reslicerTop2->SetResliceAxes(reslicer2->GetResliceAxes());
        reslicerTop2->SetBackgroundLevel(riw[2]->GetInput()->GetScalarRange()[0]); // todo: set the background value in an automatic way.
        reslicerTop2->SetOutputSpacing(outputSpacing);
        reslicerTop2->SetInterpolationModeToLinear();
        reslicerTop2->Update();
        outputToFlip = reslicerTop2->GetOutput();
    }
    
    vtkSmartPointer<vtkImageFlip> flipper = vtkImageFlip::New();
    flipper->SetInput(outputToFlip);
    flipper->SetFilteredAxis(1);
    flipper->Update();
    output = flipper->GetOutput();

    //output->setaxis
    // TODO : the output of the reslice need to receive all the info from the original image : spacing origin ...
    dtkSmartPointer<dtkAbstractData> outputData(NULL);
    qDebug () << "Type of the image " <<  output->GetScalarTypeAsString() << " "  << output->GetScalarType();

    switch (output->GetScalarType())
    {
    case VTK_CHAR:
        {
            typedef itk::Image<char,3> ImageType;
            typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
            VTKImageToImageType::Pointer filter = VTKImageToImageType::New();
            filter->SetInput(output);
            filter->Update();
            ImageType::Pointer image = filter->GetOutput();
            outputData = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageChar3");
            outputData->setData(image);
            break;
        }

    case VTK_UNSIGNED_CHAR:
        {
            typedef itk::Image<unsigned char,3> ImageType;
            typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
            VTKImageToImageType::Pointer filter = VTKImageToImageType::New();
            filter->SetInput(output);
            filter->Update();
            ImageType::Pointer image = filter->GetOutput();
            outputData = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageUChar3");
            outputData->setData(image);
            break;
        }

    case VTK_SHORT:
        {
            typedef itk::Image<short,3> ImageType;
            typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
            VTKImageToImageType::Pointer filter = VTKImageToImageType::New();
            filter->SetInput(output);
            filter->Update();
            ImageType::Pointer image = filter->GetOutput();
            outputData = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageShort3");
            outputData->setData(image);
            break;
        }

    case VTK_UNSIGNED_SHORT:
        {
            typedef itk::Image<unsigned short,3> ImageType;
            typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
            VTKImageToImageType::Pointer filter = VTKImageToImageType::New();
            filter->SetInput(output);
            filter->Update();
            ImageType::Pointer image = filter->GetOutput();
            outputData = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageUShort3");
            outputData->setData(image);
            break;
        }

    case VTK_INT:
        {
            typedef itk::Image<int,3> ImageType;
            typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
            VTKImageToImageType::Pointer filter = VTKImageToImageType::New();
            filter->SetInput(output);
            filter->Update();
            ImageType::Pointer image = filter->GetOutput();
            outputData = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageInt3");
            outputData->setData(image);
            break;
        }
    case VTK_UNSIGNED_INT:
        {
            typedef itk::Image<unsigned int,3> ImageType;
            typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
            VTKImageToImageType::Pointer filter = VTKImageToImageType::New();
            filter->SetInput(output);
            filter->Update();
            ImageType::Pointer image = filter->GetOutput();
            outputData = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageUInt3");
            outputData->setData(image);
            break;
        }
    case VTK_LONG:
        {
            typedef itk::Image<long,3> ImageType;
            typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
            VTKImageToImageType::Pointer filter = VTKImageToImageType::New();
            filter->SetInput(output);
            filter->Update();
            ImageType::Pointer image = filter->GetOutput();
            outputData = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageLong3");
            outputData->setData(image);
            break;
        }
    case VTK_UNSIGNED_LONG:
        {
            typedef itk::Image<unsigned long,3> ImageType;
            typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
            VTKImageToImageType::Pointer filter = VTKImageToImageType::New();
            filter->SetInput(output);
            filter->Update();
            ImageType::Pointer image = filter->GetOutput();
            outputData = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageULong3");
            outputData->setData(image);
            break; 
        }
    case VTK_FLOAT:
        {
            typedef itk::Image<float,3> ImageType;
            typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
            VTKImageToImageType::Pointer filter = VTKImageToImageType::New();
            filter->SetInput(output);
            filter->Update();
            ImageType::Pointer image = filter->GetOutput();
            outputData = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageFloat3");
            outputData->setData(image);
            break;
        }
    case VTK_DOUBLE:
        {
            typedef itk::Image<double,3> ImageType;
            typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;
            VTKImageToImageType::Pointer filter = VTKImageToImageType::New();
            filter->SetInput(output);
            filter->Update();
            ImageType::Pointer image = filter->GetOutput();
            outputData = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageDouble3");
            outputData->setData(image);
            break;
        }
    }

    if (outputData && outputData->data())
    {
        dtkAbstractData *currentData = reinterpret_cast< dtkAbstractData * >( _view->data() );

        foreach(QString metaData, currentData->metaDataList())
            outputData->addMetaData(metaData,currentData->metaDataValues(metaData));

        foreach(QString property,currentData->propertyList())
            outputData->addProperty(property,currentData->propertyValues(property));
        QString newDescription = "reformated";
        outputData->setMetaData(medMetaDataKeys::SeriesDescription.key(), newDescription);

        //QString generatedID = QUuid::createUuid().toString().replace("{","").replace("}","");
        //outputData->setMetaData ( medMetaDataKeys::SeriesID.key(), generatedID );

        medDataManager::instance()->importNonPersistent(outputData);
    }
}

void medReformatViewer::thickSlabChanged(double val)
{
    QDoubleSpinBox * spinBoxSender = qobject_cast<QDoubleSpinBox*>(QObject::sender());
    //if (!riw[0]->GetThickMode())
    //    return;

    if (spinBoxSender)
    {
        double x,y,z;
        riw[2]->GetResliceCursor()->GetThickness(x,y,z);
        if (spinBoxSender->accessibleName()=="SpacingX")
        {
            riw[0]->GetResliceCursor()->SetThickness(val,y,z);
            riw[1]->GetResliceCursor()->SetThickness(val,y,z);
            riw[2]->GetResliceCursor()->SetThickness(val,y,z);
            outputSpacing[0]=val;
        }

        if (spinBoxSender->accessibleName()=="SpacingY")
        {
            riw[0]->GetResliceCursor()->SetThickness(x,val,z);
            riw[1]->GetResliceCursor()->SetThickness(x,val,z);
            riw[2]->GetResliceCursor()->SetThickness(x,val,z);
            outputSpacing[1]=val;
        }

        if (spinBoxSender->accessibleName()=="SpacingZ")
        {
            riw[0]->GetResliceCursor()->SetThickness(x,y,val);
            riw[1]->GetResliceCursor()->SetThickness(x,y,val);
            riw[2]->GetResliceCursor()->SetThickness(x,y,val);
            outputSpacing[2]=val;
        }
        this->Render();
    }
}

bool medReformatViewer::eventFilter(QObject * object,QEvent * event)
{
    if (!qobject_cast<QVTKWidget*>(object))
        return true;

    if (event->type()==QEvent::FocusIn)
    {
        for(int i = 0;i<3;i++)
        {
            if (views[i]==object)
            {
                if (i==0)
                    frames[i]->setStyleSheet("QFrame {border : 5px solid #FF0000;}");
                else if (i==1)
                    frames[i]->setStyleSheet("QFrame {border : 5px solid #00FF00;}");
                else if (i==2)
                    frames[i]->setStyleSheet("QFrame {border : 5px solid #0000FF;}");

                if (selectedView==0)
                    frames[0]->setStyleSheet("* {border : 1px solid #FF0000;}");
                else if (selectedView==1)
                    frames[1]->setStyleSheet("* {border : 1px solid #00FF00;}");
                else if (selectedView==2)
                    frames[2]->setStyleSheet("* {border : 1px solid #0000FF;}");

                selectedView = i;
            }
        }
        return false;
    }
    if (event->type()==QEvent::FocusOut)
    {
        return false;
    }
    return false;
}

// TODO : redefine the vtkInteractorStyleImage to control the image the way u want

int medReformatViewer::SetCameraFromOrientation(vtkSmartPointer<vtkResliceImageViewer> riw,int SliceOrientation)
{
    vtkImageView * view2D = static_cast<medVtkViewBackend*>(_view->backend())->view2D;
    
    // We entirely rely on the slice orientation this->SliceOrientation
    // The ViewOrientation is "estimated", returned as id

    vtkCamera *cam = riw->GetRenderer() ? riw->GetRenderer()->GetActiveCamera() : NULL;
  if (!cam)
    return -1;

  double position[4] = {0,0,0,0}, focalpoint[4] = {0,0,0,0}, viewup[4] = {0,0,0,0};
  double focaltoposition[3]={0,0,0};
  std::vector<double*> viewupchoices;
  double first[3]={0,0,0}, second[3]={0,0,0}, third[3]={0,0,0}, fourth[3]={0,0,0};
  bool inverseposition = false;

  // The viewup and the cam position are set according to the convention matrix.
  for (unsigned int i=0; i<3; i++)
    viewup[i]   = this->ConventionMatrix->GetElement (i, SliceOrientation);

  // Apply the orientation matrix to all this information
  if ( view2D->GetOrientationMatrix())
    view2D->GetOrientationMatrix()->MultiplyPoint  (viewup, viewup);

  // first we find the axis the closest to the focaltoposition vector
  unsigned int id = SliceOrientation;//view2D->GetViewOrientationFromSliceOrientation (SliceOrientation, position, focalpoint);

  // Compute the vector normal to the view
  for (unsigned int i=0; i<3; i++)
    focaltoposition[i] = position[i] - focalpoint[i];

  // Now we now we have 4 choices for the View-Up information
  for(unsigned int i=0; i<3; i++)
  {
    first[i]  = viewup[i];
    second[i] = -viewup[i];
  }

  vtkMath::Cross (first, focaltoposition, third);
  vtkMath::Cross (second, focaltoposition, fourth);
  vtkMath::Normalize (third);
  vtkMath::Normalize (fourth);

  viewupchoices.push_back (first);
  viewupchoices.push_back (second);
  viewupchoices.push_back (third);
  viewupchoices.push_back (fourth);

  double conventionviewup[4]={0,0,0,0};
  // Then we choose the convention matrix vector correspondent to the
  // one we just found
  for (unsigned int i=0; i<3; i++)
    conventionviewup[i] = this->ConventionMatrix->GetElement (i, id);

  // Then we pick from the 4 solutions the closest to the
  // vector just found
  unsigned int id2 = 0;
  double dot2 = 0;
  for (unsigned int i=0; i<viewupchoices.size(); i++)
    if (dot2 < vtkMath::Dot (viewupchoices[i], conventionviewup))
    {
      dot2 = vtkMath::Dot (viewupchoices[i], conventionviewup);
      id2 = i;
    }

  // We found the solution
  for (unsigned int i=0; i<3; i++)
    viewup[i] = viewupchoices[id2][i];

  double conventionposition[4]={0,0,0,1};
  // Then we check if we are on the right side of the image
  conventionposition[id] = this->ConventionMatrix->GetElement (id, 3);
  inverseposition = (vtkMath::Dot (focaltoposition, conventionposition) < 0 );

  // invert the cam position if necessary (symmetry along the focal point)
  if (inverseposition)
    for (unsigned int i=0; i<3; i++)
      position[i] -= 2*focaltoposition[i];

  // finally set the camera with all iinformation.

  cam->SetPosition(position[0], position[1], position[2]);
  cam->SetFocalPoint(focalpoint[0], focalpoint[1], focalpoint[2]);
  cam->SetViewUp(viewup[0], viewup[1], viewup[2]);

  // return the view-orientation found by those principles.
  // this should then be set as this->ViewOrientation.
  return id;
}

void medReformatViewer::SetViewConvention(int convention)
{
  if ( (convention < vtkImageView2D::VIEW_CONVENTION_RADIOLOGICAL))
    return;

  //this->ViewConvention = convention;
  // first vector is the view up for the yz plane (sagittal), directed towards the +z direction
  this->ConventionMatrix->SetElement (2,0, 1);
  // first vector is the view up for the xz plane (coronal),  directed towards the +z direction
  this->ConventionMatrix->SetElement (2,1, 1);
  // first vector is the view up for the xy plane (axial),    directed towards the -y direction
  this->ConventionMatrix->SetElement (1,2, -1);

  int x_watcher, y_watcher, z_watcher;

  switch(convention)
  {
    case vtkImageView2D::VIEW_CONVENTION_RADIOLOGICAL:
    default:
      // for sagittal view, you look from +x point
      x_watcher =  1;
      // for coronal view,  you look from -y point
      y_watcher = -1;
      // for axial view,    you look from -z point
      z_watcher = -1;
      break;
    case vtkImageView2D::VIEW_CONVENTION_NEUROLOGICAL:
      // for sagittal view, you look from +x point
      x_watcher =  1;
      // for coronal view,  you look from +y point
      y_watcher =  1;
      // for axial view,    you look from +z point
      z_watcher =  1;
      break;
      /// \todo We can define oblique convention points of view
      /// that would match cardiologic conventions.
      /// i.e. - A cardiologist looks a heart from short axis (R.V in the righ of plane)
      /// i.e. - A cardiologist looks a heart from 4 chamber  (R.V in the left of plane)
      /// ...
  }
  this->ConventionMatrix->SetElement (0,3, x_watcher);
  this->ConventionMatrix->SetElement (1,3, y_watcher);
  this->ConventionMatrix->SetElement (2,3, z_watcher);

  //this->UpdateOrientation();
}