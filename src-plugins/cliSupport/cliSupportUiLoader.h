#pragma once

#include <ctkCmdLineModuleQtUiLoader.h>
#include <ctkCmdLineModuleFrontendQtGui.h>
#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleParameter.h>

#include <medDropSite.h>

class cliSupportFrontendQtGui;
class cliSupportUiLoader : public ctkCmdLineModuleQtUiLoader
{
    Q_OBJECT
public:
    cliSupportUiLoader(cliSupportFrontendQtGui * frontend, QObject *parent=0);
    virtual ~cliSupportUiLoader();

    virtual QWidget* createWidget(const QString & className, QWidget * parent = 0, const QString & name = QString() );

private:
    cliSupportFrontendQtGui * _frontend;
};

class cliDataSelectorWidget;

class cliSupportFrontendQtGui : public ctkCmdLineModuleFrontendQtGui
{
    Q_OBJECT
public :
    cliSupportFrontendQtGui(const ctkCmdLineModuleReference& moduleRef);
    virtual ~cliSupportFrontendQtGui();

    void preRun();
    void postRun();

    void registerDataSelector(cliDataSelectorWidget * selector);

protected:
    virtual QUiLoader* uiLoader() const;

private:
    mutable QScopedPointer<cliSupportUiLoader> _loader;
    QList<cliDataSelectorWidget*> _selectors;
    QDir _runDir;
};


class cliDataSelectorWidget : public medDropSite
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)

public:
    cliDataSelectorWidget(ctkCmdLineModuleParameter param, QWidget * parent = 0);
    virtual ~cliDataSelectorWidget();

    QString filePath() const;
    void setFilePath(QString path);

    ctkCmdLineModuleParameter parameter() const;

protected slots:
    void dataSelected(medDataIndex index);

private:
    QString _filePath;
    ctkCmdLineModuleParameter _param;
};

class dtkAbstractData;

class cliFileExporter : public QObject
{
    Q_OBJECT
public:
    cliFileExporter(QObject * parent = 0);
    virtual ~cliFileExporter();

    dtkAbstractData * importFromFile(QString file);
    QString exportToFile(dtkAbstractData * data, QString filePath, QStringList formats);

protected slots:
    void dataImported(medDataIndex index);

private:
    QEventLoop _loopyLoop;
    dtkAbstractData * _data;
};
