/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medMeshTools.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

// /////////////////////////////////////////////////////////////////
// medMeshToolsPrivate
// /////////////////////////////////////////////////////////////////

class medMeshToolsPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> input;
    dtkSmartPointer <dtkAbstractData> output;
};

// /////////////////////////////////////////////////////////////////
// medMeshTools
// /////////////////////////////////////////////////////////////////

medMeshTools::medMeshTools() : dtkAbstractProcess(), d(new medMeshToolsPrivate)
{
    
}

medMeshTools::~medMeshTools()
{
    
}

bool medMeshTools::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("medMeshTools", createMedMeshTools);
}

QString medMeshTools::description() const
{
    return "medMeshTools";
}

void medMeshTools::setInput ( dtkAbstractData *data )
{
    if ( !data )
        return;
    
    QString identifier = data->identifier();
    
    d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( identifier );
    
    d->input = data;
}    

void medMeshTools::setParameter ( double  data, int channel )
{
    // Here comes a switch over channel to handle parameters
}

int medMeshTools::update()
{
    if ( !d->input )
        return -1;
    
    // Your update code comes in here
    
    return EXIT_SUCCESS;
}        

dtkAbstractData * medMeshTools::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createMedMeshTools()
{
    return new medMeshTools;
}
