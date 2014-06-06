#pragma once

#include "vtkObject.h"
#include <reformatPluginExport.h>

class vtkImageData;
class vtkPolyData;
class vtkPlane;
class vtkPlaneCollection;

class REFORMATPLUGIN_EXPORT medResliceCursor : public vtkObject
{
public:
    vtkTypeMacro(medResliceCursor,vtkObject);

    // Description:
    static medResliceCursor *New();

    // Description:
    // Set the image (3D) that we are slicing
    virtual void SetImage(vtkImageData * );
    vtkGetObjectMacro( Image, vtkImageData );

    // Description:
    // Set/Get the cente of the reslice cursor.
    virtual void SetCenter( double, double, double );
    virtual void SetCenter( double center[3] );
    vtkGetVector3Macro( Center, double );

    // Description:
    // Set/Get the thickness of the cursor
    vtkSetVector3Macro( Thickness, double );
    vtkGetVector3Macro( Thickness, double );

    // Description:
    // Enable disable thick mode. Default is to enable it.
    vtkSetMacro( ThickMode, int );
    vtkGetMacro( ThickMode, int );
    vtkBooleanMacro( ThickMode, int );

    // Description:
    // Get the 3D PolyData representation
    virtual vtkPolyData * GetPolyData();

    // Description:
    // Get the slab and centerline polydata along an axis
    virtual vtkPolyData * GetCenterlineAxisPolyData( int axis );

    // Description:
    // Printself method.
    virtual void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Get the planes that represent normals along the X, Y and Z. The argument
    // passed to this method must be an integer in the range 0-2 (corresponding
    // to the X, Y and Z axes.
    virtual vtkPlane * GetPlane( int n );

    // Description:
    // Build the polydata
    virtual void Update();

    // Description:
    // Get the computed axes directions
    vtkGetVector3Macro( XAxis, double );
    vtkGetVector3Macro( YAxis, double );
    vtkGetVector3Macro( ZAxis, double );
    vtkSetVector3Macro( XAxis, double );
    vtkSetVector3Macro( YAxis, double );
    vtkSetVector3Macro( ZAxis, double );
    virtual double * GetAxis( int i );

    // Description:
    // Show a hole in the center of the cursor, so its easy to see the pixels
    // within the hole. ON by default
    vtkSetMacro( Hole, int );
    vtkGetMacro( Hole, int );

    // Description:
    // Set the width of the hole in mm
    vtkSetMacro( HoleWidth, double );
    vtkGetMacro( HoleWidth, double );

    // Description:
    // Set the width of the hole in pixels. If set, this will override the
    // hole with in mm.
    vtkSetMacro( HoleWidthInPixels, double );
    vtkGetMacro( HoleWidthInPixels, double );

    // Description:
    // Get the MTime. Check the MTime of the internal planes as well.
    virtual unsigned long int GetMTime();

    // Description:
    // Reset the cursor to the default position, ie with the axes, normal
    // to each other and axis aligned and with the cursor pointed at the
    // center of the image.
    virtual void Reset();

protected:
    medResliceCursor();
    ~medResliceCursor();

    virtual void BuildCursorGeometry();
    virtual void BuildPolyData();
    virtual void BuildCursorTopology();
    virtual void BuildCursorTopologyWithHole();
    virtual void BuildCursorTopologyWithoutHole();
    virtual void BuildCursorGeometryWithoutHole();
    virtual void BuildCursorGeometryWithHole();
    virtual void ComputeAxes();

    int           ThickMode;
    int           Hole;
    double        HoleWidth;
    double        HoleWidthInPixels;
    double        Thickness[3];
    double        Center[3];
    double        XAxis[3];
    double        YAxis[3];
    double        ZAxis[3];
    vtkImageData *Image;
    vtkPolyData  *PolyData;

    vtkPolyData  *CenterlineAxis[3];

    vtkPlaneCollection * ReslicePlanes;
    vtkTimeStamp  PolyDataBuildTime;

private:
    medResliceCursor(const medResliceCursor&);  // Not implemented.
    void operator=(const medResliceCursor&);  // Not implemented.
};

