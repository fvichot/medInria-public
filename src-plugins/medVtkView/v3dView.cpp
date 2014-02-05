///*=========================================================================

// medInria

// Copyright (c) INRIA 2013. All rights reserved.
// See LICENSE.txt for details.
 
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.

//=========================================================================*/

//#include <v3dView.h>

//#include <v3dViewAnnotationInteractor.h>

//#include <dtkCore/dtkAbstractViewFactory.h>
//#include <dtkCore/dtkAbstractProcess.h>
//#include <dtkCore/dtkAbstractProcessFactory.h>
//#include <dtkCore/dtkSignalBlocker.h>
//#include <dtkCore/dtkAbstractViewInteractor.h>

//#include <medMessageController.h>
//#include <medAbstractImageData.h>
//#include <medMetaDataKeys.h>

//#include <vtkCamera.h>
//#include <vtkCommand.h>
//#include <vtkRenderer.h>
//#include <vtkRenderWindow.h>
//#include <vtkTransferFunctionPresets.h>
//#include <vtkImageActor.h>
//#include <vtkImageData.h>
//#include <vtkPointSet.h>
//#include <vtkTextProperty.h>
//#include <vtkImageMapToColors.h>
//#include <vtkOrientedBoxWidget.h>
//#include <vtkMath.h>
//#include <vtkMatrix4x4.h>

//#include <vtkImageView2D.h>
//#include <vtkImageView3D.h>
//#include <vtkImageView2DCommand.h>
//#include <vtkInteractorStyleImageView2D.h>
//#include <vtkInteractorStyleTrackballCamera2.h>
//#include <vtkInteractorStyleTrackballActor.h>
//#include <vtkInriaInteractorStyleRubberBandZoom.h>
//#include <vtkImageViewCollection.h>
//#include <vtkColorTransferFunction.h>
//#include <vtkPiecewiseFunction.h>
//#include <QVTKWidget.h>
//#include <medVtkViewBackend.h>

//#include <QtGui>
//#include <QMenu>
//#include <QMouseEvent>

////=============================================================================
//// Construct a QVector3d from pointer-to-double
//inline QVector3D doubleToQtVector3D ( const double * v )
//{
//    return QVector3D ( v[0], v[1], v[2] );
//}
//// Convert a QVector3D to array of double.
//inline void qtVector3dToDouble ( const QVector3D & inV , double * outV )
//{
//    outV[0] = inV.x();
//    outV[1] = inV.y();
//    outV[2] = inV.z();
//}
//// Convert QColor to vtk's double[3]. (no alpha)
//inline void qtColorToDouble ( const QColor & color, double * cv )
//{
//    cv[0] = color.redF();
//    cv[1] = color.greenF();
//    cv[2] = color.blueF();
//}
////=============================================================================

//// /////////////////////////////////////////////////////////////////////////////////////////////////////////
//// v3dViewObserver: links a QSlider with the CurrentPointChangedEvent of a vtkImageView instance.
//// /////////////////////////////////////////////////////////////////////////////////////////////////////////

//class v3dViewObserver : public vtkCommand
//{
//public:
//    static v3dViewObserver* New()
//    {
//        return new v3dViewObserver;
//    }

//    void Execute ( vtkObject *caller, unsigned long event, void *callData );

//    void setSlider ( QSlider *slider )
//    {
//        this->slider = slider;
//    }

//    void setView ( v3dView *view )
//    {
//        this->view = view;
//    }

//    inline void   lock()
//    {
//        this->m_lock = 1;
//    }
//    inline void unlock()
//    {
//        this->m_lock = 0;
//    }

//protected:
//    v3dViewObserver();
//    ~v3dViewObserver();

//private:
//    int             m_lock;
//    QSlider        *slider;
//    v3dView        *view;
//};

//v3dViewObserver::v3dViewObserver()
//{
//    this->slider = 0;
//    this->m_lock = 0;
//}

//v3dViewObserver::~v3dViewObserver()
//{

//}

//void v3dViewObserver::Execute ( vtkObject *caller, unsigned long event, void *callData )
//{
//    if ( this->m_lock )
//        return;

//    if ( !this->slider || !this->view )
//        return;

//    switch ( event )
//    {
//    case vtkImageView::CurrentPointChangedEvent:
//    {
//        {
//            dtkSignalBlocker blocker ( this->slider );
//            unsigned int zslice = this->view->view2d()->GetSlice();
//            this->slider->setValue ( zslice );
//            this->slider->update();
//        }

//        const double *pos = this->view->currentView()->GetCurrentPoint();
//        QVector3D qpos ( doubleToQtVector3D ( pos ) );
//        this->view->emitViewPositionChangedEvent ( qpos );
//    }
//    break;

//    case vtkImageView2DCommand::CameraZoomEvent:
//    {
//        double zoom = this->view->currentView()->GetZoom();
//        this->view->emitViewZoomChangedEvent ( zoom );
//    }
//    break;

//    case vtkImageView2DCommand::CameraPanEvent:
//    {
//        const double *pan = this->view->view2d()->GetPan();
//        QVector2D qpan ( pan[0],pan[1] );
//        this->view->emitViewPanChangedEvent ( qpan );
//    }
//    break;

//    case vtkImageView::WindowLevelChangedEvent:
//    {
//        double level = this->view->currentView()->GetColorLevel();
//        double window = this->view->currentView()->GetColorWindow();

//        this->view->emitViewWindowingChangedEvent ( level, window );
//    }
//    break;

//    case vtkCommand::InteractionEvent:
//    {
//        double *pos = this->view->renderer3d()->GetActiveCamera()->GetPosition();
//        double *vup = this->view->renderer3d()->GetActiveCamera()->GetViewUp();
//        double *foc = this->view->renderer3d()->GetActiveCamera()->GetFocalPoint();
//        double   ps = this->view->renderer3d()->GetActiveCamera()->GetParallelScale();

//        QVector3D position ( doubleToQtVector3D ( pos ) );
//        QVector3D viewup ( doubleToQtVector3D ( vup ) );
//        QVector3D focal ( doubleToQtVector3D ( foc ) );

//        this->view->emitViewCameraChangedEvent ( position, viewup, focal, ps );
//    }

//    break;


//    case vtkCommand::KeyPressEvent:
//    {
//        vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
//        if (iren->GetControlKey())
//        {
//            view->setProperty( "ZoomMode" , "RubberBand" );
//            dynamic_cast<vtkInriaInteractorStyleRubberBandZoom*>(view->view2d()->GetInteractor()->GetInteractorStyle())->RightButtonModeOn();
//            if (view->property("MouseInteraction")!="Zooming")
//                dynamic_cast<vtkInriaInteractorStyleRubberBandZoom*>(view->view2d()->GetInteractor()->GetInteractorStyle())->LeftButtonModeOff();
//        }
//    }

//    break;
//    case vtkCommand::KeyReleaseEvent:
//    {
//        vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
//        if (!iren->GetControlKey())
//        {
//            if (view->property("ZoomMode")=="RubberBand")
//                view->setProperty( "ZoomMode" , "Normal" );
//        }
//    }

//    break;
//    }
//}

//// /////////////////////////////////////////////////////////////////
//// v3dViewPrivate
//// /////////////////////////////////////////////////////////////////

////Create a few icons with several states, and we'll use them as static members
////to save some memory and allocation time.
//class medPlayIcon: public QIcon{
//public:
//    medPlayIcon():QIcon(":/icons/play.png"){
//        addPixmap(QPixmap(":/icons/pause.png"),
//                  QIcon::Normal,
//                  QIcon::On);
//    }
//    ~medPlayIcon(){}
//};

//class medLinkIcon: public QIcon{
//public:
//    medLinkIcon():QIcon(":/icons/broken_link.svg"){
//        addFile(":/icons/link.svg",
//                  QSize(48,48),
//                  QIcon::Normal,
//                  QIcon::On);
//    }
//    ~medLinkIcon(){}
//};

//class medLinkWLIcon: public QIcon{
//public:
//    medLinkWLIcon():QIcon(":/icons/broken_link.svg"){
//        addFile(":/icons/link.svg",
//                  QSize(48,48),
//                  QIcon::Normal,
//                  QIcon::On);
//    }
//    ~medLinkWLIcon(){}
//};

//class medMaximizeIcon: public QIcon{
//public:
//    medMaximizeIcon():QIcon(":/icons/maximize.svg"){
//        addFile(":/icons/un_maximize.svg",
//                  QSize(48,48),
//                  QIcon::Normal,
//                  QIcon::On);
//    }
//    ~medMaximizeIcon(){}
//};


