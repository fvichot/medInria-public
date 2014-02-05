#include <medVtkView.h>

#include <QWidget>
#include <QHash>

#include <QVTKWidget2.h>

#include <vtkRenderer.h>
#include <vtkImageView2D.h>
#include <vtkImageView3D.h>
#include <vtkImageViewCollection.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera2.h>
#include <vtkCamera.h>
#include <vtkTextProperty.h>
#include <vtkImageView2DCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTransferFunctionPresets.h>
#include <vtkInriaInteractorStyleRubberBandZoom.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>

#include <medImageViewFactory.h>
#include <medVtkViewBackend.h>
#include <medAbstractImageViewInteractor.h>
#include <medAbstractImageViewNavigator.h>
#include <medVtkViewObserver.h>
#include <medBoolGroupParameter.h>
#include <medBoolParameter.h>
#include <medToolBox.h>
#include <medMetaDataKeys.h>

class medVtkViewPrivate
{
public:
    // internal state
    vtkImageView *currentView; //2d or 3d dependig on the navigator orientation.
    medAbstractData *currentData; //ie the data corresponding to the selected layer.

    vtkInteractorStyle *interactorStyle2D;
    vtkInteractorStyle *interactorStyle3D;

    // renderers
    vtkGenericOpenGLRenderWindow *renWin;
    vtkRenderer *renderer2d;
    vtkRenderer *renderer3d;

    // views
    vtkImageView2D *view2d;
    vtkImageView3D *view3d;
    vtkImageViewCollection *viewCollection;

    // widgets
    QWidget *mainWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *toolbarLayout;
    QWidget *toolBar;
    QWidget *toolBox;
    QVTKWidget2 *receiverWidget;
    static const int toolWidth = 15;
    static const int toolheight = 15;

    medVtkViewObserver *observer;

    // toolboxes
    medToolBox* navigatorToolBox;
    medToolBox* mouseStyleToolBox;
    medToolBox* layerToolBox;
    QListWidget *layersListWidget;
    QStackedWidget *interactorsWidgetStack;

    // parameter
    medBoolGroupParameter *mouseInteractionsParameter;
    medBoolParameter *windowingInteractionParameter;
    medBoolParameter *zoomInteractionParameter;
    medBoolParameter *slicingInteractionParameter;

    QScopedPointer<medVtkViewBackend> backend;

    bool multiSelectionEnabled;
};

