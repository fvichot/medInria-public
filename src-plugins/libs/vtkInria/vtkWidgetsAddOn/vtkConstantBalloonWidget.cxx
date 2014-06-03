/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "vtkConstantBalloonWidget.h"
#include "vtkBalloonWidget.h"
#include "vtkBalloonRepresentation.h"
#include "vtkStdString.h"
#include "vtkProp.h"
#include "vtkPropPicker.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkAssemblyPath.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkImageData.h"
#include <vtkstd/map>

#include <vtkContourRepresentation.h>

vtkStandardNewMacro(vtkConstantBalloonWidget);

//-- Define the PIMPLd array of vtkProp and vtkString --
struct vtkBalloon
{
  vtkStdString Text;
  vtkImageData *Image;

  vtkBalloon() : Text(), Image(0) {}
  vtkBalloon(vtkStdString *str, vtkImageData *img)
    {
      this->Text = *str;
      this->Image = img;
      if ( this->Image )
        {
        this->Image->Register(NULL);
        }
    }
  vtkBalloon(const char *str, vtkImageData *img)
    {
      this->Text = vtkStdString(str);
      this->Image = img;
      if ( this->Image )
        {
        this->Image->Register(NULL);
        }
    }
  ~vtkBalloon()
    {
      if (this->Image)
        {
        this->Image->UnRegister(NULL);
        }
    }
  void operator=(const vtkBalloon &balloon)
    {
      this->Text = balloon.Text;

      // Don't leak if we already have an image.
      if( this->Image )
        {
        this->Image->UnRegister(NULL);
        this->Image = NULL;
        }

      this->Image = balloon.Image;
      if ( this->Image )
        {
        this->Image->Register(NULL);
        }
    }
  int operator==(const vtkBalloon &balloon) const
    {
      if ( this->Image == balloon.Image )
        {
        if ( this->Text == balloon.Text )
          {
          return 1;
          }
        }
      return 0;
    }
  int operator!=(const vtkBalloon &balloon) const
    {
      return !(*this == balloon);
    }
};

class vtkPropMap : public std::map<vtkProp*,vtkBalloon> {};
typedef std::map<vtkProp*,vtkBalloon>::iterator vtkPropMapIterator;
//-------------------------------------------------------------------------
vtkConstantBalloonWidget::vtkConstantBalloonWidget(){}

//-------------------------------------------------------------------------
vtkConstantBalloonWidget::~vtkConstantBalloonWidget(){}

//-------------------------------------------------------------------------
int vtkConstantBalloonWidget::SubclassHoverAction()
{
  double e[2];
  e[0] = static_cast<double>(this->Interactor->GetEventPosition()[0]);
  e[1] = static_cast<double>(this->Interactor->GetEventPosition()[1]);
  vtkRenderer *ren = this->CurrentRenderer;
  if ( this->CurrentProp )
    {
    this->CurrentProp->UnRegister(this);
    this->CurrentProp = NULL;
    }
  this->Picker->Pick(e[0],e[1],0.0,ren);
  vtkAssemblyPath *path = this->Picker->GetPath();
  if ( path != NULL )
    {
    vtkPropMapIterator iter =
      this->PropMap->find(path->GetFirstNode()->GetViewProp());
    if ( iter != this->PropMap->end() )
      {
      this->CurrentProp = (*iter).first;
      this->CurrentProp->Register(this);
      reinterpret_cast<vtkBalloonRepresentation*>(this->WidgetRep)->
        SetBalloonText((*iter).second.Text);
      reinterpret_cast<vtkBalloonRepresentation*>(this->WidgetRep)->
        SetBalloonImage((*iter).second.Image);
      //this->WidgetRep->StartWidgetInteraction(e);
      if (this->CurrentProp) 
        AttachToRightNode(dynamic_cast<vtkContourRepresentation*>(this->CurrentProp));
      this->Render();
      }
    }
     // REMOVE THIS AT ALL COST THIS FUNCTION IS NOT SUPPOSED TO WORK ONLY WITH VTK CONTOUR WIDGET
    // OR CONSIDER THIS CLASS AS A VTKBALLOONWIDGET SPECIFIC TO VTKCONTOURWIDGET BUT IT DOES NOT SEEM TO BE THE POINT FOR THE TIME BEING

  return 1;
}

//-------------------------------------------------------------------------
int vtkConstantBalloonWidget::SubclassEndHoverAction()
{
  double e[2];
  e[0] = static_cast<double>(this->Interactor->GetEventPosition()[0]);
  e[1] = static_cast<double>(this->Interactor->GetEventPosition()[1]);
  //this->WidgetRep->EndWidgetInteraction(e);
  this->Render();

  return 1;
}

//-------------------------------------------------------------------------
// This function is to be used when the prop is a vtkContourRepresentation. It attaches the balloonWidget to the rightmost node. 
void vtkConstantBalloonWidget::AttachToRightNode(vtkContourRepresentation * contourRep)
{
    // TODO : FORCE RIGHT LAYOUT FOR THE TIME BEING
    // TODO : NEED TO ENABLE THE USER TO MOVE THE BALLOONWIDGET AS HE/SHE WISHES, THEN THE BALLOONWIDGET SHOULD ATTACHES IT SELF TO THE CLOSEST NODE AND CHANGES THE LAYOUT ACCORDINGLY
    //if (!this->CurrentProp || (dynamic_cast<vtkContourRepresentation*>(this->CurrentProp)!=contourRep))
      //  this->CurrentProp=contourRep;

    //vtkContourRepresentation * contourRep = dynamic_cast<vtkContourRepresentation*>(this->CurrentProp);
        //return;
    this->CurrentProp = contourRep;

    double x=-1;
    unsigned int n=0;
    for(int i=0;i<contourRep->GetNumberOfNodes();i++)
    {
        double pos[2];
        contourRep->GetNthNodeDisplayPosition(i,pos);
        if (pos[0]>x)
        {
            x=pos[0];
            n=i;
        }
    }
    
    if (x==-1)
        return;

    double e[2];
    contourRep->GetNthNodeDisplayPosition(n,e);
    this->WidgetRep->StartWidgetInteraction(e);
    //this->Render();
}
