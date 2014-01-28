#pragma once

#include <reformatPluginExport.h>
#include <medToolBox.h>
#include <medWorkspace.h>

class reformatToolBoxPrivate;

class REFORMATPLUGIN_EXPORT reformatToolBox : public medToolBox
{
    Q_OBJECT
public:
    reformatToolBox(QWidget *parentToolBox = 0);
    ~reformatToolBox();
    static bool registered();
    dtkPlugin* plugin();
    void setWorkspace(medWorkspace * workspace);

private:
    reformatToolBoxPrivate *d;

public slots:
    void startReformat(bool);
    void update(dtkAbstractView* view);
    void actOnContainerChange(const QString&);
    void propagateThickModeActivated();
    void propagateBlendModeChosen();
};

