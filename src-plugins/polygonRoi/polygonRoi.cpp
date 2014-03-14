/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "polygonRoi.h"
#include <vtkContourWidget.h>
#include <vtkContourOverlayRepresentation.h>
#include <vtkSmartPointer.h>
#include <vtkProperty2D.h>
#include <vtkImageView2D.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkProperty.h>

class polygonRoiObserver : public vtkCommand
{
public:
    static polygonRoiObserver* New()
    {
        return new polygonRoiObserver;
    }

    void Execute ( vtkObject *caller, unsigned long event, void *callData );

    void setRoi ( polygonRoi *roi )
    {
        this->roi = roi;
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
    polygonRoiObserver();
    ~polygonRoiObserver();

private:
    int m_lock;
    polygonRoi * roi;
};

polygonRoiObserver::polygonRoiObserver()
{
    this->m_lock = 0;
}

polygonRoiObserver::~polygonRoiObserver(){}

void polygonRoiObserver::Execute ( vtkObject *caller, unsigned long event, void *callData )
{
    if (this->m_lock )
        return;

    if (!this->roi->getView())
        return;

    switch ( event )
    {
        case vtkImageView2D::SliceChangedEvent:
        default:
        {
            roi->showOrHide(roi->getView()->GetViewOrientation(),roi->getView()->GetSlice());
            break;
        }
        case vtkCommand::EndInteractionEvent:
        {
            emit roi->selected(); // will trigger the computation of new statistics
        }
    }
}

class polygonRoiPrivate
{
public:
    ~polygonRoiPrivate()
    {
        contour->Delete();
        view->RemoveObserver(observer);
        observer->Delete();
    }

    vtkContourWidget * contour;
    //unsigned int idSlice;
    //unsigned char orientation; // 0 => Sagittal ... 1 => Coronal ... 2 => Axial
    vtkImageView2D *view;
    polygonRoiObserver *observer;
};

polygonRoi::polygonRoi(vtkImageView2D * view, medAbstractRoi *parent )
    : medAbstractRoi(parent)
    , d(new polygonRoiPrivate)
{
    vtkSmartPointer<vtkContourOverlayRepresentation> contourRep = vtkSmartPointer<vtkContourOverlayRepresentation>::New();
    contourRep->GetLinesProperty()->SetColor(0, 0, 1); 
    contourRep->GetLinesProperty()->SetLineWidth(1);
    contourRep->GetProperty()->SetPointSize(4);

    d->contour = vtkContourWidget::New();
    d->contour->SetRepresentation(contourRep);
    contourRep->SetRenderer(view->GetRenderer());
    d->contour->SetInteractor(view->GetInteractor());

    d->contour->GetEventTranslator()->SetTranslation(vtkCommand::RightButtonPressEvent,NULL);
    
    d->view = view;
    setOrientation(view->GetViewOrientation());
    setIdSlice(view->GetSlice());
    d->observer = polygonRoiObserver::New();
    d->observer->setRoi(this);
    d->view->AddObserver(vtkImageView2D::SliceChangedEvent,d->observer,0);
    d->contour->AddObserver(vtkCommand::EndInteractionEvent,d->observer,0);
}


polygonRoi::~polygonRoi( void )
{
    delete d;
    d = NULL;
}

vtkContourWidget * polygonRoi::getContour()
{
    return d->contour;
}

void polygonRoi::Off()
{
    d->contour->Off();
}
void polygonRoi::On()
{
    d->contour->On();
}

void polygonRoi::showOrHide(int orientation,int idSlice)
{
    if (!d->view->GetRenderWindow())
        return;

    if (getIdSlice()==idSlice && getOrientation()==orientation)
        On();
    else
        Off();
}

//unsigned int polygonRoi::getIdSlice()
//{
//    return d->idSlice;
//}
//
//void polygonRoi::setIdSlice(unsigned int idSlice)
//{
//    d->idSlice=idSlice;
//}
//
//unsigned char polygonRoi::getOrientation()
//{
//    return d->orientation;
//}
//
//void polygonRoi::setOrientation(unsigned char orientation)
//{
//    d->orientation=orientation;
//}

vtkImageView2D * polygonRoi::getView()
{
    return d->view;
}

void polygonRoi::lockObserver(bool val)
{
    if (val)
        d->observer->lock();
    else
        d->observer->unlock();
}

QString polygonRoi::type()
{
    return "Polygon";
}

QString polygonRoi::info()
{
    QString info = "Orientation : ";
    switch (getOrientation())
    {
    case 0:
        info = info + "Sagittal - "; 
        break;
    case 1:
        info = info + "Coronal - ";
        break;
    case 2:
        info = info + "Axial - "; 
        break;
    }

    return info + " Slice " + QString::number(getIdSlice()+1);
}

void polygonRoi::select()
{
    vtkContourOverlayRepresentation * contourRep = dynamic_cast<vtkContourOverlayRepresentation*>(d->contour->GetContourRepresentation());
    contourRep->GetLinesProperty()->SetColor(1, 0.533, 0.2);
    medAbstractRoi::select();
}

void polygonRoi::unselect()
{
    vtkContourOverlayRepresentation * contourRep = dynamic_cast<vtkContourOverlayRepresentation*>(d->contour->GetContourRepresentation());
    contourRep->GetLinesProperty()->SetColor(0, 0, 1);
    medAbstractRoi::unselect();
}

void polygonRoi::computeRoiStatistics(){}
