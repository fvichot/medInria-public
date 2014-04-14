/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medFilteringAbstractToolBox.h>

#include "meshMappingPluginExport.h"
#include "medToolBox.h"

#include "medGuiExport.h"
#include <medDataManager.h>



class meshMappingToolBoxPrivate;

class MESHMAPPINGPLUGIN_EXPORT meshMappingToolBox : public medFilteringAbstractToolBox
{
    Q_OBJECT
    
public:
    meshMappingToolBox(QWidget *parent = 0);
    ~meshMappingToolBox();
    
    dtkAbstractData *processOutput();
    
    static bool registered();
    dtkPlugin * plugin();
    void setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData);
    
signals:

    void success();
    void failure();
    
    public slots:
    void run();



protected slots:

    /**
      * When update() is called, this toolbox automatically searches for a subclass
      * of a medAbstractFiberViewInteractor and set its input to the interactor's
      * input. It adapts its GUI automatically (populate the list of bundles).
      */
    virtual void update (dtkAbstractView *view);

    /**
     * Clears the toolbox. Removes any bundle in the fiber bundle treeview,
     * any ROI previously loaded (if any), etc.
     */
    virtual void clear();

    /** Slot called when external ROI image finishes being imported. */
    virtual void onRoiImported(const medDataIndex &index);

    virtual void onImageImported(const medDataIndex &index);
    /**
     * Slot called when the @meDropSite is clicked.
     * Will open a @QFileDialog so the user can choose
     * and external ROI image to open.
     */
    virtual void onDropSiteClicked();

    /**
     * Sets the image passed as parameter as the @medDropSite image.
     */
    void setImage(const QImage& thumbnail);

    virtual void onClearRoiButtonClicked();
    virtual void onClearInputButtonClicked();

private:
    meshMappingToolBoxPrivate *d;
};


