/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medProcessOutput.h"


medProcessOutput::medProcessOutput(QString name, const QVariant& output) : medProcessPort(name)
{
    m_output = output;
}

medProcessOutput::~medProcessOutput()
{

}

const QVariant& medProcessOutput::output() const
{
    return content();
}


void medProcessOutput::setOutput(const QVariant& output)
{
    setContent(output);
}