medVtkView::medVtkView(QObject* parent): medAbstractImageView(parent),
    d(new medVtkViewPrivate)
{
    // setup initial internal state of the view
    d->currentView = NULL;
    d->currentData = NULL;
    d->interactorStyle2D = NULL;
    d->interactorStyle3D = NULL;

    // construct render window
        // renWin
    d->renWin = vtkGenericOpenGLRenderWindow::New();
    d->renWin->StereoCapableWindowOn();
    d->renWin->SetStereoTypeToCrystalEyes();
            // needed for imersive room
    if (qApp->arguments().contains("--stereo"))
        d->renWin->SetStereoRender(1);
            // Necessary options for depth-peeling
    d->renWin->SetAlphaBitPlanes(1);
    d->renWin->SetMultiSamples(0);


    //construct renderers
        // renderer3d
    d->renderer3d = vtkRenderer::New();
    d->renderer3d->GetActiveCamera()->SetPosition(0, -1, 0);
    d->renderer3d->GetActiveCamera()->SetViewUp (0, 0, 1);
    d->renderer3d->GetActiveCamera()->SetFocalPoint(0, 0, 0);
        //renderer2d
    d->renderer2d = vtkRenderer::New();

    // construct views
        // view2d
    d->view2d = vtkImageView2D::New();
    d->view2d->SetRenderer(d->renderer2d);
    d->view2d->SetBackground(0.0, 0.0, 0.0);
    d->view2d->SetLeftButtonInteractionStyle(vtkInteractorStyleImageView2D::InteractionTypeZoom);
    d->view2d->SetMiddleButtonInteractionStyle(vtkInteractorStyleImageView2D::InteractionTypePan);
    d->view2d->SetKeyboardInteractionStyle (vtkInteractorStyleImageView2D::InteractionTypeSlice);
    d->view2d->SetViewOrientation(vtkImageView2D::VIEW_ORIENTATION_AXIAL);
    d->view2d->CursorFollowMouseOff();
    d->view2d->ShowImageAxisOff();
    d->view2d->ShowScalarBarOff();
    d->view2d->ShowRulerWidgetOn();
    d->view2d->SetRenderWindow (d->renWin); // set the interactor as well
    d->interactorStyle2D = d->view2d->GetInteractorStyle(); // save interactorStyle
        // view3d.
    d->view3d = vtkImageView3D::New();
    d->view3d->SetRenderer(d->renderer3d);
    d->view3d->SetShowBoxWidget(0);
    d->view3d->SetCroppingModeToOff();
    d->view3d->ShowScalarBarOff();
    d->view3d->GetTextProperty()->SetColor(1.0, 1.0, 1.0 );
    d->view3d->ShadeOn();
            //TODO find out what the purpose of this lines - RDE.
    vtkInteractorStyleTrackballCamera2 *interactorStyle = vtkInteractorStyleTrackballCamera2::New();
    d->view3d->SetInteractorStyle(interactorStyle);
    interactorStyle->Delete();
    d->view3d->SetRenderWindowInteractor(d->renWin->GetInteractor());
    d->view3d->SetRenderWindow(d->renWin);
    d->view3d->UnInstallInteractor();
    d->renWin->RemoveRenderer(d->renderer3d);
    d->interactorStyle3D = d->view3d->GetInteractorStyle(); // save interactorStyle
        // collection.
    d->viewCollection = vtkImageViewCollection::New();
    d->viewCollection->SetLinkCurrentPoint(0);
    d->viewCollection->SetLinkSliceMove(0);
    d->viewCollection->SetLinkColorWindowLevel(0);
    d->viewCollection->SetLinkCamera (0);
    d->viewCollection->SetLinkZoom(0);
    d->viewCollection->SetLinkPan(0);
    d->viewCollection->SetLinkTimeChange(0);
    d->viewCollection->SetLinkRequestedPosition(0);
    d->viewCollection->AddItem(d->view2d);
    d->viewCollection->AddItem(d->view3d);

    d->receiverWidget = new QVTKWidget2();
    d->receiverWidget->setSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Minimum );

    d->receiverWidget->setFocusPolicy ( Qt::ClickFocus );

    // Event filter used to know if the view is selecetd or not
    d->receiverWidget->installEventFilter(this);

    d->receiverWidget->SetRenderWindow(d->renWin);

    d->backend.reset(new medVtkViewBackend(d->view2d,d->view3d,d->renWin,d->receiverWidget));


    d->observer = medVtkViewObserver::New();
    d->observer->setView ( this );

    d->view2d->AddObserver ( vtkImageView::CurrentPointChangedEvent, d->observer, 0 );
    d->view2d->AddObserver ( vtkImageView::WindowLevelChangedEvent,  d->observer, 0 );
    d->view2d->GetInteractorStyle()->AddObserver ( vtkImageView2DCommand::CameraZoomEvent, d->observer, 0 );
    d->view2d->GetInteractorStyle()->AddObserver ( vtkImageView2DCommand::CameraPanEvent, d->observer, 0 );
    d->view2d->AddObserver ( vtkImageView2DCommand::CameraPanEvent, d->observer, 0);
    d->view2d->AddObserver ( vtkImageView2DCommand::CameraZoomEvent,d->observer,0);
    d->view3d->GetInteractorStyle()->AddObserver ( vtkCommand::InteractionEvent, d->observer, 0 );

    d->view2d->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::KeyPressEvent,d->observer,0);
    d->view2d->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::KeyReleaseEvent,d->observer,0);

    this->initialiseNavigators();
    this->_prvt_buildToolBox();

    connect(this, SIGNAL(layerAdded(int)), this, SLOT(_prvt_addLayerItem(int)));
    connect(this, SIGNAL(layerRemoved(int)), this, SLOT(_prvt_removeLayerItem(int)));

    d->toolBox->show();

    // construct main widget
    d->mainWidget = new QWidget;
    d->toolBar = new QWidget;
    d->toolBar->setSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Fixed );

    d->mainLayout = new QVBoxLayout(d->mainWidget);
    d->mainLayout->setContentsMargins ( 0, 0, 0, 0 );
    d->mainLayout->setSpacing ( 0 );

    d->toolbarLayout = new QHBoxLayout(d->toolBar);
    d->toolbarLayout->setContentsMargins ( 0, 0, 0, 0 );
    d->toolbarLayout->setSpacing ( 0 );
    d->toolBar->setLayout(d->toolbarLayout);

    d->mainLayout->addWidget(d->toolBar);
    d->mainLayout->addWidget(d->receiverWidget);

    d->multiSelectionEnabled = false;
}

medVtkView::~medVtkView()
{
    delete d;
    d = NULL;
}

