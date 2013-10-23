/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "msegPluginExport.h"
#include <medToolBox.h>
#include <vtkDataSet.h>
#include <QtGui>
#include <vtkTable.h>

class dtkAbstractView;
class dtkAbstractData;

class medHistogramToolBoxPrivate;
class vtkImageData;

class MEDVIEWSEGMENTATIONPLUGIN_EXPORT medHistogramToolBox : public medToolBox
{
    Q_OBJECT

public:
    /**
    * @brief
    *
    * @param parent
    */
    medHistogramToolBox(QWidget *parent = 0);

    /**
     * @brief
     *
     * @param void
    */
    virtual ~medHistogramToolBox();

    void setPlotInput(vtkDataSet * data,double xmax,double ymax);
    void setChartInput(vtkTable * table);

private:
    medHistogramToolBoxPrivate *d;
};


