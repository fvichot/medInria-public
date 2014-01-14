#include "resampleToolBox.h"
#include <medAbstractView.h>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStringList>

#include <medToolBoxFactory.h>
#include <medPluginManager.h>

#include <medDataManager.h>
#include <medMetaDataKeys.h>
#include <dtkCore/dtkAbstractDataFactory.h>

#include <resampleProcess.h>
#include <medVtkViewBackend.h>
#include <vtkImageView2D.h>
#include <vtkImageData.h>

class resampleToolBoxPrivate
{
public:
    medAbstractView * currentView;
    resampleProcess * resample_p;


    QLabel * dimensions, * spacing;
    QRadioButton * bySpacing, * byDimension;
    QLabel *spacingXLab,*spacingYLab,*spacingZLab;
    QSpinBox * spacingX,* spacingY,* spacingZ;
    QLabel *dimXLab,*dimYLab,*dimZLab;
    QSpinBox * dimX,* dimY,* dimZ;
    QPushButton * resample;

    /*QPushButton *runFlowQ;
    QPushButton *openDICOM;
    QPushButton *doSegmentation;
    dtkSmartPointer <dtkAbstractData> dataBMode;
    dtkSmartPointer <dtkAbstractData> dataDoppler;*/

};
resampleToolBox::resampleToolBox (QWidget *parent) : medToolBox (parent), d(new resampleToolBoxPrivate)
{
    this->setTitle("Resample");
    this->setAboutPluginVisibility(false);
    this->setAboutPluginButton(this->plugin());
    // Fill the toolBox
    QWidget *resampleToolBoxBody = new QWidget(this);
    //d->runFlowQ = new QPushButton("Run flow computation", resampleToolBoxBody);
    //d->runFlowQ->setDisabled(true);
    //d->openDICOM = new QPushButton("Open DICOM", resampleToolBoxBody);
    //d->openDICOM->setEnabled(true);

    //d->doSegmentation =new QPushButton("Segment ROI", resampleToolBoxBody);
    //d->runFlowQ->setDisabled(true);

    /*d->dimensions = new QLabel("Dimensions : ",resampleToolBoxBody);
    d->spacing = new QLabel("Spacing : ",resampleToolBoxBody);*/
    d->resample = new QPushButton("Resample image",resampleToolBoxBody);
    QVBoxLayout *resampleToolBoxLayout =  new QVBoxLayout(resampleToolBoxBody);
    /*resampleToolBoxLayout->addWidget(d->dimensions);
    resampleToolBoxLayout->addWidget(d->spacing);*/
    QHBoxLayout * radioButtonLayout = new QHBoxLayout(resampleToolBoxBody);
    d->bySpacing = new QRadioButton("Spacing",resampleToolBoxBody);
    d->byDimension = new QRadioButton("Dimension",resampleToolBoxBody);
    radioButtonLayout->addWidget(d->bySpacing);
    radioButtonLayout->addWidget(d->byDimension);
    QHBoxLayout * spacingSpinBoxLayout = new QHBoxLayout(resampleToolBoxBody);
    d->spacingXLab = new QLabel("spacingX : ");
    d->spacingXLab->hide();
    d->spacingX = new QSpinBox(resampleToolBoxBody);
    d->spacingX->setRange(0,10000);
    d->spacingX->setSuffix(" mm");
    d->spacingX->hide();
    spacingSpinBoxLayout->addWidget(d->spacingXLab);
    spacingSpinBoxLayout->addWidget(d->spacingX);
    d->spacingYLab = new QLabel("spacingY : ");
    d->spacingYLab->hide();
    d->spacingY = new QSpinBox(resampleToolBoxBody);
    d->spacingY->setRange(0,10000);
    d->spacingY->setSuffix(" mm");
    d->spacingY->hide();
    spacingSpinBoxLayout->addWidget(d->spacingYLab);
    spacingSpinBoxLayout->addWidget(d->spacingY);
    d->spacingZLab = new QLabel("spacingZ : ");
    d->spacingZLab->hide();
    d->spacingZ = new QSpinBox(resampleToolBoxBody);
    d->spacingZ->setRange(0,10000);
    d->spacingZ->setSuffix(" mm");
    d->spacingZ->hide();
    spacingSpinBoxLayout->addWidget(d->spacingZLab);
    spacingSpinBoxLayout->addWidget(d->spacingZ);
    QHBoxLayout * dimSpinBoxLayout = new QHBoxLayout(resampleToolBoxBody);
    d->dimXLab = new QLabel("dimX : ");
    d->dimXLab->hide();
    d->dimX = new QSpinBox(resampleToolBoxBody);
    d->dimX->setRange(0,10000);
    d->dimX->setSuffix(" px");
    d->dimX->hide();
    dimSpinBoxLayout->addWidget(d->dimXLab);
    dimSpinBoxLayout->addWidget(d->dimX);
    d->dimYLab = new QLabel("dimY : ");
    d->dimYLab->hide();
    d->dimY = new QSpinBox(resampleToolBoxBody);
    d->dimY->setRange(0,10000);
    d->dimY->setSuffix(" px");
    d->dimY->hide();
    dimSpinBoxLayout->addWidget(d->dimYLab);
    dimSpinBoxLayout->addWidget(d->dimY);
    d->dimZLab = new QLabel("dimZ : ");
    d->dimZLab->hide();
    d->dimZ = new QSpinBox(resampleToolBoxBody);
    d->dimZ->setRange(0,10000);
    d->dimZ->setSuffix(" px");
    d->dimZ->hide();
    dimSpinBoxLayout->addWidget(d->dimZLab);
    dimSpinBoxLayout->addWidget(d->dimZ);
    resampleToolBoxLayout->addLayout(radioButtonLayout);
    resampleToolBoxLayout->addLayout(spacingSpinBoxLayout);
    resampleToolBoxLayout->addLayout(dimSpinBoxLayout);
    resampleToolBoxLayout->addWidget(d->resample);
    //resampleToolBoxLayout->addWidget(d->runFlowQ);
    //resampleToolBoxLayout->addWidget(d->openDICOM);
    //resampleToolBoxLayout->addWidget(d->doSegmentation);
    resampleToolBoxBody->setLayout(resampleToolBoxLayout);
    this->addWidget(resampleToolBoxBody);

    d->resample_p = new resampleProcess();
    // Connections
    connect(d->byDimension,SIGNAL(clicked()),this,SLOT(hideShowSpinBoxes()));
    connect(d->bySpacing,SIGNAL(clicked()),this,SLOT(hideShowSpinBoxes()));
    connect(d->resample,SIGNAL(clicked()),this,SLOT(startResampling()));
}
resampleToolBox::~resampleToolBox()
{
    delete d;
    d = NULL;
}

