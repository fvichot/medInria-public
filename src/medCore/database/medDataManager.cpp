/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medDataManager.h>

#include <QDebug>

#include <medAbstractDataFactory.h>
#include <medDatabaseController.h>
#include <medDatabaseNonPersistentController.h>
#include <medDatabaseExporter.h>
#include <medMessageController.h>
#include <medJobManager.h>

/* THESE CLASSES NEED TO BE THREAD-SAFE, don't forget to lock the mutex in the
 * methods below that access state.
 */

class medDataManagerPrivate
{
public:
    medDataManagerPrivate(medDataManager * q)
        : q_ptr(q)
        , mutex(QMutex::Recursive)
    {
        dbController = medDatabaseController::instance();
        nonPersDbController = medDatabaseNonPersistentController::instance();

        if( ! dbController || ! nonPersDbController) {
            qCritical() << "One of the DB controllers could not be created !";
        }
    }

    void cleanupTracker()
    {
        QMutexLocker lock(&mutex);
        foreach(const medDataIndex& i, loadedDataObjectTracker.keys())
            if (loadedDataObjectTracker.value(i).isNull())
                loadedDataObjectTracker.remove(i);
    }

    medAbstractDbController* controllerForDataSource(int id) {
        if (dbController->dataSourceId() == id)
            return dbController;
        else if (nonPersDbController->dataSourceId() == id)
            return dbController;
        else
            return NULL;
    }

    Q_DECLARE_PUBLIC(medDataManager)

    medDataManager * const q_ptr;
    QMutex mutex;
    QHash<medDataIndex, dtkSmartPointer<medAbstractData> > loadedDataObjectTracker;
    medAbstractDbController * dbController;
    medAbstractDbController * nonPersDbController;
};

// ------------------------- medDataManager -----------------------------------

medDataManager * medDataManager::s_instance = NULL;

// Not thread-safe, but should only be called once, at application start-up
void medDataManager::initialize()
{
    if ( ! s_instance) {
        s_instance = new medDataManager();
    }
}


medDataManager * medDataManager::instance()
{
    return s_instance;
}


medAbstractData* medDataManager::retrieveData(const medDataIndex& index)
{
    Q_D(medDataManager);
    QMutexLocker locker(&(d->mutex));

    // If nothing in the tracker, we'll get a null weak pointer, thus a null shared pointer
    medAbstractData *dataObjRef = d->loadedDataObjectTracker.value(index);

    if(dataObjRef) {
        // we found an existing instance of that object
        qDebug()<<"medDataManager we found an existing instance of that object" <<dataObjRef->count();
        return dataObjRef;
    }

    QHashIterator <medDataIndex, dtkSmartPointer<medAbstractData> > it(d->loadedDataObjectTracker);
    while(it.hasNext()) {
        medAbstractData *data = it.next().value();
        qDebug()<<"medDataManager deleting ?" <<data->count();
        if(data->count() <= 1) {
            qDebug()<<"medDataManager deleting !" <<data->dataIndex();
            d->loadedDataObjectTracker.remove(data->dataIndex());
        }
    }

    // No existing ref, we need to load from the file DB, then the non-persistent DB
    if (d->dbController->contains(index)) {
        dataObjRef = d->dbController->retrieve(index);
    } else if(d->nonPersDbController->contains(index)) {
        dataObjRef = d->nonPersDbController->retrieve(index);
    }

    if (dataObjRef) {
        dataObjRef->setDataIndex(index);

        d->loadedDataObjectTracker.insert(index, dataObjRef);
        return dataObjRef;
    }
    return NULL;
}


QUuid medDataManager::importData(medAbstractData *data, bool nonPersistent)
{
    if (!data)
        return QUuid();

    Q_D(medDataManager);
    QUuid uuid = QUuid::createUuid();
    medAbstractDbController * controller = nonPersistent ? d->nonPersDbController : d->dbController;
    controller->importData(data, uuid);
    return uuid;
}


