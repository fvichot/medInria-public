#pragma once

#include <QObject>

#include <medDataIndex.h>

class medJobTracker : public QObject {

    Q_OBJECT
public:

    bool isReady();

    medDataIndex index();

    bool waitForDataReady();
    QString error();

signals:
    void dataReady();
    void progressed(int percent);
    void error();

private:
    friend class medDataManager;

    medDataIndex index;
};
