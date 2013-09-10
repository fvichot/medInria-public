/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medMeshTools.h"
#include "medMeshToolsToolBox.h"

#include <QtGui>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <dtkCore/dtkAbstractViewFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractView.h>
#include <medRunnableProcess.h>
#include <medJobManager.h>

#include <medAbstractDataImage.h>

#include <medToolBoxFactory.h>
#include <medFilteringSelectorToolBox.h>
#include <medProgressionStack.h>
#include <medPluginManager.h>

#include <medSliderSpinboxPair.h>

class medMeshToolsToolBoxPrivate
{
public:

    dtkSmartPointer <dtkAbstractProcess> process;
    medProgressionStack * progression_stack;
    medAbstractView * view;
    medSliderSpinboxPair * slider;
};

medMeshToolsToolBox::medMeshToolsToolBox(QWidget *parent)
 : medToolBox(parent)
 , d(new medMeshToolsToolBoxPrivate)
{
    this->setTitle("medMeshTools");
    
    QWidget *widget = new QWidget(this);
    d->slider = new medSliderSpinboxPair(this);
    d->slider->setMinimum(0);
    d->slider->setMaximum(2000);
    QPushButton *runButton = new QPushButton(tr("Run"), this);
    
    // progression stack
    d->progression_stack = new medProgressionStack(widget);
    QHBoxLayout *progressStackLayout = new QHBoxLayout;
    progressStackLayout->addWidget(d->progression_stack);
    
    this->addWidget(d->slider);
    this->addWidget(runButton);
    this->addWidget(d->progression_stack);
    
    d->view = 0;

    connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
}

medMeshToolsToolBox::~medMeshToolsToolBox()
{
    delete d;
    d = NULL;
}

bool medMeshToolsToolBox::registered()
{
    return medToolBoxFactory::instance()->
    registerToolBox<medMeshToolsToolBox>("medMeshToolsToolBox",
                               tr("Mesh ToolBox"),
                               tr("Tools for meshes"),
                               QStringList()<< "mesh" << "view");
}

dtkPlugin* medMeshToolsToolBox::plugin()
{
    medPluginManager* pm = medPluginManager::instance();
    dtkPlugin* plugin = pm->plugin ( "medMeshToolsPlugin" );
    return plugin;
}

dtkAbstractData* medMeshToolsToolBox::processOutput()
{
    if(!d->process)
        return NULL;
    
    return d->process->output();
}

void medMeshToolsToolBox::update(dtkAbstractView *view)
{
    medToolBox::update(view);
    if(!view)
    {
        clear();
        return;
    }

    medAbstractView * medView = dynamic_cast<medAbstractView *> (view);
    if ( !medView )
        return;

    if ((d->view) && (d->view != medView) )
    {
        d->view->disconnect(this,0);
        clear();
    }

    d->view = medView;

    QObject::connect(d->view, SIGNAL(dataAdded(dtkAbstractData*, int)),
                     this, SLOT(addData(dtkAbstractData*, int)),
                     Qt::UniqueConnection);

    QObject::connect(d->view, SIGNAL(dataRemoved(dtkAbstractData*,int)),
                     this, SLOT(removeData(dtkAbstractData*, int)),
                     Qt::UniqueConnection);
}


void medMeshToolsToolBox::addData(dtkAbstractData* data, int layer)
{
    if(!data)
        return;

    if (!d->view)
        return;

    if(data->identifier().contains("vtkDataMesh")) {
    }
}


void medMeshToolsToolBox::removeData(dtkAbstractData* data, int layer)
{
    if(!data)
        return;

    if (!d->view)
        return;

    if(data->identifier().contains("vtkDataMesh")) {
    }
}

void medMeshToolsToolBox::addMeshToView() {
    qDebug() << "Adding mesh to view";

    dtkSmartPointer<dtkAbstractData> ptr;
    dtkAbstractData * data = d->process->output();
    ptr.takePointer(data);
    d->view->setSharedDataPointer(ptr);
}


void medMeshToolsToolBox::clear(void) {

}



void medMeshToolsToolBox::run()
{
    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("medMeshTools");
    
    if( ! d->view->data())
        return;
    
    d->process->setInput( static_cast<dtkAbstractData*>(d->view->data()));
    
    d->process->setParameter(d->slider->value(), 0); // iso value
    
    medRunnableProcess *runProcess = new medRunnableProcess;
    runProcess->setProcess (d->process);
    
    d->progression_stack->addJobItem(runProcess, "Progress:");
    
    d->progression_stack->disableCancel(runProcess);
    
    connect (runProcess, SIGNAL (success  (QObject*)),  this, SIGNAL (success ()));
    connect (runProcess, SIGNAL (success  (QObject*)),  this, SLOT(addMeshToView()));
    connect (runProcess, SIGNAL (failure  (QObject*)),  this, SIGNAL (failure ()));
    connect (runProcess, SIGNAL (cancelled (QObject*)),  this, SIGNAL (failure ()));
    
    connect (runProcess, SIGNAL(activate(QObject*,bool)),
             d->progression_stack, SLOT(setActive(QObject*,bool)));
    
    medJobManager::instance()->registerJobItem(runProcess);
    QThreadPool::globalInstance()->start(dynamic_cast<QRunnable*>(runProcess));
}
