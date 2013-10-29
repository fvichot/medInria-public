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

class statisticsLabel : public QWidget
{
public:
    statisticsLabel(QWidget * parent):QWidget(parent)
    {
        /*this->setColumnCount(7);
        this->setRowCount(1);
        QStringList list;
        list.append("Area");
        list.append("Perimeter");
        list.append("Mean");
        list.append("SDev");
        list.append("Sum");
        list.append("Min");
        list.append("Max");
        this->setHorizontalHeaderLabels(list);
        for(int i=0;i<this->columnCount();i++)
        {
            this->setItem(0,i,new QTableWidgetItem());
        }*/
        area = new QLabel();
        max = new QLabel();
        min = new QLabel();
        mean = new QLabel();
        std = new QLabel();
        sum = new QLabel();
        perimeter = new QLabel();
        
        QWidget * firstLine = new QWidget();
        QWidget * secondLine = new QWidget();
        QWidget * thirdLine = new QWidget();
        QHBoxLayout *firstLineLayout =  new QHBoxLayout(firstLine);
        QHBoxLayout *secondLineLayout = new QHBoxLayout(secondLine);
        QHBoxLayout *thirdLineLayout = new QHBoxLayout(thirdLine);
        
        firstLineLayout->addWidget(area);
        firstLineLayout->addWidget(perimeter);
        firstLine->setLayout(firstLineLayout);

        secondLineLayout->addWidget(mean);
        secondLineLayout->addWidget(std);
        secondLineLayout->addWidget(sum);
        secondLine->setLayout(secondLineLayout);
    
        thirdLineLayout->addWidget(min);
        thirdLineLayout->addWidget(max);
        thirdLine->setLayout(thirdLineLayout);
        
        QVBoxLayout * layout = new QVBoxLayout(this);
        layout->addWidget(firstLine);
        layout->addWidget(secondLine);
        layout->addWidget(thirdLine);
        this->setLayout(layout);
        //setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
        //setMaximumSize(320,200);
        //this->adjustSize();
    }
    
    QLabel *area,*max,*min,*mean,*std,*sum,*perimeter;

    void setLabels(double area,double perimeter,double mean,double std,double sum,double min,double max)
    {
        //this->itemAt(0,0)->setText(QString::number(area/100.0)+ " cm2 "); // TODO : changes the metric according to the value of area or perimeter
        //this->itemAt(0,1)->setText(QString::number(perimeter/10.0)+ " cm ");
        //this->itemAt(0,2)->setText(QString::number(mean));
        //this->itemAt(0,3)->setText(QString::number(std));
        //this->itemAt(0,4)->setText(QString::number(sum));
        //this->itemAt(0,5)->setText(QString::number(min));
        //this->itemAt(0,6)->setText(QString::number(max));

        this->area->setText("Area: " + QString::number(area/100.0)+ " cm2"); // TODO : changes the metric according to the value of area or perimeter
        this->perimeter->setText("Perimeter: " + QString::number(perimeter/10.0)+ " cm");
        this->mean->setText("Mean: " + QString::number(mean));
        this->std->setText("StDev: " + QString::number(std));
        this->sum->setText("Sum: " + QString::number(sum));
        this->min->setText("Min: " + QString::number(min));
        this->max->setText("Max: " + QString::number(max));
    }
};


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
    statisticsLabel * statisticsLabel;
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
    
    //d->vtkWidget->setSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
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
    
    d->view->GetRenderer()->AddActor(d->plot);

    d->plotWidget = vtkXYPlotWidget::New();
    d->plotWidget->SetXYPlotActor(d->plot);
    
    d->chart = vtkChartXY::New();
    d->view->GetScene()->AddItem(d->chart);

    d->statisticsLabel = new statisticsLabel(this);

    layout->addWidget(d->statisticsLabel);
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

void medHistogramToolBox::setStatistics(double area,double perimeter,double mean,double std,double sum,double min,double max)
{
    d->statisticsLabel->setLabels(area,perimeter,mean,std,sum,min,max);
}