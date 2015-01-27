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

class medProcessOutput : public medProcessPort
{

public:
    medProcessOutput(QString name, const QVariant& output = QVariant());
    virtual ~medProcessOutput();

public:
    const QVariant& output() const;
    void setOutput(const QVariant& output);
};
