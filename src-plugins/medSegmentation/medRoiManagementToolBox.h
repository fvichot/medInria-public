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
<<<<<<< HEAD
=======

>>>>>>> 9139638... Working on series of Roi display ...

//! Roi Management toolbox
class MEDVIEWSEGMENTATIONPLUGIN_EXPORT medRoiManagementToolBox : public medToolBox
{
    Q_OBJECT;

public:

    typedef QList<medAbstractRoi*> * ListRois;
    typedef QList<medSeriesOfRoi*> * ListOfSeriesOfRois;
    
    medRoiManagementToolBox(QWidget *parent = 0);
    ~medRoiManagementToolBox();

    QHash<medAbstractView*,ListOfSeriesOfRois> * getSeriesOfRoi();
    /*QList<medAbstractRoi*> getSelectedRois();*/

signals:
    
public slots:
    // Override base class
    void update (dtkAbstractView *view);
    void addRoi(medAbstractView * view, medAbstractRoi * roi,QString seriesName = "");
    void updateDisplay();
    void clearDisplay();
    void saveCurrentPageIndex(int);
    void selectRois();
    void unselectRois();
    void deleteRoi(unsigned int);

private:
    medRoiManagementToolBoxPrivate *d;
};

class MEDVIEWSEGMENTATIONPLUGIN_EXPORT medSeriesOfRoi 
{
public:
    typedef QList<medAbstractRoi*> * ListRois;
    
    medSeriesOfRoi(QString name,ListRois rois,medRoiManagementToolBox * toolbox):name(name),rois(rois),toolbox(toolbox){};
    ~medSeriesOfRoi(){}; // delete everything

    QString getName(){return name;};
    ListRois getIndices(){return rois;};
private:
    QString name;
    ListRois rois; // indices of roi in Series
    medRoiManagementToolBox * toolbox;
};
