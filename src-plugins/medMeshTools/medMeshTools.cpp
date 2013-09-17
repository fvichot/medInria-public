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
#include <vtkTriangleFilter.h>
#include <vtkSmartPointer.h>
#include <vtkAlgorithmOutput.h>

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
    double isoValue;
    double targetReduction;
    bool decimate;
    bool smooth;
    int iterations;
    double relaxationFactor;


    template <class PixelType> int update();
};

template <class PixelType> int medMeshToolsPrivate::update()
{
    typedef itk::Image<PixelType, 3> ImageType;
    typedef itk::ImageToVTKImageFilter<ImageType>  FilterType;

    typename FilterType::Pointer filter = FilterType::New();

    ImageType * img = static_cast<ImageType *>(input->data());

    filter->SetInput(img);
    filter->Update();

    vtkImageData * vtkImage = filter->GetOutput();

    vtkContourFilter* contour = vtkContourFilter::New();
    contour->SetInput( vtkImage );
    contour->SetValue(0, isoValue);
    contour->Update();

    vtkTriangleFilter* contourTrian = vtkTriangleFilter::New();
    contourTrian->SetInputConnection(contour->GetOutputPort());
    contourTrian->PassVertsOn();
    contourTrian->PassLinesOn();
    contourTrian->Update();

    vtkAlgorithmOutput * outputPort = contourTrian->GetOutputPort();
    vtkPolyDataAlgorithm * lastAlgo = contourTrian;

    vtkDecimatePro* contourDecimated = 0;
    if (decimate) {
        // Decimate the mesh if required
        contourDecimated = vtkDecimatePro::New();
        contourDecimated->SetInputConnection(outputPort);
        contourDecimated->SetTargetReduction(targetReduction);
        contourDecimated->SplittingOff();
        contourDecimated->PreserveTopologyOn();
        contourDecimated->Update();
        outputPort = contourDecimated->GetOutputPort();
        lastAlgo = contourDecimated;
    }

    vtkSmoothPolyDataFilter* contourSmoothed = 0;
    if(smooth) {
        // Smooth the mesh if required
        contourSmoothed = vtkSmoothPolyDataFilter::New();
        contourSmoothed->SetInputConnection(outputPort);
        contourSmoothed->SetNumberOfIterations(iterations);
        contourSmoothed->SetRelaxationFactor(relaxationFactor);
        contourSmoothed->Update();
        outputPort = contourSmoothed->GetOutputPort();
        lastAlgo = contourSmoothed;
    }

    vtkMetaSurfaceMesh * smesh = vtkMetaSurfaceMesh::New();
    smesh->SetDataSet(lastAlgo->GetOutput());

    contour->Delete();
    contourTrian->Delete();
    if (contourDecimated) contourDecimated->Delete();
    if (contourSmoothed) contourSmoothed->Delete();

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

void medMeshTools::setParameter ( double data, int channel )
{
    switch (channel) {
        case 0:
            d->isoValue = data;
            break;
        case 1:
            d->decimate = (data > 0) ? true : false;
            break;
        case 2:
            d->targetReduction = data;
            break;
        case 3:
            d->smooth = (data > 0) ? true : false;
            break;
        case 4:
            d->iterations = data;
            break;
        case 5:
            d->relaxationFactor = data;
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
