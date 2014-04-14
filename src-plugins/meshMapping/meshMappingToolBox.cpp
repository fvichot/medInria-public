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
    medProgressionStack * progression_stack;
    medDropSite *dropOrOpenRoi;
    medDropSite *dropOrOpenRoi2;

    dtkSmartPointer<dtkAbstractView> view;
    dtkSmartPointer<dtkAbstractData> data;
    dtkSmartPointer<dtkAbstractData> mesh;
};

meshMappingToolBox::meshMappingToolBox(QWidget *parent) : medFilteringAbstractToolBox(parent), d(new meshMappingToolBoxPrivate)
{
    QWidget *widget = new QWidget(this);
    
        d->dropOrOpenRoi = new medDropSite(widget);
    d->dropOrOpenRoi->setToolTip(tr("Drop the binary mesh"));
    d->dropOrOpenRoi->setText(tr("Drop the mesh"));
    d->dropOrOpenRoi->setCanAutomaticallyChangeAppereance(false);

    d->dropOrOpenRoi2 = new medDropSite(widget);
    d->dropOrOpenRoi2->setToolTip(tr("Drop the image"));
    d->dropOrOpenRoi2->setText(tr("Drop the image"));
    d->dropOrOpenRoi2->setCanAutomaticallyChangeAppereance(true);

    QPushButton *clearRoiButton = new QPushButton("Clear mesh", widget);
    clearRoiButton->setToolTip(tr("Clear previously loaded mesh."));
    QPushButton *clearInputButton = new QPushButton("Clear Image", widget);
    clearInputButton->setToolTip(tr("Clear previously loaded image."));

    QVBoxLayout *roiButtonLayout = new QVBoxLayout;
    roiButtonLayout->addWidget(d->dropOrOpenRoi);
    roiButtonLayout->addWidget (clearRoiButton);
    roiButtonLayout->addWidget(d->dropOrOpenRoi2);
    roiButtonLayout->addWidget (clearInputButton);
    roiButtonLayout->setAlignment(Qt::AlignCenter);

    QVBoxLayout *bundlingLayout = new QVBoxLayout(widget);
    bundlingLayout->addLayout(roiButtonLayout);

    connect (d->dropOrOpenRoi, SIGNAL(objectDropped(const medDataIndex&)), this, SLOT(onRoiImported(const medDataIndex&)));
    connect (d->dropOrOpenRoi, SIGNAL(clicked()),                          this, SLOT(onDropSiteClicked()));
    connect (d->dropOrOpenRoi2, SIGNAL(objectDropped(const medDataIndex&)), this, SLOT(onImageImported(const medDataIndex&)));
    connect (d->dropOrOpenRoi2, SIGNAL(clicked()),                          this, SLOT(onDropSiteClicked()));
    connect (clearRoiButton,   SIGNAL(clicked()),                          this, SLOT(onClearRoiButtonClicked()));
    connect (clearInputButton,   SIGNAL(clicked()),                          this, SLOT(onClearInputButtonClicked()));


    this->setTitle("Mesh Mapping");
    this->addWidget(widget);

    QPushButton *runButton = new QPushButton(tr("Run"), this);
    
    // progression stack
    d->progression_stack = new medProgressionStack(widget);
    QHBoxLayout *progressStackLayout = new QHBoxLayout;
    progressStackLayout->addWidget(d->progression_stack);
    
    this->addWidget(runButton);
    this->addWidget(d->progression_stack);
    
    connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
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
    //if(!this->parentToolBox())
    //    return;
    
    if (!d->process)
        d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("meshMapping");
    
    //if(!this->parentToolBox()->data())
    //    return;
    //if(!d->process)
    //    return;
    //d->process->setInput(d->mesh, 0);
    //d->process->setInput(d->data, 1);
    // Set your parameters here
    
    //medRunnableProcess *runProcess = new medRunnableProcess;
    //runProcess->setProcess (d->process);
    //
    //d->progression_stack->addJobItem(runProcess, "Progress:");
    //
    //d->progression_stack->disableCancel(runProcess);
    //
    //connect (runProcess, SIGNAL (success  (QObject*)),  this, SIGNAL (success ()));
    //connect (runProcess, SIGNAL (failure  (QObject*)),  this, SIGNAL (failure ()));
    //connect (runProcess, SIGNAL (cancelled (QObject*)),  this, SIGNAL (failure ()));
    //
    //connect (runProcess, SIGNAL(activate(QObject*,bool)),
    //         d->progression_stack, SLOT(setActive(QObject*,bool)));
    //
    //medJobManager::instance()->registerJobItem(runProcess);
    //QThreadPool::globalInstance()->start(dynamic_cast<QRunnable*>(runProcess));
    d->process->update();
    QString newSeriesDescription = d->data->metadata ( medMetaDataKeys::SeriesDescription.key() );
    newSeriesDescription += " with mesh :" + d->mesh->metadata ( medMetaDataKeys::SeriesDescription.key() );

    //dtkSmartPointer <dtkAbstractData> output = processOutput();
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

void meshMappingToolBox::onRoiImported(const medDataIndex& index)
{
    dtkSmartPointer<dtkAbstractData> data = medDataManager::instance()->data(index);
    // we accept only ROIs (itkDataImageUChar3)
    if (!data || data->identifier() != "vtkDataMesh")
    {
        return;
    }
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
            d->dropOrOpenRoi->setPixmap(QPixmap::fromImage(thumbImage));
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

    d->mesh = data;
    if (!d->process)
        return;

    d->process->setInput(data, 0);
}

void meshMappingToolBox::onImageImported(const medDataIndex& index)
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

void meshMappingToolBox::onClearRoiButtonClicked(void)
{
    d->dropOrOpenRoi->clear();
    d->dropOrOpenRoi->setText(tr("Drop the mesh"));
}

void meshMappingToolBox::onClearInputButtonClicked(void)
{
    d->dropOrOpenRoi2->clear();
    d->dropOrOpenRoi2->setText(tr("Drop the image"));
}

void meshMappingToolBox::clear(void)
{
    // clear ROIs and related GUI elements
    onClearRoiButtonClicked();

    d->view = 0;
    d->data = 0;
}

void meshMappingToolBox::update(dtkAbstractView *view)
{
    //if (d->view==view) {
    //    if (view)
    //        if (dtkAbstractViewInteractor *interactor = view->interactor ("v3dViewFiberInteractor"))
    //            this->setData (interactor->data()); // data may have changed
    //    return;
    //}

    //d->bundlingModel->removeRows(0, d->bundlingModel->rowCount(QModelIndex()), QModelIndex());

    //if (d->view) {
    //    if (dtkAbstractViewInteractor *interactor = d->view->interactor ("v3dViewFiberInteractor")) {
    //    }
    //}

    //if (!view) {
    //    d->view = 0;
    //    d->data = 0;
    //    return;
    //}

    ///*
    //if (view->property ("Orientation")!="3D") { // only interaction with 3D views is authorized
    //    d->view = 0;
    //    d->data = 0;
    //    return;
    //}
    //*/

    //d->view = view;

    //if (dtkAbstractViewInteractor *interactor = view->interactor ("v3dViewFiberInteractor")) {


    //    this->setData (interactor->data());
    //}
}

void meshMappingToolBox::onDropSiteClicked()
{
    //if (!d->view)
    //    return;

    //QString roiFileName = QFileDialog::getOpenFileName(this, tr("Open ROI"), "", tr("Image file (*.*)"));

    //if (roiFileName.isEmpty())
    //    return;

    //medDataManager* mdm = medDataManager::instance();
    //connect(mdm, SIGNAL(dataAdded(const medDataIndex &)), this, SLOT(onRoiImported(const medDataIndex &)));
    //mdm->importNonPersistent(roiFileName);
}

void meshMappingToolBox::setImage(const QImage& thumbnail)
{
    d->dropOrOpenRoi->setPixmap(QPixmap::fromImage(thumbnail));
}