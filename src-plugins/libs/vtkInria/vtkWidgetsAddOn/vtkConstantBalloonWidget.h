/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "vtkBalloonWidget.h"
#include <vtkContourRepresentation.h>
#include "vtkWidgetsAddOnExport.h"

class vtkBalloonRepresentation;
class vtkProp;
class vtkAbstractPropPicker;
class vtkStdString;
class vtkPropMap;
class vtkImageData;


class VTK_WIDGETSADDON_EXPORT vtkConstantBalloonWidget : public vtkBalloonWidget
{
public:
  // Description:
  // Instantiate this class.
  static vtkConstantBalloonWidget *New();

  // Description:
  // Standard methods for a VTK class.
  vtkTypeMacro(vtkConstantBalloonWidget,vtkBalloonWidget);
   void AttachToRightNode(vtkContourRepresentation * contourRep);
 protected:
  vtkConstantBalloonWidget();
  ~vtkConstantBalloonWidget();

  // This class implements the method called from its superclass.
  virtual int SubclassEndHoverAction();
  virtual int SubclassHoverAction();
  //void AttachToRightNode(vtkContourRepresentation * contourRep);
 
private:
  vtkConstantBalloonWidget(const vtkConstantBalloonWidget&);  //Not implemented
  void operator=(const vtkConstantBalloonWidget&);  //Not implemented
};

