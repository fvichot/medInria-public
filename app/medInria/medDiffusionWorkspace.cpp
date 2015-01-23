/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medDiffusionWorkspace.h"

#include <dtkCore/dtkSmartPointer.h>
#include <medAbstractDiffusionProcess.h>
#include <dtkCore/dtkAbstractProcessFactory.h>

#include <medAbstractLayeredView.h>
#include <medAbstractImageData.h>
#include <medAbstractDiffusionModelImageData.h>
#include <medDataManager.h>

#include <medViewContainer.h>
#include <medViewContainerManager.h>
#include <medTabbedViewContainers.h>
#include <medToolBox.h>
#include <medToolBoxFactory.h>

#include <medJobManager.h>
#include <medMessageController.h>

#include <medProcessSelectorToolBox.h>

#include <medTriggerParameter.h>

class medDiffusionWorkspacePrivate
{
public:

    QPointer<medProcessSelectorToolBox> scalarMapsToolbox;
    medViewContainer *inputContainer;
    medViewContainer *outputContainer;

    medAbstractData *scalarMapInput;
    medAbstractData *scalarMapOutput;

    dtkSmartPointer <medAbstractDiffusionProcess> process;
};

medDiffusionWorkspace::medDiffusionWorkspace(QWidget *parent) : medAbstractWorkspace(parent), d(new medDiffusionWorkspacePrivate)
{
    d->scalarMapInput = NULL;
    d->scalarMapOutput = NULL;

    d->scalarMapsToolbox = new medProcessSelectorToolBox(parent);
    d->scalarMapsToolbox->setTitle("Scalar Map");

    this->addToolBox(d->scalarMapsToolbox);

    QStringList implementations = dtkAbstractProcessFactory::instance()->implementations("medAbstractTensorScalarMapsProcess");
    d->scalarMapsToolbox->setAvailableProcesses(implementations);

//    medViewParameterGroup *viewGroup1 = new medViewParameterGroup("View Group 1", this, this->identifier());
//    viewGroup1->setLinkAllParameters(true);
//    viewGroup1->removeParameter("DataList");

//    medLayerParameterGroup *layerGroup1 = new medLayerParameterGroup("Layer Group 1", this,  this->identifier());
//    layerGroup1->setLinkAllParameters(true);

    connect(d->scalarMapsToolbox, SIGNAL(processSelected(QString)), this, SLOT(setupProcess(QString)));
}

medDiffusionWorkspace::~medDiffusionWorkspace()
{
    delete d;
    d = NULL;
}

void medDiffusionWorkspace::setupProcess(QString process)
{
    medAbstractProcess *temp = d->process;
    d->process = dynamic_cast<medAbstractDiffusionProcess*>(dtkAbstractProcessFactory::instance()->create(process));
    if(d->process)
    {
        d->scalarMapsToolbox->setProcessToolbox(d->process->toolbox());
        connect(d->process->runParameter(), SIGNAL(triggered()), this, SLOT(startProcess()));
        this->tabbedViewContainers()->setSplitter(0, d->process->viewContainerSplitter());
    }

    if(d->process && temp)
    {
        d->process->retrieveInputs(temp);
    }

    delete temp;
}

void medDiffusionWorkspace::startProcess()
{
    if(!d->process)
        return;

    d->scalarMapsToolbox->setEnabled(false);

    medRunnableProcess *runProcess = new medRunnableProcess(d->process, d->process->name());
    connect (runProcess, SIGNAL (success()),this,SLOT(enableSelectorToolBox()));
    connect (runProcess, SIGNAL (failure()),this,SLOT(enableSelectorToolBox()));

    runProcess->start();
}

void medDiffusionWorkspace::enableSelectorToolBox()
{
    d->scalarMapsToolbox->setEnabled(true);
}

bool medDiffusionWorkspace::isUsable()
{
//    medToolBoxFactory * tbFactory = medToolBoxFactory::instance();
//    bool workspaceUsable = (tbFactory->toolBoxesFromCategory("diffusion-estimation").size()!=0)||
//                           (tbFactory->toolBoxesFromCategory("diffusion-scalar-maps").size()!=0)||
//                           (tbFactory->toolBoxesFromCategory("diffusion-tractography").size()!=0);

//    return workspaceUsable;
    return true;
}
