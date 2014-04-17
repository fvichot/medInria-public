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
#include <medAbstractDataImage.h>

#include <medToolBoxFactory.h>
#include <medFilteringSelectorToolBox.h>
#include <medPluginManager.h>
#include <medViewManager.h>
#include <medDataManager.h>
#include <medAbstractDbController.h>
#include <medDbControllerFactory.h>
#include <medMetaDataKeys.h>

class meshMappingToolBoxPrivate
{
public:
    
    dtkSmartPointer <dtkAbstractProcess> process;

    dtkAbstractView * view;
    dtkSmartPointer<dtkAbstractData> data;
    dtkSmartPointer<dtkAbstractData> structure;

    QComboBox * layersForStructure;
    QComboBox * layersForData;
    int nbOfLayers; //need to reimplement the counting of layers because pb with meshes in the current counting

};

meshMappingToolBox::meshMappingToolBox(QWidget *parent) : medFilteringAbstractToolBox(parent), d(new meshMappingToolBoxPrivate)
{
    this->setTitle("Mesh Mapping");

    QWidget *widget = new QWidget(this);
    
    QLabel * dataLabel = new QLabel("Select the data to map: ");
    dataLabel->setToolTip(tr("Select the dataset from which to obtain\n probe values (image or mesh)."));
    d->layersForData = new QComboBox;
    d->layersForData->addItem("Select the layer", 0);

    QLabel * structureLabel = new QLabel("Select the structure:");
    structureLabel->setToolTip(tr("Select the dataset whose geometry will be used\n \
                               in determining positions to probe (typically a mesh)."));
    d->layersForStructure = new QComboBox;
    d->layersForStructure->addItem("Select the layer", 0);

    QPushButton *runButton = new QPushButton(tr("Run"), this);
    connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addWidget(dataLabel);
    layout->addWidget(d->layersForData);
    layout->addWidget(structureLabel);
    layout->addWidget(d->layersForStructure);
    layout->addWidget(runButton);

    widget->setLayout(layout);
    this->addWidget(widget);

    d->nbOfLayers = 0;
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
                               QStringList()<< "view");
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

    medAbstractView * medView = static_cast<medAbstractView *> (d->view);
    if ( !medView )
        return;
    int structureLayer = d->layersForStructure->currentIndex() -1;
    d->process->setInput(medView->dataInList(structureLayer), 0);

    int dataLayer = d->layersForData->currentIndex() -1;
    d->process->setInput(medView->dataInList(dataLayer), 1);

    if(d->process->update())
        return;
    QString newSeriesDescription = medView->dataInList(dataLayer)->metadata ( medMetaDataKeys::SeriesDescription.key() );
    newSeriesDescription += " mapped on :" + medView->dataInList(structureLayer)->metadata ( medMetaDataKeys::SeriesDescription.key() );

    dtkSmartPointer <dtkAbstractData> output = d->process->output();
    setOutputMetadata(medView->dataInList(structureLayer), output);
    output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    medDataManager::instance()->importNonPersistent(output);
}

void meshMappingToolBox::update(dtkAbstractView *view)
{
    medToolBox::update(view);

    medAbstractView * medView = dynamic_cast<medAbstractView *> (view);
    if ( !medView )
        return;

    d->view = medView;
    if(medView->layerCount()==1 && d->nbOfLayers == 0)
        addData(static_cast<dtkAbstractData*>(d->view->data()), 0); //dataAdded is not sent for the first image dropped

    QObject::connect(d->view, SIGNAL(dataAdded(dtkAbstractData*, int)),
                     this, SLOT(addData(dtkAbstractData*, int)),
                     Qt::UniqueConnection);

    //TODO: deal with closing a layer, once refactoring is done

    QObject::connect(d->view, SIGNAL(closing()),
                     this, SLOT(resetComboBoxes()),
                     Qt::UniqueConnection);
}

void meshMappingToolBox::resetComboBoxes()
{
    for (int i=0; i<d->layersForStructure->count();i++)
    {
        d->layersForStructure->removeItem(i);
        d->layersForData->removeItem(i);
    }
    d->layersForStructure->setItemText(0,"Select a layer");
    d->layersForData->setItemText(0,"Select a layer");
    d->nbOfLayers = 0;
}

void meshMappingToolBox::addData(dtkAbstractData* data, int layer) //we can't trust layer value with meshes
{
    d->nbOfLayers += 1;
    d->layersForStructure->addItem("Layer "+ QString::number(d->nbOfLayers-1)); //start with layer 0
    d->layersForData->addItem("Layer "+ QString::number(d->nbOfLayers-1)); //start with layer 0
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