QUuid medDataManager::importFile(const QString& dataPath, bool indexWithoutCopying, bool nonPersistent)
{
    if ( ! QFile::exists(dataPath))
        return QUuid();

    Q_D(medDataManager);
    QUuid uuid = QUuid::createUuid();
    medAbstractDbController * controller = nonPersistent ? d->nonPersDbController : d->dbController;
    controller->importPath(dataPath, uuid, indexWithoutCopying);
    return uuid;
}


void medDataManager::exportData(medAbstractData* data)
{
    if (!data)
        return;

    Q_D(medDataManager);

    QList<QString> allWriters = medAbstractDataFactory::instance()->writers();
    QHash<QString, dtkAbstractDataWriter*> possibleWriters;

    foreach(QString writerType, allWriters) {
        dtkAbstractDataWriter * writer = medAbstractDataFactory::instance()->writer(writerType);
        if (writer->handled().contains(data->identifier()))
            possibleWriters[writerType] = writer;
        else
            delete writer;
    }

    if (possibleWriters.isEmpty()) {
        medMessageController::instance()->showError("Sorry, we have no exporter for this format.");
        return;
    }

    QFileDialog * exportDialog = new QFileDialog(0, tr("Exporting : please choose a file name and directory"));
    exportDialog->setOption(QFileDialog::DontUseNativeDialog);
    exportDialog->setAcceptMode(QFileDialog::AcceptSave);

    QComboBox* typesHandled = new QComboBox(exportDialog);
    // we use allWriters as the list of keys to make sure we traverse possibleWriters
    // in the order specified by the writers priorities.
    foreach(QString type, allWriters) {
        if (!possibleWriters.contains(type))
            continue;

        QStringList extensionList = possibleWriters[type]->supportedFileExtensions();
        QString label = possibleWriters[type]->description() + " (" + extensionList.join(", ") + ")";
        QString extension = (extensionList.isEmpty()) ? QString() : extensionList.first();
        typesHandled->addItem(label, type);
        typesHandled->setItemData(typesHandled->count()-1, extension, Qt::UserRole+1);
        typesHandled->setItemData(typesHandled->count()-1, QVariant::fromValue<QObject*>(exportDialog), Qt::UserRole+2);
    }
    connect(typesHandled, SIGNAL(currentIndexChanged(int)), this, SLOT(exportDialog_updateSuffix(int)));

    QLayout* layout = exportDialog->layout();
    QGridLayout* gridbox = qobject_cast<QGridLayout*>(layout);

    // nasty hack to hide the filter list
    QWidget * filtersLabel = gridbox->itemAtPosition(gridbox->rowCount()-1, 0)->widget();
    QWidget * filtersList = gridbox->itemAtPosition(gridbox->rowCount()-1, 1)->widget();
    filtersLabel->hide(); filtersList->hide();

    if (gridbox) {
        gridbox->addWidget(new QLabel("Export format:", exportDialog), gridbox->rowCount()-1, 0);
        gridbox->addWidget(typesHandled, gridbox->rowCount()-1, 1);
    }

    exportDialog->setLayout(gridbox);

    // Set a default filename based on the series's description
    medAbstractDbController * dbController = d->controllerForDataSource(data->dataIndex().dataSourceId());
    if (dbController) {
        QString defaultName = dbController->metaData(data->dataIndex(), medMetaDataKeys::SeriesDescription);
        defaultName += typesHandled->itemData(typesHandled->currentIndex(), Qt::UserRole+1).toString();
        exportDialog->selectFile(defaultName);
    }

    if ( exportDialog->exec() ) {
        QString chosenFormat = typesHandled->itemData(typesHandled->currentIndex()).toString();
        this->exportDataToFile(data, exportDialog->selectedFiles().first(), chosenFormat);
    }

    qDeleteAll(possibleWriters);
    delete exportDialog;
}


