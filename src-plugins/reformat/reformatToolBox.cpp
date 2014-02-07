#include "reformatToolBox.h"
#include <reformatWorkspace.h>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStringList>

#include <medToolBoxFactory.h>
#include <medPluginManager.h>

#include <medDataManager.h>
#include <medMetaDataKeys.h>
#include <dtkCore/dtkAbstractDataFactory.h>
#include <medReformatViewer.h>
#include <medAbstractView.h>
#include <medTabbedViewContainers.h>
#include <QGroupBox>
#include <medVtkViewBackend.h>
#include <vtkImageView2D.h>
#include <vtkImageData.h>

class reformatToolBoxPrivate
{
public:
    QPushButton * b_startReformat,* b_saveImage,* b_reset;
    QLabel *spacingXLab,*spacingYLab,*spacingZLab;
    QDoubleSpinBox * spacingX,* spacingY,* spacingZ;
    medAbstractView * currentView;
    medWorkspace * workspace;
    QGroupBox * thickMode;
    QSlider * thickSlab;
    /*QGroupBox * blendModes;*/
    QRadioButton * maxIP,*minIP,*meanIP;
    medReformatViewer * reformatViewer;
};

reformatToolBox::reformatToolBox (QWidget *parent) : medToolBox (parent), d(new reformatToolBoxPrivate)
{
    this->setTitle("Reformat");
    this->setAboutPluginVisibility(false);
    this->setAboutPluginButton(this->plugin());
    // Fill the toolBox
    QWidget *reformatToolBoxBody = new QWidget(this);
    d->b_startReformat = new QPushButton("Start Reformat", reformatToolBoxBody);
    d->b_startReformat->setCheckable(true);
    d->b_saveImage = new QPushButton("Save Image", reformatToolBoxBody);
    d->b_saveImage->setCheckable(false);
    
    QWidget * spinBoxes = new QWidget(reformatToolBoxBody);
    QHBoxLayout * spacingSpinBoxLayout = new QHBoxLayout(reformatToolBoxBody);
    d->spacingXLab = new QLabel("SpacingX : ");
    d->spacingXLab->setEnabled(false);
    d->spacingX = new QDoubleSpinBox(reformatToolBoxBody);
    d->spacingX->setAccessibleName("SpacingX");
    d->spacingX->setRange(0,10000);
    d->spacingX->setSuffix(" mm");
    d->spacingX->setSingleStep(0.1f);
    d->spacingX->setEnabled(false);
    spacingSpinBoxLayout->addWidget(d->spacingXLab);
    spacingSpinBoxLayout->addWidget(d->spacingX);
    d->spacingYLab = new QLabel("SpacingY : ");
    d->spacingYLab->setEnabled(false);
    d->spacingY = new QDoubleSpinBox(reformatToolBoxBody);
    d->spacingY->setAccessibleName("SpacingY");
    d->spacingY->setRange(0,10000);
    d->spacingY->setSuffix(" mm");
    d->spacingY->setSingleStep(0.1f);
    d->spacingY->setEnabled(false);
    spacingSpinBoxLayout->addWidget(d->spacingYLab);
    spacingSpinBoxLayout->addWidget(d->spacingY);
    d->spacingZLab = new QLabel("SpacingZ : ");
    d->spacingZLab->setEnabled(false);
    d->spacingZ = new QDoubleSpinBox(reformatToolBoxBody);
    d->spacingZ->setAccessibleName("SpacingZ");
    d->spacingZ->setRange(0,10000);
    d->spacingZ->setSuffix(" mm");
    d->spacingZ->setSingleStep(0.1f);
    d->spacingZ->setEnabled(false);
    spacingSpinBoxLayout->addWidget(d->spacingZLab);
    spacingSpinBoxLayout->addWidget(d->spacingZ);
    spinBoxes->setLayout(spacingSpinBoxLayout);

    d->thickSlab = new QSlider(Qt::Horizontal,reformatToolBoxBody);
    d->thickSlab->setRange(0,10); // TODO : set based on the image 
    d->thickMode = new QGroupBox("Thick Mode",reformatToolBoxBody);
    QVBoxLayout * thickModeLayout = new QVBoxLayout(reformatToolBoxBody);
    d->maxIP = new QRadioButton("MIP Max Intensity Projection",d->thickMode);
    d->minIP = new QRadioButton("MinIP Min Intensity Projection",d->thickMode);
    d->meanIP = new QRadioButton("Mean",d->thickMode);
    thickModeLayout->addWidget(d->thickSlab);
    thickModeLayout->addWidget(d->minIP);
    thickModeLayout->addWidget(d->maxIP);
    thickModeLayout->addWidget(d->meanIP);
    d->thickMode->setLayout(thickModeLayout);
    d->thickMode->setCheckable(true);
    d->thickMode->setChecked(false);
    d->thickMode->setEnabled(false);
    QVBoxLayout *reformatToolBoxLayout =  new QVBoxLayout(reformatToolBoxBody);
    reformatToolBoxLayout->addWidget(d->b_startReformat);
    reformatToolBoxLayout->addWidget(spinBoxes);
    reformatToolBoxLayout->addWidget(d->thickMode);
    reformatToolBoxLayout->addWidget(d->b_saveImage);
    /*reformatToolBoxLayout->addWidget(d->thickSlab);*/
    /*reformatToolBoxLayout->addWidget(d->thickMode);*/
    reformatToolBoxBody->setLayout(reformatToolBoxLayout);
    this->addWidget(reformatToolBoxBody);

    d->reformatViewer = 0;
    // Connections
    connect(d->b_startReformat,SIGNAL(toggled(bool)),this,SLOT(startReformat(bool)));
}
reformatToolBox::~reformatToolBox()
{
    delete d;
    d = NULL;
}

