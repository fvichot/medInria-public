/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medAbstractRoi.h"

#include <dtkCore/dtkSmartPointer.h>

class medAbstractRoiPrivate
{
public:
    unsigned char orientation;
    unsigned int idSlice;
    bool isSelected;
};

medAbstractRoi::medAbstractRoi( dtkAbstractObject *parent )
    : dtkAbstractObject(parent)
    , d(new medAbstractRoiPrivate)
{
    d->isSelected = false;
}

medAbstractRoi::~medAbstractRoi( void )
{
    delete d;
    d = NULL;
}

unsigned int medAbstractRoi::getIdSlice()
{
    return d->idSlice;
}

void medAbstractRoi::setIdSlice(unsigned int idSlice)
{
    d->idSlice=idSlice;
}

unsigned char medAbstractRoi::getOrientation()
{
    return d->orientation;
}

void medAbstractRoi::setOrientation(unsigned char orientation)
{
    d->orientation=orientation;
}

bool medAbstractRoi::isSelected()
{
    return d->isSelected;
}

void medAbstractRoi::select()
{
    d->isSelected = true;
    selected();
}

void medAbstractRoi::unselect()
{
    d->isSelected = false;
}


