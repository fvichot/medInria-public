/*=========================================================================

medInria

Copyright (c) INRIA 2013. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.

=========================================================================*/

#include <medVtkViewNavigator.h>

#include <vtkImageView2D.h>
#include <vtkImageView3D.h>
#include <vtkImageViewCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

#include <QVTKWidget2.h>

#include <medVtkViewBackend.h>
#include <medImageViewFactory.h>
#include <medBoolGroupParameter.h>
#include <medBoolParameter.h>
#include <medDoubleParameter.h>
#include <medVector2DParameter.h>
#include <medVector3DParameter.h>
#include <medDoubleParameter.h>
#include <medCompositeParameter.h>

class medVtkViewNavigatorPrivate
{
    public:
    medAbstractImageView *medVtkView;
    vtkRenderWindow *render;
    QVTKWidget2 *qvtkWidget;
    vtkImageView2D *view2d;
    vtkImageView3D *view3d;
    vtkImageView *currentView;

    vtkRenderer *renderer2d;
    vtkRenderer *renderer3d;
    vtkRenderWindow *renWin;
    vtkImageViewCollection *collection;

    medImageView::Orientation orientation;

    QWidget *widgetForToolBox;
    QWidget *widgetForToolBar;

    medBoolGroupParameter *orientationParameter;
    medBoolParameter *oAxialParameter;
    medBoolParameter *oSagittalParameter;
    medBoolParameter *oCoronalParameter;
    medBoolParameter *o3dParameter;

    medDoubleParameter *zoomParameter;

    medVector2DParameter *panParameter;

    medCompositeParameter *cameraParameter;

    medBoolParameter *showAxesParameter;
    medBoolParameter *showRulerParameter;
    medBoolParameter *showAnnotationParameter;
    medBoolParameter *showScalarBarParameter;
    QWidget* showOptionsWidget;

};

