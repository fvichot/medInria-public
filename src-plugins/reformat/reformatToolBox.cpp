#include "reformatToolBox.h"
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStringList>

#include <medToolBoxFactory.h>
#include <medPluginManager.h>

#include <medDataManager.h>
#include <medMetaDataKeys.h>
#include <dtkCore/dtkAbstractDataFactory.h>

class reformatToolBoxPrivate
{
public:
    QPushButton * b_startReformat;
    //dtkSmartPointer <dtkAbstractData> dataBMode;
    //dtkSmartPointer <dtkAbstractData> dataDoppler;

};
reformatToolBox::reformatToolBox (QWidget *parent) : medToolBox (parent), d(new reformatToolBoxPrivate)
{
    this->setTitle("Reformat");
    this->setAboutPluginVisibility(false);
    this->setAboutPluginButton(this->plugin());
    // Fill the toolBox
    QWidget *reformatToolBoxBody = new QWidget(this);
    d->b_startReformat = new QPushButton("Start Reformat", reformatToolBoxBody);
    
    QVBoxLayout *reformatToolBoxLayout =  new QVBoxLayout(reformatToolBoxBody);
    reformatToolBoxLayout->addWidget(d->b_startReformat);
    reformatToolBoxBody->setLayout(reformatToolBoxLayout);
    this->addWidget(reformatToolBoxBody);
    // Connections
    connect(d->b_startReformat,SIGNAL(clicked()),this,SLOT(startReformat()));
}
reformatToolBox::~reformatToolBox()
{
    delete d;
    d = NULL;
}

bool reformatToolBox::registered()
{
    medToolBoxFactory* factory = medToolBoxFactory::instance();
    return factory->registerToolBox<reformatToolBox> ("reformatToolBox",
            "reformat",
            "Used to reformat an image",
            QStringList()<<"reformatToolBox"
                                                               );
}

dtkPlugin* reformatToolBox::plugin()
{
    medPluginManager* pm = medPluginManager::instance();
    dtkPlugin* plugin = pm->plugin ( "reformatPlugin" );
    return plugin;
}

void reformatToolBox::startReformat()
{

}
