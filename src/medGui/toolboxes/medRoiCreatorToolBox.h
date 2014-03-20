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
#include <medGuiExport.h>
#include <medAbstractRoi.h>

class dtkAbstractView;
class medRoiCreatorToolBoxPrivate;
class medWorkspace;
class medSeriesOfRoi;


//! Roi Management toolbox
class MEDGUI_EXPORT medRoiCreatorToolBox : public medToolBox
{
    Q_OBJECT;

public:

    typedef QList<medAbstractRoi*> * ListRois;
    typedef QList<medSeriesOfRoi*> * ListOfSeriesOfRois;
    typedef QPair<unsigned int,unsigned int> PairInd;
    
    medRoiCreatorToolBox(QWidget *parent = 0);
    ~medRoiCreatorToolBox();

    QHash<medAbstractView*,ListOfSeriesOfRois> * getSeriesOfRoi();
    /*QList<medAbstractRoi*> getSelectedRois();*/

    medAbstractView * getCurrentView();
    QList<PairInd> getSelectedRois();
    void setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData);

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
    void deleteRoi(PairInd);
    void onInterpolate();
    void onGenerateBinaryImage();
    void applyRoiToImage();
    void onContextTreeMenu( const QPoint point );

private:
    medRoiCreatorToolBoxPrivate *d;
};

class MEDGUI_EXPORT medSeriesOfRoi 
{
public:
    typedef QList<medAbstractRoi*> * ListRois;
    typedef QPair<unsigned int,unsigned int> PairInd;
    
    medSeriesOfRoi(QString name,ListRois rois,medRoiCreatorToolBox * toolbox):name(name),rois(rois),toolbox(toolbox){};
    ~medSeriesOfRoi(){}; // delete everything

    QString getName(){return name;};
    ListRois getIndices(){return rois;};

    void Off();
    void On();

    virtual QString info();
    
    void select();
    void unselect();
    void computeStatistics();

private:
    QString name;
    ListRois rois; // indices of roi in Series
    medRoiCreatorToolBox * toolbox;
};
