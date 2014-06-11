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


#include <medCoreExport.h>
#include <medDataIndex.h>

class medAbstractData;
class dtkAbstractDataFactory;

class medOldDataManagerPrivate;
class medAbstractDbController;

/**
 * This class is the global access point to data stored in the database.
 * It tries to use several database-controllers to provide/store data
 * Another role is to cache data to provide faster access (work in progress)
 */
class MEDCORE_EXPORT medOldDataManager : public QObject
{
    Q_OBJECT

public:
#if 0
    
    bool setMetaData( const medDataIndex& index, const QString& key, const QString& value );

    /**
    * Use this function to insert data into the database,
    * Do *not* use the concrete database controller implementation for it
    * The data-manager will take over this task
    * @params medAbstractData  *data
    */
    void import(medAbstractData *data);

    /**
    * Import data into the db read from file
    * @params const QString & file The file containing the data
    * @params bool indexWithoutCopying true if the file must only be indexed by its current path,
    * false if the file will be imported (copied or converted to the internal storage format)
    */
    void import(const QString& file,bool indexWithoutCopying);

    /**
    * Use this function to insert data into the non-persistent database,
    * Do *not* use the concrete database controller implementation for it
    * The data-manager will take over this task
    * @params const medAbstractData & data
    */
    void importNonPersistent(medAbstractData *data);

    /**
    * Use this function to insert data into the non-persistent database,
    * Do *not* use the concrete database controller implementation for it
    * The data-manager will take over this task
    * @params const medAbstractData & data
    * @params QString uuid Universally unique identifier associated with the data
    */
    void importNonPersistent(medAbstractData *data, QString uuid);

    /**
    * Overload to insert data directly from a file into the no-persistent database
    * @params QString file
    */
    void importNonPersistent(QString file);

    /**
    * Overload to insert data directly from a file into the no-persistent database
    * @params QString file
    * @params QString & uuid Universally unique identifier associated with the data
    */
    void importNonPersistent(QString file, const QString &uuid);

    /**
    * Use this function to save all non-persistent data to the sql database.
    * The list of non-persistent data will then be cleared, and any subsequent
    * access to those data will trigger a reading from the database.
    */
    void storeNonPersistentDataToDatabase();

    /**
     * Use this function to save a set of non-persistent data (e.g. from the same patient, study or series) to the sql database.
     * The data is specified by its medDataIndex , it is then removed from the non persistent database
     */
    void storeNonPersistentMultipleDataToDatabase( const medDataIndex &index );

    /**
     * Use this function to save one non-persistent data to the sql database.
     * The data is specified by ots medDataIndex , it is then removed from the non persistent database
     */
    void storeNonPersistentSingleDataToDatabase( const medDataIndex &index );

    /**
    * Returns the number of non-persistent data contained in the data manager
    */
    int nonPersistentDataCount() const;

    /**
    * Clear the list of non-persistent data
    */
    void clearNonPersistentData();


    /** Remove an item or items from the database
     *  Will remove a whole patient / study / series depending on the index.
     */
    void removeData(const medDataIndex& index);

    /**
     * Moves study and its series from one patient to another and returns the list of new indexes.
     * Moves across different datasources are not supported.
     * @params const medDataIndex & indexStudy The data index of the study to be moved
     * @params const medDataIndex & toPatient The data index to move the study to.
     */
    QList<medDataIndex> moveStudy(const medDataIndex& indexStudy, const medDataIndex& toPatient);

    /**
     * Moves serie from one study to another and returns the new index of the serie.
     * Moves across different datasources are not supported.
     * @params const medDataIndex & indexSerie The data index of the serie to be moved
     * @params const medDataIndex & toStudy The data index to move the serie to.
     */    
    medDataIndex moveSerie(const medDataIndex& indexSerie, const medDataIndex& toStudy);


    /** return the DB controller for given data source. */
    medAbstractDbController *controllerForDataSource( int dataSource );
    const medAbstractDbController *controllerForDataSource( int dataSource ) const;

    /** Return a list of available dataSource Ids.*/
    QList<int> dataSourceIds() const;

    /**
     * Clear all items stored in the data manager
     */
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
     medOldDataManager();
    ~medOldDataManager();

    /** Remove all matching items from the cache. */
    void removeDataFromCache( const medDataIndex &index);

    /**
    * Defines writers priorities
    */
    void setWriterPriorities();

protected slots:

    void exportDialog_updateSuffix(int index);

private:
    medOldDataManagerPrivate *d;
#endif
};


