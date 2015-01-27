/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medAbstractModelEstimationProcess.h"

#include <medAbstractImageData.h>
#include <medAbstractDiffusionModelImageData.h>


medAbstractModelEstimationProcess::medAbstractModelEstimationProcess(medAbstractProcess *parent): medAbstractProcess(parent)
{
    medProcessInput *input = new medProcessInput("Image", false);
    this->appendInput( input );

    this->appendOutput( new medProcessOutput("Output"));
}

bool medAbstractModelEstimationProcess::isInteractive() const
{
    return false;
}
