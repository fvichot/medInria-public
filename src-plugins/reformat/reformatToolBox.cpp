#include "reformatToolBox.h"
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

class reformatToolBoxPrivate
{
public:
    QPushButton * b_startReformat;
    medAbstractView * currentView;
    medWorkspace * workspace;
    QCheckBox * orthogonalAxes;
    QRadioButton * maxIP,*minIP,*meanIP,*normal;
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
    d->orthogonalAxes = new QCheckBox("Orthogonal Axes",reformatToolBoxBody);
    d->orthogonalAxes->setEnabled(false);
    QVBoxLayout *reformatToolBoxLayout =  new QVBoxLayout(reformatToolBoxBody);
    reformatToolBoxLayout->addWidget(d->b_startReformat);
    reformatToolBoxLayout->addWidget(d->orthogonalAxes);
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
            d->orthogonalAxes->setEnabled(true);
            d->reformatViewer = new medReformatViewer(d->currentView,d->workspace->stackedViewContainers());
            d->reformatViewer->setAcceptDrops(false);
            d->workspace->stackedViewContainers()->addContainer("Reformat",d->reformatViewer);
            d->workspace->setCurrentViewContainer("Reformat");
            d->workspace->stackedViewContainers()->lockTabs();
            //d->workspace->stackedViewContainers()->hideTabBar();
            connect(d->orthogonalAxes,SIGNAL(checked(bool)),d->reformatViewer,SLOT(orthogonalAxisModeEnabled(bool)));
        }

    }
    else
    {
        d->b_startReformat->setText("Start Reformat");
        d->orthogonalAxes->setEnabled(false);
        // d->workspace->stackedViewContainers()->show // TODO: go get commit in varSegITK4 branch for showTabBar() function
        d->workspace->stackedViewContainers()->unlockTabs();
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
