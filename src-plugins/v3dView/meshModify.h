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
#include <v3dView.h>
#include <v3dViewPluginExport.h>

#include <vtkSmartPointer.h>

class vtkBoxWidget;
class vtkMyCallback;
class vtkMetaDataSet;

class V3DVIEWPLUGIN_EXPORT meshModifyToolBox : public medToolBox
{
    Q_OBJECT
    
public:
    meshModifyToolBox(QWidget * parent = 0);
    virtual ~meshModifyToolBox();
    
    virtual QString description() const;
    void update(dtkAbstractView * view);
    
    static bool registered();

public slots:

    void toggleWidget();
    void cancel();
    void dataAdded(dtkAbstractData* data, int index);

private:
    v3dView * _view;
    QPushButton * _modifyButton;
    QPushButton * _cancelButton;
    QSpinBox * _spinBox;
    vtkSmartPointer<vtkBoxWidget> _boxWidget;
    vtkSmartPointer<vtkMyCallback> _callback;
    vtkMetaDataSet * _dataset;
    bool _modifying;
};




