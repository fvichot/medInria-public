/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <pointRoiToolBox.h>
#include <pointRoi.h>

#include <QtGui>

#include <medDataManager.h>
#include <medAbstractDbController.h>
#include <medRoiToolBox.h>
#include <medToolBoxBody.h>
#include <medToolBoxFactory.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>
#include <vtkImageView2D.h>

#include <medAbstractView.h>
#include <medVtkViewBackend.h>
#include <vtkProperty2D.h>
#include <vtkSeedRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>

class toolBoxObserver : public vtkCommand
{
public:
    static toolBoxObserver* New()
    {
        return new toolBoxObserver;
    }

    void Execute ( vtkObject *caller, unsigned long event, void *callData );

    void setView ( vtkImageView2D *view )
    {
        this->view = view;
        //view->AddObserver(vtkImageView2D::SliceChangedEvent,this);
    }

    void setToolBox ( pointRoiToolBox * toolBox )
    {
        this->toolBox = toolBox;
    }

    inline void lock()
    {
        this->m_lock = 1;
    }
    inline void unlock()
    {
        this->m_lock = 0;
    }

protected:
    toolBoxObserver();
    ~toolBoxObserver();

private:
    int m_lock;
    vtkImageView2D *view;
    pointRoiToolBox * toolBox;
};

toolBoxObserver::toolBoxObserver()
{
    this->m_lock = 0;
}

toolBoxObserver::~toolBoxObserver(){}

void toolBoxObserver::Execute ( vtkObject *caller, unsigned long event, void *callData )
{
    if ( this->m_lock )
        return;

    if (!this->view || !toolBox)
        return;

    switch ( event )
    {
    case vtkCommand::PlacePointEvent:
        {
            vtkSeedWidget * seedWidget = dynamic_cast<vtkSeedWidget*>(caller);
            pointRoi * roi = new pointRoi(view,seedWidget);
            toolBox->emitRoiCreated(toolBox->getCurrentView(), roi,"Seeds");
            toolBox->seedMode(true); // create new seedWidget for next point
            break;
        }
    case vtkCommand::EndInteractionEvent:
        {
            
            break;
        }
    }
}

class pointRoiToolBoxPrivate
{
public:
     vtkSmartPointer<vtkSeedWidget> seedWidget;
     medAbstractView * currentView;
     toolBoxObserver * observer;
};

pointRoiToolBox::pointRoiToolBox(QWidget *parent) : medRoiToolBox(parent), d(new pointRoiToolBoxPrivate)
{
    QPushButton *seedButton = new QPushButton("Point");
    seedButton->setCheckable(true);
    this->addWidget(seedButton);
    connect(seedButton, SIGNAL(toggled(bool)), this, SLOT(seedMode(bool)));
    
    d->observer = toolBoxObserver::New();
    d->observer->setToolBox(this);
}

pointRoiToolBox::~pointRoiToolBox()
{
    delete d;
    d = NULL;
}

bool pointRoiToolBox::registered()
{
    return medToolBoxFactory::instance()->registerToolBox<pointRoiToolBox>("pointRoiToolBox",
                                                                                "pointRoiToolBox",
                                                                                "point Roi ToolBox",
                                                                                QStringList()<<"RoiTools");
}

void pointRoiToolBox::seedMode(bool checked) // TODO if currentView changes need to change the renderer of the representation and interactor ...
{ // TODO should be activated via a toggle button if on seedwidget ON else seedWidget Off;
    if(!d->currentView)
        return;

    if (d->seedWidget)
        d->seedWidget->CompleteInteraction();

    d->seedWidget = NULL;

    if (!checked)
        return;
    
    vtkImageView2D * view2d = static_cast<medVtkViewBackend*>(d->currentView->backend())->view2D;

    // Create the representation
    vtkSmartPointer<vtkPointHandleRepresentation2D> handle = vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
    handle->GetProperty()->SetColor(1,0,0);
    vtkSmartPointer<vtkSeedRepresentation> rep = vtkSmartPointer<vtkSeedRepresentation>::New();
    rep->SetHandleRepresentation(handle);
    rep->SetRenderer(view2d->GetRenderer());

    // Seed widget
    d->seedWidget = vtkSmartPointer<vtkSeedWidget>::New();
    d->seedWidget->SetInteractor(view2d->GetInteractor());
    d->seedWidget->SetRepresentation(rep);

    d->seedWidget->AddObserver(vtkCommand::PlacePointEvent,d->observer); 
    d->seedWidget->On();
}

void pointRoiToolBox::update(dtkAbstractView * view)
{
    if (view)
    {
        d->currentView = qobject_cast<medAbstractView*>(view);
        d->observer->setView(static_cast<medVtkViewBackend*>(d->currentView->backend())->view2D);
    }
}

medAbstractView * pointRoiToolBox::getCurrentView()
{
    return d->currentView;
}

void pointRoiToolBox::emitRoiCreated(medAbstractView * view, medAbstractRoi* roi, QString type)
{
    emit roiCreated(view,roi,type);
}

QString pointRoiToolBox::roi_description()
{
    return "Point";
}