medVtkViewNavigator::medVtkViewNavigator(medAbstractImageView* parent) :
    medAbstractImageViewNavigator(parent), d(new medVtkViewNavigatorPrivate)
{
    medVtkViewBackend* backend = static_cast<medVtkViewBackend*>(parent->backend());
    d->view2d = backend->view2D;
    d->view3d = backend->view3D;
    d->renWin = backend->renWin;

    d->currentView = NULL;
    d->showOptionsWidget = NULL;

    d->renderer2d = d->view2d->GetRenderer();
    d->renderer3d = d->view3d->GetRenderer();

    d->collection = vtkImageViewCollection::New();
    d->collection->AddItem(d->view2d);
    d->collection->AddItem(d->view3d);
    d->render = backend->renWin;


    d->orientationParameter = new medBoolGroupParameter("Orientation", this);
    d->orientationParameter->setPushButtonDirection(QBoxLayout::LeftToRight);
    d->orientationParameter->getLabel()->hide();


    d->oAxialParameter = new medBoolParameter("axial", this);
    d->oAxialParameter->setIcon(QIcon(":/icons/AxialIcon.png"));
    d->oAxialParameter->setIconSize(QSize(64,64));
    connect(d->oAxialParameter, SIGNAL(valueChanged(bool)),
            this, SLOT(setAxial(bool)));

    d->oCoronalParameter = new medBoolParameter("coronal", this);
    d->oCoronalParameter->setIcon(QIcon(":/icons/CoronalIcon.png"));
    d->oCoronalParameter->setIconSize(QSize(64,64));
    connect(d->oCoronalParameter, SIGNAL(valueChanged(bool)),
            this, SLOT(setCoronal(bool)));

    d->oSagittalParameter = new medBoolParameter("sagittal", this);
    d->oSagittalParameter->setIcon(QIcon(":/icons/SagittalIcon.png"));
    d->oSagittalParameter->setIconSize(QSize(64,64));
    connect(d->oSagittalParameter, SIGNAL(valueChanged(bool)),
            this, SLOT(setSagittal(bool)));

    d->o3dParameter = new medBoolParameter("3d", this);
    d->o3dParameter->setIcon(QIcon(":/icons/3DIcon.png"));
    d->o3dParameter->setIconSize(QSize(64,64));
    connect(d->o3dParameter, SIGNAL(valueChanged(bool)),
            this, SLOT(set3d(bool)));

    d->orientationParameter->addParameter(d->oAxialParameter);
    d->orientationParameter->addParameter(d->oCoronalParameter);
    d->orientationParameter->addParameter(d->oSagittalParameter);
    d->orientationParameter->addParameter(d->o3dParameter);
    d->oAxialParameter->setValue(true);

    d->zoomParameter = new medDoubleParameter("zoom", this);
    connect(d->zoomParameter, SIGNAL(valueChanged(double)),this, SLOT(setZoom(double)));
    connect(parent, SIGNAL(zoomChanged(double)), d->zoomParameter, SLOT(setValue(double)));
    d->zoomParameter->setValue(1);

    d->panParameter = new medVector2DParameter("pan", this);
    connect(d->panParameter, SIGNAL(valueChanged(QVector2D)),this, SLOT(setPan(QVector2D)));
    connect(parent, SIGNAL(panChanged(QVector2D)), d->panParameter, SLOT(setValue(QVector2D)));

    d->cameraParameter = new medCompositeParameter("Camera", this);
    d->cameraParameter->addVariant("Camera Position", QVariant(QVector3D()));
    d->cameraParameter->addVariant("Camera Up", QVariant(QVector3D()));
    d->cameraParameter->addVariant("Camera Focal", QVariant(QVector3D()));
    d->cameraParameter->addVariant("Parallel Scale", QVariant((double)0.0));

    connect(d->cameraParameter, SIGNAL(valueChanged(QList<QVariant>)), this, SLOT(setCamera(QList<QVariant>)));
    connect(parent, SIGNAL(cameraChanged(QVector3D,QVector3D,QVector3D,double)),
            this,SLOT(updateCameraParam(QVector3D,QVector3D,QVector3D,double)));

    d->showAxesParameter = new medBoolParameter("Axes", this);
    d->showRulerParameter = new medBoolParameter("Ruler", this);
    d->showAnnotationParameter = new medBoolParameter("Annotations", this);
    d->showScalarBarParameter = new medBoolParameter("Scalar Bar", this);

    d->showAxesParameter->setText("Axes");
    d->showRulerParameter->setText("Ruler");
    d->showAnnotationParameter->setText("Annotations");
    d->showScalarBarParameter->setText("Scalar Bar");

    connect(d->showAxesParameter, SIGNAL(valueChanged(bool)), this, SLOT(showAxes(bool)));
    connect(d->showRulerParameter, SIGNAL(valueChanged(bool)), this, SLOT(showRuler(bool)));
    connect(d->showAnnotationParameter, SIGNAL(valueChanged(bool)), this, SLOT(showAnnotations(bool)));
    connect(d->showScalarBarParameter, SIGNAL(valueChanged(bool)), this, SLOT(showScalarBar(bool)));

    d->showAxesParameter->setValue(false);
    d->showRulerParameter->setValue(true);
    d->showAnnotationParameter->setValue(true);
    d->showScalarBarParameter->setValue(false);

    d->widgetForToolBar = NULL;
    d->widgetForToolBox = new QWidget();

    d->showOptionsWidget = new QWidget;
    QHBoxLayout* showOptionsLayout = new QHBoxLayout(d->showOptionsWidget);
    showOptionsLayout->addWidget(d->showAxesParameter->getCheckBox());
    showOptionsLayout->addWidget(d->showRulerParameter->getCheckBox());
    showOptionsLayout->addWidget(d->showAnnotationParameter->getCheckBox());
    showOptionsLayout->addWidget(d->showScalarBarParameter->getCheckBox());

    QVBoxLayout* layout = new QVBoxLayout(d->widgetForToolBox);
    layout->addWidget(d->orientationParameter->getLabel());
    layout->addWidget(d->orientationParameter->getPushButtonGroup());
    layout->addWidget(d->showOptionsWidget);


    //TODO GPR-RDE: better solution?
    connect(this, SIGNAL(orientationChanged()), parent, SIGNAL(orientationChanged()));
}

medVtkViewNavigator::~medVtkViewNavigator()
{
    delete d;
}

QList<medAbstractParameter*> medVtkViewNavigator::parameters()
{
    QList<medAbstractParameter*> params;
    params.append(d->orientationParameter);
    params.append(d->zoomParameter);
    params.append(d->panParameter);
    params.append(d->cameraParameter);
    params.append(d->showAxesParameter);
    params.append(d->showRulerParameter);
    params.append(d->showAnnotationParameter);
    params.append(d->showRulerParameter);

    return params;
}

QString  medVtkViewNavigator::identifier() const
{
    return medVtkViewNavigator::s_identifier();
}

QString  medVtkViewNavigator::s_identifier()
{
    return "medVtkViewNavigator";
}

QStringList medVtkViewNavigator::handled(void) const
{
    return QStringList() << "medVtkView";
}

