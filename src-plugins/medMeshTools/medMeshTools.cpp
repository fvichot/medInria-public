/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medMeshTools.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

#include <itkImage.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkImageToIsosurface.h>
#include <vtkMetaDataSet.h>
#include <vtkMetaSurfaceMesh.h>

#include <vtkMarchingCubes.h>
#include <vtkSmartPointer.h>

//#include <vtkActor.h>
//#include <vtkPolyDataMapper.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkRenderWindow.h>
//#include <vtkRenderer.h>

// /////////////////////////////////////////////////////////////////
// medMeshToolsPrivate
// /////////////////////////////////////////////////////////////////

class medMeshToolsPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> input;
    dtkSmartPointer <dtkAbstractData> output;
    int isoValue;
    template <class PixelType> int update();
};

template <class PixelType> int medMeshToolsPrivate::update()
{
    typedef itk::Image<PixelType, 3> ImageType;
    typedef itk::ImageToVTKImageFilter<ImageType>  FilterType;

    typename FilterType::Pointer filter = FilterType::New();

    ImageType * img = static_cast<ImageType *>(input->data());

    qDebug() << "Ptr :" << (void*)img << img->GetImageDimension();

    filter->SetInput(img);
    filter->Update();

    vtkImageData * vtkImage = filter->GetOutput();

    double dim[10] = {};
    vtkImage->GetBounds(dim);
    qDebug() << "Ptr :" << (void*)vtkImage << dim[0]<< dim[1]<< dim[2];


    vtkImageToIsosurface * surfacer = vtkImageToIsosurface::New();

    surfacer->SetInput(vtkImage);

    double colors[4] = {0.5, 0.5, 0.5, 0.5};
    surfacer->SetParameters(isoValue, 50, colors);

//    vtkSmartPointer<vtkMarchingCubes> surfacer =
//      vtkSmartPointer<vtkMarchingCubes>::New();

//    surfacer->SetInput(filter->GetOutput());
//    surfacer->ComputeNormalsOn();
//    surfacer->SetValue(0, 400);
    surfacer->Update();

//    qDebug() << surfacer->GetPolyData();

    vtkMetaSurfaceMesh * smesh = vtkMetaSurfaceMesh::New();
    smesh->SetDataSet(surfacer->GetPolyData());

//    smesh->Write("/tmp/test.vtk");

//    vtkSmartPointer<vtkRenderer> renderer =
//      vtkSmartPointer<vtkRenderer>::New();
//    renderer->SetBackground(.1, .2, .3);

//    vtkSmartPointer<vtkRenderWindow> renderWindow =
//      vtkSmartPointer<vtkRenderWindow>::New();
//    renderWindow->AddRenderer(renderer);
//    vtkSmartPointer<vtkRenderWindowInteractor> interactor =
//      vtkSmartPointer<vtkRenderWindowInteractor>::New();
//    interactor->SetRenderWindow(renderWindow);

//    vtkSmartPointer<vtkPolyDataMapper> mapper =
//      vtkSmartPointer<vtkPolyDataMapper>::New();
//    mapper->SetInputConnection(surfacer->GetOutputPort());
//    mapper->ScalarVisibilityOff();

//    vtkSmartPointer<vtkActor> actor =
//      vtkSmartPointer<vtkActor>::New();
//    actor->SetMapper(mapper);

//    renderer->AddActor(actor);

//    renderWindow->Render();
//    interactor->Start();


    output->setData(smesh);

    return EXIT_SUCCESS;
}

// /////////////////////////////////////////////////////////////////
// medMeshTools
// /////////////////////////////////////////////////////////////////

medMeshTools::medMeshTools() : dtkAbstractProcess(), d(new medMeshToolsPrivate)
{
    
}

medMeshTools::~medMeshTools()
{
    
}

bool medMeshTools::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("medMeshTools", createMedMeshTools);
}

QString medMeshTools::description() const
{
    return "medMeshTools";
}

void medMeshTools::setInput ( dtkAbstractData *data )
{
    if ( !data )
        return;
    
    d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( "vtkDataMesh" );
    
    d->input = data;
}    

void medMeshTools::setParameter ( int  data, int channel )
{
    switch (channel) {
        case 0:
            d->isoValue = (int)data;
            qDebug() << d->isoValue;
            break;
    }
}

int medMeshTools::update()
{
    if ( !d->input )
        return -1;
        
    const QString& id = d->input->identifier();

    if (id == "itkDataImageChar3") {
        d->update<char>();
     }
    else if (id == "itkDataImageUChar3") {
        d->update<unsigned char>();
     }
    else if (id == "itkDataImageShort3") {
        d->update<short>();
     }
    else if (id == "itkDataImageUShort3") {
        d->update<unsigned short>();
     }
    else if (id == "itkDataImageInt3") {
        d->update<int>();
     }
    else if (id == "itkDataImageUInt3") {
        d->update<unsigned int>();
     }
    else if (id == "itkDataImageLong3") {
        d->update<long>();
     }
    else if (id== "itkDataImageULong3") {
        d->update<unsigned long>();
     }
    else if (id == "itkDataImageFloat3") {
        d->update<float>();
     }
    else if (id == "itkDataImageDouble3") {
        d->update<double>();
     }
    else
    {
        qDebug() << "Error : pixel type not yet implemented ("
                 << id
                 << ")";
        return -1;
    }
    
    return EXIT_SUCCESS;
}


dtkAbstractData * medMeshTools::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createMedMeshTools()
{
    return new medMeshTools;
}
