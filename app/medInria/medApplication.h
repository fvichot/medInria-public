/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <QtSingleApplication>

class medApplicationPrivate;
class medMainWindow;

class medApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    medApplication(int & argc, char **argv);
    ~medApplication();
    
    bool event(QEvent *event);
    void setMainWindow(medMainWindow *mw);

signals:
    void showMessage(const QString& message);

public slots:
    void redirectMessageToSplash(const QString& message);
    void redirectMessageToLog(const QString & message);
    void redirectErrorMessageToLog(const QString & message);

protected:
    void initialize();

private:
    medApplicationPrivate *d;
};


