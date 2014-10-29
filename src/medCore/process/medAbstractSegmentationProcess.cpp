/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medAbstractSegmentationProcess.h"

#include <medAbstractImageData.h>

medAbstractSegmentationProcess::medAbstractSegmentationProcess(medAbstractProcess *parent): medAbstractProcess(parent)
{
    medProcessInput<medAbstractImageData*> *input = new medProcessInput<medAbstractImageData*>;
    input->name = "Image";
    input->isOptional = false;
    this->appendInput( input );

    this->appendOutput( new medProcessInput<medAbstractImageData*>);
}

bool medAbstractSegmentationProcess::isInteractive()
{
    return true;
}