QString medVtkView::s_identifier()
{
    return "medVtkView";
}

QString medVtkView::identifier() const
{
    return medVtkView::s_identifier();
}

bool medVtkView::registered()
{
    return medImageViewFactory::instance()->registerView<medVtkView>(QString("medVtkView"), QStringList()<<"medAbstractData");
}

medViewBackend * medVtkView::backend() const
{
    return d->backend.data();
}

QString medVtkView::description() const
{
    return "medVtkView";
}

QWidget* medVtkView::widget()
{
    return d->mainWidget;
}

QWidget* medVtkView::receiverWidget()
{
    return d->receiverWidget;
}

QWidget* medVtkView::toolBar()
{
    return d->toolBar;
}

QWidget* medVtkView::toolBox()
{
    return d->toolBox;
}

QPointF medVtkView::mapWorldToDisplayCoordinates(const QVector3D & worldVec)
{
    // The following code is implemented without calling ren->SetWorldPoint,
    // because that generates an unnecessary modified event.

    vtkRenderer * ren = d->currentView->GetRenderer();

    // Get window for dimensions
    vtkWindow * win = ren->GetVTKWindow();

    if ( !win )
        return QPointF();

    double wx = worldVec.x();
    double wy = worldVec.y();
    double wz = worldVec.z();

    ren->WorldToView( wx, wy, wz );

    // get physical window dimensions
    const int * size = win->GetSize();
    int sizex = size[0];
    int sizey = size[1];

    const double * viewport = ren->GetViewport( );

    double dx = (wx + 1.0) * (sizex*(viewport[2] - viewport[0])) / 2.0 + sizex*viewport[0];
    double dy = (wy + 1.0) * (sizey*(viewport[3] - viewport[1])) / 2.0 + sizey*viewport[1];

    // Convert VTK display coordinates to Qt (flipped in Y)
    return QPointF( dx, sizey - 1 - dy );
}
QVector3D medVtkView::mapDisplayToWorldCoordinates(const QPointF & scenePoint)
{
    // The following code is implemented without calling ren->SetWorldPoint,
    // because that generates an unnecessary modified event.

    vtkRenderer * ren = d->currentView->GetRenderer();

    /* get physical window dimensions */
    vtkWindow * win = ren->GetVTKWindow();

    if ( !win )
        return QVector3D();

    const int * size = win->GetSize();
    int sizex = size[0];
    int sizey = size[1];

    const double * viewport = ren->GetViewport();

    // Convert Qt display coordinates to VTK (flipped in Y)
    const double dx = scenePoint.x();
    const double dy = sizey - 1 - scenePoint.y();

    double vx = 2.0 * (dx - sizex * viewport[0]) / (sizex * (viewport[2] - viewport[0])) - 1.0;
    double vy = 2.0 * (dy - sizey * viewport[1]) / (sizey * (viewport[3] - viewport[1])) - 1.0;
    double vz = 0.;

    if (this->is2D())
    {
        // Project the point into the view plane.
        //vtkCamera * cam = ren->GetActiveCamera();
        double pointInDisplayPlane[3];

        d->currentView->GetCurrentPoint(pointInDisplayPlane);

        ren->WorldToView(pointInDisplayPlane[0],pointInDisplayPlane[1],pointInDisplayPlane[2]);
        vz = pointInDisplayPlane[2];
    }
    ren->ViewToWorld(vx,vy,vz);

    return QVector3D( vx, vy, vz );
}

QVector3D medVtkView::viewCenter()
{
    vtkRenderer * ren = d->currentView->GetRenderer();
    double fp[3];
    ren->GetActiveCamera()->GetFocalPoint( fp);
    return QVector3D( fp[0], fp[1], fp[2] );
}

QVector3D medVtkView::viewPlaneNormal()
{
    double vpn[3];
    vtkRenderer * ren = d->currentView->GetRenderer();
    ren->GetActiveCamera()->GetViewPlaneNormal(vpn);
    return QVector3D(vpn[0], vpn[1], vpn[2]);
}

QVector3D medVtkView::viewUp()
{
    double vup[3];
    vtkRenderer * ren = d->currentView->GetRenderer();
    ren->GetActiveCamera()->GetViewUp(vup);
    return QVector3D(vup[0], vup[1], vup[2]);
}

bool medVtkView::is2D()
{
    return this->orientation() == medImageView::VIEW_ORIENTATION_AXIAL ||
            medImageView::VIEW_ORIENTATION_CORONAL ||
            medImageView::VIEW_ORIENTATION_SAGITTAL;
}