bool medVtkViewNavigator::registered()
{
    medImageViewFactory * factory = medImageViewFactory::instance();
    return factory->registerNavigator<medVtkViewNavigator>(medVtkViewNavigator::s_identifier(),
                                                           QStringList() << "medVtkView");
}

QString medVtkViewNavigator::description() const
{
    return "Navigator to interact with a medVtkView";
}

QWidget* medVtkViewNavigator::widgetForToolBox() const
{
    return d->widgetForToolBox;
}

QWidget* medVtkViewNavigator::widgetForToolBar() const
{
    return d->widgetForToolBar;
}

QVector3D medVtkViewNavigator::positionBeingViewed() const
{
    return QVector3D(0.0,0.0,0.0);
}

double medVtkViewNavigator::zoom() const
{
    if(d->medVtkView->is2D())
        return d->view2d->GetZoom();
    else
        return d->view3d->GetZoom();
}

void medVtkViewNavigator::setZoom(double zoom)
{
    d->currentView->SetZoom(zoom);
    d->currentView->Render();
}

QVector2D medVtkViewNavigator::pan() const
{
    const double* pan = d->view2d->GetPan();

    QVector2D qpan(pan[0], pan[1]);
    return qpan;
}

void medVtkViewNavigator::setPan(const QVector2D &pan)
{
    double stdpan[2] = {pan.x(), pan.y()};
    d->view2d->SetPan(stdpan);
    d->view2d->Render();
}

void medVtkViewNavigator::camera(QVector3D &position,
                    QVector3D &viewup,
                    QVector3D &focal,
                    double &parallelScale) const
{
    double p[3];
    this->cameraPosition(p);
    position = QVector3D(p[1], p[2], p[3]);

    double v[3];
    this->cameraUp(v);
    viewup = QVector3D(v[1], v[2], v[3]);

    double f[3];
    this->cameraFocalPoint(f);
    focal = QVector3D(f[1], f[2], f[3]);

    this->cameraParallelScale(parallelScale);
}

void medVtkViewNavigator::setCamera(const QVector3D &position,
                       const QVector3D &viewup,
                       const QVector3D &focal,
                       double parallelScale)
{
    setCameraPosition(position);
    setCameraUp(viewup);
    setCameraFocalPoint(focal);
    setCameraParallelScale(parallelScale);
    d->currentView->Render();
}

void medVtkViewNavigator::setCamera(QList<QVariant> cameraOptions)
{
    if(cameraOptions.count() != 4)
    {
        qWarning() << "Camera options are incorrect.";
        return;
    }

    QVector3D cameraPostion(cameraOptions.at(0).value<QVector3D>());
    QVector3D cameraUp(cameraOptions.at(1).value<QVector3D>());
    QVector3D cameraFocalPoint(cameraOptions.at(2).value<QVector3D>());

    double parallelScale = cameraOptions.at(3).toReal();

    setCameraPosition(cameraPostion);
    setCameraUp(cameraUp);
    setCameraFocalPoint(cameraFocalPoint);
    setCameraParallelScale(parallelScale);

    d->currentView->Render();
}

void medVtkViewNavigator::cameraUp (double *coordinates) const
{
    if (d->orientation != medImageView::VIEW_ORIENTATION_3D)
    {
        vtkCamera *camera = d->renderer2d->GetActiveCamera();
        camera->GetViewUp (coordinates);
    }
    else
    {
        vtkCamera *camera = d->renderer3d->GetActiveCamera();
        camera->GetViewUp (coordinates);
    }
}

void medVtkViewNavigator::setCameraUp(const QVector3D& viewup)
{
    double vup[3];

    vup[0] = viewup.x();
    vup[1] = viewup.y();
    vup[2] = viewup.z();

    d->view3d->GetInteractorStyle()->HandleObserversOff();
    d->renderer3d->GetActiveCamera()->SetViewUp(vup);
    d->view3d->GetInteractorStyle()->HandleObserversOn();
}

void medVtkViewNavigator::cameraPosition(double *coordinates) const
{
    if ( d->orientation != medImageView::VIEW_ORIENTATION_3D )
    {
        vtkCamera *camera = d->renderer2d->GetActiveCamera();
        camera->GetPosition(coordinates);
    }
    else
    {
        vtkCamera *camera = d->renderer3d->GetActiveCamera();
        camera->GetPosition(coordinates);
    }
}

void medVtkViewNavigator::setCameraPosition(double x, double y, double z)
{
    if (d->orientation != medImageView::VIEW_ORIENTATION_3D)
        return;

    d->view3d->GetInteractorStyle()->HandleObserversOff();
    vtkCamera *camera = d->renderer3d->GetActiveCamera();
    camera->SetPosition (x, y, z);
    d->renderer3d->ResetCameraClippingRange();
    d->view3d->GetInteractorStyle()->HandleObserversOn();
}

