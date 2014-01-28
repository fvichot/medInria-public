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
#include <qgroupbox.h>
class reformatToolBoxPrivate
{
public:
    QPushButton * b_startReformat;
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
    /*d->thickMode = new QCheckBox("Thick Mode",reformatToolBoxBody);
    d->thickMode->setEnabled(false);*/
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
    reformatToolBoxLayout->addWidget(d->thickMode);
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
            d->reformatViewer = new medReformatViewer(d->currentView,d->workspace->stackedViewContainers());
            d->reformatViewer->setAcceptDrops(false);
            d->workspace->stackedViewContainers()->addContainer("Reformat",d->reformatViewer);
            d->workspace->setCurrentViewContainer("Reformat");
            d->workspace->stackedViewContainers()->lockTabs();
            //d->workspace->stackedViewContainers()->hideTabBar();
            connect(d->thickMode,SIGNAL(toggled(bool)),this,SLOT(propagateThickModeActivated()));
            connect(d->thickSlab,SIGNAL(valueChanged(int)),d->reformatViewer,SLOT(thickSlabChanged(int)));
            connect(d->maxIP,SIGNAL(toggled(bool)),this,SLOT(propagateBlendModeChosen()));
            connect(d->minIP,SIGNAL(toggled(bool)),this,SLOT(propagateBlendModeChosen()));
            connect(d->meanIP,SIGNAL(toggled(bool)),this,SLOT(propagateBlendModeChosen()));
        }
    }
    else
    {
        d->b_startReformat->setText("Start Reformat");
        d->thickMode->setEnabled(false);
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