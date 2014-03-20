/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medRoiCreatorToolBox.h"

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
#include <medRoiToolBox.h>
#include <medToolBoxTab.h>
#include <medViewManager.h>
#include <medWorkspace.h>
#include <medAbstractRoi.h>
#include <QtGui>
#include <medRoiItemWidget.h>
#include <medToolBoxFactory.h>
#include <medToolBoxBody.h>
#include <medDataManager.h>

class medRoiCreatorToolBoxPrivate
{
public:
    medRoiCreatorToolBoxPrivate(){};

    typedef QList<medAbstractRoi*> * ListRois;
    typedef QList<medSeriesOfRoi*> * ListOfSeriesOfRois;
    typedef QPair<unsigned int,unsigned int> PairInd;

    dtkSmartPointer<medAbstractView> currentView;
    medToolBoxTab * layoutToolBoxTab;
    
    QTreeWidget * ListAllRois;
    //QTreeWidget * ListCurrentSliceRois;
    
    QList<QTreeWidget*> listOfPages;
    //QList<QList<int>*> listOfIndices;
    
    //QHash<medAbstractView*,ListRois> * viewsRoisMap;
    QHash<medAbstractView*,QList<medSeriesOfRoi*> *> * viewsRoisSeries;

    QList<PairInd> roisSelected;
    medAbstractRoi* roi;
    
    int currentPageIndex;
    QList<medRoiToolBox*> roiToolsTB;

    QTreeWidgetItem * contextMenuItem;
};

medRoiCreatorToolBox::medRoiCreatorToolBox(QWidget *parent) : medToolBox(parent), d(new medRoiCreatorToolBoxPrivate)
{
    this->setTitle("Roi Management");
    
    d->currentView = NULL;

    QWidget * displayWidget = new QWidget();
    d->layoutToolBoxTab = new medToolBoxTab(this);

    QWidget * allRoisTab = new QWidget(this);
    QVBoxLayout *allLayout = new QVBoxLayout(allRoisTab);
    d->ListAllRois = new QTreeWidget(this);
    d->ListAllRois->setColumnCount(5);
    d->ListAllRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QStringList headers;
    headers << tr("Type") << tr("Name") << tr("Area") << tr("Volume") << tr("");
    d->ListAllRois->setHeaderLabels(headers);
    allLayout->addWidget(d->ListAllRois);
    d->ListAllRois->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(d->ListAllRois, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(onContextTreeMenu(QPoint)));

    /*QWidget * currentSliceTab = new QWidget(this);
    QVBoxLayout *currentSliceLayout = new QVBoxLayout(currentSliceTab);
    d->ListCurrentSliceRois = new QTreeWidget(this);
    d->ListCurrentSliceRois->setColumnCount(5);
    d->ListCurrentSliceRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->ListCurrentSliceRois->setHeaderLabels(headers);
    currentSliceLayout->addWidget(d->ListCurrentSliceRois);
*/
    d->layoutToolBoxTab->addTab(allRoisTab, tr("All"));
    //d->layoutToolBoxTab->addTab(currentSliceTab, tr("Current Slice"));

    d->listOfPages.append(d->ListAllRois);
    //d->listOfPages.append(d->ListCurrentSliceRois);
    d->currentPageIndex = 0;
    saveCurrentPageIndex(0);

    d->viewsRoisSeries = new QHash<medAbstractView*,QList<medSeriesOfRoi*> *>();

    QVBoxLayout * displayLayout = new QVBoxLayout();
    this->addWidget(d->layoutToolBoxTab);
    displayWidget->setLayout(displayLayout);
    this->addWidget(displayWidget);

    connect(d->layoutToolBoxTab,SIGNAL(currentChanged(int)),this,SLOT(saveCurrentPageIndex(int)));

    QGridLayout * toolsLayout = new QGridLayout;
    
    foreach(QString toolbox_name,medToolBoxFactory::instance()->toolBoxesFromCategory("RoiTools"))
    { 
        d->roiToolsTB.append(qobject_cast<medRoiToolBox*>(medToolBoxFactory::instance()->createToolBox(toolbox_name)));
        toolsLayout->addWidget(d->roiToolsTB[d->roiToolsTB.size()-1]->body());
        connect(d->roiToolsTB[d->roiToolsTB.size()-1],SIGNAL(roiCreated(medAbstractView*,medAbstractRoi*,QString)),this,SLOT(addRoi(medAbstractView*,medAbstractRoi*,QString)));
    }

    displayLayout->addLayout(toolsLayout);

    d->contextMenuItem=0;
}