void medDataManager::exportDataToFile(medAbstractData *data, const QString & filename, const QString & writer)
{
    medDatabaseExporter *exporter = new medDatabaseExporter (data, filename, writer);
    QFileInfo info(filename);
    medMessageProgress *message = medMessageController::instance()->showProgress("Exporting data to " + info.baseName());

    connect(exporter, SIGNAL(progressed(int)), message, SLOT(setProgress(int)));
    connect(exporter, SIGNAL(success(QObject *)), message, SLOT(success()));
    connect(exporter, SIGNAL(failure(QObject *)), message, SLOT(failure()));

    medJobManager::instance()->registerJobItem(exporter);
    QThreadPool::globalInstance()->start(exporter);
}


QList<medDataIndex> medDataManager::changePatientForStudy(const medDataIndex& indexStudy, const medDataIndex& toPatient)
{
    Q_D(medDataManager);
    medAbstractDbController * dbcSource = d->controllerForDataSource(indexStudy.dataSourceId());
    medAbstractDbController * dbcDest = d->controllerForDataSource(toPatient.dataSourceId());

    QList<medDataIndex> newIndexList;

    if(!dbcSource || !dbcDest) {
        qWarning() << "Incorrect controllers";
    } else if( dbcSource != dbcDest ) {
        qWarning() << "Moving studies across different controllers is currently not supported";
    } else if( !dbcSource->isPersistent() && dbcDest->isPersistent() ) {
        qWarning() << "Move from non persistent to persistent controller not allowed. Please save data first.";
    } else {
        newIndexList = dbcSource->moveStudy(indexStudy,toPatient);

        if(!dbcSource->isPersistent()) {
            foreach(medDataIndex newIndex, newIndexList)
                d->volatileDataCache[newIndex] = dbcSource->retrieve(newIndex);
        }
    }

    return newIndexList;
}


medDataIndex medDataManager::moveSerie(const medDataIndex& indexSerie, const medDataIndex& toStudy)
{
    medAbstractDbController *dbcSource = controllerForDataSource(indexSerie.dataSourceId());
    medAbstractDbController *dbcDest = controllerForDataSource(toStudy.dataSourceId());

    medDataIndex newIndex;

    if(!dbcSource || !dbcDest)
    {
      qWarning() << "Incorrect controllers";
    }
    else if( dbcSource->isPersistent() && !dbcDest->isPersistent() )
    {
      qWarning() << "Move from persistent to non persistent controller not allowed";
    }
    else if( !dbcSource->isPersistent() && dbcDest->isPersistent() )
    {
      qWarning() << "Move from non persistent to persistent controller not allowed. Please save data first.";
    }
    else
    {
      newIndex =  dbcSource->moveSerie(indexSerie,toStudy);

      if(!dbcSource->isPersistent())
        d->volatileDataCache[newIndex] = dbcSource->retrieve(newIndex);
    }

    return newIndex;

}


bool medDataManager::setMetadata(const medDataIndex &index, const QString& key, const QString & value)
{
    if ( ! index.isValid())
        return false;

    Q_D(medDataManager);
    medAbstractDbController * dbc = d->controllerForDataSource(index.dataSourceId());
    return dbc->setMetaData(index, key, value);
}


bool medDataManager::makePersistent(medAbstractData* data)
{

}


bool medDataManager::updateMetadata(const medDataIndex& index, const medMetaDataKeys::Key& md, const QString& value)
{

}


void medDataManager::removeData(const medDataIndex& index)
{

}


medDataManager::medDataManager() : d_ptr(new medDataManagerPrivate(this))
{
    Q_D(medDataManager);
    connect(d->dbController, SIGNAL(updated(medDataIndex)), this, SIGNAL(dataImported(medDataIndex)));
    connect(d->nonPersDbController, SIGNAL(updated(medDataIndex)), this, SIGNAL(dataImported(medDataIndex)));
}


medDataManager::~medDataManager()
{

}
