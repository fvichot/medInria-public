#pragma once

#include "vtkSmartPointer.h"
#include "vtkResliceImageViewer.h"
#include "vtkImagePlaneWidget.h"
//#include "vtkDistanceWidget.h"
#include "vtkResliceImageViewerMeasurements.h"
#include <QVTKWidget.h>
#include <medAbstractView.h>
#include <medCustomViewContainer.h>
#include <itkImageToVTKImageFilter.h>
#include <itkImage.h>

class QVTKFrame : public QFrame
{
public:
    QVTKFrame(QWidget * parent):QFrame(parent)
    {
        QHBoxLayout * layout = new QHBoxLayout(this);
        view = new QVTKWidget(this);
        layout->addWidget(view);
        this->setLayout(layout);
    };
    ~QVTKFrame() {}
    QVTKWidget * getView(){return view;};
private:
    QVTKWidget * view;

};

class medReformatViewer : public medCustomViewContainer
{
  Q_OBJECT
public:

  // Constructor/Destructor
  medReformatViewer(medAbstractView * view,QWidget *parent =0);
  ~medReformatViewer() {}

public slots:

  //virtual void resliceMode(int);
  virtual void thickMode(int);
  virtual void blendMode(int);
  virtual void SetBlendModeToMaxIP();
  virtual void SetBlendModeToMinIP();
  virtual void SetBlendModeToMeanIP();
  virtual void SetBlendMode(int);
  virtual void ResetViews();
  virtual void Render();
  //virtual void AddDistanceMeasurementToView1();
  //virtual void AddDistanceMeasurementToView( int );
  //void setView(medAbstractView * view){_view = view;};

  void orthogonalAxisModeEnabled(bool);
  void saveImage();
  void thickSlabChanged(double);
  bool eventFilter(QObject * object,QEvent * event);
  int SetCameraFromOrientation(vtkSmartPointer<vtkResliceImageViewer> riw,int SliceOrientation);
  void SetViewConvention(int convention);

protected:
  vtkSmartPointer< vtkResliceImageViewer > riw[3];
  vtkSmartPointer< vtkImagePlaneWidget > planeWidget[3];
  /*vtkSmartPointer< vtkDistanceWidget > DistanceWidget[3];*/
  vtkSmartPointer< vtkResliceImageViewerMeasurements > ResliceMeasurements;
  QVTKWidget * views[4];
  QVTKFrame * frames[4];
  medAbstractView * _view;
  double outputSpacing[3];
  unsigned char selectedView;
  vtkMatrix4x4 * ConventionMatrix;
  vtkSmartPointer<vtkImageData> vtkViewData;
  typedef itk::Image<short,3> ImageType;
  typedef itk::ImageToVTKImageFilter<ImageType>       ConnectorType;
  ConnectorType::Pointer converter;
  
  
protected slots:

private:

};

