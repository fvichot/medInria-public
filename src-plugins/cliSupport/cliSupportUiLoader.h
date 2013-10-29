#pragma once

#include <ctkCmdLineModuleQtUiLoader.h>
#include <ctkCmdLineModuleFrontendQtGui.h>


class cliSupportUiLoader : public ctkCmdLineModuleQtUiLoader
{
    Q_OBJECT
public:
    cliSupportUiLoader(QObject *parent=0);
    virtual ~cliSupportUiLoader();

    virtual QWidget* createWidget(const QString & className, QWidget * parent = 0, const QString & name = QString() );
};


class cliSupportFrontendQtGui : public ctkCmdLineModuleFrontendQtGui
{
    Q_OBJECT
public :
    cliSupportFrontendQtGui(const ctkCmdLineModuleReference& moduleRef);
    virtual ~cliSupportFrontendQtGui();

protected:
    virtual QUiLoader* uiLoader() const;

private:
    mutable QScopedPointer<cliSupportUiLoader> loader;
};
