#pragma once

#include <reformatPluginExport.h>
#include <medToolBox.h>

class reformatToolBoxPrivate;

class REFORMATPLUGIN_EXPORT reformatToolBox : public medToolBox
{
    Q_OBJECT
public:
    reformatToolBox(QWidget *parentToolBox = 0);
    ~reformatToolBox();
    static bool registered();
    dtkPlugin* plugin();

private:
    reformatToolBoxPrivate *d;

public slots:
    void startReformat();
};