medRoiCreatorToolBox::~medRoiCreatorToolBox(void)
{
    delete d;

    d = NULL;
}

void medRoiCreatorToolBox::update( dtkAbstractView *view )
{
    if (!view)
        return;
    medToolBox::update(view);
    unselectRois(); // remove all selected rois of previous view
    d->currentView = dynamic_cast<medAbstractView*>(view);
    // TODO : update all the tabs for this current view

    //d->observer->setView(static_cast<medVtkViewBackend*>(d->currentView->backend())->view2D);

    foreach(medToolBox *tb, d->roiToolsTB)
        tb->update(view);

    connect(d->currentView,SIGNAL(sliceChanged(int,bool)),this,SLOT(updateDisplay()),Qt::UniqueConnection);
    updateDisplay();
}

void medRoiCreatorToolBox::addRoi(medAbstractView * view, medAbstractRoi * roi, QString seriesName)
{
    bool added = false;
    if (!d->viewsRoisSeries->contains(view))
    {
        QList<medSeriesOfRoi*> * series = new QList<medSeriesOfRoi*>();
        d->viewsRoisSeries->insert(view,series);
    }
    else
    {
        QList<medSeriesOfRoi*> *series = d->viewsRoisSeries->value(view);
        
        for(int i=0;i<series->size();i++)
        {
            if (series->at(i)->getName()==seriesName)
            {
                added = true;
                series->at(i)->getIndices()->append(roi);
            }
        }
    }
    if (!added)
    {
        QList<medAbstractRoi*> * listRois = new QList<medAbstractRoi*>();
        listRois->append(roi);
        d->viewsRoisSeries->value(view)->append(new medSeriesOfRoi(seriesName,listRois,this));        
    }
    updateDisplay();
}


void medRoiCreatorToolBox::updateDisplay()
{
    clearDisplay();
    if (!d->currentView)
        return;
    
    if (d->viewsRoisSeries->contains(d->currentView))
    {
        ListOfSeriesOfRois listOfSeries = d->viewsRoisSeries->value(d->currentView);
        for(unsigned int k=0;k<listOfSeries->size();k++)
        {
            ListRois list = d->viewsRoisSeries->value(d->currentView)->at(k)->getIndices();
            
            QTreeWidgetItem * serieItem = new QTreeWidgetItem(d->ListAllRois);
            medRoiItemWidget * widget = new medRoiItemWidget(d->viewsRoisSeries->value(d->currentView)->at(k)->getName(),PairInd(k,-1));
            connect(widget,SIGNAL(deleteWidget(PairInd)),this,SLOT(deleteRoi(PairInd)));
            serieItem->setSizeHint(0,widget->sizeHint());
            d->ListAllRois->insertTopLevelItem(k,serieItem);
            d->ListAllRois->setItemWidget(serieItem,0,widget);
            
            for(unsigned int i=0;i<list->size();i++)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(serieItem);
                medRoiItemWidget * widget = new medRoiItemWidget(QString::number(i+1) +  " - "  + list->at(i)->type() + " - " + list->at(i)->info(),PairInd(k,i));
                connect(widget,SIGNAL(deleteWidget(PairInd)),this,SLOT(deleteRoi(PairInd)));
                item->setSizeHint(0,widget->sizeHint());
                d->ListAllRois->setItemWidget(item,0,widget);

            }
        }
    }
}

void medRoiCreatorToolBox::clearDisplay()
{
    d->ListAllRois->clear();
    //d->ListCurrentSliceRois->clear();
}

QHash<medAbstractView*,medRoiCreatorToolBox::ListOfSeriesOfRois> * medRoiCreatorToolBox::getSeriesOfRoi()
{
    return d->viewsRoisSeries;
}

void medRoiCreatorToolBox::saveCurrentPageIndex(int index)
{
    if (d->listOfPages.size() > d->currentPageIndex)
        disconnect(d->listOfPages.at(d->currentPageIndex),SIGNAL(itemSelectionChanged()),this,SLOT(SelectRois()));
    d->currentPageIndex = index;
    connect(d->listOfPages.at(index),SIGNAL(itemSelectionChanged()),this,SLOT(selectRois()));
}