//class v3dViewPrivate
//{
//public:
//    // Utility to determine if a given orientation is 2D
//    inline bool is2dOrientation ( const QString & name ) const
//    {
//        return ( name == "Axial" ) || ( name == "Sagittal" ) || ( name == "Coronal" ) || ( name == "Oblique" );
//    }

//    inline bool is2dOrientation( ) const
//    {
//        return is2dOrientation ( this->orientation );
//    }

//    vtkRenderer *renderer2d;
//    vtkRenderer *renderer3d;
//    vtkSmartPointer<vtkRenderer> overlayRenderer2d;
//    vtkImageView2D *view2d;
//    vtkImageView3D *view3d;

//    vtkInteractorStyle *interactorStyle2D;
    
//    vtkImageView *currentView;

//    vtkImageViewCollection *collection;

//    v3dViewObserver *observer;

//    vtkRenderWindow *renWin;

//    QWidget    *widget;
//    QSlider    *slider;
//    QPushButton *linkButton;
//    QPushButton *linkWLButton;
//    QPushButton *playButton;
//    QPushButton *closeButton;
//    QPushButton *fullScreenButton;
//    QVTKWidget *vtkWidget;
//    QString orientation;

//    medAbstractData *data;
//    QMap<int, dtkSmartPointer<medAbstractData> > sharedData;
//    medAbstractImageData *imageData;
    
//    bool interactorsInitialized;

//    QList<QString> LUTList;
//    QList<QString> PresetList;

//    QTimeLine *timeline;

//    QScopedPointer<medVtkViewBackend> backend;

//    typedef void ( v3dView::* PropertyFuncType ) ( const QString & );
//    typedef QHash<  QString, PropertyFuncType > PropertyFuncMapType;
//    PropertyFuncMapType setPropertyFunctions;

//    static QList<QColor> presetColors;
//    static int nextColorIndex;
//    static medPlayIcon playIcon;
//    static medLinkIcon linkIcon;
//    static medLinkWLIcon linkWLIcon;
//    static medMaximizeIcon maximizeIcon;
//};



//QList<QColor> v3dViewPrivate::presetColors;
//int v3dViewPrivate::nextColorIndex = -1;
//medPlayIcon v3dViewPrivate::playIcon;
//medLinkIcon v3dViewPrivate::linkIcon;
//medLinkWLIcon v3dViewPrivate::linkWLIcon;
//medMaximizeIcon v3dViewPrivate::maximizeIcon;

//// /////////////////////////////////////////////////////////////////
//// v3dView
//// /////////////////////////////////////////////////////////////////

//v3dView::v3dView() : medAbstractView(), d ( new v3dViewPrivate )
//{
//    if ( d->nextColorIndex < 0 )
//    {
//        d->nextColorIndex = 0;
//        d->presetColors.push_back ( QColor ( "red" ) );
//        d->presetColors.push_back ( QColor ( "green" ) );
//        d->presetColors.push_back ( QColor ( "blue" ) );
//        d->presetColors.push_back ( QColor ( "darkRed" ) );
//        d->presetColors.push_back ( QColor ( "darkGreen" ) );
//        d->presetColors.push_back ( QColor ( "darkBlue" ) );
//        d->presetColors.push_back ( QColor ( "cyan" ) );
//        d->presetColors.push_back ( QColor ( "magenta" ) );
//        d->presetColors.push_back ( QColor ( "yellow" ) );
//        d->presetColors.push_back ( QColor ( "darkCyan" ) );
//        d->presetColors.push_back ( QColor ( "darkMagenta" ) );
//// Actually this does not exist. Even though the QColor doc mentions it.
////        d->presetColors.push_back( QColor( "darkYellow" ) );
//// For full list see   ${QT_DIR}/src/gui/painting/qcolor_p.cpp
//    }

//    d->setPropertyFunctions["Closable"] = &v3dView::onClosablePropertySet;

//    d->data       = 0;
//    d->imageData  = 0;
//    d->orientation = "Axial";
    
//    d->interactorsInitialized = false;

//    d->timeline = new QTimeLine ( 1000, this );
//    d->timeline->setLoopCount ( 0 );
//    connect ( d->timeline, SIGNAL ( frameChanged ( int ) ), this, SLOT ( onZSliderValueChanged ( int ) ) );

//    // Setting up 2D view

//    d->renderer2d = vtkRenderer::New();
//    d->view2d = vtkImageView2D::New();
//    d->view2d->SetRenderer ( d->renderer2d );
//    d->view2d->SetBackground ( 0.0, 0.0, 0.0 );
//    d->view2d->SetLeftButtonInteractionStyle ( vtkInteractorStyleImageView2D::InteractionTypeZoom );
//    d->view2d->SetMiddleButtonInteractionStyle ( vtkInteractorStyleImageView2D::InteractionTypePan );
//    d->view2d->SetKeyboardInteractionStyle ( vtkInteractorStyleImageView2D::InteractionTypeSlice);
//    d->view2d->SetViewOrientation ( vtkImageView2D::VIEW_ORIENTATION_AXIAL );
//    d->view2d->CursorFollowMouseOff();
//    d->view2d->ShowImageAxisOff();
//    d->view2d->ShowScalarBarOff();
//    d->view2d->ShowRulerWidgetOn();
//    d->overlayRenderer2d = vtkSmartPointer<vtkRenderer>::New();
//    d->view2d->SetOverlayRenderer(d->overlayRenderer2d);

//    // Setting up 3D view
//    d->renderer3d = vtkRenderer::New();
//    d->renderer3d->GetActiveCamera()->SetPosition ( 0, -1, 0 );
//    d->renderer3d->GetActiveCamera()->SetViewUp ( 0, 0, 1 );
//    d->renderer3d->GetActiveCamera()->SetFocalPoint ( 0, 0, 0 );

//    d->view3d = vtkImageView3D::New();
//    d->view3d->SetRenderer ( d->renderer3d );
//    d->view3d->SetShowBoxWidget ( 0 );
//    d->view3d->SetCroppingModeToOff();
//    d->view3d->ShowScalarBarOff();
//    d->view3d->GetTextProperty()->SetColor ( 1.0, 1.0, 1.0 );
//    d->view3d->ShadeOn();

//    d->currentView = d->view2d;
    
//    vtkInteractorStyleTrackballCamera2 *interactorStyle = vtkInteractorStyleTrackballCamera2::New();
//    d->view3d->SetInteractorStyle ( interactorStyle );
//    interactorStyle->Delete();

//    QMainWindow * mainWindowApp = dynamic_cast< QMainWindow * >(
//        qApp->property( "MainWindow" ).value< QObject * >() );

//    connect(mainWindowApp,SIGNAL(mainWindowDeactivated()),this,SLOT(onMainWindowDeactivated()));
    
//    d->widget = new QWidget( mainWindowApp);
    
//    d->slider = new QSlider ( Qt::Horizontal, d->widget );
//    d->slider->setSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Fixed );
//    d->slider->setFocusPolicy ( Qt::NoFocus );

//    d->linkButton = new QPushButton ( d->widget );
//    d->linkButton->setIcon (d->linkIcon );
//    d->linkButton->setToolTip(tr("Link the position with other views"));
//    d->linkButton->setCheckable ( true );
//    d->linkButton->setMaximumHeight ( 15 );
//    d->linkButton->setMaximumWidth ( 15 );
//    d->linkButton->setFocusPolicy ( Qt::NoFocus );
//    d->linkButton->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Fixed );
//    d->linkButton->setObjectName ( "tool" );

//    connect ( d->linkButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( setLinkPosition ( bool ) ) );
//    connect ( d->linkButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( setLinkCamera ( bool ) ) );

//    d->linkWLButton = new QPushButton ( d->widget );
//    d->linkWLButton->setIcon ( d->linkWLIcon);
//    d->linkWLButton->setToolTip(tr("Link the window/level with other views"));
//    d->linkWLButton->setCheckable ( true );
//    d->linkWLButton->setMaximumHeight ( 15 );
//    d->linkWLButton->setMaximumWidth ( 15 );
//    d->linkWLButton->setFocusPolicy ( Qt::NoFocus );
//    d->linkWLButton->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Fixed );
//    d->linkWLButton->setObjectName ( "tool" );

//    connect ( d->linkWLButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( setLinkWindowing ( bool ) ) );

