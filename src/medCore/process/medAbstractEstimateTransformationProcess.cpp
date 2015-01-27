/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medAbstractEstimateTransformationProcess.h>

#include <medAbstractImageData.h>
#include <medAbstractTransformation.h>

medAbstractEstimateTransformationProcess::medAbstractEstimateTransformationProcess(medAbstractProcess *parent):
    medAbstractProcess(parent)
{
    medProcessInput *fixed = new medProcessInput("Fixed Image", false);
    this->appendInput( fixed );

    medProcessInput *moving = new medProcessInput("Moving Image", false);
    this->appendInput( moving );

    this->appendOutput( new medProcessOutput("Output"));

}

medAbstractEstimateTransformationProcess::~medAbstractEstimateTransformationProcess()
{

}

bool medAbstractEstimateTransformationProcess::isInteractive() const
{
    return false;
}

QList<medAbstractParameter*> medAbstractEstimateTransformationProcess::parameters()
{
    //TODO GPR: temporary
    return QList<medAbstractParameter*>();
}
