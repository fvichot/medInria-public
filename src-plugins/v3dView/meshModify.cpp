/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/


#include <QDebug>

#include <dtkCore/dtkAbstractData>
#include <dtkCore/dtkAbstractDataFactory>

#include <medMetaDataKeys.h>
#include <meshModify.h>
#include <medToolBoxFactory.h>
#include <medDataManager.h>

#include <vtkActor.h>
#include <vtkBoxWidget.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkTransform.h>
#include <vtkPolyData.h>

#include <vtkImageView3D.h>
#include <vtkMetaDataSet.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMetaSurfaceMesh.h>

class vtkMyCallback : public vtkCommand
{
public:
    static vtkMyCallback *New()
    { return new vtkMyCallback; }
    virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
        vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
        vtkBoxWidget * widget = reinterpret_cast<vtkBoxWidget*>(caller);
        widget->GetTransform(t);
        for(int i = 0; i < _dataset->GetNumberOfActors(); i++) {
            _dataset->GetActor(i)->SetUserTransform(t);
        }

    }
    void setDataSet(vtkMetaDataSet * dataset) {_dataset = dataset;}

private:
    vtkMetaDataSet * _dataset;
};


meshModifyToolBox::meshModifyToolBox(QWidget * parent)
    : medToolBox(parent)
{
    this->setTitle(tr("Mesh manipulation"));
    QWidget * w = new QWidget(this);
    this->addWidget(w);
    w->setLayout(new QVBoxLayout);
    _modifyButton = new QPushButton("Modify");
    _modifyButton->setEnabled(false);
    _modifying = true;
    w->layout()->addWidget(_modifyButton);

    connect(_modifyButton, SIGNAL(clicked()), this, SLOT(toggleWidget()));
}


meshModifyToolBox::~meshModifyToolBox()
{
    
}


bool meshModifyToolBox::registered()
{
    return medToolBoxFactory::instance()->registerToolBox<meshModifyToolBox>("meshModifyToolbox",
                                                                             "meshModifyToolbox",
                                                                             "Toolbox to translate/rotate a mesh",
                                                                             QStringList() << "mesh" << "view");
}


QString meshModifyToolBox::description() const
{
    return "meshModify";
}


void meshModifyToolBox::update(dtkAbstractView * view)
{
    v3dView * view3d = qobject_cast<v3dView*>(view);
    if (! view3d) {
        _modifyButton->setEnabled(false);
        return;
    }

    _view = view3d;
    _modifyButton->setEnabled(true);
}


void meshModifyToolBox::toggleWidget()
{
    dtkAbstractData * data = _view->dataInList(_view->currentLayer());
    if ( ! data->identifier().contains("vtkDataMesh"))
        return;

    _dataset = reinterpret_cast<vtkMetaDataSet*>(data->data());
    if ( ! _dataset ) return;

    vtkPolyData * polydata = dynamic_cast<vtkPolyData*>(_dataset->GetDataSet());
    if ( ! polydata ) return;

    if (_modifying) {
        _boxWidget = vtkSmartPointer<vtkBoxWidget>::New();
        _boxWidget->SetInteractor(_view->view3d()->GetInteractor());
        _boxWidget->SetPlaceFactor(1.25);

        double bounds[6] = {}; // init to zero
        polydata->GetBounds(bounds);
        _boxWidget->PlaceWidget(bounds);
        _callback = vtkSmartPointer<vtkMyCallback>::New();
        _callback->setDataSet(_dataset);
        _boxWidget->AddObserver(vtkCommand::InteractionEvent, _callback);

        _boxWidget->On();

    } else {
        vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
        _boxWidget->GetTransform(t);

        vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
          vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        transformFilter->SetInput(polydata);
        transformFilter->SetTransform(t);
        transformFilter->Update();

        vtkPolyData * newPolydata = vtkPolyData::New();
        newPolydata->DeepCopy(transformFilter->GetOutput());

        dtkSmartPointer<dtkAbstractData> newData = dtkAbstractDataFactory::instance()->createSmartPointer("vtkDataMesh");

        newData->setMetaData(medMetaDataKeys::PatientName.key(), "John Doe");
        newData->setMetaData(medMetaDataKeys::StudyDescription.key(), "generated");
        newData->setMetaData(medMetaDataKeys::SeriesDescription.key(), "generated mesh");

        vtkMetaSurfaceMesh * smesh = vtkMetaSurfaceMesh::New();
        smesh->SetDataSet(newPolydata);
        newData->setData(smesh);
        medDataManager::instance()->importNonPersistent( newData.data() );
//        _view->setSharedDataPointer(newData);

        // reset transforms on the original
        vtkSmartPointer<vtkTransform> t_id = vtkSmartPointer<vtkTransform>::New();
        for(int i = 0; i < _dataset->GetNumberOfActors(); i++) {
            _dataset->GetActor(i)->SetUserTransform(t_id);
        }

        _boxWidget->Off();
    }

    _modifying = ! _modifying;
    _modifyButton->setText(_modifying ? "Modify" : "Save");
}


void meshModifyToolBox::dataAdded(dtkAbstractData * data, int index)
{
    if (data->identifier().contains("vtkDataMesh"))
    _modifyButton->setEnabled(true);
}
