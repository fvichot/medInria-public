/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medAbstractFilteringProcess.h>

#include <medAbstractImageData.h>
#include <medViewContainerSplitter.h>
#include <medViewContainer.h>


medAbstractFilteringProcess::medAbstractFilteringProcess(medAbstractProcess *parent)
{
    medProcessInput *input = new medProcessInput("Input", false);
    medProcessOutput*output = new medProcessOutput("Output");
    //output->output= NULL;

    //TODO: Should this be done here or by implementation
    // (would allow to precise types)
    this->appendInput(input);
    this->appendOutput(output);
}

bool medAbstractFilteringProcess::isInteractive() const
{
    return false;
}