void medRoiCreatorToolBox::selectRois()
{
    if (!d->currentView)
        return;

    unselectRois();
    QList<QTreeWidgetItem*> treeWidgetItem = d->listOfPages.at(d->currentPageIndex)->selectedItems();
    QTreeWidget* treeWidget = d->listOfPages.at(d->currentPageIndex);
    for(int i=0;i<treeWidgetItem.size();i++)
    {
        //int ind = indices->at(d->listOfPages.at(d->currentPageIndex)->row(list.at(i))-1);
        medRoiItemWidget * itemWidget = dynamic_cast<medRoiItemWidget*>(treeWidget->itemWidget(treeWidgetItem.at(i),0));
        PairInd indexes = itemWidget->getIndex();

        if (d->viewsRoisSeries->value(d->currentView)->size()>indexes.first) // check we never know 
        {
            if (indexes.second==-1)
                d->viewsRoisSeries->value(d->currentView)->at(indexes.first)->select();
            else if (d->viewsRoisSeries->value(d->currentView)->at(indexes.first)->getIndices()->size()>indexes.second)
                d->viewsRoisSeries->value(d->currentView)->at(indexes.first)->getIndices()->at(indexes.second)->select();
        }
        else
            continue;

        d->roisSelected.append(indexes);
    }
    d->currentView->update();
}

void medRoiCreatorToolBox::unselectRois()
{
    if (!d->currentView)
        return;

    for(int i=0;i<d->roisSelected.size();i++) 
        if (d->viewsRoisSeries->value(d->currentView)->size()>d->roisSelected.at(i).first) // check we never know
        {
            if (d->roisSelected.at(i).second==-1)
                d->viewsRoisSeries->value(d->currentView)->at(d->roisSelected.at(i).first)->unselect();
            else if (d->viewsRoisSeries->value(d->currentView)->at(d->roisSelected.at(i).first)->getIndices()->size()>d->roisSelected.at(i).second)
                d->viewsRoisSeries->value(d->currentView)->at(d->roisSelected.at(i).first)->getIndices()->at(d->roisSelected.at(i).second)->unselect();
        }
        else 
            continue;
        
    //d->currentView->update();
    d->roisSelected.clear();
}

void medRoiCreatorToolBox::deleteRoi(PairInd indexes)
{
    if (!d->currentView)
        return;
    unselectRois();

    if (indexes.second==-1) // get rid of the serie.
    {
        for (int i=0;i<d->viewsRoisSeries->value(d->currentView)->at(indexes.first)->getIndices()->size();i++)
            delete d->viewsRoisSeries->value(d->currentView)->at(indexes.first)->getIndices()->at(i);
        delete d->viewsRoisSeries->value(d->currentView)->takeAt(indexes.first);
    }
    else if ((d->viewsRoisSeries->value(d->currentView)->size()>indexes.first)
        && (d->viewsRoisSeries->value(d->currentView)->at(indexes.first)->getIndices()->size()>indexes.second))
            delete d->viewsRoisSeries->value(d->currentView)->at(indexes.first)->getIndices()->takeAt(indexes.second); // TODO : make sure that every vtkObject is deleted not sure how
    
    d->currentView->update();
    updateDisplay();
}

//QList<medAbstractRoi*> medRoiCreatorToolBox::getSelectedRois()
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

medAbstractView * medRoiCreatorToolBox::getCurrentView()
{
    return d->currentView;
}

QList<medRoiCreatorToolBox::PairInd> medRoiCreatorToolBox::getSelectedRois()
{
    return d->roisSelected;
}

void medRoiCreatorToolBox::applyRoiToImage()
{
    //roiItem item selected
    medRoiItemWidget * roiItem = qobject_cast<medRoiItemWidget*>(d->ListAllRois->itemWidget(d->contextMenuItem,0));
    medRoiItemWidget::PairInd pair = roiItem->getIndex();
    ListRois list = d->viewsRoisSeries->value(d->currentView)->at(pair.first)->getIndices();

    dtkAbstractData * data;
    foreach(medRoiToolBox * roitb,d->roiToolsTB)
    {
        if (roitb->roi_description()==list->at(0)->type())
        {
            data = roitb->convertToBinaryImage(list);
            break;
        }
    }
    dtkAbstractProcess * maskApplicationProcess = dtkAbstractProcessFactory::instance()->create("medMaskApplication");
    if (!maskApplicationProcess)
        return; // TODO : add message error like the plugin is not here or something like that
    int layer = 0; // TODO : add also a way to choose the layer on which u want to apply the roi

    maskApplicationProcess->setInput(data,0);
    maskApplicationProcess->setInput(static_cast<dtkAbstractData*>(d->currentView->data()),layer);
    maskApplicationProcess->update();

    d->currentView->removeOverlay(layer);
    d->currentView->setData(maskApplicationProcess->output(),layer);
}

