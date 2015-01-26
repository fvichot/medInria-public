/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medProcessInput.h>


medProcessInput::medProcessInput(QString name, bool isOptional, const QVariant& input) : medProcessPort(name)
{
    m_isOptional = isOptional;
}

medProcessInput::~medProcessInput()
{

}


bool medProcessInput::isOptional() const
{
    return m_isOptional;
}


const QVariant& medProcessInput::input() const
{
    return content();
}


void medProcessInput::setInput(const QVariant& input)
{
    setContent(input);
}
