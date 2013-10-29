/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medRoiManagementToolBox.h"

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <dtkCore/dtkAbstractViewInteractor.h>
#include <dtkLog/dtkLog.h>

#include <medAbstractView.h>
#include <medMessageController.h>
#include <medMetaDataKeys.h>
#include <medToolBoxFactory.h>
#include <medToolBoxTab.h>
#include <medViewManager.h>
#include <medWorkspace.h>
#include <medToolBoxTab.h>
#include <medAbstractRoi.h>
#include <QtGui>
#include <vtkImageView2D.h>
#include <medRoiItemWidget.h>

class medRoiManagementToolBoxPrivate
{
public:
    medRoiManagementToolBoxPrivate(){};

    typedef QList<medAbstractRoi*> * ListRois;

    dtkSmartPointer<medAbstractView> currentView;
    medToolBoxTab * layoutToolBoxTab;
    
    medToolBox * toolBoxTab;
    QListWidget * ListAllRois;
    QListWidget * ListCurrentSliceRois;
    QListWidget * ListPolygonRois;
    QListWidget * ListBrushRois;

    QList<QListWidget*> listOfPages;
    //QList<QList<int>*> listOfIndices;
    
    QHash<medAbstractView*,ListRois> * viewsRoisMap;
    QList<unsigned int> roisSelected;

    int currentPageIndex;
};

medRoiManagementToolBox::medRoiManagementToolBox(QWidget *parent) : medToolBox(parent), d(new medRoiManagementToolBoxPrivate)
{
    this->setTitle("Roi Management");
    
    d->currentView = NULL;
    
    d->layoutToolBoxTab = new medToolBoxTab(this);
    d->toolBoxTab = new medToolBox(this);
    d->toolBoxTab->setTitle("Regions of interest");
    d->toolBoxTab->setTabWidget(d->layoutToolBoxTab);
    
    QWidget * allRoisTab = new QWidget(this);
    QVBoxLayout *allLayout = new QVBoxLayout(allRoisTab);
    d->ListAllRois = new QListWidget(this);
    d->ListAllRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    allLayout->addWidget(d->ListAllRois);

    QWidget * currentSliceTab = new QWidget(this);
    QVBoxLayout *currentSliceLayout = new QVBoxLayout(currentSliceTab);
    d->ListCurrentSliceRois = new QListWidget(this);
    d->ListCurrentSliceRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    currentSliceLayout->addWidget(d->ListCurrentSliceRois);
    
    QWidget * PolygonRoisTab = new QWidget(this);
    QVBoxLayout *polygonLayout = new QVBoxLayout(PolygonRoisTab);
    d->ListPolygonRois = new QListWidget(this);
    d->ListPolygonRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    polygonLayout->addWidget(d->ListPolygonRois);

    QWidget * brushRoisTab = new QWidget(this);
    QVBoxLayout *brushLayout = new QVBoxLayout(brushRoisTab);
    d->ListBrushRois = new QListWidget(this);
    d->ListBrushRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    brushLayout->addWidget(d->ListBrushRois);

    d->layoutToolBoxTab->addTab(allRoisTab, tr("All"));
    d->layoutToolBoxTab->addTab(currentSliceTab, tr("Current Slice"));
    d->layoutToolBoxTab->addTab(PolygonRoisTab, tr("Bezier"));
    d->layoutToolBoxTab->addTab(brushRoisTab, tr("Brush"));

    d->listOfPages.append(d->ListAllRois);
    d->listOfPages.append(d->ListCurrentSliceRois);
    d->listOfPages.append(d->ListPolygonRois);
    d->listOfPages.append(d->ListBrushRois);
    d->currentPageIndex = 0;
    saveCurrentPageIndex(0);

    d->viewsRoisMap = new QHash<medAbstractView*,ListRois>();

    this->addWidget(d->toolBoxTab);

    connect(d->layoutToolBoxTab,SIGNAL(currentChanged(int)),this,SLOT(saveCurrentPageIndex(int)));
}

medRoiManagementToolBox::~medRoiManagementToolBox(void)
{
    delete d;

    d = NULL;
}

void medRoiManagementToolBox::update( dtkAbstractView *view )
{
    if (!view)
        return;
    medToolBox::update(view);
    unselectRois(); // remove all selected rois of previous view
    d->currentView = dynamic_cast<medAbstractView*>(view);
    // TODO : update all the tabs for this current view
    
    connect(d->currentView,SIGNAL(sliceChanged(int,bool)),this,SLOT(updateDisplay()),Qt::UniqueConnection);
    updateDisplay();
    
}

void medRoiManagementToolBox::addRoi(medAbstractView * view, medAbstractRoi * roi)
{
    if (!d->viewsRoisMap->contains(view))
    {
        QList<medAbstractRoi*> * listRois = new QList<medAbstractRoi*>();
        d->viewsRoisMap->insert(view,listRois);
    }
    d->viewsRoisMap->value(view)->append(roi);

    updateDisplay();
}


