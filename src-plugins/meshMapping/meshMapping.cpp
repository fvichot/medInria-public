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

#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkMetaDataSet.h>
#include <vtkMetaSurfaceMesh.h>
#include <vtkMatrix4x4.h>
#include <vtkImageGradientMagnitude.h>
#include <vtkProbeFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkImageCast.h>

#include <itkImage.h>
#include <itkImageToVTKImageFilter.h>

// /////////////////////////////////////////////////////////////////
// meshMappingPrivate
// /////////////////////////////////////////////////////////////////

class meshMappingPrivate
{
    public:
        dtkSmartPointer <dtkAbstractData> structure;
        dtkSmartPointer <dtkAbstractData> data;
        dtkSmartPointer <dtkAbstractData> output;

    template <class PixelType> int mapImageOnMesh()
    {
        typedef itk::Image<PixelType, 3> ImageType;

        if ( !data ||!data->data() || !structure ||!structure->data())
            return -1;

        //Converting the mesh
        if(!structure->identifier().contains("vtkDataMesh"))
            return -1;
        vtkMetaDataSet * structureDataset = static_cast<vtkMetaDataSet*>(structure->data());
        vtkPolyData * structurePolydata = static_cast<vtkPolyData*>(structureDataset->GetDataSet());

        // Converting the image
        typedef itk::ImageToVTKImageFilter<ImageType> FilterType;
        typename FilterType::Pointer filter = FilterType::New();
        typename ImageType::Pointer img = static_cast<ImageType *>(data->data());
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

        vtkImageCast * cast = vtkImageCast::New();
        cast->SetInput(vtkImage);
        cast->SetOutputScalarTypeToFloat();

        // Probe magnitude with iso-surface.
        vtkProbeFilter* probe = vtkProbeFilter::New();
        probe->SetInput(structurePolydata);
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

    int mapMeshOnMesh()
    {
        if ( !data ||!data->data() || !structure ||!structure->data())
            return -1;

        //Converting the meshes
        if(!structure->identifier().contains("vtkDataMesh"))
            return -1;
        vtkMetaDataSet * structureDataset = static_cast<vtkMetaDataSet*>(structure->data());
        vtkPolyData * structurePolydata = static_cast<vtkPolyData*>(structureDataset->GetDataSet());

        vtkMetaDataSet * dataDataset = static_cast<vtkMetaDataSet*>(data->data());
        vtkPolyData * dataPolydata = static_cast<vtkPolyData*>(dataDataset->GetDataSet());

        // Probe magnitude with iso-surface.
        vtkProbeFilter* probe = vtkProbeFilter::New();
        probe->SetInput(structurePolydata);
        probe->SetSource(dataPolydata);
        probe->SpatialMatchOn();
        probe->Update();
        vtkPolyData * polydata = probe->GetPolyDataOutput();

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
    d->data = NULL;
    d->structure = NULL;
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
        d->structure = data;
    }

    if ( channel == 1 )
    {
        QString identifier = data->identifier();
        d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( "vtkDataMesh" );
        d->data = data;
    }
}


void meshMapping::setParameter ( double  data, int channel )
{
    // Here comes a switch over channel to handle parameters
}

int meshMapping::update()
{
    if ( !d->data )
        return -1;

    QString id = d->data->identifier();

    qDebug() << "itkFilters, update : " << id;

    if ( id == "itkDataImageChar3" ) //TODO: create new function for meshes
    {
        d->mapImageOnMesh<char>();
    }
    else if ( id == "itkDataImageUChar3" )
    {
        d->mapImageOnMesh<unsigned char>();
    }
    else if ( id == "itkDataImageShort3" )
    {
        d->mapImageOnMesh<short>();
    }
    else if ( id == "itkDataImageUShort3" )
    {
        d->mapImageOnMesh<unsigned short>();
    }
    else if ( id == "itkDataImageInt3" )
    {
        d->mapImageOnMesh<int>();
    }
    else if ( id == "itkDataImageUInt3" )
    {
        d->mapImageOnMesh<unsigned int>();
    }
    else if ( id == "itkDataImageLong3" )
    {
        d->mapImageOnMesh<long>();
    }
    else if ( id== "itkDataImageULong3" )
    {
        d->mapImageOnMesh<unsigned long>();
    }
    else if ( id == "itkDataImageFloat3" )
    {
        d->mapImageOnMesh<float>();
    }
    else if ( id == "itkDataImageDouble3" )
    {
        d->mapImageOnMesh<double>();
    }
    else if ( id == "vtkDataMesh" )
    {
        d->mapMeshOnMesh();
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