void medVtkViewNavigator::setCameraPosition(const QVector3D& position)
{
    setCameraPosition(position.x(), position.y(), position.z());
}


void medVtkViewNavigator::cameraFocalPoint(double *coordinates ) const
{
    if ( d->orientation != medImageView::VIEW_ORIENTATION_3D )
    {
        vtkCamera *camera = d->renderer2d->GetActiveCamera();
        camera->GetFocalPoint(coordinates);
    }
    else
    {
        vtkCamera *camera = d->renderer3d->GetActiveCamera();
        camera->GetFocalPoint(coordinates);
    }
}

void medVtkViewNavigator::setCameraFocalPoint(const QVector3D& focal)
{
    double foc[3];

    foc[0] = focal.x();
    foc[1] = focal.y();
    foc[2] = focal.z();

    d->view3d->GetInteractorStyle()->HandleObserversOff();
    d->renderer3d->GetActiveCamera()->SetFocalPoint(foc);
    d->view3d->GetInteractorStyle()->HandleObserversOn();
}

void medVtkViewNavigator::cameraParallelScale(double &parallelScale) const
{
    parallelScale = d->renderer3d->GetActiveCamera()->GetParallelScale();
}

void medVtkViewNavigator::setCameraParallelScale(double parallelScale)
{
    d->view3d->GetInteractorStyle()->HandleObserversOff();
    d->renderer3d->GetActiveCamera()->SetParallelScale(parallelScale);
    d->view3d->GetInteractorStyle()->HandleObserversOn();
}

void medVtkViewNavigator::setCameraClippingRange(double nearRange, double farRange)
{
    if (d->orientation != medImageView::VIEW_ORIENTATION_3D)
        return;

    vtkCamera *camera = d->renderer3d->GetActiveCamera();
    camera->SetClippingRange(nearRange, farRange);
}

//TODO is that usefull ?
QString medVtkViewNavigator::cameraProjectionMode()
{
    vtkCamera *camera = NULL;

    if (d->orientation != medImageView::VIEW_ORIENTATION_3D)
        camera = d->renderer2d->GetActiveCamera();
    else
        camera = d->renderer3d->GetActiveCamera();

    if ( !camera )
        return QString("None");

    if (camera->GetParallelProjection())
        return QString ("Parallel");
    else
        return QString ("Perspective");
}

double medVtkViewNavigator::cameraViewAngle()
{
    vtkCamera *camera = NULL;

    if (d->orientation != medImageView::VIEW_ORIENTATION_3D)
        camera = d->renderer2d->GetActiveCamera();
    else
        camera = d->renderer3d->GetActiveCamera();

    if ( !camera )
        return 0.0;
    else
        return camera->GetViewAngle();
}

double medVtkViewNavigator::cameraZoom()
{
    //TODO it's exactly the same as zoom() - RDE
    vtkImageView *view = NULL;

    if (d->orientation != medImageView::VIEW_ORIENTATION_3D)
        view = d->view2d;

    else  view = d->view3d;

    if ( !view )
        return 1.0;
    else
        return view->GetZoom();
}

void medVtkViewNavigator::updateCameraParam(const QVector3D& position,const QVector3D& viewUp,const QVector3D& focal,double parallelScale)
{
    QList<QVariant> options;
    options.append( QVariant::fromValue(position) );
    options.append( QVariant::fromValue(viewUp) );
    options.append( QVariant::fromValue(focal) );
    options.append( parallelScale );

    d->cameraParameter->setValue(options);
}

void medVtkViewNavigator::bounds(float& xmin, float& xmax, float& ymin, float& ymax, float& zmin, float& zmax) const
{
    double bounds[6];
    switch(d->orientation)
    {
    case medImageView::VIEW_ORIENTATION_3D:
        d->renderer3d->ComputeVisiblePropBounds(bounds);
        xmin = bounds[0];
        xmax = bounds[1];
        ymin = bounds[2];
        ymax = bounds[3];
        zmin = bounds[4];
        zmax = bounds[5];
        break;
    case medImageView::VIEW_ORIENTATION_AXIAL:
        d->renderer2d->ComputeVisiblePropBounds(bounds);
        xmin = bounds[0];
        xmax = bounds[1];
        ymin = bounds[2];
        ymax = bounds[3];
        zmin = bounds[4];
        zmax = bounds[5];
        break;
    case medImageView::VIEW_ORIENTATION_CORONAL:
        d->renderer2d->ComputeVisiblePropBounds(bounds);
        xmin = bounds[0];
        xmax = bounds[1];
        ymin = bounds[2];
        ymax = bounds[3];
        zmin = bounds[4];
        zmax = bounds[5];
        break;
    case medImageView::VIEW_ORIENTATION_SAGITTAL:
        d->renderer2d->ComputeVisiblePropBounds(bounds);
        xmin = bounds[0];
        xmax = bounds[1];
        ymin = bounds[2];
        ymax = bounds[3];
        zmin = bounds[4];
        zmax = bounds[5];
        break;
    }
}

