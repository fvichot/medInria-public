/*=========================================================================

medInria

Copyright (c) INRIA 2013. All rights reserved.
See LICENSE.txt for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.

=========================================================================*/

#include "medDataSourceManager.h"
#include <dtkCore>

#include <QList>
#include <medAbstractDataSource.h>
#include <medAbstractDataSourceFactory.h>
#include <medDataManager.h>
#include <medPacsWidget.h>
#include <medMetaDataKeys.h>
#include <medStorage.h>
#include <medMessageController.h>

#include <medFileSystemDataSource.h>
#include <medDatabaseDataSource.h>
#include <medPacsDataSource.h>

class medDataSourceManagerPrivate
{
public:
    QList <medAbstractDataSource*> dataSources;

    medDatabaseDataSource *dbSource;
    medFileSystemDataSource *fsSource;
    medPacsDataSource *pacsSource;
};

medDataSourceManager::medDataSourceManager(): d(new medDataSourceManagerPrivate)
{
    // Data base data source
    d->dbSource = new medDatabaseDataSource();
    d->dataSources.push_back(d->dbSource);
    connectDataSource(d->dbSource);

    // File system data source
    d->fsSource = new medFileSystemDataSource();
    d->dataSources.push_back(d->fsSource);
    connectDataSource(d->fsSource);

    // Pacs data source
    medPacsDataSource *pacsDataSource = new medPacsDataSource;
    medPacsWidget * mainPacsWidget = qobject_cast<medPacsWidget*> (pacsDataSource->mainViewWidget());
    //make the widget hide if not functional (otehrwise it flickers in and out).
    mainPacsWidget->hide();
    if (mainPacsWidget->isServerFunctional())
    {
        d->pacsSource = new medPacsDataSource();
        d->dataSources.push_back(d->pacsSource);
        connectDataSource(d->pacsSource);
    }
    else mainPacsWidget->deleteLater();

    // dynamic data sources (from plugins)

    foreach(QString dataSourceName, medAbstractDataSourceFactory::instance()->dataSourcePlugins())
    {
        qDebug()<< "factory creates dataSource:" << dataSourceName;
        medAbstractDataSource *dataSource = medAbstractDataSourceFactory::instance()->create(dataSourceName, 0);
        d->dataSources.push_back(dataSource);
        connectDataSource(dataSource);
    }

    connect(medDataManager::instance(), SIGNAL(dataAdded(const medDataIndex &)),
            d->dbSource, SLOT(update(const medDataIndex&)));
    connect(medDataManager::instance(), SIGNAL(dataRemoved(const medDataIndex &)),
            d->dbSource, SLOT(update(const medDataIndex&)));
    connect(medDataManager::instance(), SIGNAL(failedToOpen(const medDataIndex&)),
            d->dbSource , SLOT(onOpeningFailed(const medDataIndex&)));

    connect(d->fsSource, SIGNAL(open(QString)),
            this, SIGNAL(open(QString)));
    connect(d->fsSource, SIGNAL(load(QString)),
            this, SIGNAL(load(QString)));
    connect(d->dbSource, SIGNAL(open(const medDataIndex&)),
            this, SIGNAL(open(const medDataIndex&)));
}


void medDataSourceManager::connectDataSource(medAbstractDataSource *dataSource)
{
    connect(dataSource, SIGNAL(exportData(const medDataIndex&)),
            this, SLOT(exportData(const medDataIndex&)));

    connect(dataSource, SIGNAL(dataReceived(medAbstractData*)),
            this, SLOT(importData(medAbstractData*)));

    connect(dataSource, SIGNAL(dataReceivingFailed(QString)),
            this, SLOT(emitDataReceivingFailed(QString)));

    connect(dataSource, SIGNAL(dataToImportReceived(QString)),
            this, SLOT(importFile(QString)));

    connect(dataSource, SIGNAL(dataToIndexReceived(QString)),
            this, SLOT(indexFile(QString)));
}

//TODO: Maybe it is not the best place to put it (medDataManager?)
void medDataSourceManager::importData(medAbstractData *data)
{
    QString patientName = data->metaDataValues(medMetaDataKeys::PatientName.key())[0];
    QString studyName = data->metaDataValues(medMetaDataKeys::StudyDescription.key())[0];
    QString seriesName = data->metaDataValues(medMetaDataKeys::SeriesDescription.key())[0];

    QString s_patientName = patientName.simplified();
    QString s_studyName = studyName.simplified();
    QString s_seriesName = seriesName.simplified();

    if ((s_patientName == "")||(s_studyName == "")||(s_seriesName == ""))
        return;

    QFileInfo fileInfo (medStorage::dataLocation() + "/" + s_patientName + "/" + s_studyName + "/");

    if (!fileInfo.dir().exists() && !medStorage::mkpath (fileInfo.dir().path()))
    {
        qDebug() << "Cannot create directory: " << fileInfo.dir().path();
        return;
    }

    dtkSmartPointer<medAbstractData> data_smart(data);
    medDataManager::instance()->import(data_smart);
}

void medDataSourceManager::exportData(const medDataIndex &index)
{
    dtkSmartPointer<medAbstractData> data = medDataManager::instance()->data(index);
        medDataManager::instance()->exportDataToFile(data);
}

void medDataSourceManager::importFile(QString path)
{
    medDataManager::instance()->import(path,false);
}

void medDataSourceManager::indexFile(QString path)
{
    medDataManager::instance()->import(path, true);

}


void medDataSourceManager::emitDataReceivingFailed(QString fileName)
{
  medMessageController::instance()->showError(tr("Unable to get from source the data named ") + fileName, 3000);
}

medDataSourceManager * medDataSourceManager::instance( void )
{
    if(!s_instance)
        s_instance = new medDataSourceManager;
    return s_instance;
}

medDataSourceManager::~medDataSourceManager( void )
{
    delete d;
    d = NULL;
}

void medDataSourceManager::destroy( void )
{
    if (s_instance)
    {
        delete s_instance;
        s_instance = 0;
    }
}

QList <medAbstractDataSource*> medDataSourceManager::dataSources()
{
    return d->dataSources;
}

medDatabaseDataSource* medDataSourceManager::databaseDataSource()
{
    return d->dbSource;
}

medDataSourceManager *medDataSourceManager::s_instance = NULL;