void medRoiManagementToolBox::updateDisplay()
{
    clearDisplay();
    if (!d->currentView)
        return;
    
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(d->currentView->getView2D());
    
    unsigned int currentSlice = view2d->GetSlice();
    unsigned char currentOrientation = view2d->GetViewOrientation();

    if (d->viewsRoisMap->contains(d->currentView))
    {
        ListRois list = d->viewsRoisMap->value(d->currentView);
        for(unsigned int i=0;i<list->size();i++)
        {
            QListWidgetItem *item = new QListWidgetItem(d->ListAllRois);
            medRoiItemWidget * widget = new medRoiItemWidget(QString::number(i+1) +  " - "  + list->at(i)->type() + " - " + list->at(i)->info(),i);
            connect(widget,SIGNAL(deleteWidget(unsigned int)),this,SLOT(deleteRoi(unsigned int)));
            item->setSizeHint(widget->sizeHint());
            d->ListAllRois->addItem(item);
            d->ListAllRois->setItemWidget(item,widget);
                        
            if (list->at(i)->getIdSlice()==currentSlice && list->at(i)->getOrientation()==currentOrientation)
            {
                QListWidgetItem *item = new QListWidgetItem(d->ListCurrentSliceRois);
                medRoiItemWidget * widget = new medRoiItemWidget(QString::number(i+1) +  " - "  + list->at(i)->type() + " - " + list->at(i)->info(),i);
                connect(widget,SIGNAL(deleteWidget(unsigned int)),this,SLOT(deleteRoi(unsigned int)));
                item->setSizeHint(widget->sizeHint());
                d->ListCurrentSliceRois->addItem(item);
                d->ListCurrentSliceRois->setItemWidget(item,widget);
            }
            if (list->at(i)->type()=="Polygon")
            {
                QListWidgetItem *item = new QListWidgetItem(d->ListPolygonRois);
                medRoiItemWidget * widget = new medRoiItemWidget(QString::number(i+1) +  " - "  + list->at(i)->type() + " - " + list->at(i)->info(),i);
                connect(widget,SIGNAL(deleteWidget(unsigned int)),this,SLOT(deleteRoi(unsigned int)));
                item->setSizeHint(widget->sizeHint());
                d->ListPolygonRois->addItem(item);
                d->ListPolygonRois->setItemWidget(item,widget);
            }
        }
    }
}

void medRoiManagementToolBox::clearDisplay()
{
    d->ListAllRois->clear();
    d->ListCurrentSliceRois->clear();
    d->ListPolygonRois->clear();
    d->ListBrushRois->clear();
}

QHash<medAbstractView*,medRoiManagementToolBox::ListRois> * medRoiManagementToolBox::getRois()
{
    return d->viewsRoisMap;
}

void medRoiManagementToolBox::saveCurrentPageIndex(int index)
{
    if (d->listOfPages.size() > d->currentPageIndex)
        disconnect(d->listOfPages.at(d->currentPageIndex),SIGNAL(itemSelectionChanged()),this,SLOT(SelectRois()));
    d->currentPageIndex = index;
    connect(d->listOfPages.at(index),SIGNAL(itemSelectionChanged()),this,SLOT(selectRois()));
}

void medRoiManagementToolBox::selectRois()
{
    if (!d->currentView)
        return;

    unselectRois();
    QList<QListWidgetItem*> listWidgetItem = d->listOfPages.at(d->currentPageIndex)->selectedItems();
    QListWidget* listWidget = d->listOfPages.at(d->currentPageIndex);
    for(int i=0;i<listWidgetItem.size();i++)
    {
        //int ind = indices->at(d->listOfPages.at(d->currentPageIndex)->row(list.at(i))-1);
        medRoiItemWidget * itemWidget = dynamic_cast<medRoiItemWidget*>(listWidget->itemWidget(listWidgetItem.at(i)));
        unsigned int index = itemWidget->getIndex();

        if (d->viewsRoisMap->value(d->currentView)->size()>index) // check we never know
            d->viewsRoisMap->value(d->currentView)->at(index)->select();
        else 
            continue;

        d->roisSelected.append(index);
    }
    d->currentView->update();
}

void medRoiManagementToolBox::unselectRois()
{
    if (!d->currentView)
        return;

    for(int i=0;i<d->roisSelected.size();i++)
        if (d->viewsRoisMap->value(d->currentView)->size()>d->roisSelected.at(i)) // check we never know
            d->viewsRoisMap->value(d->currentView)->at(d->roisSelected.at(i))->unselect();
        else 
            continue;
        
    d->currentView->update();
    d->roisSelected.clear();
}

void medRoiManagementToolBox::deleteRoi(unsigned int index)
{
    if (!d->currentView)
        return;
    unselectRois();
    delete d->viewsRoisMap->value(d->currentView)->takeAt(index); // TODO : make sure that every vtkObject is deleted not sure how
    d->currentView->update();
    updateDisplay();
}

//QList<medAbstractRoi*> medRoiManagementToolBox::getSelectedRois()
//{
//    if (!d->currentView)
//        return;
//    ListRois list = d->viewsRoisMap->value(d->currentView);
//    QList<medAbstractRoi*> listOfSelectedRois;
//    for(int i=0;i<d->roisSelected.size();i++)
//    {
//        listOfSelectedRois.append(list[d->roisSelected[i]]);
//    }
//    return listOfSelectedRois;
//}