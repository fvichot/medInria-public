/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <dtkCore/dtkAbstractData.h>

#include <medHistogramToolBox.h>
#include <medToolBox.h>
#include <medCore/medAbstractView.h>

#include <QVTKWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkXYPlotActor.h>
#include <vtkXYPlotWidget.h>
#include <vtkImageData.h>
#include <medButton.h>

#include <dtkCore/dtkGlobal.h>
#include <dtkCore/dtkPlugin>
#include <dtkGui/dtkAboutPlugin.h>

#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkChartXY.h>
#include <vtkTable.h>
#include <vtkPlot.h>
#include <vtkPen.h>
#include <vtkAxis.h>

class medHistogramToolBoxPrivate
{
public:
    QVTKWidget * vtkWidget;
    vtkRenderWindow *renWin;
    vtkRenderWindowInteractor * interactor;
    vtkContextView * view;
    vtkRenderer * ren;
    vtkXYPlotActor * plot;
    vtkXYPlotWidget * plotWidget;
    vtkChartXY * chart;
};

medHistogramToolBox::medHistogramToolBox(QWidget *parent) : medToolBox(parent), d(new medHistogramToolBoxPrivate)
{
    this->setTitle("HistogramToolBox");
    
    QWidget *displayWidget = new QWidget(this);
    this->addWidget(displayWidget);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    displayWidget->setLayout(layout);

    d->vtkWidget = new QVTKWidget (displayWidget);
    
    d->vtkWidget->setSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    d->vtkWidget->setMinimumSize(320,240);
    d->vtkWidget->setFocusPolicy ( Qt::NoFocus );

    d->view=vtkContextView::New();
    d->view->GetRenderer()->SetBackground(0.8,0.8,0.8);
    d->view->SetInteractor(d->vtkWidget->GetInteractor());
    d->vtkWidget->SetRenderWindow(d->view->GetRenderWindow());
       
    // Create a vtkXYPlotActor
    d->plot = vtkXYPlotActor::New();
    d->plot->ExchangeAxesOff();
    
    d->plot->SetLabelFormat( "%g" );
    d->plot->SetXTitle( "Grayscale" );
    d->plot->SetYTitle( "Number of pixels" );
    d->plot->SetXValuesToValue();  
    
    //d->plot->adu
    
    d->view->GetRenderer()->AddActor(d->plot);

    d->plotWidget = vtkXYPlotWidget::New();
    d->plotWidget->SetXYPlotActor(d->plot);
    
    d->chart = vtkChartXY::New();
    d->view->GetScene()->AddItem(d->chart);

    layout->addWidget(d->vtkWidget);
}

medHistogramToolBox::~medHistogramToolBox(void)
{
    delete d;
    d = NULL;
}

void medHistogramToolBox::setPlotInput(vtkDataSet * data,double xmax,double ymax)
{
    d->plot->RemoveAllInputs(); // For the time being we consider only one input;
    d->plot->AddInput(data);
    d->plot->SetPlotColor(0,0,0,1);
    d->plot->SetXRange(0,xmax); // becarefull 0 is not the min value of the roi so need to get that info xmin
    d->plot->SetYRange(0,ymax);
    d->plot->SetWidth(1);
    d->plot->SetPosition(0,0);
    d->plot->SetHeight(1);
    d->plot->SetTitle("Histogram of selected ROI");
    d->plot->SetPlotPoints(1);
    d->vtkWidget->update();
    d->view->GetInteractor()->Initialize();
    d->plotWidget->On();
}

void medHistogramToolBox::setChartInput(vtkTable * table)
{
    d->chart->ClearPlots();
    vtkPlot * line = d->chart->AddPlot(vtkChart::LINE);
    line->SetInput(table,0,1);
    line->SetColor(0,0,0,255);
    line->GetPen()->SetLineType(vtkPen::SOLID_LINE);
    d->chart->GetAxis(0)->SetTitle("Number of pixels");
    d->chart->GetAxis(1)->SetTitle("Grayscale");
    d->chart->SetTitle("Histogram of selected ROI");
    d->chart->RecalculateBounds();
    d->chart->SetForceAxesToBounds(1);
    d->chart->Update();
}