/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "meshMapping.h"
#include "meshMappingToolBox.h"

#include <QtGui>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <dtkCore/dtkAbstractViewFactory.h>
#include <dtkCore/dtkAbstractViewInteractor.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractView.h>
#include <medRunnableProcess.h>
#include <medJobManager.h>

#include <medAbstractDataImage.h>

#include <medToolBoxFactory.h>
#include <medFilteringSelectorToolBox.h>
#include <medProgressionStack.h>
#include <medPluginManager.h>
#include <medViewManager.h>
#include <medDataManager.h>
#include <medAbstractDbController.h>
#include <medDbControllerFactory.h>
#include <medMetaDataKeys.h>
#include <medDropSite.h>
#include <medImageFileLoader.h>

class meshMappingToolBoxPrivate
{
public:
    
    dtkSmartPointer <dtkAbstractProcess> process;
    medDropSite *dropStructure;
    medDropSite *dropData;

    dtkSmartPointer<dtkAbstractView> view;
    dtkSmartPointer<dtkAbstractData> data;
    dtkSmartPointer<dtkAbstractData> structure;
};

meshMappingToolBox::meshMappingToolBox(QWidget *parent) : medFilteringAbstractToolBox(parent), d(new meshMappingToolBoxPrivate)
{
    this->setTitle("Mesh Mapping");

    QWidget *widget = new QWidget(this);
    
    d->dropStructure = new medDropSite(widget);
    d->dropStructure->setToolTip(tr("Drop the dataset whose geometry will be used\n in determining positions to probe (typically a mesh)."));
    d->dropStructure->setText(tr("Drop the structure"));
    d->dropStructure->setCanAutomaticallyChangeAppereance(false);
    connect (d->dropStructure, SIGNAL(objectDropped(const medDataIndex&)),  this, SLOT(importStructure(const medDataIndex&)));

    d->dropData = new medDropSite(widget);
    d->dropData->setToolTip(tr("Drop the dataset from which to obtain\n probe values (image or mesh)."));
    d->dropData->setText(tr("Drop the data"));
    d->dropData->setCanAutomaticallyChangeAppereance(true);
    connect (d->dropData, SIGNAL(objectDropped(const medDataIndex&)),       this, SLOT(importData(const medDataIndex&)));

    QPushButton *clearStructureButton = new QPushButton("Clear Structure", widget);
    clearStructureButton->setToolTip(tr("Clear previously loaded structure."));
    connect (clearStructureButton,   SIGNAL(clicked()),                     this, SLOT(clearStructure()));

    QPushButton *clearDataButton = new QPushButton("Clear Data", widget);
    clearDataButton->setToolTip(tr("Clear previously loaded data."));
    connect (clearDataButton,   SIGNAL(clicked()),                          this, SLOT(clearData()));

    QVBoxLayout *roiButtonLayout = new QVBoxLayout;
    roiButtonLayout->addWidget(d->dropStructure);
    roiButtonLayout->addWidget (clearStructureButton);
    roiButtonLayout->addWidget(d->dropData);
    roiButtonLayout->addWidget (clearDataButton);
    roiButtonLayout->setAlignment(Qt::AlignCenter);

    QPushButton *runButton = new QPushButton(tr("Run"), this);
    connect(runButton, SIGNAL(clicked()), this, SLOT(run()));


    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addLayout(roiButtonLayout);
    layout->addWidget(runButton);

    widget->setLayout(layout);
    this->addWidget(widget);
}

meshMappingToolBox::~meshMappingToolBox()
{
    delete d;
    d = NULL;
}

bool meshMappingToolBox::registered()
{
    return medToolBoxFactory::instance()->
    registerToolBox<meshMappingToolBox>("meshMappingToolBox",
                               tr("Mesh Mapping"),
                               tr("Map data on a mesh"),
                               QStringList()<< "filtering");
}

dtkPlugin* meshMappingToolBox::plugin()
{
    medPluginManager* pm = medPluginManager::instance();
    dtkPlugin* plugin = pm->plugin ( "meshMappingPlugin" );
    return plugin;
}

dtkAbstractData* meshMappingToolBox::processOutput()
{
    if(!d->process)
        return NULL;
    
    return d->process->output();
}

void meshMappingToolBox::run()
{ 
    if (!d->process)
        d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("meshMapping");

    if(d->process->update())
        return;
    QString newSeriesDescription = d->data->metadata ( medMetaDataKeys::SeriesDescription.key() );
    newSeriesDescription += " mapped on :" + d->structure->metadata ( medMetaDataKeys::SeriesDescription.key() );

    dtkSmartPointer <dtkAbstractData> output = d->process->output();
    setOutputMetadata(d->data, output);
    output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    medDataManager::instance()->importNonPersistent(output);
}

void meshMappingToolBox::setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData)
{
    Q_ASSERT(outputData && inputData);

    QStringList metaDataToCopy;
    metaDataToCopy
        << medMetaDataKeys::PatientName.key()
        << medMetaDataKeys::StudyDescription.key();

    foreach( const QString & key, metaDataToCopy ) {
        outputData->setMetaData(key, inputData->metadatas(key));
    }
}

void meshMappingToolBox::importStructure(const medDataIndex& index)
{
    dtkSmartPointer<dtkAbstractData> data = medDataManager::instance()->data(index);
    // we only accept meshes
    if (!data || data->identifier() != "vtkDataMesh")
        return;

    // put the thumbnail in the medDropSite as well
    // (code copied from @medDatabasePreviewItem)
    medAbstractDbController* dbc = medDataManager::instance()->controllerForDataSource(index.dataSourceId());
    QString thumbpath = dbc->metaData(index, medMetaDataKeys::ThumbnailPath);

    bool shouldSkipLoading = false;
    if ( thumbpath.isEmpty() )
    {
        // first try to get it from controller
        QImage thumbImage = dbc->thumbnail(index);
        if (!thumbImage.isNull())
        {
            d->dropStructure->setPixmap(QPixmap::fromImage(thumbImage));
            shouldSkipLoading = true;
        }
    }
    if (!shouldSkipLoading) {
        medImageFileLoader *loader = new medImageFileLoader(thumbpath);

        connect(loader, SIGNAL(completed(const QImage&)), this, SLOT(setImage(const QImage&)));
        QThreadPool::globalInstance()->start(loader);
    }
    if(!d->process)
        d->process= dtkAbstractProcessFactory::instance()->create("meshMapping");

    d->structure = data;
    if (!d->process)
        return;

    d->process->setInput(data, 0);
}

void meshMappingToolBox::importData(const medDataIndex& index)
{
    dtkSmartPointer<dtkAbstractData> data = medDataManager::instance()->data(index);

    if (!data)
        return;
    
    if(!d->process)
        d->process= dtkAbstractProcessFactory::instance()->create("meshMapping");

    d->data = data;
    
    if (!d->process)
        return;

    d->process->setInput(data, 1);
}

void meshMappingToolBox::clearStructure(void)
{
    d->dropStructure->clear();
    d->dropStructure->setText(tr("Drop the structure"));
}

void meshMappingToolBox::clearData(void)
{
    d->dropData->clear();
    d->dropData->setText(tr("Drop the data"));
}

void meshMappingToolBox::setImage(const QImage& thumbnail)
{
    d->dropStructure->setPixmap(QPixmap::fromImage(thumbnail));
}