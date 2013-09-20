
#ifndef __vtkContourOverlayRepresentation_h
#define __vtkContourOverlayRepresentation_h

#include "vtkOrientedGlyphFocalPlaneContourRepresentation.h"
#include "vtkWidgetsAddOnExport.h"

class vtkProperty2D;
class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPolyData;
class vtkGlyph2D;
class vtkPoints;
class vtkPolyData;

class VTK_WIDGETSADDON_EXPORT vtkContourOverlayRepresentation: public vtkOrientedGlyphFocalPlaneContourRepresentation
{

public:
  // Description:
  // Instantiate this class.
  static vtkContourOverlayRepresentation *New();

  // Description:
  // Standard methods for instances of this class.
  vtkTypeMacro(vtkContourOverlayRepresentation, vtkOrientedGlyphFocalPlaneContourRepresentation);

  // Description:
  // The class maintains its true contour locations based on display co-ords
  // This method syncs the world co-ords data structure with the display co-ords.
  virtual void UpdateContourWorldPositionsBasedOnDisplayPositions();

protected:
  vtkContourOverlayRepresentation();
  ~vtkContourOverlayRepresentation();

 // void BuildLines();
  
private:
  vtkContourOverlayRepresentation(const vtkContourOverlayRepresentation&);  //Not implemented
  void operator=(const vtkContourOverlayRepresentation&);  //Not implemented
  
};

#endif

