#include "cliSupportToolBox.h"

#include <dtkCore/dtkAbstractView.h>
#include <ctkCmdLineModuleManager.h>
#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleBackendLocalProcess.h>
#include <ctkCmdLineModuleFrontendFactoryQtGui.h>

#include <ctkException.h>

class cliSupportToolBoxPrivate
{
public:
    dtkAbstractView * view;
};

cliSupportToolBox::cliSupportToolBox(QWidget *parent) : medToolBox(parent), d(new cliSupportToolBoxPrivate)
{
    d->view = 0;
    this->setTitle("CTK CLI Support");

    // Instantiate a ctkCmdLineModuleManager class.
    ctkCmdLineModuleManager moduleManager(
                // Use "strict" validation mode, rejecting modules with non-valid XML descriptions.
                ctkCmdLineModuleManager::STRICT_VALIDATION,
                // Use the default cache location for this application
                QDesktopServices::storageLocation(QDesktopServices::CacheLocation)
                );

    // Instantiate a back-end for running executable modules in a local process.
    // This back-end handles the "file" Url scheme.
    QScopedPointer<ctkCmdLineModuleBackend> processBackend(new ctkCmdLineModuleBackendLocalProcess);
    // Register the back-end with the module manager.
    moduleManager.registerBackend(processBackend.data());

    ctkCmdLineModuleReference moduleRef;
    try
    {
        // Register a local executable as a module, the ctkCmdLineModuleBackendLocalProcess
        // can handle it.
        moduleRef = moduleManager.registerModule(QUrl::fromLocalFile("/home/fvichot/dev/CTK/build/CTK-build/bin/ctkCmdLineModuleTestBed"));
    }
    catch (const ctkInvalidArgumentException& e)
    {
        // Module validation failed.
        qDebug() << "lksdjlksdjf:" << e;
//        return EXIT_FAILURE;
    }

    // We use the "Qt Gui" frontend factory.
    QScopedPointer<ctkCmdLineModuleFrontendFactory> frontendFactory(new ctkCmdLineModuleFrontendFactoryQtGui);
    qApp->addLibraryPath("/home/fvichot/dev/CTK/build/CTK-build/bin");
    QScopedPointer<ctkCmdLineModuleFrontend> frontend(frontendFactory->create(moduleRef));
    // Create the actual GUI representation.
    QWidget* gui = qobject_cast<QWidget*>(frontend->guiHandle());


   this->addWidget(gui);
}


cliSupportToolBox::~cliSupportToolBox()
{
    delete d;
    d = NULL;
}


QString cliSupportToolBox::name()
{
    return "CTK CLI Support";
}


QString cliSupportToolBox::identifier()
{
    return "cliSupportToolBox";
}


QString cliSupportToolBox::description()
{
    return "CommandLine plugin support using CTK implementation";
}


void cliSupportToolBox::setData(dtkAbstractData *data)
{
    qDebug() << "CLI setData called";
}


void cliSupportToolBox::clear(void)
{
    d->view = 0;
}


void cliSupportToolBox::update(dtkAbstractView * view)
{
    if (d->view == view)
        return;

    if (d->view)
        QObject::disconnect(d->view, SIGNAL(propertySet(QString, QString)), this, 0);


    qDebug() << "CLI update called";
    d->view = view;

//    connect(d->view, SIGNAL(propertySet(QString,QString)), this, SLOT(updateLandmarksRenderer(QString,QString)));
}
