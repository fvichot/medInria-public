/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "itkFiltersProcessBase.h"
#include <dtkCore/dtkAbstractData.h>

#include "itkFiltersPluginExport.h"

class itkFiltersThresholdingProcessPrivate;
class dtkAbstractData;

class ITKFILTERSPLUGIN_EXPORT itkFiltersThresholdingProcess : public itkFiltersProcessBase
{
    Q_OBJECT
    
public:
    itkFiltersThresholdingProcess(itkFiltersThresholdingProcess * parent = 0);
    itkFiltersThresholdingProcess(const itkFiltersThresholdingProcess& other);
    virtual ~itkFiltersThresholdingProcess(void);

    static bool registered ( void );
    
public slots:

    void setParameter ( int  data, int channel );
    int update ( void );

private:
    DTK_DECLARE_PRIVATE(itkFiltersThresholdingProcess)
};

dtkAbstractProcess * createitkFiltersThresholdingProcess(void);


