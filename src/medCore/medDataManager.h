/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <QtCore/QObject>

#include "medGlobal.h"
#include "medCoreExport.h"
#include "medDataIndex.h"

class medAbstractData;
class medDataManagerPrivate;

class MEDCORE_EXPORT medDataManager : public QObject
{
    Q_OBJECT

public:
    static medDataManager* instance();

    medAbstractData* data(const medDataIndex& index);

    void import(medAbstractData* data);
    void import(const QString& dataPath, bool indexWithoutCopying);

    void importNonPersistent(medAbstractData* data);
    void importNonPersistent(const QString& dataPath);

    void exportDataToFile(medAbstractData* data);

    bool updateData(const medDataIndex& index, medAbstractData* data);
    bool updateMetadata( const medDataIndex& index, const QString& key, const QString& value );

    void removeData(const medDataIndex& index);

private:
    void importNonPersistent(dtkAbstractData *data, QString uuid);

    void importNonPersistent(QString file, const QString &uuid);

    void storeNonPersistentDataToDatabase();

    void storeNonPersistentMultipleDataToDatabase( const medDataIndex &index );

    void storeNonPersistentSingleDataToDatabase( const medDataIndex &index );

    int nonPersistentDataCount() const;

    void clearNonPersistentData();



    QList<medDataIndex> moveStudy(const medDataIndex& indexStudy, const medDataIndex& toPatient);

    medDataIndex moveSerie(const medDataIndex& indexSerie, const medDataIndex& toStudy);


    medAbstractDbController *controllerForDataSource( int dataSource );
    const medAbstractDbController *controllerForDataSource( int dataSource ) const;

    QList<int> dataSourceIds() const;

    static bool is32Bit();

    static size_t getProcessMemoryUsage();

    static size_t getTotalSizeOfPhysicalRam();

    static quint64 getUpperMemoryThreshold();

    static size_t getOptimalMemoryThreshold();

    void clearCache();
   

signals:
    /**
    * This signal is emitted whenever a data was added in either the persistent
    * or non persistent database by calling import() or importNonPersistentData().
    */
    void dataAdded (const medDataIndex&);

    /**
    * This signal is emitted whenever a data is removed in either the persistent
    * or non persistent database by calling remove().
    */
    void dataRemoved (const medDataIndex&);

    /**
     * @brief Emitted when an image fails to open
     * @param the @medDataIndex of the image
    */
    void failedToOpen(const medDataIndex&);

    /**
     * This signal is emitted when the operation has progressed
     * @param QObject *obj Pointer to the operating QObject
     * @param int value Progress bar's current value
     */
    void progressed(QObject* obj,int value);

    void openRequested(const medDataIndex& index, int slice);

    /**
     * @brief Emitted when the user double clicks on a medDatabasenavigatorItem (thumbnail)
     * @param the @medDataIndex of the image
    */
    void openRequested(const medDataIndex& index);

public slots:
    void onNonPersistentDataImported(const medDataIndex &index, QString uuid);
    void onPersistentDatabaseUpdated(const medDataIndex &index);
    void onSingleNonPersistentDataStored(const medDataIndex &index, const QString &uuid);

protected:
     medDataManager();
     virtual ~medDataManager();



    bool manageMemoryUsage(const medDataIndex& index, medAbstractDbController* controller);


    static int ReadStatmFromProcFS( int* size, int* res, int* shared, int* text, int* sharedLibs, int* stack, int* dirtyPages );

    void removeDataFromCache( const medDataIndex &index);

    void printMemoryStatus(size_t requiredMemoryInKb = 0);


    bool tryFreeMemory(size_t memoryLimit);
    

    void setWriterPriorities();

protected slots:

    void exportDialog_updateSuffix(int index);

private:
    static medDataManager* s_instance;
    medDataManagerPrivate* d;
};


