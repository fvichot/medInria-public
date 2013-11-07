#pragma once

#include <ctkCmdLineModuleQtUiLoader.h>
#include <ctkCmdLineModuleFrontendQtGui.h>
#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleParameter.h>

#include <dtkCore/dtkSmartPointer>

#include <medDropSite.h>
#include <QComboBox>

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

class cliDataInputWidget;
class cliDataOutputWidget;
class medWorkspace;
class cliSupportFrontendQtGui : public ctkCmdLineModuleFrontendQtGui
{
    Q_OBJECT
public :
    cliSupportFrontendQtGui(const ctkCmdLineModuleReference& moduleRef, medWorkspace * workspace);
    virtual ~cliSupportFrontendQtGui();

    void preRun();
    void postRun();

    void addDataInput(cliDataInputWidget * input);
    void addDataOutput(cliDataOutputWidget * output);

protected:
    virtual QUiLoader* uiLoader() const;

private:
    mutable QScopedPointer<cliSupportUiLoader> _loader;
    QList<cliDataInputWidget*> _inputList;
    QList<cliDataOutputWidget*> _outputList;
    QDir _runDir;
    medWorkspace * _workspace;
};


class cliDataInputWidget : public medDropSite
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)

public:
    cliDataInputWidget(ctkCmdLineModuleParameter param, QWidget * parent = 0);
    virtual ~cliDataInputWidget();

    QString filePath() const;
    void setFilePath(QString path);

    ctkCmdLineModuleParameter parameter() const;

private:
    QString _filePath;
    ctkCmdLineModuleParameter _param;
};

class cliDataOutputWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)

public:
    cliDataOutputWidget(ctkCmdLineModuleParameter param, QWidget * parent = 0);
    virtual ~cliDataOutputWidget();

    QString filePath() const;
    void setFilePath(QString path);

    ctkCmdLineModuleParameter parameter() const;

    enum OutputTarget {OpenInNewView, OpenInNewTab, OutputToFile};

    OutputTarget target() const;

private:
    QString _filePath;
    ctkCmdLineModuleParameter _param;
    QComboBox * _targetList;
};

class dtkAbstractData;
class cliFileHandler : public QObject
{
    Q_OBJECT
public:
    cliFileHandler(QObject * parent = 0);
    virtual ~cliFileHandler();

    static QString compatibleImportExtension(QStringList supportedExtensions);

    dtkAbstractData * importFromFile(QString file);
    QString exportToFile(dtkAbstractData * data, QString filePath, QStringList formats);

protected slots:
    void dataImported(medDataIndex index);

private:
    QEventLoop _loopyLoop;
    dtkSmartPointer<dtkAbstractData> _data;
};
