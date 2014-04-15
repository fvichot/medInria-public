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

    /** Slot called when dropped object finishes being imported. */
    void importStructure(const medDataIndex &index);

    void importData(const medDataIndex &index);

    /**
     * Sets the image passed as parameter as the @medDropSite image.
     */
    void setImage(const QImage& thumbnail);

    void clearStructure();
    void clearData();

private:
    meshMappingToolBoxPrivate *d;
};


