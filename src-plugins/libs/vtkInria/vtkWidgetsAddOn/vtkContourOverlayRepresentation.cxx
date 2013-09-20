#include "vtkOrientedGlyphFocalPlaneContourRepresentation.h"
#include <vtkContourOverlayRepresentation.h>
#include "vtkCleanPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkActor2D.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"
#include "vtkProperty2D.h"
#include "vtkAssemblyPath.h"
#include "vtkMath.h"
#include "vtkInteractorObserver.h"
#include "vtkLine.h"
#include "vtkCoordinate.h"
#include "vtkGlyph2D.h"
#include "vtkCursor2D.h"
#include "vtkCylinderSource.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkCamera.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkFocalPlanePointPlacer.h"
#include "vtkBezierContourLineInterpolator.h"

vtkStandardNewMacro(vtkContourOverlayRepresentation);

//----------------------------------------------------------------------
vtkContourOverlayRepresentation::vtkContourOverlayRepresentation()
{
}

//----------------------------------------------------------------------
vtkContourOverlayRepresentation::~vtkContourOverlayRepresentation()
{
}

void vtkContourOverlayRepresentation::UpdateContourWorldPositionsBasedOnDisplayPositions()
{
    double dispPos[3];
    double W[4];
      
    for(unsigned int i=0;i<this->Internal->Nodes.size();i++)
    {
        W[0] = this->Internal->Nodes[i]->WorldPosition[0];
        W[1] = this->Internal->Nodes[i]->WorldPosition[1];
        W[2] = this->Internal->Nodes[i]->WorldPosition[2];

        vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer,W[0], W[1], W[2], dispPos);
        this->Renderer->DisplayToNormalizedDisplay( dispPos[0], dispPos[1] );
        this->Internal->Nodes[i]->NormalizedDisplayPosition[0] = dispPos[0];
        this->Internal->Nodes[i]->NormalizedDisplayPosition[1] = dispPos[1];
    
        for (unsigned int j=0;j<this->Internal->Nodes[i]->Points.size();j++)
        {
            W[0] = this->Internal->Nodes[i]->Points[j]->WorldPosition[0];
            W[1] = this->Internal->Nodes[i]->Points[j]->WorldPosition[1];
            W[2] = this->Internal->Nodes[i]->Points[j]->WorldPosition[2];
      
            vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer, W[0], W[1], W[2], dispPos);
            this->Renderer->DisplayToNormalizedDisplay( dispPos[0], dispPos[1] );
            this->Internal->Nodes[i]->Points[j]->NormalizedDisplayPosition[0] = dispPos[0];
            this->Internal->Nodes[i]->Points[j]->NormalizedDisplayPosition[1] = dispPos[1];
        }
    }
}