//    d->fullScreenButton = new QPushButton ( d->widget );
//    d->fullScreenButton->setIcon (d->maximizeIcon);
//    d->fullScreenButton->setToolTip(tr("(Un)Maximize the view"));
//    d->fullScreenButton->setCheckable ( true );
//    d->fullScreenButton->setMaximumHeight ( 15 );
//    d->fullScreenButton->setMaximumWidth ( 15 );
//    d->fullScreenButton->setFocusPolicy ( Qt::NoFocus );
//    d->fullScreenButton->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Fixed );
//    d->fullScreenButton->setObjectName ( "tool" );

//    connect ( d->fullScreenButton, SIGNAL ( clicked ( bool ) ), this, SIGNAL ( fullScreen ( bool ) ) );

//    d->playButton = new QPushButton ( d->widget );
//    d->playButton->setIcon(d->playIcon);
//    d->playButton->setToolTip(tr("Play through the slices") );
//    d->playButton->setCheckable ( true );
//    d->playButton->setMaximumHeight ( 15 );
//    d->playButton->setMaximumWidth ( 15 );
//    d->playButton->setFocusPolicy ( Qt::NoFocus );
//    d->playButton->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Fixed );
//    d->playButton->setObjectName ( "tool" );

//    connect ( d->playButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( play ( bool ) ) );

//    d->closeButton = new QPushButton ( d->widget );
//    d->closeButton->setIcon(QIcon(":/icons/whitecross.svg"));
//    d->closeButton->setToolTip(tr("Close View"));
//    d->closeButton->setCheckable ( false );
//    d->closeButton->setMaximumHeight ( 15 );
//    d->closeButton->setMaximumWidth ( 15 );
//    d->closeButton->setFocusPolicy ( Qt::NoFocus );
//    d->closeButton->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Fixed );
//    d->closeButton->setObjectName ( "tool" );

//    connect ( d->closeButton, SIGNAL ( clicked() ), this, SIGNAL ( closing() ) );

//    QButtonGroup *toolButtonGroup = new QButtonGroup ( d->widget );
//    toolButtonGroup->addButton ( d->linkButton );
//    toolButtonGroup->setExclusive ( false );

//    d->vtkWidget = new QVTKWidget ( d->widget );
//    d->vtkWidget->setSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Minimum );
//    d->vtkWidget->setFocusPolicy ( Qt::ClickFocus );
//    d->vtkWidget->installEventFilter(this);
    
//    d->renWin = vtkRenderWindow::New();
//    d->renWin->StereoCapableWindowOn();
//    d->renWin->SetStereoTypeToCrystalEyes();
//    if (qApp->arguments().contains("--stereo"))
//        d->renWin->SetStereoRender(1);

//    // Necessary options for depth-peeling
//    d->renWin->SetAlphaBitPlanes(1);
//    d->renWin->SetMultiSamples(0);

//    d->vtkWidget->SetRenderWindow ( d->renWin );

//    QHBoxLayout *toolsLayout = new QHBoxLayout;
//    toolsLayout->setContentsMargins ( 0, 0, 0, 0 );
//    toolsLayout->setSpacing ( 0 );
//    toolsLayout->addWidget ( d->playButton );
//    toolsLayout->addWidget ( d->slider );
//    toolsLayout->addWidget ( d->linkButton );
//    toolsLayout->addWidget ( d->linkWLButton );
//    toolsLayout->addWidget ( d->fullScreenButton );
//    toolsLayout->addWidget ( d->closeButton );

//    QVBoxLayout *layout = new QVBoxLayout ( d->widget );
//    layout->setContentsMargins ( 0, 0, 0, 0 );
//    layout->setSpacing ( 0 );
//    layout->addLayout ( toolsLayout );
//    layout->addWidget ( d->vtkWidget );

//    d->view3d->SetRenderWindowInteractor ( d->renWin->GetInteractor() );
//    d->view3d->SetRenderWindow ( d->renWin );
//    d->view3d->UnInstallInteractor();
//    d->renWin->RemoveRenderer ( d->renderer3d );

//    d->view2d->SetRenderWindow ( d->renWin ); // set the interactor as well

//    d->collection = vtkImageViewCollection::New();
//    d->collection->SetLinkCurrentPoint ( 0 );
//    d->collection->SetLinkSliceMove ( 0 );
//    d->collection->SetLinkColorWindowLevel ( 0 );
//    d->collection->SetLinkCamera ( 0 );
//    d->collection->SetLinkZoom ( 0 );
//    d->collection->SetLinkPan ( 0 );
//    d->collection->SetLinkTimeChange ( 0 );
//    d->collection->SetLinkRequestedPosition ( 0 );

//    d->collection->AddItem ( d->view2d );
//    d->collection->AddItem ( d->view3d );

//    d->observer = v3dViewObserver::New();
//    d->observer->setSlider ( d->slider );
//    d->observer->setView ( this );

//    d->view2d->AddObserver ( vtkImageView::CurrentPointChangedEvent, d->observer, 0 );
//    d->view2d->AddObserver ( vtkImageView::WindowLevelChangedEvent,  d->observer, 0 );
//    d->view2d->GetInteractorStyle()->AddObserver ( vtkImageView2DCommand::CameraZoomEvent, d->observer, 0 );
//    d->view2d->GetInteractorStyle()->AddObserver ( vtkImageView2DCommand::CameraPanEvent, d->observer, 0 );
//    d->view2d->AddObserver ( vtkImageView2DCommand::CameraPanEvent, d->observer, 0);
//    d->view2d->AddObserver ( vtkImageView2DCommand::CameraZoomEvent,d->observer,0);
//    d->view3d->GetInteractorStyle()->AddObserver ( vtkCommand::InteractionEvent, d->observer, 0 );

//    d->view2d->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::KeyPressEvent,d->observer,0);
//    d->view2d->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::KeyReleaseEvent,d->observer,0);

//    d->interactorStyle2D = d->view2d->GetInteractorStyle();

//    // set property to actually available presets
//    QStringList lut = this->getAvailableTransferFunctionPresets();
//    this->addProperty ( "LookupTable",           lut );

//    this->setProperty ( "Closable",         "true"  );

//    this->addProperty ("ZoomMode",QStringList() << "Normal" << "RubberBand" );
//    this->setProperty ( "ZoomMode" , "Normal" );

//    int colorIndex = d->nextColorIndex;
//    if ( colorIndex >= d->presetColors.size() )
//    {
//        colorIndex = d->nextColorIndex = 0;
//    }
//    else
//    {
//        ++d->nextColorIndex;
//    }
//    this->setColor ( d->presetColors.at ( colorIndex ) );

//    connect ( d->slider,       SIGNAL ( valueChanged ( int ) ),            this, SLOT ( onZSliderValueChanged ( int ) ) );

//    connect ( d->widget, SIGNAL ( destroyed() ), this, SLOT ( widgetDestroyed() ) );

//    d->backend.reset(new medVtkViewBackend(d->view2d,d->view3d,d->renWin));
//}

//v3dView::~v3dView()
//{
//    foreach ( dtkAbstractViewInteractor *interactor, this->interactors() )
//    {
//        interactor->disable();
//        interactor->deleteLater();
//    }

//    d->renderer2d->SetRenderWindow ( NULL );
//    d->renderer3d->SetRenderWindow ( NULL );

//    d->renWin->RemoveRenderer ( d->renderer2d );
//    d->renWin->RemoveRenderer ( d->renderer3d );

//    d->view2d->SetRenderWindow ( NULL );
//    d->view2d->SetRenderWindowInteractor ( NULL );
//    d->view3d->SetRenderWindow ( NULL );
//    d->view3d->SetRenderWindowInteractor ( NULL );

//    d->renWin->Delete();

//    d->view2d->Delete();
//    d->renderer2d->Delete();
//    d->view3d->UnInstallInteractor();
//    d->view3d->Delete();
//    d->renderer3d->Delete();

//    d->collection->Delete();

//    d->observer->Delete();

//    if ( d->widget )
//    {
//        if ( !d->widget->parent() )
//        {
//            // If the widget has no parent then delete now.
//            delete d->widget;
//        }
//        else
//        {
//            // this can only be used if an event loop is (still) running.
//            d->widget->deleteLater();
//        }
//    }

//    delete d;

//    d = NULL;
//}

//bool v3dView::registered()
//{
//    return dtkAbstractViewFactory::instance()->registerViewType ( v3dView::s_identifier(), createV3dView );
//}

//QString v3dView::description() const
//{
//    return tr ( "A view based on vtkInria3d" );
//}


