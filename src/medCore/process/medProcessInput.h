/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medProcessPort.h>

class medProcessInput : public medProcessPort
{
public:
    medProcessInput(QString name, bool isOptional, const QVariant& input = QVariant());
    virtual ~medProcessInput();

public:
    bool isOptional() const;

    const QVariant& input() const;
    void setInput(const QVariant& input);

private:
    bool m_isOptional;
};
