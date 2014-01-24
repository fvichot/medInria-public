#pragma once

#include "vtkSmartPointer.h"
#include "vtkResliceImageViewer.h"
#include "vtkImagePlaneWidget.h"
//#include "vtkDistanceWidget.h"
#include "vtkResliceImageViewerMeasurements.h"
#include <QVTKWidget.h>
#include <medAbstractView.h>
#include <medCustomViewContainer.h>

class medReformatViewer : public medCustomViewContainer
{
  Q_OBJECT
public:

  // Constructor/Destructor
  medReformatViewer(medAbstractView * view,QWidget *parent =0);
  ~medReformatViewer() {}

public slots:

  virtual void resliceMode(int);
  virtual void thickMode(int);
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


protected:
  vtkSmartPointer< vtkResliceImageViewer > riw[3];
  vtkSmartPointer< vtkImagePlaneWidget > planeWidget[3];
  /*vtkSmartPointer< vtkDistanceWidget > DistanceWidget[3];*/
  vtkSmartPointer< vtkResliceImageViewerMeasurements > ResliceMeasurements;
  QVTKWidget * views[4];
  medAbstractView * _view;
  
protected slots:

private:

};