bool resampleToolBox::registered()
{
    medToolBoxFactory* factory = medToolBoxFactory::instance();
    return factory->registerToolBox<resampleToolBox> ("resampleToolBox",
            "resample",
            "Used to resample an image",
            QStringList()<<"resampleToolBox"
                                                               );
}
dtkPlugin* resampleToolBox::plugin()
{
    medPluginManager* pm = medPluginManager::instance();
    dtkPlugin* plugin = pm->plugin ( "reformatPlugin" );
    return plugin;
}

void resampleToolBox::update(dtkAbstractView* view)
{
    if (d->currentView==qobject_cast<medAbstractView*>(view))
        return;

    d->currentView = qobject_cast<medAbstractView*>(view);

    if (d->currentView)
    {
        displayInfoOnCurrentView();
    }
    else
    {
        d->dimX->setValue(0);
        d->dimY->setValue(0);
        d->dimZ->setValue(0);
        d->spacingX->setValue(0);
        d->spacingY->setValue(0);
        d->spacingZ->setValue(0);
    }
}

void resampleToolBox::displayInfoOnCurrentView()
{
    vtkImageView2D * view2d = static_cast<medVtkViewBackend*>(d->currentView->backend())->view2D;
    double* spacing = view2d->GetInput()->GetSpacing();
    int* dimensions = view2d->GetInput()->GetDimensions();

    //d->dimensions->setText("Dimensions : " + QString::number(dimensions[0]) + " " + QString::number(dimensions[1]) + " " + QString::number(dimensions[2]));
    //// TODO : dimensions does not work for godin image .... I dont know why
    //d->spacing->setText("Spacing : " + QString::number(spacing[0]) + " " + QString::number(spacing[1]) + " " + QString::number(spacing[2]));

    d->dimX->setValue(dimensions[0]);
    d->dimY->setValue(dimensions[1]);
    d->dimZ->setValue(dimensions[2]);
    d->spacingX->setValue(spacing[0]);
    d->spacingY->setValue(spacing[1]);
    d->spacingZ->setValue(spacing[2]);
}

void resampleToolBox::hideShowSpinBoxes()
{
    if (d->bySpacing->isChecked())
    {
        d->dimX->hide();d->dimXLab->hide();
        d->dimY->hide();d->dimYLab->hide();
        d->dimZ->hide();d->dimZLab->hide();
        d->spacingX->show();d->spacingXLab->show();
        d->spacingY->show();d->spacingYLab->show();
        d->spacingZ->show();d->spacingZLab->show();
    }
    else
    {
        d->dimX->show();d->dimXLab->show();
        d->dimY->show();d->dimYLab->show();
        d->dimZ->show();d->dimZLab->show();
        d->spacingX->hide();d->spacingXLab->hide();
        d->spacingY->hide();d->spacingYLab->hide();
        d->spacingZ->hide();d->spacingZLab->hide();
    }
}

void resampleToolBox::startResampling()
{
    if (d->bySpacing->isChecked())
    {
        d->resample_p->setParameter((double)(d->dimX->value()),0);
        d->resample_p->setParameter((double)(d->dimY->value()),0);
        d->resample_p->setParameter((double)(d->dimZ->value()),0);
    }
    else
    {
        d->resample_p->setParameter((double)(d->spacingX->value()),0);
        d->resample_p->setParameter((double)(d->spacingY->value()),0);
        d->resample_p->setParameter((double)(d->spacingZ->value()),0);
    }

    dtkAbstractData *currentData = reinterpret_cast< dtkAbstractData * >( d->currentView->data() );
    d->resample_p->setInput(currentData);
    d->resample_p->update();
}