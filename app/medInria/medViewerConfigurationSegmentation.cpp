// /////////////////////////////////////////////////////////////////
// Generated by dtkPluginGenerator
// /////////////////////////////////////////////////////////////////

#include "medViewerConfigurationSegmentation.h"

#include "medToolBoxSegmentation.h"

#include <medCore/medAbstractView.h>

#include <medProgressionStack.h>
#include <medTabbedViewContainers.h>
#include <medViewContainer.h>
#include <medViewerConfigurationFactory.h>
#include <medViewerToolBoxLayout.h>
#include <medViewerToolBoxViewProperties.h>

#include <dtkCore/dtkLog.h>

#include <stdexcept>

// /////////////////////////////////////////////////////////////////
// ConfigurationPrivate
// /////////////////////////////////////////////////////////////////
class medViewerConfigurationSegmentationPrivate
{
public:
    // Give values to items without a constructor.
    medViewerConfigurationSegmentationPrivate() : 
      layoutToolBox(NULL), viewPropertiesToolBox(NULL), segmentationToolBox(NULL)
    {}

    medViewerToolBoxLayout              *layoutToolBox;
    medViewerToolBoxViewProperties      *viewPropertiesToolBox;

    medToolBoxSegmentation          *segmentationToolBox;
};

// /////////////////////////////////////////////////////////////////
// medViewerConfigurationSegmentation
// /////////////////////////////////////////////////////////////////
//static
medViewerConfiguration * medViewerConfigurationSegmentation::createMedSegmentationConfiguration(QWidget * parent)
{
    return new medViewerConfigurationSegmentation(parent);
}

medViewerConfigurationSegmentation::medViewerConfigurationSegmentation(QWidget * parent /* = NULL */ ) : 
medViewerConfiguration(parent), d(new medViewerConfigurationSegmentationPrivate)
{
    d->segmentationToolBox = new medToolBoxSegmentation(this, parent );

    // Always have a parent.
    if ( !parent) 
        throw (std::runtime_error ("Must have a parent widget"));

    // -- Layout toolbox --
    d->layoutToolBox = new medViewerToolBoxLayout(parent);

    connect (d->layoutToolBox, SIGNAL(modeChanged(const QString&)),
        this,             SIGNAL(layoutModeChanged(const QString&)));
    connect (d->layoutToolBox, SIGNAL(presetClicked(int)),
        this,             SIGNAL(layoutPresetClicked(int)));
    connect (d->layoutToolBox, SIGNAL(split(int,int)),
        this,             SIGNAL(layoutSplit(int,int)));

    connect(this,SIGNAL(setLayoutTab(const QString &)), d->layoutToolBox, SLOT(setTab(const QString &)));

    // -- View toolbox --

    d->viewPropertiesToolBox = new medViewerToolBoxViewProperties(parent);


    connect ( this, SIGNAL(layoutModeChanged(const QString &)),
        stackedViewContainers(), SLOT(changeCurrentContainerType(const QString &)));
    connect ( stackedViewContainers(), SIGNAL(currentChanged(const QString &)),
        this, SLOT(connectToolboxesToCurrentContainer(const QString &)));

    this->addToolBox( d->segmentationToolBox );
    this->addToolBox( d->layoutToolBox );
    this->addToolBox( d->viewPropertiesToolBox );
}

medViewerConfigurationSegmentation::~medViewerConfigurationSegmentation(void)
{
    delete d;
    d = NULL;
}

bool medViewerConfigurationSegmentation::registerWithViewerConfigurationFactory()
{
    return medViewerConfigurationFactory::instance()->registerConfiguration( 
        medViewerConfigurationSegmentation::ConfigurationName(), 
        medViewerConfigurationSegmentation::createMedSegmentationConfiguration
        );
}

QString medViewerConfigurationSegmentation::ConfigurationName()
{
    // This is the string that appears in the drop-down.
    static QString confName( tr("Segmentation") );
    return confName;
}

void medViewerConfigurationSegmentation::setupViewContainerStack()
{
    const QString identifier(this->containerIdentifier());
    if (!stackedViewContainers()->count())
    {
        //Containers:
        this->addMultiContainer(identifier);

        //Default container:
        this->connectToolboxesToCurrentContainer(identifier);
    }

    this->stackedViewContainers()->unlockTabs();
}

//static
QString medViewerConfigurationSegmentation::description( void ) const
{
    static QString descString( "Segmentation Configuration" );
    return descString;
}

void medViewerConfigurationSegmentation::onViewAdded( dtkAbstractView* view )
{
    emit viewAdded(view);
}

void medViewerConfigurationSegmentation::onViewRemoved( dtkAbstractView* view )
{
    emit viewRemoved(view);
}


medProgressionStack * medViewerConfigurationSegmentation::progressionStack()
{
    return d->segmentationToolBox->progressionStack();
}

void medViewerConfigurationSegmentation::buildConfiguration(  )
{
    QWidget * parent = qobject_cast<QWidget *>(this->parent());
    if ( !d->segmentationToolBox)
        d->segmentationToolBox = new medToolBoxSegmentation( this, parent );

    this->addToolBox( d->segmentationToolBox );
}

medToolBoxSegmentation * medViewerConfigurationSegmentation::segmentationToobox()
{
    return d->segmentationToolBox;
}

QString medViewerConfigurationSegmentation::containerIdentifier() const
{
    return "Segmentation";
}


void medViewerConfigurationSegmentation::connectToolboxesToCurrentContainer(const QString &name)
{
    connect(stackedViewContainers()->container(name),SIGNAL(viewAdded(dtkAbstractView*)),
        this,SLOT(onViewAdded(dtkAbstractView*)));
    connect(stackedViewContainers()->container(name),SIGNAL(viewRemoved(dtkAbstractView*)),
        this,SLOT(onViewRemoved(dtkAbstractView*)));
}

QString medViewerConfigurationSegmentation::identifier( void ) const
{
    static QString id( "msegConfiguration" );
    return id;
}

medViewerConfiguration *createMedViewerConfigurationSegmentation(QWidget* parent)
{
    return new medViewerConfigurationSegmentation(parent);
}