//QString v3dView::identifier() const
//{
//    return v3dView::s_identifier();
//}

//// /////////////////////////////////////////////////////////////////
////
//// /////////////////////////////////////////////////////////////////

//void v3dView::clear()
//{
//    d->collection->SyncSetInput ( 0 ); // to be tested
//}

//void v3dView::reset()
//{
//    if ( !d->collection )
//        return;

//    d->collection->SyncReset();

//    // update slider position
//    if ( d->currentView )
//        d->currentView->GetInteractorStyle()->InvokeEvent ( vtkImageView2DCommand::SliceMoveEvent, NULL );
//}

//void v3dView::update()
//{
//    if ( d->currentView )
//    {
//        d->currentView->Render();
//    }
//    d->vtkWidget->update();
//}

//void v3dView::initializeInteractors()
//{
//    if (d->interactorsInitialized)
//        return;
    
//    foreach ( dtkAbstractViewInteractor *interactor, this->interactors() )
//    {
//        this->enableInteractor(interactor->identifier());
//    }
    
//    d->interactorsInitialized = true;
//}

//void *v3dView::view()
//{
//    return d->currentView;
//}

//vtkImageView2D *v3dView::view2d()
//{
//    return d->view2d;
//}

//vtkImageView3D *v3dView::view3d()
//{
//    return d->view3d;
//}

//vtkImageView *v3dView::currentView()
//{
//    return d->currentView;
//}

//vtkRenderWindowInteractor *v3dView::interactor()
//{
//    return d->renWin->GetInteractor();
//}

//vtkRenderer *v3dView::renderer2d()
//{
//    return d->renderer2d;
//}

//vtkRenderer *v3dView::renderer3d()
//{
//    return d->renderer3d;
//}

//void v3dView::setSharedDataPointer ( dtkSmartPointer<medAbstractData> data )
//{
//    if ( !data )
//        return;
//     int layer = 0, imageLayer = 0;
//     medAbstractData * dataInLayer;
//     while ( (dataInLayer = medAbstractView::dataInList( layer )) )
//     {
//         if(!dataInLayer->identifier().contains ( "vtkDataMesh" ))
//             imageLayer++;

//         layer++;

//     }

//     d->sharedData[layer] = data;

//     this->setData ( data.data(), imageLayer );

//}

//void v3dView::setData ( medAbstractData *data )
//{
//    if(!data)
//        return;

//    /*
//    if(medAbstractView::isInList(data)) // called in setData(data, layer) !
//        return;
//*/

//    ///*
//    //if(medAbstractView::isInList(data)) // called in setData(data, layer) !
//    //    return;
//    //*/

//    //int layer = 0;
//    //while ( d->view2d->GetImageInput ( layer ) )
//    //{
//    //    layer++;
//    //}

//    //if ( data->identifier().contains ( "vtkDataMesh" ) && layer )
//    //{
//    //    layer--;
//    //}

//    //this->setData ( data, layer );

//    //// this->update(); // update is not the role of the plugin, but of the app
//}

////  TO: TODO
////  What to return if the dynamic cast does not work ??

//template <typename IMAGE>
//bool v3dView::SetViewInput(const char* type,medAbstractData* data,const int layer)
//{
//    if (data->identifier()!=type)
//        return false;

//    if (IMAGE* image = dynamic_cast<IMAGE*>((itk::Object*)(data->data()))) {
//        d->view2d->SetITKInput(image,layer);
//        d->view3d->SetITKInput(image,layer);
//    }
//    dtkAbstractView::setData(data);
//    return true;
//}

//bool v3dView::SetView(const char* type,medAbstractData* data)
//{
//    if (data->identifier()!=type)
//        return false;

//    dtkAbstractView::setData(data);
//    return true;
//}

//template <typename IMAGE>
//bool v3dView::SetViewInputWithConversion(const char* type,const char* newtype,medAbstractData* data,const int layer)
//{
//    if (data->identifier()!=type)
//        return false;

//    if (IMAGE* image = dynamic_cast<IMAGE*>((itk::Object*)(data->convert(newtype)->data()))) {
//        d->view2d->SetITKInput(image,layer);
//        d->view3d->SetITKInput(image,layer);
//    }
//    dtkAbstractView::setData(data);
//    return true;
//}

//void v3dView::setData ( medAbstractData *data, int layer )
//{
//    if ( !data )
//        return;

//    if ( medAbstractView::isInList ( data, layer ) )
//        return;

//    if (data->identifier().contains( "vtkDataMesh" ) && medAbstractView::isInList(data))
//    {
//        medMessageController::instance()->showError (tr ( "The mesh is already visualized" ), 5000 );
//        return;
//    }

//    this->initializeInteractors();

//    if (SetViewInput<itk::Image<char,3> >("itkDataImageChar3",data,layer) ||
//        SetViewInput<itk::Image<unsigned char,3> >("itkDataImageUChar3",data,layer) ||
//        SetViewInput<itk::Image<short,3> >("itkDataImageShort3",data,layer) ||
//        SetViewInput<itk::Image<unsigned short,3> >("itkDataImageUShort3",data,layer) ||
//        SetViewInput<itk::Image<int,3> >("itkDataImageInt3",data,layer) ||
//        SetViewInput<itk::Image<unsigned,3> >("itkDataImageUInt3",data,layer) ||
//        SetViewInput<itk::Image<long,3> >("itkDataImageLong3",data,layer) ||
//        SetViewInput<itk::Image<unsigned long,3> >("itkDataImageULong3",data,layer) ||
//        SetViewInput<itk::Image<float,3> >("itkDataImageFloat3",data,layer) ||
//        SetViewInput<itk::Image<double,3> >("itkDataImageDouble3",data,layer) ||
//        SetViewInput<itk::Image<itk::RGBPixel<unsigned char>,3> >("itkDataImageRGB3",data,layer) ||
//        SetViewInput<itk::Image<itk::RGBAPixel<unsigned char>,3> >("itkDataImageRGBA3",data,layer) ||
//        SetViewInput<itk::Image<itk::Vector<unsigned char,3>,3> >("itkDataImageVector3",data,layer) ||
//        SetView("itkDataImageShort4",data) ||
//        SetView("itkDataImageInt4",data) ||
//        SetView("itkDataImageLong4",data) ||
//        SetView("itkDataImageChar4",data) ||
//        SetView("itkDataImageUShort4",data) ||
//        SetView("itkDataImageUInt4",data) ||
//        SetView("itkDataImageULong4",data) ||
//        SetView("itkDataImageUChar4",data) ||
//        SetView("itkDataImageFloat4",data) ||
//        SetView("itkDataImageDouble4",data) ||
//        SetViewInputWithConversion<itk::Image<char,3> >("vistalDataImageChar3","itkDataImageChar3",data,layer) ||
//        SetViewInputWithConversion<itk::Image<unsigned char,3> >("vistalDataImageUChar3","itkDataImageUChar3",data,layer) ||
//        SetViewInputWithConversion<itk::Image<short,3> >("vistalDataImageShort3","itkDataImageShort3",data,layer) ||
//        SetViewInputWithConversion<itk::Image<unsigned short,3> >("vistalDataImageUShort3","itkDataImageUShort3",data,layer) ||
//        SetViewInputWithConversion<itk::Image<int,3> >("vistalDataImageInt3","itkDataImageInt3",data,layer) ||
//        SetViewInputWithConversion<itk::Image<unsigned,3> >("vistalDataImageUInt3","itkDataImageUInt3",data,layer) ||
//        SetViewInputWithConversion<itk::Image<float,3> >("vistalDataImageFloat3","itkDataImageFloat3",data,layer) ||
//        SetViewInputWithConversion<itk::Image<double,3> >("vistalDataImageDouble3","itkDataImageDouble3",data,layer)) {

//    }
//    else if (data->identifier()=="v3dDataImage")
//    {
//        if(vtkImageData *dataset = dynamic_cast<vtkImageData*>((vtkDataObject *)(data->data())))
//        {
//            d->view2d->SetInput(dataset, 0, layer);
//            d->view3d->SetInput(dataset, 0, layer);
//        }
//    }
//    else
//    {
//        bool isDataTypeHandled = false;
//        foreach ( dtkAbstractViewInteractor *interactor, this->interactors() )
//        {
//            medAbstractViewInteractor *medInteractor = dynamic_cast <medAbstractViewInteractor *> (interactor);
//            if (medInteractor->isDataTypeHandled(data->identifier()))
//            {
//                isDataTypeHandled = true;
//                break;
//            }
//        }
        
