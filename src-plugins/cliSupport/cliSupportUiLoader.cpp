#include "cliSupportUiLoader.h"

#include <QDebug>
#include <QScopedPointer>


cliSupportUiLoader::cliSupportUiLoader(QObject *parent)
    : ctkCmdLineModuleQtUiLoader(parent)
{
}


cliSupportUiLoader::~cliSupportUiLoader()
{
}


QWidget * cliSupportUiLoader::createWidget(const QString &className, QWidget *parent, const QString &name)
{
    qDebug() << className;
    return ctkCmdLineModuleQtUiLoader::createWidget(className, parent, name);
}


cliSupportFrontendQtGui::cliSupportFrontendQtGui(const ctkCmdLineModuleReference & moduleRef)
    : ctkCmdLineModuleFrontendQtGui(moduleRef)
{
}


cliSupportFrontendQtGui::~cliSupportFrontendQtGui()
{
}


QUiLoader * cliSupportFrontendQtGui::uiLoader() const
{
    if (loader == NULL)
    {
      loader.reset(new cliSupportUiLoader());
    }
    return loader.data();
}
