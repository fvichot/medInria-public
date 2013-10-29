/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <dtkCore/dtkAbstractObject.h>

#include "medCoreExport.h"

class medAbstractRoiPrivate;

class RoiStatistics
{
public:
    double max;
    double min;
    double mean;
    double sum;
    double std;
    double area;
    double perimeter;
};

/**
 * 
 */
class MEDCORE_EXPORT medAbstractRoi : public dtkAbstractObject
{
    Q_OBJECT

public:
    medAbstractRoi( dtkAbstractObject * parent = NULL );
    virtual ~medAbstractRoi();

    virtual void Off()=0;
    virtual void On()=0;

    virtual QString info()=0;
    virtual QString type()=0;

    unsigned int getIdSlice();
    void setIdSlice(unsigned int idSlice);

    unsigned char getOrientation();
    void setOrientation(unsigned char orientation);

    bool isSelected();
    virtual void computeRoiStatistics() = 0;
    virtual void setRoiStatistics(RoiStatistics s); 

public slots:

    virtual void select();
    virtual void unselect();
    
signals:
    void selected();
    
private:
    medAbstractRoiPrivate * d;
};


