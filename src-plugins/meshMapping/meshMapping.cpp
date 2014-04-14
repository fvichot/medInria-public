/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "meshMapping.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

#include <medMetaDataKeys.h>
#include <medAbstractDataImage.h>
#include <medDataManager.h>

#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkMetaDataSet.h>
#include <vtkMetaSurfaceMesh.h>
#include <vtkMatrix4x4.h>
#include <vtkImageGradientMagnitude.h>
#include <vtkProbeFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMetaSurfaceMesh.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkImageCast.h>

#include <itkImage.h>
#include <itkImageToVTKImageFilter.h>
#include <itkMaskImageFilter.h>

// /////////////////////////////////////////////////////////////////
// meshMappingPrivate
// /////////////////////////////////////////////////////////////////

class meshMappingPrivate
{
    public:
        dtkSmartPointer <dtkAbstractData> mesh;
        dtkSmartPointer <dtkAbstractData> image;
        dtkSmartPointer <dtkAbstractData> output;

    template <class PixelType> int update()
    {
        typedef itk::Image<PixelType, 3> ImageType;

        if ( !image ||!image->data() || !mesh ||!mesh->data())
            return -1;

        //Converting the mesh
        if(!mesh->identifier().contains("vtkDataMesh"))
            return 0;
        vtkMetaDataSet * _dataset = static_cast<vtkMetaDataSet*>(mesh->data());
        vtkPolyData * polyDataMesh = static_cast<vtkPolyData*>(_dataset->GetDataSet());


        // Converting the image
        typedef itk::ImageToVTKImageFilter<ImageType> FilterType;
        typename FilterType::Pointer filter = FilterType::New();
        typename ImageType::Pointer img = static_cast<ImageType *>(image->data());
        filter->SetInput(img);
        filter->Update();

        // ----- Hack to keep the itkImages info (origin and orientation)
        vtkMatrix4x4* matrix = vtkMatrix4x4::New();
        matrix->Identity();
        for (unsigned int x=0; x<3; x++) {
            for (unsigned int y=0; y<3; y++)
            {
                matrix->SetElement(x,y,img->GetDirection()[x][y]);
            }
        }
        typename itk::ImageBase<3>::PointType origin = img->GetOrigin();
        double v_origin[4], v_origin2[4];
        for (int i=0; i<3; i++)
            v_origin[i] = origin[i];
        v_origin[3] = 1.0;
        matrix->MultiplyPoint (v_origin, v_origin2);
        for (int i=0; i<3; i++)
            matrix->SetElement (i, 3, v_origin[i]-v_origin2[i]);
    //------------------------------------------------------

        vtkImageData * vtkImage = filter->GetOutput();

        vtkImageCast* cast = vtkImageCast::New();
        cast->SetInput(vtkImage);
        cast->SetOutputScalarTypeToFloat();

        // Probe magnitude with iso-surface.
        vtkProbeFilter* probe = vtkProbeFilter::New();
        probe->SetInput(polyDataMesh);
        probe->SetSource(cast->GetOutput());
        probe->SpatialMatchOn();
        probe->Update();
        vtkPolyData * polydata = probe->GetPolyDataOutput();

        //To get the itkImage infos back
        vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
        t->SetMatrix(matrix);

        vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        transformFilter->SetInput(polydata);
        transformFilter->SetTransform(t);
        transformFilter->Update();

        polydata->DeepCopy(transformFilter->GetOutput());

        vtkMetaSurfaceMesh * smesh = vtkMetaSurfaceMesh::New();
        smesh->SetDataSet(polydata);


        output->setData(smesh);

        return EXIT_SUCCESS;
    }
};

// /////////////////////////////////////////////////////////////////
// meshMapping
// /////////////////////////////////////////////////////////////////

meshMapping::meshMapping() : dtkAbstractProcess(), d(new meshMappingPrivate)
{
    d->image = NULL;
    d->mesh = NULL;
    d->output = NULL;
}

meshMapping::~meshMapping()
{
    
}

bool meshMapping::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("meshMapping", createMeshMapping);
}

QString meshMapping::description() const
{
    return "meshMapping";
}

void meshMapping::setInput ( dtkAbstractData *data, int channel)
{
    if ( !data )
        return;
    if ( channel == 0)
    {
        QString identifier = data->identifier();
        d->mesh = data;
    }

    if ( channel == 1 )
    {
        QString identifier = data->identifier();
        d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( "vtkDataMesh" );
        d->image = data;
    }
}


void meshMapping::setParameter ( double  data, int channel )
{
    // Here comes a switch over channel to handle parameters
}

int meshMapping::update()
{
    if ( !d->image )
        return -1;

    QString id = d->image->identifier();

    qDebug() << "itkFilters, update : " << id;

    if ( id == "itkDataImageChar3" )
    {
        d->update<char>();
    }
    else if ( id == "itkDataImageUChar3" )
    {
        d->update<unsigned char>();
    }
    else if ( id == "itkDataImageShort3" )
    {
        d->update<short>();
    }
    else if ( id == "itkDataImageUShort3" )
    {
        d->update<unsigned short>();
    }
    else if ( id == "itkDataImageInt3" )
    {
        d->update<int>();
    }
    else if ( id == "itkDataImageUInt3" )
    {
        d->update<unsigned int>();
    }
    else if ( id == "itkDataImageLong3" )
    {
        d->update<long>();
    }
    else if ( id== "itkDataImageULong3" )
    {
        d->update<unsigned long>();
    }
    else if ( id == "itkDataImageFloat3" )
    {
        d->update<float>();
    }
    else if ( id == "itkDataImageDouble3" )
    {
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

dtkAbstractData * meshMapping::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createMeshMapping()
{
    return new meshMapping;
}