qreal medVtkView::sliceThickness()
{
    double cr[2] = {0, 0};
    d->renderer2d->GetActiveCamera()->GetClippingRange(cr);
    return std::fabs(cr[1] - cr[0]);
}

qreal medVtkView::scale()
{
    double scale;
    if (this->is2D())
    {
        // The height of the viewport in world coordinates
        double camScale = d->currentView->GetRenderer()->GetActiveCamera()->GetParallelScale();
        double heightInPx = d->currentView->GetRenderWindow()->GetSize()[1];
        // return pixels per world coord.
        scale = heightInPx / camScale;
    }
    else
    {
        // Return scale at fp.
        double vup[4];
        d->currentView->GetRenderer()->GetActiveCamera()->GetViewUp(vup);
        vup[3] = 0;  //intentionally zero and not one.
        double MVup[4];
        d->currentView->GetRenderer()->GetActiveCamera()->GetViewTransformMatrix()->MultiplyPoint(vup, MVup);
        double lScale = vtkMath::Norm(MVup) / vtkMath::Norm(vup);
        //We now have the scale in viewport coords. (normalised).
        double heightInPx = d->currentView->GetRenderWindow()->GetSize()[1];
        scale = heightInPx *lScale;
    }

    if (scale < 0)
        scale *= -1;
    return scale;
}

void medVtkView::_prvt_setWindowingInteracStyle(bool windowing)
{
    if(windowing)
        d->view2d->SetLeftButtonInteractionStyle(vtkInteractorStyleImageView2D::InteractionTypeWindowLevel);
}

void medVtkView::_prvt_setZoomIntercaStyle(bool zoom)
{
    if(zoom)
        d->view2d->SetLeftButtonInteractionStyle(vtkInteractorStyleImageView2D::InteractionTypeZoom);
}

void medVtkView::_prvt_setSLicingInteracStyle(bool slicing)
{
    if(slicing)
        d->view2d->SetLeftButtonInteractionStyle(vtkInteractorStyleImageView2D::InteractionTypeSlice);
}

void medVtkView::_prvt_buildToolBox()
{
    d->toolBox = new QWidget;
    QVBoxLayout *toolBoxLayout = new QVBoxLayout(d->toolBox);
    toolBoxLayout->setContentsMargins(0,0,0,0);
    toolBoxLayout->setMargin(0);
    d->toolBox->setContentsMargins(0,0,0,0);

    d->navigatorToolBox = new medToolBox(d->toolBox);
    d->navigatorToolBox->setTitle(tr("View settings"));
    d->navigatorToolBox->addWidget(primaryNavigator()->widgetForToolBox());
    foreach (medAbstractNavigator* navigator, this->extraNavigators())
        d->navigatorToolBox->addWidget(navigator->widgetForToolBox());

    toolBoxLayout->addWidget(d->navigatorToolBox);

    this->_prvt_buildMouseInteracToolBox();
    toolBoxLayout->addWidget(d->mouseStyleToolBox);

    d->layerToolBox = new medToolBox(d->toolBox);
    d->layerToolBox->setTitle(tr("Layers Settings"));

    d->layersListWidget = new QListWidget(d->layerToolBox);
    d->layersListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->layerToolBox->addWidget(d->layersListWidget);
    connect(d->layersListWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(setSelectedLayer(int)));
    connect(this, SIGNAL(selectedLayerChanged()),
            this, SLOT(updateInteractorsWidget()));


    d->interactorsWidgetStack = new QStackedWidget(d->layerToolBox);
    d->layerToolBox->addWidget(d->interactorsWidgetStack);

    toolBoxLayout->addWidget(d->layerToolBox);

}

