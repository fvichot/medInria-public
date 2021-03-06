/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <QtGui>
#include <QUuid>

#include <medGuiExport.h>

class medAbstractView;
class medDataIndex;
class medViewContainer;
class medViewContainerSplitter;
class medTabbedViewContainersPrivate;

/**
 * @brief A QStackedWidget that contains medViewContainers.
 * There is one such stack per medViewWorkspace.
 *
*/
class MEDGUI_EXPORT medTabbedViewContainers : public QTabWidget
{
    Q_OBJECT

public:
     medTabbedViewContainers(QWidget *parent = 0);
    ~medTabbedViewContainers();

    void lockTabs();
    void unlockTabs();
    void hideTabBar();
    QList<QUuid> containersSelected();
    void adjustContainersSize();
    QList<medAbstractView*> viewsInTab(int index = 0);
    QList<medViewContainer*> containersInTab(int index = 0);


public slots:
    medViewContainer* addContainerInTab();
    medViewContainer* addContainerInTab(const QString &name);
    medViewContainer* insertContainerInTab(int index, const QString &name);
    void closeCurrentTab();
    void closeTab(int index);

protected:
    // Not sure of the name - RDE
    void resetTabState();

private slots:
    void disconnectTabFromSplitter(int index);
    void repopulateCurrentTab();
    void addContainerToSelection(QUuid container);
    void removeContainerFromSelection(QUuid container);
    void connectContainer(QUuid container);
    void buildTemporaryPool();
    void connectContainerSelectedForCurrentTab();
    void minimizeOtherContainers(QUuid containerMaximized, bool maximized);
    void minimizeSplitterContainers(QUuid containerMaximized, bool maximized,
                                                             medViewContainerSplitter *splitter);

signals:
    void containersSelectedChanged();

private:
    medTabbedViewContainersPrivate *d;
};