void medRoiCreatorToolBox::onInterpolate() 
{
    //roiItem item selected
    medRoiItemWidget * roiItem = qobject_cast<medRoiItemWidget*>(d->ListAllRois->itemWidget(d->contextMenuItem,0));
    medRoiItemWidget::PairInd pair = roiItem->getIndex();
    ListRois list = d->viewsRoisSeries->value(d->currentView)->at(pair.first)->getIndices();

    foreach(medRoiToolBox * roitb,d->roiToolsTB)
    {
        if (roitb->roi_description()==list->at(0)->type())
        {
            roitb->interpolateRois(list);
            break;
        }
    }
}

void medRoiCreatorToolBox::onGenerateBinaryImage() 
{
    //roiItem item selected
    medRoiItemWidget * roiItem = qobject_cast<medRoiItemWidget*>(d->ListAllRois->itemWidget(d->contextMenuItem,0));
    medRoiItemWidget::PairInd pair = roiItem->getIndex();
    ListRois list = d->viewsRoisSeries->value(d->currentView)->at(pair.first)->getIndices();

    foreach(medRoiToolBox * roitb,d->roiToolsTB)
    {
        if (roitb->roi_description()==list->at(0)->type())
        {
            dtkAbstractData * outputData = roitb->convertToBinaryImage(list);
            dtkAbstractData *inputData = reinterpret_cast<dtkAbstractData*>(d->currentView->data());
            setOutputMetadata(inputData,outputData);    
            medDataManager::instance()->importNonPersistent(outputData);
            break;
        }
    }
}

void medRoiCreatorToolBox::onContextTreeMenu( const QPoint point )
{

    QTreeWidgetItem * item = 0;
    item = d->ListAllRois->itemAt(point);

    if (!item)
        return;

    d->contextMenuItem = item;

    item->setSelected(true);

    QMenu * menu = new QMenu(d->ListAllRois);
    menu->setFocusPolicy(Qt::NoFocus);
    QAction * interpolate = new QAction(this);
    interpolate->setText(tr("Interpolate"));
    menu->addAction(interpolate);
    connect(interpolate, SIGNAL(triggered()), this, SLOT(onInterpolate()));
    
    QAction * generateBinaryImage = new QAction(this);
    generateBinaryImage->setText(tr("Generate Binary Image"));
    menu->addAction(generateBinaryImage);
    connect(generateBinaryImage,SIGNAL(triggered()),this,SLOT(onGenerateBinaryImage()));

    dtkAbstractProcess * maskApplicationProcess = dtkAbstractProcessFactory::instance()->create("medMaskApplication");
    if (maskApplicationProcess)
    {
        QAction * applyRoiToImage_action = new QAction(this);
        applyRoiToImage_action->setText(tr("Apply Roi To Image"));
        menu->addAction(applyRoiToImage_action);
        connect(applyRoiToImage_action,SIGNAL(triggered()),this,SLOT(applyRoiToImage()));
    }

    menu->exec(d->ListAllRois->mapToGlobal(point));
    delete menu;
}
    
void medRoiCreatorToolBox::setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData)
{
    Q_ASSERT(outputData && inputData);

    QStringList metaDataToCopy;
    metaDataToCopy 
        << medMetaDataKeys::PatientName.key()
        << medMetaDataKeys::StudyDescription.key();

    foreach( const QString & key, metaDataToCopy ) {
        outputData->setMetaData(key, inputData->metadatas(key));
    }

    QString seriesDesc;
    seriesDesc = tr("Segmented from ") + medMetaDataKeys::SeriesDescription.getFirstValue( inputData );
    medMetaDataKeys::SeriesDescription.set(outputData,seriesDesc);
}



/*******************************MEDSERIESOFROI*///////////////// MOVE THIS CLASS IN A FILE

void medSeriesOfRoi::Off()
{
    for(int i=0;i<rois->size();i++)
        rois->at(i)->Off();
}
    
void medSeriesOfRoi::On()
{
    for(int i=0;i<rois->size();i++)
        rois->at(i)->On();
}

QString medSeriesOfRoi::info()
{
    return QString();
}

void medSeriesOfRoi::select()
{
    for(int i=0;i<rois->size();i++)
        rois->at(i)->select();
}
void medSeriesOfRoi::unselect()
{
    for(int i=0;i<rois->size();i++)
        rois->at(i)->unselect();
}

void medSeriesOfRoi::computeStatistics()
{
    // TODO
}