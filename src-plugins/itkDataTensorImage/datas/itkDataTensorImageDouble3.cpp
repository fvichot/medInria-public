/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <itkDataTensorImageDouble3.h>

#include <medAbstractDataFactory.h>

#include <itkImage.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageFileWriter.h>
#include <itkVectorImage.h>
#include <itkVector.h>
#include <itkTensor.h>


// /////////////////////////////////////////////////////////////////
// itkDataTensorImagePrivate
// /////////////////////////////////////////////////////////////////

class itkDataTensorImageDouble3Private
{
public:
    typedef itk::Tensor<double, 3>    TensorType;
    typedef itk::Image<TensorType, 3> TensorImageType;

    TensorImageType::Pointer tensors;
};

// /////////////////////////////////////////////////////////////////
// itkDataTensorImage
// /////////////////////////////////////////////////////////////////

itkDataTensorImageDouble3::itkDataTensorImageDouble3() : medAbstractTypedDiffusionModelImageData<3,double>(), d(new itkDataTensorImageDouble3Private)
{
    d->tensors = 0;

}

itkDataTensorImageDouble3::~itkDataTensorImageDouble3()
{
    delete d;
    d = 0;
}

bool itkDataTensorImageDouble3::registered()
{
    return medAbstractDataFactory::instance()->registerDataType("itkDataTensorImageDouble3", createItkDataTensorImageDouble3);
}

QString itkDataTensorImageDouble3::description() const
{
    return tr("itk tensor 3d image data (double)");
}


QString itkDataTensorImageDouble3::identifier() const
{
    return "itkDataTensorImageDouble3";
}

void *itkDataTensorImageDouble3::data()
{
    return d->tensors.GetPointer();
}

void *itkDataTensorImageDouble3::output()
{
    return d->tensors.GetPointer();
}

void itkDataTensorImageDouble3::setData(void *data)
{
    typedef itkDataTensorImageDouble3Private::TensorImageType TensorImageType;

    if( TensorImageType* tensors = dynamic_cast<TensorImageType*>( (itk::Object*)(data) ) ){
        d->tensors = tensors;
    }
    else
      qDebug() << "Cannot cast pointer to correct tensor type";
}

int itkDataTensorImageDouble3::xDimension()
{
    if (!d->tensors.IsNull())
      return d->tensors->GetLargestPossibleRegion().GetSize()[0];
    return -1;
}

int itkDataTensorImageDouble3::yDimension()
{
    if (!d->tensors.IsNull())
        return d->tensors->GetLargestPossibleRegion().GetSize()[1];
    return -1;
}

int itkDataTensorImageDouble3::zDimension()
{
    if (!d->tensors.IsNull())
        return d->tensors->GetLargestPossibleRegion().GetSize()[2];
    return -1;
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

medAbstractData *createItkDataTensorImageDouble3()
{
    return new itkDataTensorImageDouble3;
}