void medVtkView::_prvt_buildMouseInteracToolBox()
{
    d->mouseInteractionsParameter = new medBoolGroupParameter("Mouse interactions", this);
    d->mouseInteractionsParameter->setPushButtonDirection(QBoxLayout::LeftToRight);
    d->windowingInteractionParameter = new medBoolParameter("window / level", this);
    d->windowingInteractionParameter->setIcon(QIcon(":/icons/wlww.png"));
    d->zoomInteractionParameter = new medBoolParameter("zoom", this);
    d->zoomInteractionParameter->setIcon(QIcon(":/icons/magnify.png"));
    d->slicingInteractionParameter = new medBoolParameter("slicing", this);
    d->slicingInteractionParameter->setIcon(QIcon(":/icons/stack.png"));

    d->mouseInteractionsParameter->addParameter(d->windowingInteractionParameter);
    d->mouseInteractionsParameter->addParameter(d->zoomInteractionParameter);
    d->mouseInteractionsParameter->addParameter(d->slicingInteractionParameter);

    connect(d->windowingInteractionParameter, SIGNAL(valueChanged(bool)),
            this, SLOT(_prvt_setWindowingInteracStyle(bool)));

    connect(d->zoomInteractionParameter, SIGNAL(valueChanged(bool)),
            this, SLOT(_prvt_setZoomIntercaStyle(bool)));

    connect(d->slicingInteractionParameter, SIGNAL(valueChanged(bool)),
            this, SLOT(_prvt_setSLicingInteracStyle(bool)));

    d->windowingInteractionParameter->setValue(true);

    d->mouseStyleToolBox = new medToolBox(d->toolBox);
    d->mouseStyleToolBox->setTitle(tr("Mouse interactions"));
    d->mouseStyleToolBox->addWidget(d->mouseInteractionsParameter->getPushButtonGroup());
}

void medVtkView::_prvt_addLayerItem(int layer)
{
    medAbstractData *data = this->data(layer);
    QString thumbPath = medMetaDataKeys::SeriesThumbnail.getFirstValue(data,":icons/layer.png");
    QString name = medMetaDataKeys::SeriesDescription.getFirstValue(data,"<i>no name</i>");

    QWidget *layerWidget = new QWidget(d->layersListWidget);

    QHBoxLayout* layout = new QHBoxLayout(layerWidget);
    layout->setContentsMargins(0,0,10,0);

    QPushButton* thumbnailButton = new QPushButton(layerWidget);
    QIcon thumbnailIcon;
    // Set the off icon to the greyed out version of the regular icon
    thumbnailIcon.addPixmap(QPixmap(thumbPath), QIcon::Normal, QIcon::On);
    QStyleOption opt(0);
    opt.palette = QApplication::palette();
    QPixmap pix = QApplication::style()->generatedIconPixmap(QIcon::Disabled, QPixmap(thumbPath), &opt);
    thumbnailIcon.addPixmap(pix, QIcon::Normal, QIcon::Off);
    thumbnailButton->setFocusPolicy(Qt::NoFocus);
    thumbnailButton->setIcon(thumbnailIcon);
    thumbnailButton->setIconSize(QSize(22,22));
    thumbnailButton->setCheckable(true);
    thumbnailButton->setChecked(true);
    thumbnailButton->setFlat(true);

    QLabel *layerName = new QLabel(name, layerWidget);

    QPushButton *removeButton = new QPushButton;
    removeButton->setIcon(QIcon(":/icons/cross.svg"));
    connect(removeButton,SIGNAL(clicked()), this, SLOT(_prvt_removeSelectedLayer()));

    layout->addWidget(thumbnailButton);
    layout->addWidget(layerName);
    layout->addStretch();
    layout->addWidget(this->primaryInteractor(data)->layerWidget());
    foreach(medAbstractInteractor* interactor, this->extraInteractors(data))
        layout->addWidget(interactor->layerWidget());
    layout->addWidget(removeButton);

    QListWidgetItem * item = new QListWidgetItem(d->layersListWidget);
    item->setSizeHint(QSize(layerWidget->width(), 25));

    d->layersListWidget->addItem(item);
    d->layersListWidget->setItemWidget(item, layerWidget);
}


void medVtkView::_prvt_removeLayerItem(int layer)
{
    QListWidgetItem* item = d->layersListWidget->item(layer);
    d->layersListWidget->removeItemWidget(item);
}

void medVtkView::_prvt_removeSelectedLayer()
{
    this->removeLayer(this->selectedLayer());
}

void medVtkView::updateInteractorsWidget()
{
    d->interactorsWidgetStack->addWidget( this->primaryInteractor( this->selectedLayer() )->toolBoxWidget() );

    foreach(medAbstractInteractor* interactor, this->extraInteractors(this->selectedLayer()))
        d->interactorsWidgetStack->addWidget(interactor->toolBoxWidget());

    d->toolbarLayout->addWidget(this->primaryInteractor( this->selectedLayer() )->toolBarWidget());
}

bool medVtkView::eventFilter(QObject * obj, QEvent * event)
{
    if (obj == d->receiverWidget)
    {
        if (event->type() == QEvent::FocusIn)
        {
            emit selected();
        } else if (event->type() == QEvent::FocusOut)
        {
            emit unselected();
        }
    }

    return medAbstractView::eventFilter(obj, event);
}