bool reformatToolBox::registered()
{
    medToolBoxFactory* factory = medToolBoxFactory::instance();
    return factory->registerToolBox<reformatToolBox> ("reformatToolBox",
            "reformat",
            "Used to reformat an image",
            QStringList()<<"reformatToolBox"
                                                               );
}

dtkPlugin* reformatToolBox::plugin()
{
    medPluginManager* pm = medPluginManager::instance();
    dtkPlugin* plugin = pm->plugin ( "reformatPlugin" );
    return plugin;
}

void reformatToolBox::startReformat(bool val)
{
    if (val)
    {
        if (d->currentView && d->workspace)
        {
            d->b_startReformat->setText("Stop Reformat");
            d->thickMode->setEnabled(true);
            d->b_saveImage->setEnabled(true);
            d->spacingX->setEnabled(true);
            d->spacingXLab->setEnabled(true);
            d->spacingY->setEnabled(true);
            d->spacingYLab->setEnabled(true);
            d->spacingZ->setEnabled(true);
            d->spacingZLab->setEnabled(true);

            d->reformatViewer = new medReformatViewer(d->currentView,d->workspace->stackedViewContainers());
            d->reformatViewer->setAcceptDrops(false);
            d->workspace->stackedViewContainers()->addContainer("Reformat",d->reformatViewer);
            d->workspace->setCurrentViewContainer("Reformat");
            d->workspace->stackedViewContainers()->lockTabs();
            //d->workspace->stackedViewContainers()->hideTabBar();
            connect(d->thickMode,SIGNAL(toggled(bool)),this,SLOT(propagateThickModeActivated()));
            //connect(d->thickSlab,SIGNAL(valueChanged(int)),d->reformatViewer,SLOT(thickSlabChanged(double)));
            connect(d->spacingX,SIGNAL(valueChanged(double)),d->reformatViewer,SLOT(thickSlabChanged(double)));
            connect(d->spacingY,SIGNAL(valueChanged(double)),d->reformatViewer,SLOT(thickSlabChanged(double)));
            connect(d->spacingZ,SIGNAL(valueChanged(double)),d->reformatViewer,SLOT(thickSlabChanged(double)));
            connect(d->maxIP,SIGNAL(toggled(bool)),this,SLOT(propagateBlendModeChosen()));
            connect(d->minIP,SIGNAL(toggled(bool)),this,SLOT(propagateBlendModeChosen()));
            connect(d->meanIP,SIGNAL(toggled(bool)),this,SLOT(propagateBlendModeChosen()));
            connect(d->b_saveImage,SIGNAL(clicked()),d->reformatViewer,SLOT(saveImage()));
        }
    }
    else
    {
        d->b_startReformat->setText("Start Reformat");
        d->thickMode->setEnabled(false);
        d->b_saveImage->setEnabled(false);
        d->spacingX->setEnabled(false);
        d->spacingXLab->setEnabled(false);
        d->spacingY->setEnabled(false);
        d->spacingYLab->setEnabled(false);
        d->spacingZ->setEnabled(false);
        d->spacingZLab->setEnabled(false);
 
        // d->workspace->stackedViewContainers()->show // TODO: go get commit in varSegITK4 branch for showTabBar() function
        d->workspace->stackedViewContainers()->unlockTabs();
        d->workspace->setCurrentViewContainer("Reformat/Resample");
        d->workspace->stackedViewContainers()->removeContainer("Reformat");
        disconnect(d->reformatViewer);
        delete d->reformatViewer;        
        d->reformatViewer = 0;
    }
     // TODO : hide navigator to increase space ?? you probably can access the navigator via the workspace area which is accesssible probably via the main window
        //   QMainWindow * mainWindow = dynamic_cast< QMainWindow * >(
        //qApp->property( "MainWindow" ).value< QObject * >() );
}
void reformatToolBox::setWorkspace(medWorkspace * workspace)
{
    d->workspace = workspace;
    connect(d->workspace->stackedViewContainers(),SIGNAL(currentChanged(const QString&)),this,SLOT(actOnContainerChange(const QString&)));
}

void reformatToolBox::update(dtkAbstractView* view)
{
    if (d->currentView==qobject_cast<medAbstractView*>(view))
        return;

    d->currentView = qobject_cast<medAbstractView*>(view);

    if (d->currentView)
    {
        displayInfoOnCurrentView();
    }
}

void reformatToolBox::actOnContainerChange(const QString & name)
{
    if (!qobject_cast<reformatWorkspace*>(d->workspace))
        return;
    if (name == "Reformat")
        qobject_cast<reformatWorkspace*>(d->workspace)->showViewPropertiesToolBox(false);
    else
        qobject_cast<reformatWorkspace*>(d->workspace)->showViewPropertiesToolBox(true);
}

void reformatToolBox::propagateBlendModeChosen()
{
    if (d->maxIP->isChecked())
        d->reformatViewer->SetBlendModeToMaxIP();
    else 
        if (d->minIP->isChecked())
            d->reformatViewer->SetBlendModeToMinIP();
        else
            d->reformatViewer->SetBlendModeToMeanIP();
}

void reformatToolBox::propagateThickModeActivated()
{
    if (d->thickMode->isChecked())
    {
        d->reformatViewer->thickMode(1);
        d->maxIP->toggle();
    }
    else
        d->reformatViewer->thickMode(0);
}

void reformatToolBox::displayInfoOnCurrentView()
{
    vtkImageView2D * view2d = static_cast<medVtkViewBackend*>(d->currentView->backend())->view2D;
    double* spacing = view2d->GetInput()->GetSpacing();
    
    d->spacingX->setValue(spacing[0]);
    d->spacingY->setValue(spacing[1]);
    d->spacingZ->setValue(spacing[2]);
}