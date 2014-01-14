#include "resampleProcess.h"
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>
#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <medMetaDataKeys.h>

#include <itkResampleImageFilter.h>
#include <itkBSplineInterpolateImageFunction.h>

// /////////////////////////////////////////////////////////////////
// resampleProcessPrivate
// /////////////////////////////////////////////////////////////////
class resampleProcessPrivate
{
public:
    resampleProcess *parent;
    resampleProcessPrivate(resampleProcess *p)
    {
        parent = p;
    }
    dtkSmartPointer <dtkAbstractData> input;
    dtkSmartPointer <dtkAbstractData> output;
    int interpolator;
};

// /////////////////////////////////////////////////////////////////
// resampleProcess
// /////////////////////////////////////////////////////////////////
resampleProcess::resampleProcess(void) : dtkAbstractProcess(), d(new resampleProcessPrivate(this))
{
}

resampleProcess::~resampleProcess(void)
{
    delete d;
    d = NULL;
}
bool resampleProcess::registered(void)
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("resampleProcess", createResampleProcess);
}
QString resampleProcess::description(void) const
{
    return "resampleProcess";
}
void resampleProcess::setInput ( dtkAbstractData *data )
{
    if ( !data )
        return;
    d->input = data;
}

void resampleProcess::setParameter ( double data, int channel)
{

}

int resampleProcess::update ( void )
{
    if ( !d->input )
    {
        qDebug() << "in update method : d->input is NULL";
        return -1;
    }
    QString type = QString (d->input->identifier());

    if ( type == "itkDataImageChar3" )
    {
        resample<itk::Image<char, 3> >("itkDataImageChar3");
    }
    else if ( type == "itkDataImageUChar3" )
    {
        resample<itk::Image<unsigned char, 3> >("itkDataImageUChar3");
    }
    else if ( type == "itkDataImageShort3" )
    {
        resample<itk::Image<short, 3> >("itkDataImageShort3");
    }
    else if ( type == "itkDataImageUShort3" )
    {
        resample<itk::Image<unsigned short, 3> >("itkDataImageUShort3");
    }
    else if ( type == "itkDataImageInt3" )
    {
        resample<itk::Image<int, 3> >("itkDataImageInt3");
    }
    else if ( type == "itkDataImageUInt3" )
    {
        resample<itk::Image<unsigned int, 3> >("itkDataImageUInt3");
    }
    else if ( type == "itkDataImageLong3" )
    {
        resample<itk::Image<long, 3> >("itkDataImageLong3");
    }
    else if ( type == "itkDataImageULong3" )
    {
        resample<itk::Image<unsigned long, 3> >("itkDataImageULong3");
    }
    else if ( type == "itkDataImageFloat3" )
    {
        resample<itk::Image<float, 3> >("itkDataImageFloat3");
    }
    else if ( type == "itkDataImageDouble3" )
    {
        resample<itk::Image<double, 3> >("itkDataImageDouble3" );
    }
    else if ( type == "itkDataImageChar4" )
    {
        resample<itk::Image<char, 4> >("itkDataImageChar4");
    }
    else if ( type == "itkDataImageUChar4" )
    {
        resample<itk::Image<unsigned char, 4> >("itkDataImageUChar4");
    }
    else if ( type == "itkDataImageShort4" )
    {
        resample<itk::Image<short, 4> >("itkDataImageShort4");
    }
    else if ( type == "itkDataImageUShort4" )
    {
        resample<itk::Image<unsigned short, 4> >("itkDataImageUShort4");
    }
    else if ( type == "itkDataImageInt4" )
    {
        resample<itk::Image<int, 4> >("itkDataImageInt4");
    }
    else if ( type == "itkDataImageUInt4" )
    {
        resample<itk::Image<unsigned int, 4> >("itkDataImageUInt4");
    }
    else if ( type == "itkDataImageLong4" )
    {
        resample<itk::Image<long, 4> >("itkDataImageLong4");
    }
    else if ( type == "itkDataImageULong4" )
    {
        resample<itk::Image<unsigned long, 4> >("itkDataImageULong4");
    }
    else if ( type == "itkDataImageFloat4" )
    {
        resample<itk::Image<float, 4> >("itkDataImageFloat4");
    }
    else if ( type == "itkDataImageDouble4" )
    {
        resample<itk::Image<double, 4> >("itkDataImageDouble4" );
    }
    return EXIT_SUCCESS;
}

template <class ImageType> void resampleProcess::resample(const char *str)
{
    typename ImageType::Pointer inputImage =dynamic_cast<ImageType*>((itk::Object*)(d->input->data()));

    typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleFilterType;
    typedef itk::IdentityTransform<double,3> TransformType; //TODO : 3 -> ImageType ::Dimension
    
    typedef itk::BSplineInterpolateImageFunction<ImageType, double, double> InterpolatorType; // by default
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    interpolator->SetSplineOrder(3);

    //switch (d->interpolator) // choose between different type of interpolator via toolbox // set the d->interpolator via setParameter
    //{
    //case 0:
    //    typedef itk::BSplineInterpolateImageFunction<ImageType, double, double> InterpolatorType;
    //    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    //    interpolator->SetSplineOrder(3);
    //case 1:
    //    typedef itk::BSplineInterpolateImageFunction<ImageType, double, double> Interpolator;
    //case 2:
    //    typedef itk::BSplineInterpolateImageFunction<ImageType, double, double> Interpolator;
    //}

    TransformType::Pointer transform = TransformType::New();
    transform->SetIdentity();

    ResampleFilterType::Pointer resampleFilter = ResampleFilterType::New();
   // resampleFilter->SetTransform(transform);
  //  resampleFilter->SetInterpolator(interpolator);

    //// TODO finish 
    //unsigned int nNewWidth = 1;/*atoi(argv[3]);*/
    //unsigned int nNewHeight =1;/* atoi(argv[4]);*/
 
    //// Fetch original image size.
    //const ImageType::RegionType& inputRegion = inputImage->GetLargestPossibleRegion();
    //const ImageType::SizeType& vnInputSize = inputRegion.GetSize();
    //unsigned int nOldWidth = vnInputSize[0];
    //unsigned int nOldHeight = vnInputSize[1];

    //// Fetch original image spacing.
    //const ImageType::SpacingType& vfInputSpacing = inputImage->GetSpacing();

    //double vfOutputSpacing[2];
    //vfOutputSpacing[0] = vfInputSpacing[0] * (double) nOldWidth / (double) nNewWidth;
    //vfOutputSpacing[1] = vfInputSpacing[1] * (double) nOldHeight / (double) nNewHeight;

    //// Set the output spacing. If you comment out the following line, the original
    //// image will be simply put in the upper left corner of the new image without
    //// any scaling.
    //resampleFilter->SetOutputSpacing(vfOutputSpacing);

    //// Set the output size as specified on the command line.

    //itk::Size<3> vnOutputSize = { {nNewWidth, nNewHeight,1} };
    //resampleFilter->SetSize(vnOutputSize);

    //// Specify the input.

    //resampleFilter->SetInput(inputImage);

    //d->output = dtkAbstractDataFactory::instance()->createSmartPointer(str);
    //d->output->setData(resampleFilter->GetOutput());
}

dtkAbstractData * resampleProcess::output ( void )
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////
dtkAbstractProcess *createResampleProcess(void)
{
    return new resampleProcess;
}
