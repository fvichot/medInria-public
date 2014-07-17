/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <QObject>
#include <QUuid>

#include <medCoreExport.h>

class medDataManagerPrivate;
class medAbstractData;
class medAbstractDbController;
class medDataIndex;

class MEDCORE_EXPORT medDataManager : public QObject
{
    Q_OBJECT

public:
    static void initialize();
    static medDataManager * instance();

    medAbstractData* retrieveData(const medDataIndex& index);

    QUuid importData(medAbstractData* data, bool nonPersistent = true);
    QUuid importFile(const QString& dataPath, bool indexWithoutCopying, bool nonPersistent = true);

    void exportData(medAbstractData* data);
    void exportDataToFile(medAbstractData* data, const QString& path, const QString& format = "");

    // ------------------------- Move to controllers ? -----------------------

    QList<medDataIndex> moveStudy(const medDataIndex& indexStudy, const medDataIndex& toPatient);
    medDataIndex moveSerie(const medDataIndex& indexSerie, const medDataIndex& toStudy);

    // ------------------------- TODO -----------------------------------------

    bool makePersistent(medAbstractData* data);

    bool setMetadata(const medDataIndex& index, const QString& key, const QString& value);
    void removeData(const medDataIndex& index);

signals:
    void metadataModified(const medDataIndex& index, const QString& key, const QString& value);
    void dataImported(const medDataIndex& index, QUuid importId);
    void dataRemoved(const medDataIndex& index);

private slots:
    void exportDialog_updateSuffix(int index);
    void removeFromCache(QObject * obj);

protected:
    medDataManagerPrivate * const d_ptr;

private:
    medDataManager();
    virtual ~medDataManager();

    static medDataManager * s_instance;

    Q_DECLARE_PRIVATE(medDataManager)
};
