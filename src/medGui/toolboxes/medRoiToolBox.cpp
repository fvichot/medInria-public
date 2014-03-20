/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medRoiToolBox.h"
#include <QtGui>

class medRoiToolBoxPrivate
{
public:
    medRoiToolBoxPrivate(){};
};

medRoiToolBox::medRoiToolBox(QWidget *parent) : medToolBox(parent), d(new medRoiToolBoxPrivate)
{
}

medRoiToolBox::~medRoiToolBox(void)
{
    delete d;

    d = NULL;
}

void medRoiToolBox::applyRoiToImage()
{
    // conversion vers brush roi puis appel du maskapplication process
}

void medRoiToolBox::interpolateRois(QList<medAbstractRoi*>* list) 
{

}

dtkAbstractData * medRoiToolBox::convertToBinaryImage(QList<medAbstractRoi*>* list)
{
    return NULL;
}

QString medRoiToolBox::roi_description()
{
    return "";
}