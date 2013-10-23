/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medToolBox.h>
#include <medAbstractView.h>
#include <msegPluginExport.h>
#include <medAbstractRoi.h>

class dtkAbstractView;
class medRoiManagementToolBoxPrivate;
class medWorkspace;

//! Roi Management toolbox
class MEDVIEWSEGMENTATIONPLUGIN_EXPORT medRoiManagementToolBox : public medToolBox
{
    Q_OBJECT;

public:

    typedef QList<medAbstractRoi*> * ListRois;

    medRoiManagementToolBox(QWidget *parent = 0);
    ~medRoiManagementToolBox();

    QHash<medAbstractView*,ListRois> * getRois();

signals:
    
public slots:
    // Override base class
    void update (dtkAbstractView *view);
    void addRoi(medAbstractView * view, medAbstractRoi * roi);
    void updateDisplay();
    void clearDisplay();
    void saveCurrentPageIndex(int);
    void selectRois();
    void unselectRois();
    void deleteRoi(unsigned int);

private:
    medRoiManagementToolBoxPrivate *d;
};