//        dtkAbstractView::setData(data);
//        if (!isDataTypeHandled)
//            return;
//    }

//    if ( layer==0 )
//    {
//        if ( medAbstractImageData *imageData = dynamic_cast<medAbstractImageData*> ( data ) )
//        {
//            d->data = data;
//            d->imageData = imageData;

//            if ( data->hasMetaData ( medMetaDataKeys::PatientName.key() ) )
//            {
//                const QString patientName = data->metaDataValues ( medMetaDataKeys::PatientName.key() ) [0];
//                d->view2d->SetPatientName ( patientName.toAscii().constData() );
//                d->view3d->SetPatientName ( patientName.toAscii().constData() );
//            }

//            if ( data->hasMetaData ( medMetaDataKeys::StudyDescription.key() ) )
//            {
//                const QString studyName = data->metaDataValues ( medMetaDataKeys::StudyDescription.key() ) [0];
//                d->view2d->SetStudyName ( studyName.toAscii().constData() );
//                d->view3d->SetStudyName ( studyName.toAscii().constData() );
//            }

//            if ( data->hasMetaData ( medMetaDataKeys::SeriesDescription.key() ) )
//            {
//                const QString seriesName = data->metaDataValues ( medMetaDataKeys::SeriesDescription.key() ) [0];
//                d->view2d->SetSeriesName ( seriesName.toAscii().constData() );
//                d->view3d->SetSeriesName ( seriesName.toAscii().constData() );
//            }

//            dtkSignalBlocker blocker (d->slider );
//            // slice orientation may differ from view orientation. Adapt slider range accordingly.
//            int orientationId = d->view2d->GetSliceOrientation();
//            if (orientationId==vtkImageView2D::SLICE_ORIENTATION_XY)
//                d->slider->setRange (0, d->imageData->zDimension()-1);
//            else if (orientationId==vtkImageView2D::SLICE_ORIENTATION_XZ)
//                d->slider->setRange (0, d->imageData->yDimension()-1);
//            else if (orientationId==vtkImageView2D::SLICE_ORIENTATION_YZ)
//                d->slider->setRange (0, d->imageData->xDimension()-1);
//        }
//    }

//    this->addDataInList ( data);
//    setCurrentLayer(layer);
//    emit dataAdded ( data );
//    emit dataAdded ( data, layer );

//    if (layer ==0 )
//    {
//        if ( dynamic_cast<medAbstractImageData*> ( data ) )
//        {
//            if ( d->view2d )
//            {
//                switch ( d->view2d->GetViewOrientation() )
//                {
//                case vtkImageView2D::VIEW_ORIENTATION_SAGITTAL:
//                    this->setProperty("Orientation","Sagittal");
//                    break;
//                case vtkImageView2D::VIEW_ORIENTATION_CORONAL:
//                    this->setProperty("Orientation","Coronal");
//                    break;
//                case vtkImageView2D::VIEW_ORIENTATION_AXIAL:
//                    this->setProperty("Orientation","Axial");
//                    break;
//                }
//            }
//        }
//    }
//}

//void *v3dView::data()
//{
//    return d->data;
//}

//QWidget *v3dView::receiverWidget()
//{
//    return d->vtkWidget;
//}

//QWidget *v3dView::widget()
//{
//    return d->widget;
//}

//void v3dView::play ( bool start )
//{
//    d->timeline->setFrameRange ( d->slider->minimum(), d->slider->maximum() );

//    if ( start )
//        d->timeline->start();
//    else
//        d->timeline->stop();
//}

//void v3dView::onPropertySet ( const QString &key, const QString &value )
//{
//// Look up which property set function to call from table
//    v3dViewPrivate::PropertyFuncMapType::const_iterator it = d->setPropertyFunctions.find ( key );

//    if ( it != d->setPropertyFunctions.end() )
//    {
//        // Get member function pointer and call it.

//        const v3dViewPrivate::PropertyFuncType funcPtr = it.value();
//        ( this->*funcPtr ) ( value );
//    }

//    // never update after setting a property, it is not our role
//    //this->update();
//}

//void v3dView::setOrientation ( const QString &value )
//{
//    if ( value==d->orientation )
//        return;

//    dtkSignalBlocker thisBlocker ( this );

//    double pos[3];
//    QVector <double> windows;
//    QVector <double> levels;
//    int timeIndex = 0;
//    if ( d->currentView )
//    {
//        d->currentView->GetCurrentPoint ( pos );
//        //fo the moment only do this if going to 3d mode:
//        if (d->currentView == d->view2d)
//        {
//            for (int i =0; i < d->currentView->GetNumberOfLayers(); i++)
//            {
//                windows.push_back( d->view2d->GetColorWindow(i));
//                levels.push_back( d->view2d->GetColorLevel(i));
//            }
//        }
//        timeIndex = d->currentView->GetTimeIndex();

//        d->currentView->UnInstallInteractor();
//        d->currentView->SetRenderWindow ( 0 );

//        d->renWin->RemoveRenderer ( d->currentView->GetRenderer() );
//    }

//    if ( value=="3D" )
//    {
//        d->orientation = "3D";
//        d->currentView = d->view3d;
//    }

//    // in case the max range becomes smaller than the actual value, a signal is emitted and
//    // we don't want it
//    dtkSignalBlocker sliderBlocker ( d->slider );

//    if ( value == "Axial" )
//    {
//        d->orientation = "Axial";
//        d->view2d->SetViewOrientation ( vtkImageView2D::VIEW_ORIENTATION_AXIAL );
//        d->currentView = d->view2d;
//    }

//    if ( value == "Sagittal" )
//    {
//        d->orientation = "Sagittal";
//        d->view2d->SetViewOrientation ( vtkImageView2D::VIEW_ORIENTATION_SAGITTAL );
//        d->currentView = d->view2d;
//    }

//    if ( value == "Coronal" )
//    {
//        d->orientation = "Coronal";
//        d->view2d->SetViewOrientation ( vtkImageView2D::VIEW_ORIENTATION_CORONAL );
//        d->currentView = d->view2d;
//    }

//    if ( value == "Axial"   ||
//            value == "Coronal" ||
//            value == "Sagittal" )
//    {
//        if ( d->imageData )
//        {
//            // slice orientation may differ from view orientation. Adapt slider range accordingly.
//            int orientationId = d->view2d->GetSliceOrientation();
//            if ( orientationId==vtkImageView2D::SLICE_ORIENTATION_XY )
//                d->slider->setRange ( 0, d->imageData->zDimension()-1 );
//            else if ( orientationId==vtkImageView2D::SLICE_ORIENTATION_XZ )
//                d->slider->setRange ( 0, d->imageData->yDimension()-1 );
//            else if ( orientationId==vtkImageView2D::SLICE_ORIENTATION_YZ )
//                d->slider->setRange ( 0, d->imageData->xDimension()-1 );
//        }

//    }

//    if ( !d->currentView )
//    {
//        d->slider->blockSignals ( false );
//        return;
//    }

//    d->currentView->SetRenderWindow ( d->renWin );

//    //d->observer->setView ( d->currentView );
//    d->currentView->SetCurrentPoint (pos);
//    //for the moment only act when going to 3d mode:
//    //update the color level and window
//    if (d->currentView == d->view3d)
//    {
//        for (int i=0; i < d->currentView->GetNumberOfLayers();i++)
//        {
//            d->currentView->SetColorWindow ( windows.at(i),i );
//            d->currentView->SetColorLevel ( levels.at(i),i );
//        }
//    }
//    d->currentView->SetTimeIndex ( timeIndex );


//    // force a correct display of the 2D axis for planar views
//    d->currentView->InvokeEvent ( vtkImageView::CurrentPointChangedEvent, NULL ); // seems not needed anymore

//    // update slider position
//    if ( vtkImageView2D *view2d = vtkImageView2D::SafeDownCast ( d->currentView ) )
//    {
//        unsigned int zslice = view2d->GetSlice();
//        d->slider->setValue ( zslice );
//    }
//}

//void v3dView::set3DMode ( const QString &value )
//{
//    if ( value=="VR" )
//    {
//        d->view3d->SetRenderingModeToVR();
//        d->view3d->SetVolumeRayCastFunctionToComposite();
//    }

//    if ( value=="MPR" )
//    {
//        d->view3d->SetRenderingModeToPlanar();
//        d->view3d->ShowActorXOn();
//        d->view3d->ShowActorYOn();
//        d->view3d->ShowActorZOn();
//    }

