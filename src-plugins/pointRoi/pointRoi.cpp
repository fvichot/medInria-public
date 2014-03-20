/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "pointRoi.h"
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSeedRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkProperty2D.h>
#include <vtkHandleWidget.h>



class vtkSeedCallback : public vtkCommand
{
public:
    static vtkSeedCallback *New()
    { 
        return new vtkSeedCallback; 
    }

    vtkSeedCallback() {}

    void setView ( vtkImageView2D *view )
    {
        this->view = view;
        //view->AddObserver(vtkImageView2D::SliceChangedEvent,this);
    }

    virtual void Execute(vtkObject*, unsigned long event, void *calldata)
    {
        if(event == vtkCommand::StartInteractionEvent)
        {
            // when clicking update slice and orientation
            roi->setIdSlice(roi->getView()->GetSlice());
            roi->setOrientation(roi->getView()->GetViewOrientation());
        }

        if(event == vtkImageView2D::SliceChangedEvent)
        {
            roi->showOrHide(roi->getView()->GetViewOrientation(),roi->getView()->GetSlice());
        }
    }

    void setRoi(pointRoi *roi)
    {
        this->roi = roi;
    }

private:
    pointRoi* roi;
    vtkImageView2D *view;
};

class pointRoiPrivate
{
public : 
    pointRoiPrivate(){};
    ~pointRoiPrivate()
    {
        view->RemoveObserver(seedCallback);
    };
    vtkImageView2D *view;
    vtkSmartPointer<vtkSeedWidget> seedWidget;
    vtkSmartPointer<vtkSeedCallback> seedCallback;
};

pointRoi::pointRoi(vtkImageView2D * view, vtkSeedWidget * widget,medAbstractRoi *parent )
    : medAbstractRoi(parent)
    , d(new pointRoiPrivate)
{
    // Create the representation
    d->seedWidget = widget;
    d->seedWidget->CompleteInteraction();
    d->view = view;
    setOrientation(view->GetViewOrientation());
    setIdSlice(view->GetSlice());

    d->seedCallback = vtkSmartPointer<vtkSeedCallback>::New();
    d->seedCallback->setRoi(this);
    d->view->AddObserver(vtkImageView2D::SliceChangedEvent,d->seedCallback);
}

pointRoi::~pointRoi()
{
    delete d;
    d= NULL;
}

void pointRoi::Off()
{
    vtkHandleWidget * handleWidget = d->seedWidget->GetSeed(0); 
    vtkPointHandleRepresentation2D*  handleRep = dynamic_cast<vtkPointHandleRepresentation2D*>(handleWidget->GetRepresentation()); 
    handleWidget->Off();
}
void pointRoi::On()
{
    vtkHandleWidget * handleWidget = d->seedWidget->GetSeed(0); 
    vtkPointHandleRepresentation2D*  handleRep = dynamic_cast<vtkPointHandleRepresentation2D*>(handleWidget->GetRepresentation()); 
    handleWidget->On();
}

QString pointRoi::info(){return QString();}
QString pointRoi::type(){return "Point";}
void pointRoi::computeRoiStatistics(){}

vtkImageView2D * pointRoi::getView()
{
    return d->view;
}

void pointRoi::showOrHide(int orientation, int idSlice)
{
    if (!d->view->GetRenderWindow())
        return;
    if (getIdSlice()==idSlice && getOrientation()==orientation)
        On();
    else
        Off();
}

void pointRoi::select()
{
    vtkHandleWidget * handleWidget = d->seedWidget->GetSeed(0); 
    vtkPointHandleRepresentation2D*  handleRep = dynamic_cast<vtkPointHandleRepresentation2D*>(handleWidget->GetRepresentation()); 
    handleRep->GetProperty()->SetColor(0,1,0);
    medAbstractRoi::select();
}

void pointRoi::unselect()
{
    vtkHandleWidget * handleWidget = d->seedWidget->GetSeed(0);
    vtkPointHandleRepresentation2D*  handleRep = dynamic_cast<vtkPointHandleRepresentation2D*>(handleWidget->GetRepresentation()); 
    handleRep->GetProperty()->SetColor(1,0,0);
    medAbstractRoi::unselect();
}

vtkSeedWidget * pointRoi::getSeedWidget()
{
    return d->seedWidget;
}