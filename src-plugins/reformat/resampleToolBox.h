#pragma once
#include <medToolBox.h>
#include <reformatPluginExport.h>

class resampleToolBoxPrivate;

class REFORMATPLUGIN_EXPORT resampleToolBox : public medToolBox
{
    Q_OBJECT
public:
    resampleToolBox(QWidget *parentToolBox = 0);
    ~resampleToolBox();
    static bool registered();
    dtkPlugin* plugin();

    virtual void update(dtkAbstractView*);

private:
    resampleToolBoxPrivate *d;

public slots:
    void displayInfoOnCurrentView();
    void hideShowSpinBoxes();
    void startResampling();

};