//    if ( value=="MIP - Maximum" )
//    {
//        d->view3d->SetRenderingModeToVR();
//        d->view3d->SetVolumeRayCastFunctionToMaximumIntensityProjection();
//    }

//    if ( value=="MIP - Minimum" )
//    {
//        d->view3d->SetRenderingModeToVR();
//        d->view3d->SetVolumeRayCastFunctionToMinimumIntensityProjection();
//    }

//    if ( value=="Off" )
//    {
//        d->view3d->SetRenderingModeToPlanar();
//        d->view3d->ShowActorXOff();
//        d->view3d->ShowActorYOff();
//        d->view3d->ShowActorZOff();
//    }
//}

//void v3dView::setRenderer ( const QString &value )
//{
//    if ( value=="GPU" )
//        d->view3d->SetVolumeMapperToGPU();

//    if ( value=="Ray Cast / Texture" )
//        d->view3d->SetVolumeMapperToRayCastAndTexture();

//    if ( value=="Ray Cast" )
//        d->view3d->SetVolumeMapperToRayCast();

//    if ( value=="Default" )
//        d->view3d->SetVolumeMapperToDefault();
//}


//void v3dView::setDepthPeeling ( const bool &value )
//{
//    if ( value )
//    {
//        // Activate depth-peeling to have a proper opacity rendering
//        d->renderer3d->SetUseDepthPeeling(1);
//        d->renderer3d->SetMaximumNumberOfPeels(100);
//        d->renderer3d->SetOcclusionRatio(0.01);
//    }

//    else  d->renderer3d->SetUseDepthPeeling(0);
//}


//void v3dView::showScalarBar ( const bool &value )
//{
//    if ( value )
//        d->collection->SyncSetShowScalarBar(true);
    
//    else d->collection->SyncSetShowScalarBar(false);
//}

//QString v3dView::getLUT ( int layer ) const
//{
//    if ( layer < d->LUTList.size() )
//        return d->LUTList.at ( layer );
//    else
//        return "Default";

//}

//void v3dView::showAxis ( const bool &value )
//{
//    if ( value )
//    {
//        d->collection->SyncSetShowImageAxis ( 1 );
//        if ( d->currentView )
//        {
//            d->currentView->InvokeEvent ( vtkImageView2D::CurrentPointChangedEvent );
//        }
//    }

//    else d->collection->SyncSetShowImageAxis ( 0 );
//}

//void v3dView::showRuler ( const bool &value )
//{
//    d->collection->SyncSetShowRulerWidget ( value );
//}

//void v3dView::showAnnotations ( const bool &value )
//{
//    d->collection->SyncSetShowAnnotations ( value );
//}

//void v3dView::setMouseInteraction ( const QString &value )
//{
//    d->collection->SyncSetMiddleButtonInteractionStyle ( vtkInteractorStyleImageView2D::InteractionTypePan );

//    if ( value == "Zooming" )
//    {
//        d->collection->SyncSetLeftButtonInteractionStyle ( vtkInteractorStyleImageView2D::InteractionTypeZoom );
//    }

//    if ( value == "Windowing" )
//    {
//        d->collection->SyncSetLeftButtonInteractionStyle ( vtkInteractorStyleImageView2D::InteractionTypeWindowLevel );
//    }

//    if ( value == "Slicing" )
//    {
//        d->collection->SyncSetLeftButtonInteractionStyle ( vtkInteractorStyleImageView2D::InteractionTypeSlice );
//    }

//    if ( value == "Measuring" )
//    {
//        d->view2d->ShowDistanceWidgetOn();
//    }
//    else
//    {
//        d->view2d->ShowDistanceWidgetOff();
//    }
//}

//void v3dView::setZoomMode ( const QString &value )
//{
//    if ( value=="RubberBand" )
//    {
//        vtkInriaInteractorStyleRubberBandZoom * interactorStyle = vtkInriaInteractorStyleRubberBandZoom::New();
//        interactorStyle->AddObserver( vtkImageView2DCommand::CameraZoomEvent,d->observer,0 );
//        interactorStyle->AddObserver( vtkImageView2DCommand::CameraPanEvent,d->observer,0 );
//        d->view2d->GetInteractor()->SetInteractorStyle(interactorStyle);
//        interactorStyle->Delete();
//    }
//    else
//        d->view2d->GetInteractor()->SetInteractorStyle(d->interactorStyle2D);
//}

//QString v3dView::getPreset ( int layer ) const
//{
//    if ( layer < d->PresetList.size() )
//        return d->PresetList.at ( layer );
//    else
//        return "Default";
//}

//void v3dView::setCropping ( const bool &value )
//{
//    if ( value )
//    {
//        if ( d->view3d->GetBoxWidget()->GetInteractor() ) // avoid VTK warnings
//        {
//            d->view3d->SetCroppingModeToOutside();
//            d->view3d->SetShowBoxWidget ( 1 );
//        }
//    }
//    else
//    {
//        if ( d->view3d->GetBoxWidget()->GetInteractor() )
//        {
//            d->view3d->SetShowBoxWidget ( 0 );
//        }
//    }
//}

///**
//* Slot called to visualize a specific slice
//* @param value - the slice number
//**/
//void v3dView::setSlider( int value)
//{
//    d->slider->setSliderPosition(value);
//    disconnect(sender(), SIGNAL(sliceSelected(int)), this, 0);
//}

//void v3dView::onZSliderValueChanged ( int value )
//{
//    if ( !d->currentView )
//        return;

//    if ( vtkImageView2D *view = vtkImageView2D::SafeDownCast ( d->currentView ) )
//    {
//        d->observer->lock();
//        view->SetSlice ( value );
//        d->observer->unlock();

//        double *pos = view->GetCurrentPoint();
//        QVector3D position ( pos[0], pos[1], pos[2] );
//        emit positionChanged ( position, this->positionLinked() );
//    }

//    d->currentView->Render();

//}


//void v3dView::onClosablePropertySet( const QString &value ){

//    if ( value == "true" )
//        d->closeButton->show();
//    else if ( value == "false" )
//        d->closeButton->hide();
//}


//// /////////////////////////////////////////////////////////////////
//// Type instantiation
//// /////////////////////////////////////////////////////////////////

//dtkAbstractView *createV3dView()
//{
//    return new v3dView;
//}

//QStringList v3dView::getAvailableTransferFunctionPresets()
//{
//    QStringList lut;
//    typedef std::vector< std::string > StdStrVec;
//    StdStrVec presets = vtkTransferFunctionPresets::GetAvailablePresets();
//    for ( StdStrVec::iterator it ( presets.begin() ), end ( presets.end() );
//            it != end; ++it )
//        lut << QString::fromStdString ( * it );

//    return lut;
//}

//void v3dView::getTransferFunctions ( QList<double> & scalars,
//                                     QList<QColor> & colors )
//{
//    vtkColorTransferFunction * color   =
//        d->currentView->GetColorTransferFunction();
//    vtkPiecewiseFunction     * opacity =
//        d->currentView->GetOpacityTransferFunction();

//    if ( color == NULL || opacity == NULL )
//        return;

//    if ( color->GetSize() != opacity->GetSize() )
//        qWarning() << Q_FUNC_INFO << " sizes of color and opacity transfer "
//        "functions don't match!";
//    int size = qMin ( color->GetSize(), opacity->GetSize() );

//    scalars.clear();
//    colors.clear();

//    bool ok = true;
//    for ( int i = 0; i < size; i++ )
//    {
//        double xrgb[6], xalpha[4];
//        color->GetNodeValue ( i, xrgb );
//        opacity->GetNodeValue ( i, xalpha );
//        if ( xrgb[0] != xalpha[0] )
//            ok = false;

//        scalars << xrgb[0];
//        QColor c;
//        c.setRgbF ( xrgb[1], xrgb[2], xrgb[3], xalpha[1] );
//        colors << c;
//    }

//    if ( !ok )
//        qWarning() << Q_FUNC_INFO << " x values of color and opacity transfer "
//        "functions don't match!";
//}

//void v3dView::setTransferFunctions ( QList< double > scalars,
//                                     QList< QColor > colors )
//{
//    int size = qMin ( scalars.count(), colors.count() );
//    vtkColorTransferFunction * color   = vtkColorTransferFunction::New();
//    vtkPiecewiseFunction     * opacity = vtkPiecewiseFunction::New();