void medVtkViewNavigator::setOrientation(medImageView::Orientation orientation)
{
    if (orientation == d->orientation)
        return;

    switch(orientation)
    {
    case medImageView::VIEW_ORIENTATION_3D:
        d->o3dParameter->setValue(true);
        break;
    case medImageView::VIEW_ORIENTATION_AXIAL:
        d->oAxialParameter->setValue(true);
        break;
    case medImageView::VIEW_ORIENTATION_CORONAL:
        d->oCoronalParameter->setValue(true);
        break;
    case medImageView::VIEW_ORIENTATION_SAGITTAL:
        d->oSagittalParameter->setValue(true);
        break;
    }

    d->orientation = orientation;
}

medImageView::Orientation medVtkViewNavigator::orientation() const
{
    return d->orientation;
}

void medVtkViewNavigator::setAxial(bool axial)
{
    if(axial)
        this->changeOrientation(medImageView::VIEW_ORIENTATION_AXIAL);
}

void medVtkViewNavigator::setCoronal(bool coronal)
{
    if(coronal)
        this->changeOrientation(medImageView::VIEW_ORIENTATION_CORONAL);
}

void medVtkViewNavigator::setSagittal(bool sagittal)
{
    if(sagittal)
        this->changeOrientation(medImageView::VIEW_ORIENTATION_SAGITTAL);
}

void medVtkViewNavigator::set3d(bool o3d)
{
    if(o3d)
        this->changeOrientation(medImageView::VIEW_ORIENTATION_3D);
}

void medVtkViewNavigator::changeOrientation(medImageView::Orientation orientation)
{
    double pos[3];
    int timeIndex = 0;
    if(d->currentView)
    {
        d->currentView->GetCurrentPoint(pos);
        timeIndex = d->currentView->GetTimeIndex();
        d->currentView->UnInstallInteractor();
        d->currentView->SetRenderWindow(NULL);
        d->renWin->RemoveRenderer(d->currentView->GetRenderer());
    }

    switch(orientation)
    {
    case medImageView::VIEW_ORIENTATION_3D:
        d->currentView = d->view3d;
        break;
    case medImageView::VIEW_ORIENTATION_AXIAL:
        d->view2d->SetViewOrientation(vtkImageView2D::VIEW_ORIENTATION_AXIAL);
        d->currentView = d->view2d;
      break;
    case medImageView::VIEW_ORIENTATION_CORONAL:
        d->view2d->SetViewOrientation(vtkImageView2D::VIEW_ORIENTATION_CORONAL);
        d->currentView = d->view2d;
      break;
    case medImageView::VIEW_ORIENTATION_SAGITTAL:
        d->view2d->SetViewOrientation(vtkImageView2D::VIEW_ORIENTATION_SAGITTAL);
        d->currentView = d->view2d;

        break;
    }

    if(d->showOptionsWidget)
    {
        if(orientation == medImageView::VIEW_ORIENTATION_3D)
            d->showOptionsWidget->hide();
        else d->showOptionsWidget->show();
    }

    d->currentView->SetRenderWindow(d->renWin);
    d->currentView->SetCurrentPoint(pos);
    d->currentView->SetTimeIndex(timeIndex);
    d->currentView->Render();

    d->orientation = orientation;

    emit orientationChanged();
}

void medVtkViewNavigator::showAxes(bool show)
{
    d->collection->SyncSetShowImageAxis( show );
    d->currentView->InvokeEvent ( vtkImageView2D::CurrentPointChangedEvent );
    d->currentView->Render();
}

void medVtkViewNavigator::showRuler(bool show)
{
    d->collection->SyncSetShowRulerWidget( show );
    d->currentView->Render();
}

void medVtkViewNavigator::showAnnotations(bool show)
{
    d->collection->SyncSetShowAnnotations( show );
    d->currentView->Render();
}

void medVtkViewNavigator::showScalarBar(bool show)
{
    d->collection->SyncSetShowScalarBar( show );
    d->view3d->SetShowScalarBar(show);
    d->currentView->Render();
}
