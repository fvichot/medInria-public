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
    medSliderSpinboxPair * thresholdSlider;
    QDoubleSpinBox * reductionSpinBox;
    QCheckBox * decimateCheckbox;
    QCheckBox * smoothCheckbox;
    QSpinBox * iterationsSpinBox;
    QDoubleSpinBox * relaxationSpinBox;
};

medMeshToolsToolBox::medMeshToolsToolBox(QWidget *parent)
 : medToolBox(parent)
 , d(new medMeshToolsToolBoxPrivate)
{
    this->setTitle("medMeshTools");

    QWidget *displayWidget = new QWidget(this);

    d->thresholdSlider = new medSliderSpinboxPair(displayWidget);
    d->thresholdSlider->setMinimum(0);
    d->thresholdSlider->setMaximum(2000);

    QLabel * thresholdLabel = new QLabel("Threshold : ", displayWidget);
    QHBoxLayout *thresholdLayout = new QHBoxLayout;
    thresholdLayout->addWidget(thresholdLabel);
    thresholdLayout->addWidget(d->thresholdSlider);

    d->decimateCheckbox = new QCheckBox("Decimate mesh", displayWidget);
    d->decimateCheckbox->setChecked(true);

    d->reductionSpinBox = new QDoubleSpinBox(displayWidget);
    d->reductionSpinBox->setRange(0.0, 1.0);
    d->reductionSpinBox->setSingleStep(0.01);
    d->reductionSpinBox->setDecimals(2);
    d->reductionSpinBox->setValue(0.8);

    connect(d->decimateCheckbox, SIGNAL(toggled(bool)), d->reductionSpinBox, SLOT(setEnabled(bool)));

    QLabel * reductionLabel = new QLabel("Reduction target : ", displayWidget);
    QHBoxLayout *reductionLayout = new QHBoxLayout;
    reductionLayout->addWidget(reductionLabel);
    reductionLayout->addWidget(d->reductionSpinBox);

    d->smoothCheckbox = new QCheckBox("Smooth mesh", displayWidget);
    d->smoothCheckbox->setChecked(true);

    d->iterationsSpinBox = new QSpinBox(displayWidget);
    d->iterationsSpinBox->setRange(0, 100);
    d->iterationsSpinBox->setSingleStep(1);
    d->iterationsSpinBox->setValue(30);

    QLabel * iterationsLabel = new QLabel("Iterations : ", displayWidget);
    QHBoxLayout * iterationsLayout = new QHBoxLayout;
    iterationsLayout->addWidget(iterationsLabel);
    iterationsLayout->addWidget(d->iterationsSpinBox);

    d->relaxationSpinBox = new QDoubleSpinBox(displayWidget);
    d->relaxationSpinBox->setRange(0.0, 1.0);
    d->relaxationSpinBox->setSingleStep(0.01);
    d->relaxationSpinBox->setDecimals(2);
    d->relaxationSpinBox->setValue(0.2);

    QLabel * relaxationLabel = new QLabel("Relaxation factor : ", displayWidget);
    QHBoxLayout * relaxationLayout = new QHBoxLayout;
    relaxationLayout->addWidget(relaxationLabel);
    relaxationLayout->addWidget(d->relaxationSpinBox);

    QPushButton * runButton = new QPushButton(tr("Run"), displayWidget);
    
    // progression stack
    QWidget *widget = new QWidget(displayWidget);
    d->progression_stack = new medProgressionStack(widget);
    QHBoxLayout *progressStackLayout = new QHBoxLayout;
    progressStackLayout->addWidget(d->progression_stack);
    
    QVBoxLayout *displayLayout = new QVBoxLayout(displayWidget);

    displayLayout->addLayout(thresholdLayout);
    displayLayout->addWidget(d->decimateCheckbox);
    displayLayout->addLayout(reductionLayout);
    displayLayout->addWidget(d->smoothCheckbox);
    displayLayout->addLayout(iterationsLayout);
    displayLayout->addLayout(relaxationLayout);
    displayLayout->addWidget(runButton);
    displayLayout->addLayout(progressStackLayout);
    
    this->addWidget(displayWidget);

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
    
    d->process->setParameter((double)(d->thresholdSlider->value()), 0); // iso value
    d->process->setParameter((double)(d->decimateCheckbox->isChecked()), 1); // decimation
    d->process->setParameter(d->reductionSpinBox->value(), 2); // reduction
    d->process->setParameter((double)(d->smoothCheckbox->isChecked()), 3); // smooth
    d->process->setParameter((double)(d->iterationsSpinBox->value()), 4); // iterations
    d->process->setParameter(d->relaxationSpinBox->value(), 5); // relaxation factor
    
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