//    for ( int i = 0; i < size; i++ )
//    {
//        color->AddRGBPoint ( scalars.at ( i ),
//                             colors.at ( i ).redF(),
//                             colors.at ( i ).greenF(),
//                             colors.at ( i ).blueF() );
//        opacity->AddPoint ( scalars.at ( i ), colors.at ( i ).alphaF() );
//    }

//    double * range = color->GetRange();
//    d->collection->SyncSetColorRange ( range );

//    d->collection->SyncSetColorTransferFunction ( color );
//    d->collection->SyncSetOpacityTransferFunction ( opacity );

//    color->Delete();
//    opacity->Delete();
//}

//void v3dView::setColorLookupTable ( QList<double> scalars, QList<QColor> colors )
//{
//    int size= qMin ( scalars.count(),colors.count() );
//    vtkColorTransferFunction * ctf = vtkColorTransferFunction::New();
//    vtkPiecewiseFunction * pf = vtkPiecewiseFunction::New();
//    for ( int i=0;i<size;i++ )
//    {
//        ctf->AddRGBPoint ( scalars.at ( i ),
//                           colors.at ( i ).redF(),
//                           colors.at ( i ).greenF(),
//                           colors.at ( i ).blueF() );
//        pf->AddPoint ( scalars.at ( i ),colors.at ( i ).alphaF() );
//    }

//    double min = scalars.first();
//    double max = scalars.last();
//    int n = static_cast< int > ( max - min ) + 1;
//    n = std::max(n, size);
//    double * table = new double[3*n];
//    double * alphaTable = new double[n];
//    ctf->GetTable ( min, max, n, table );
//    ctf->Delete();
//    pf->GetTable ( min,max,n,alphaTable );
//    pf->Delete();

//    vtkLookupTable * lut = vtkLookupTable::New();
//    lut->SetNumberOfTableValues ( n + 2 );
//    lut->SetTableRange ( min - 1.0, max + 1.0 );

//    lut->SetTableValue ( 0, 0.0, 0.0, 0.0, 0.0 );
//    for ( int i = 0, j = 0; i < n; ++i, j += 3 )
//        lut->SetTableValue ( i+1, table[j], table[j+1], table[j+2], alphaTable[i] );
//    lut->SetTableValue ( n + 1, 0.0, 0.0, 0.0, 0.0 );

//    if ( d->currentView == d->view2d ) {
//        d->view2d->SetLookupTable( lut , this->currentLayer() );
//    } else {
//        d->currentView->SetLookupTable ( lut );
//    }
//    d->currentView->Render();
//    lut->Delete();
//    delete [] table;
//    delete [] alphaTable;
//}

//bool v3dView::visibility ( int layer ) const
//{
//    return ( d->view2d->GetVisibility ( layer ) == 1 );
//}

//double v3dView::opacity ( int layer ) const
//{
//    return d->view2d->GetOpacity ( layer );
//}

//int v3dView::layerCount() const
//{
//    return d->view2d->GetNumberOfLayers();
//}

//void v3dView::removeOverlay ( int layer )
//{
//    d->view2d->RemoveLayer ( layer );
//    d->view3d->RemoveLayer ( layer );
//    medAbstractView::removeOverlay ( layer );
//}

//// -- head tracking support

//void v3dView::enableInteraction()
//{
//    if ( this->property ( "Orientation" ) != "3D" )
//        return;

//    d->widget->setAttribute ( Qt::WA_TransparentForMouseEvents, false );
//}

//void v3dView::disableInteraction()
//{
//    if ( this->property ( "Orientation" ) != "3D" )
//        return;

//    d->widget->setAttribute ( Qt::WA_TransparentForMouseEvents, true );
//}

//void v3dView::bounds ( float& xmin, float& xmax, float& ymin, float& ymax, float& zmin, float& zmax )
//{
//    if ( this->property ( "Orientation" ) == "Axial" )
//    {
//        double bounds[6];
//        d->renderer2d->ComputeVisiblePropBounds ( bounds );

//        xmin = bounds[0];
//        xmax = bounds[1];
//        ymin = bounds[2];
//        ymax = bounds[3];
//        zmin = bounds[4];
//        zmax = bounds[5];
//    }

//    if ( this->property ( "Orientation" ) == "Sagittal" )
//    {
//        double bounds[6];
//        d->renderer2d->ComputeVisiblePropBounds ( bounds );

//        xmin = bounds[0];
//        xmax = bounds[1];
//        ymin = bounds[2];
//        ymax = bounds[3];
//        zmin = bounds[4];
//        zmax = bounds[5];
//    }

//    if ( this->property ( "Orientation" ) == "Coronal" )
//    {
//        double bounds[6];
//        d->renderer2d->ComputeVisiblePropBounds ( bounds );

//        xmin = bounds[0];
//        xmax = bounds[1];
//        ymin = bounds[2];
//        ymax = bounds[3];
//        zmin = bounds[4];
//        zmax = bounds[5];
//    }

//    if ( this->property ( "Orientation" ) == "3D" )
//    {
//        double bounds[6];
//        d->renderer3d->ComputeVisiblePropBounds ( bounds );

//        xmin = bounds[0];
//        xmax = bounds[1];
//        ymin = bounds[2];
//        ymax = bounds[3];
//        zmin = bounds[4];
//        zmax = bounds[5];
//    }
//}

//void v3dView::cameraUp ( double *coordinates )
//{
//    if ( d->orientation != "3D" )
//    {
//        vtkCamera *camera = d->renderer2d->GetActiveCamera();
//        camera->GetViewUp ( coordinates );
//    }
//    else
//    {
//        vtkCamera *camera = d->renderer3d->GetActiveCamera();
//        camera->GetViewUp ( coordinates );
//    }
//}

//void v3dView::cameraPosition ( double *coordinates )
//{
//    if ( d->orientation != "3D" )
//    {
//        vtkCamera *camera = d->renderer2d->GetActiveCamera();
//        camera->GetPosition ( coordinates );
//    }
//    else
//    {
//        vtkCamera *camera = d->renderer3d->GetActiveCamera();
//        camera->GetPosition ( coordinates );
//    }
//}

//void v3dView::cameraFocalPoint ( double *coordinates )
//{
//    if ( d->orientation != "3D" )
//    {
//        vtkCamera *camera = d->renderer2d->GetActiveCamera();
//        camera->GetFocalPoint ( coordinates );
//    }
//    else
//    {
//        vtkCamera *camera = d->renderer3d->GetActiveCamera();
//        camera->GetFocalPoint ( coordinates );
//    }
//}

//void v3dView::setCameraPosition ( double x, double y, double z )
//{
//    if ( d->orientation != "3D" )
//        return;

//    vtkCamera *camera = d->renderer3d->GetActiveCamera();

//    camera->SetPosition ( x, y, z );

//    d->renderer3d->ResetCameraClippingRange();
//}

//// Avoid using the variable names 'near' and 'far' as windows #defines them out of existence.

//void v3dView::setCameraClippingRange ( double nearRange, double farRange )
//{
//    if ( d->orientation != "3D" )
//        return;

//    vtkCamera *camera = d->renderer3d->GetActiveCamera();

//    camera->SetClippingRange ( nearRange, farRange );
//}

//QString v3dView::cameraProjectionMode()
//{
//    vtkCamera *camera = NULL;

//    if ( d->orientation != "3D" )
//        camera = d->renderer2d->GetActiveCamera();

//    else camera = d->renderer3d->GetActiveCamera();

//    if ( !camera )
//        return QString ( "None" );

//    if ( camera->GetParallelProjection() )
//        return QString ( "Parallel" );
//    else
//        return QString ( "Perspective" );
//}

//double v3dView::cameraViewAngle()
//{
//    vtkCamera *camera = NULL;

//    if ( d->orientation != "3D" )
//        camera = d->renderer2d->GetActiveCamera();

//    else camera = d->renderer3d->GetActiveCamera();

//    if ( !camera )
//        return 0.0;
//    else
//        return camera->GetViewAngle();
//}

//double v3dView::cameraZoom()
//{
//    vtkImageView *view = NULL;

//    if ( d->orientation != "3D" )
//        view = d->view2d;

//    else  view = d->view3d;

//    if ( !view )
//        return 1.0;
//    else
//        return view->GetZoom();
//}

//void v3dView::close()
//{
//    d->widget->close();
//    medAbstractView::close();
//}

