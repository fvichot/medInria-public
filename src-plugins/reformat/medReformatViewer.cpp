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


medReformatViewer::medReformatViewer(medAbstractView * view,QWidget * parent): medCustomViewContainer(parent)
{
    int * imageDims;
    vtkImageData * vtkViewData;
    vtkImageView2D * view2d;
    if (view)
    {
        view2d = static_cast<medVtkViewBackend*>(view->backend())->view2D;
        vtkViewData = view2d->GetInput();
        imageDims = vtkViewData->GetDimensions();
    }
    else
        return;
    QWidget * widgetbody = new QWidget(this);
    for (int i = 0; i < 3; i++)
    {
        riw[i] = vtkSmartPointer< vtkResliceImageViewer >::New();
        views[i] = new QVTKWidget(widgetbody);
        views[i]->setSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Minimum );
    }
    views[3] = new QVTKWidget(widgetbody);
    views[3]->setSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Minimum );
    QGridLayout * gridLayout = new QGridLayout(this);
    gridLayout->addWidget(views[0],0,0);
    gridLayout->addWidget(views[1],0,1);
    gridLayout->addWidget(views[2],1,0);
    gridLayout->addWidget(views[3],1,1);

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
        riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());

        rep->GetResliceCursorActor()->
            GetCursorAlgorithm()->SetReslicePlaneNormal(i);

        riw[i]->SetInput(vtkViewData); 
        riw[i]->SetSliceOrientation(i);
        riw[i]->SetResliceModeToOblique();
    }

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
        riw[i]->GetRenderer()->SetBackground( color );

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
        riw[i]->SetLookupTable(riw[0]->GetLookupTable());
        planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
        //planeWidget[i]->GetColorMap()->SetInput(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
        planeWidget[i]->SetColorMap(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());

    }

    riw[0]->GetResliceCursorWidget()->ResetResliceCursor();
    riw[1]->GetResliceCursorWidget()->ResetResliceCursor();
    riw[2]->GetResliceCursorWidget()->ResetResliceCursor();
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


void medReformatViewer::resliceMode(int mode)
{
    //this->ui->thickModeCheckBox->setEnabled(mode ? 1 : 0);
    //this->ui->blendModeGroupBox->setEnabled(mode ? 1 : 0);

    for (int i = 0; i < 3; i++)
    {
        riw[i]->SetResliceMode(mode ? 1 : 0);
        riw[i]->GetRenderer()->ResetCamera();
        riw[i]->Render();
    }
}

void medReformatViewer::thickMode(int mode)
{
    for (int i = 0; i < 3; i++)
    {
        riw[i]->SetThickMode(mode ? 1 : 0);
        riw[i]->Render();
    }
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

    // Also sync the Image plane widget on the 3D top left view with any
    // changes to the reslice cursor.
    for (int i = 0; i < 3; i++)
    {
        vtkPlaneSource *ps = static_cast< vtkPlaneSource * >(
            planeWidget[i]->GetPolyDataAlgorithm());
        ps->SetNormal(riw[0]->GetResliceCursor()->GetPlane(i)->GetNormal());
        ps->SetCenter(riw[0]->GetResliceCursor()->GetPlane(i)->GetOrigin());

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

}