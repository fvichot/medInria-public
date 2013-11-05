#include <cliSupportUiLoader.h>
#include <ctkCmdLineModuleXslTransform.h>
#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCmdLineModuleParameter.h>

#include <dtkCore/dtkAbstractDataFactory.h>

#include <medDataManager.h>

#include <QDebug>
#include <QScopedPointer>
#include <QComboBox>

bool removeDir(const QString & dirName)
{
    bool result = false;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        foreach(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}

// ------------------------- UiLoader ----------------------------------------

cliSupportUiLoader::cliSupportUiLoader(cliSupportFrontendQtGui * frontend, QObject *parent)
    : ctkCmdLineModuleQtUiLoader(parent)
    , _frontend(frontend)
{
}


cliSupportUiLoader::~cliSupportUiLoader()
{
}


QWidget * cliSupportUiLoader::createWidget(const QString &className, QWidget *parent, const QString &name)
{
    if ( ! name.startsWith("parameter:"))
        return ctkCmdLineModuleQtUiLoader::createWidget(className, parent, name);

    qDebug() << name;

    ctkCmdLineModuleDescription modDescription = _frontend->moduleReference().description();
    QString paramName = name.mid(10);
    if ( ! modDescription.hasParameter(paramName)) {
        qDebug() << "cliSupport : Parameter" << paramName << "doesn't exist !";
        return ctkCmdLineModuleQtUiLoader::createWidget(className, parent, name);
    }

    ctkCmdLineModuleParameter param = modDescription.parameter(paramName);

    if (param.tag() == "image" && param.channel() == "input") {
        qDebug() << "found an image input";
        cliDataSelectorWidget * s = new cliDataSelectorWidget(param, parent);
        _frontend->registerDataSelector(s);
        return s;
    }

    return ctkCmdLineModuleQtUiLoader::createWidget(className, parent, name);
}

// ---------------------------- Frontend -------------------------------------

cliSupportFrontendQtGui::cliSupportFrontendQtGui(const ctkCmdLineModuleReference & moduleRef)
    : ctkCmdLineModuleFrontendQtGui(moduleRef)
{
    ctkCmdLineModuleXslTransform * t = this->xslTransform();

    QStringList paramTagsList;
    paramTagsList << "imageInput" //<< "imageOutput"
                  << "geometryInput" << "geometryOutput"
                  << "transformInput" << "transformOutput"
                  << "tableInput" << "tableOutput"
                  << "measurementInput" << "measurementOutput";

    foreach(const QString& xslParamName, paramTagsList) {
        t->bindVariable(QString("%1Widget").arg(xslParamName), "cliDataSelector");
        t->bindVariable(QString("%1SetProperty").arg(xslParamName), "");
        t->bindVariable(QString("%1ValueProperty").arg(xslParamName), "filePath");
    }
}


cliSupportFrontendQtGui::~cliSupportFrontendQtGui()
{
}


void cliSupportFrontendQtGui::preRun()
{
    // create temp dir for input files
    int count = 0;
    _runDir = QDir::temp();
    while ( ! _runDir.mkdir(QString("medInria_cli_run_%1").arg(count)))
        count++;

    foreach(cliDataSelectorWidget * s, _selectors) {
        dtkAbstractData * data = medDataManager::instance()->data(s->index());
        if ( ! data) continue;

        cliFileExporter exporter;
        QString exportPath = exporter.exportToFile(data, _runDir.absoluteFilePath(s->parameter().name()), s->parameter().fileExtensions());
        s->setFilePath(exportPath);
    }
}


void cliSupportFrontendQtGui::postRun()
{

    //removeDir(_runDir.absolutePath());
}


void cliSupportFrontendQtGui::registerDataSelector(cliDataSelectorWidget *selector)
{
    _selectors.append(selector);
}


QUiLoader * cliSupportFrontendQtGui::uiLoader() const
{
    if (_loader == NULL) {
      _loader.reset(new cliSupportUiLoader(const_cast<cliSupportFrontendQtGui*>(this)));
    }
    return _loader.data();
}

// ------------------------ Widgets ------------------------------------------

cliDataSelectorWidget::cliDataSelectorWidget(ctkCmdLineModuleParameter param, QWidget * parent)
    : medDropSite(parent)
    , _param(param)
{
    connect(this, SIGNAL(objectDropped(medDataIndex)), this, SLOT(dataSelected(medDataIndex)));
}


cliDataSelectorWidget::~cliDataSelectorWidget()
{

}


QString cliDataSelectorWidget::filePath() const
{
    return _filePath;
}


void cliDataSelectorWidget::setFilePath(QString path)
{
    _filePath = path;
}


ctkCmdLineModuleParameter cliDataSelectorWidget::parameter() const
{
    return _param;
}


void cliDataSelectorWidget::dataSelected(medDataIndex index)
{
}

// --------------------- File exporter ------------------------------------

cliFileExporter::cliFileExporter(QObject * parent)
    : QObject(parent)
{

}

cliFileExporter::~cliFileExporter()
{

}

dtkAbstractData * cliFileExporter::importFromFile(QString file)
{
    connect(medDataManager::instance(), SIGNAL(dataAdded(medDataIndex)),
            this, SLOT(dataImported(medDataIndex)));
    // We connect fail to import on the same slot, as failure will end up with NULL _data
    connect(medDataManager::instance(), SIGNAL(failedToOpen(medDataIndex)),
            this, SLOT(dataImported(medDataIndex)));
    medDataManager::instance()->importNonPersistent(file);
    _loopyLoop.exec(QEventLoop::ExcludeUserInputEvents);
    return _data;
}

QString cliFileExporter::exportToFile(dtkAbstractData * data, QString filePath, QStringList formats)
{
    QList<QString> writers = dtkAbstractDataFactory::instance()->writers();
    bool written = false;
    QString finalFullPath;
    QScopedPointer<dtkAbstractDataWriter> dataWriter;
    foreach(QString currentFormat, formats) {
        for (int i=0; i<writers.size(); i++) {
            dataWriter.reset(dtkAbstractDataFactory::instance()->writer(writers[i]));

            if (! dataWriter->handled().contains(data->identifier()) ||
                ! dataWriter->supportedFileExtensions().contains(currentFormat))
                continue;

            dataWriter->setData (data);

            finalFullPath = filePath+currentFormat;
            qDebug() << "=============== finalFullpath:" << finalFullPath;
            if (dataWriter->canWrite(finalFullPath) && dataWriter->write(finalFullPath)) {
                written = true;
                break;
            }
        }
        if (written)
            break;
    }
    qDebug() << "+++++++++++++++++ FullPath:"<< finalFullPath;
    return finalFullPath;
}


void cliFileExporter::dataImported(medDataIndex index)
{
    _data = medDataManager::instance()->data(index);
    _loopyLoop.quit();
}