//void v3dView::onPositionChanged ( const QVector3D &position )
//{
//    double pos[3];
//    pos[0] = position.x();
//    pos[1] = position.y();
//    pos[2] = position.z();
//    d->observer->lock();
//    d->currentView->SetCurrentPoint ( pos );
//    d->currentView->UpdateCursorPosition(pos);
//    d->observer->unlock();

//    // update slider, if currentView is 2D view
//    if ( vtkImageView2D *view2d = vtkImageView2D::SafeDownCast ( d->currentView ) )
//    {
//        unsigned int zslice = view2d->GetSlice();
//        dtkSignalBlocker sliderBlocker( d->slider );
//        d->slider->setValue ( zslice );
//    }
//}

//void v3dView::onZoomChanged ( double zoom )
//{
//    d->observer->lock();
//    d->view2d->SetZoom ( zoom );
//    d->observer->unlock();
//}

//void v3dView::onPanChanged ( const QVector2D &pan )
//{
//    double ppan[2];
//    ppan[0] = pan.x();
//    ppan[1] = pan.y();

//    d->observer->lock();
//    d->view2d->SetPan ( ppan );
//    d->observer->unlock();
//}

//void v3dView::onWindowingChanged ( double level, double window )
//{
//    d->observer->lock();
//    d->currentView->SetColorWindow ( window );
//    d->currentView->SetColorLevel ( level );
//    d->observer->unlock();
//}

//void v3dView::onCameraChanged ( const QVector3D &position, const QVector3D &viewup, const QVector3D &focal, double parallelScale )
//{
//    double pos[3], vup[3], foc[3];
//    pos[0] = position.x();
//    pos[1] = position.y();
//    pos[2] = position.z();

//    vup[0] = viewup.x();
//    vup[1] = viewup.y();
//    vup[2] = viewup.z();

//    foc[0] = focal.x();
//    foc[1] = focal.y();
//    foc[2] = focal.z();

//    d->observer->lock();
//    d->renderer3d->GetActiveCamera()->SetPosition ( pos );
//    d->renderer3d->GetActiveCamera()->SetViewUp ( vup );
//    d->renderer3d->GetActiveCamera()->SetFocalPoint ( foc );
//    d->renderer3d->GetActiveCamera()->SetParallelScale ( parallelScale );

//    d->renderer3d->ResetCameraClippingRange();
//    d->observer->unlock();

//    d->view3d->Modified();
//}

//void v3dView::widgetDestroyed()
//{
//    d->widget = NULL;
//}

//bool v3dView::eventFilter(QObject * obj, QEvent * event)
//{
//    if (obj == d->vtkWidget) {
//        if (event->type() == QEvent::FocusIn) {
//            emit selected();
//        } else if (event->type() == QEvent::FocusOut) {
//            emit unselected();
//        }
//    }
//    return medAbstractView::eventFilter(obj, event);
//}

//medAbstractViewCoordinates * v3dView::coordinates()
//{
//    return this;
//}

//medViewBackend * v3dView::backend() const
//{
//    return d->backend.data();
//}

//QPointF v3dView::worldToDisplay( const QVector3D & worldVec ) const
//{
//    // The following code is implemented without calling ren->SetWorldPoint,
//    // because that generates an unnecessary modified event.

//    vtkRenderer * ren = d->currentView->GetRenderer();

//    // Get window for dimensions
//    vtkWindow * win = ren->GetVTKWindow();

//    if ( !win )
//        return QPointF();

//    double wx = worldVec.x();
//    double wy = worldVec.y();
//    double wz = worldVec.z();

//    ren->WorldToView( wx, wy, wz );

//    // get physical window dimensions
//    const int * size = win->GetSize();
//    int sizex = size[0];
//    int sizey = size[1];

//    const double * viewport = ren->GetViewport( );

//    double dx = (wx + 1.0) *
//        (sizex*(viewport[2]-viewport[0])) / 2.0 +
//        sizex*viewport[0];
//    double dy = (wy + 1.0) *
//        (sizey*(viewport[3]-viewport[1])) / 2.0 +
//        sizey*viewport[1];

//    // Convert VTK display coordinates to Qt (flipped in Y)
//    return QPointF( dx, sizey - 1 - dy );
//}

//QVector3D v3dView::displayToWorld( const QPointF & scenePoint ) const
//{
//    // The following code is implemented without calling ren->SetWorldPoint,
//    // because that generates an unnecessary modified event.

//    vtkRenderer * ren = d->currentView->GetRenderer();

//    /* get physical window dimensions */
//    vtkWindow * win = ren->GetVTKWindow();

//    if ( !win )
//        return QVector3D();

//    const int * size = win->GetSize();
//    int sizex = size[0];
//    int sizey = size[1];

//    const double * viewport = ren->GetViewport( );

//    // Convert Qt display coordinates to VTK (flipped in Y)
//    const double dx = scenePoint.x();
//    const double dy = sizey - 1 - scenePoint.y();

//    double vx = 2.0 * (dx - sizex*viewport[0])/
//        (sizex*(viewport[2]-viewport[0])) - 1.0;
//    double vy = 2.0 * (dy - sizey*viewport[1])/
//        (sizey*(viewport[3]-viewport[1])) - 1.0;
//    double vz = 0.;

//    if (this->is2D() ) {
//        // Project the point into the view plane.
//        //vtkCamera * cam = ren->GetActiveCamera();
//        double pointInDisplayPlane[3];

//        d->currentView->GetCurrentPoint(pointInDisplayPlane);

//        ren->WorldToView(pointInDisplayPlane[0],pointInDisplayPlane[1],pointInDisplayPlane[2]);
//        vz = pointInDisplayPlane[2];
//    }

//    ren->ViewToWorld(vx,vy,vz);

//    return QVector3D( vx, vy, vz );
//}

//QVector3D v3dView::viewCenter() const
//{
//    vtkRenderer * ren = d->currentView->GetRenderer();
//    double fp[3];
//    ren->GetActiveCamera()->GetFocalPoint( fp);
//    return QVector3D( fp[0], fp[1], fp[2] );
//}

//QVector3D v3dView::viewPlaneNormal() const
//{
//    double vpn[3];
//    vtkRenderer * ren = d->currentView->GetRenderer();
//    ren->GetActiveCamera()->GetViewPlaneNormal(vpn);
//    return QVector3D( vpn[0], vpn[1], vpn[2] );
//}

//QVector3D v3dView::viewUp() const
//{
//    double vup[3];
//    vtkRenderer * ren = d->currentView->GetRenderer();
//    ren->GetActiveCamera()->GetViewUp(vup);
//    return QVector3D( vup[0], vup[1], vup[2] );
//}

//bool v3dView::is2D() const
//{
//    return d->currentView == d->view2d;
//}

//qreal v3dView::sliceThickness() const
//{
//    double cr[2] = { 0,0 };
//    d->renderer2d->GetActiveCamera()->GetClippingRange(cr);
//    return std::fabs(cr[1] - cr[0]);
//}

//qreal v3dView::scale() const
//{
//    double scale;
//    if ( this->is2D() ) {
//        // The height of the viewport in world coordinates
//        double camScale = d->currentView->GetRenderer()->GetActiveCamera()->GetParallelScale();
//        double heightInPx = d->currentView->GetRenderWindow()->GetSize()[1];
//        // return pixels per world coord.
//        scale = heightInPx / camScale;
//    } else {
//        // Return scale at fp.
//        double vup[4];
//        d->currentView->GetRenderer()->GetActiveCamera()->GetViewUp(vup);
//        vup[3] = 0;  //intentionally zero and not one.
//        double MVup[4];
//        d->currentView->GetRenderer()->GetActiveCamera()->GetViewTransformMatrix()->MultiplyPoint(vup, MVup);
//        double lScale = vtkMath::Norm(MVup) / vtkMath::Norm(vup);
//        //We now have the scale in viewport coords. (normalised).
//        double heightInPx = d->currentView->GetRenderWindow()->GetSize()[1];
//        scale = heightInPx *lScale;
//    }

//    if ( scale < 0 )
//        scale *= -1;
//    return scale;
//}

//QString v3dView::s_identifier()
//{
//    return "v3dView";
//}

//void v3dView::setCurrentLayer(int layer)
//{
//    medAbstractView::setCurrentLayer(layer);
//    d->view2d->SetCurrentLayer(layer);
//    d->view3d->SetCurrentLayer(layer);
//}

//void v3dView::onMainWindowDeactivated()
//{
//    //This function must contains all the different actions that we want to happen in case the software loses the focus
//    if (property("ZoomMode")=="RubberBand")
//        setZoomMode("Normal");
//}