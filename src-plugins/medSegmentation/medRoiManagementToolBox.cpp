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
#include <vtkPointHandleRepresentation2D.h>
#include <vtkProperty2D.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkHandleWidget.h>
#include <seedPointRoi.h>

class toolBoxObserver : public vtkCommand
{
public:
    typedef QPair<unsigned int,unsigned int> PlaneIndexSlicePair;

    static toolBoxObserver* New()
    {
        return new toolBoxObserver;
    }

    void Execute ( vtkObject *caller, unsigned long event, void *callData );

    void setView ( vtkImageView2D *view )
    {
        this->view = view;
        //view->AddObserver(vtkImageView2D::SliceChangedEvent,this);
    }

    void setToolBox ( medRoiManagementToolBox * toolBox )
    {
        this->toolBox = toolBox;
    }

    inline void lock()
    {
        this->m_lock = 1;
    }
    inline void unlock()
    {
        this->m_lock = 0;
    }

protected:
    toolBoxObserver();
    ~toolBoxObserver();

private:
    int m_lock;
    vtkImageView2D *view;
    medRoiManagementToolBox * toolBox;
};

toolBoxObserver::toolBoxObserver()
{
    this->m_lock = 0;
}

toolBoxObserver::~toolBoxObserver(){}

void toolBoxObserver::Execute ( vtkObject *caller, unsigned long event, void *callData )
{
    if ( this->m_lock )
        return;

    if (!this->view || !toolBox)
        return;

    switch ( event )
    {
    case vtkCommand::PlacePointEvent:
        {
            vtkSeedWidget * seedWidget = dynamic_cast<vtkSeedWidget*>(caller);
            seedPointRoi * seedRoi = new seedPointRoi(view,seedWidget);
            toolBox->addRoi(toolBox->getCurrentView(), seedRoi,"Seeds");
            toolBox->seedMode(true); // create new seedWidget for next point
            break;
        }
    case vtkCommand::EndInteractionEvent:
        {
            
            break;
        }
    }
}




class medRoiManagementToolBoxPrivate
{
public:
    medRoiManagementToolBoxPrivate(){};

    typedef QList<medAbstractRoi*> * ListRois;
    typedef QList<medSeriesOfRoi*> * ListOfSeriesOfRois;
    typedef QPair<unsigned int,unsigned int> PairInd;


    dtkSmartPointer<medAbstractView> currentView;
    medToolBoxTab * layoutToolBoxTab;
    
    medToolBox * toolBoxTab;
    QTreeWidget * ListAllRois;
    QTreeWidget * ListCurrentSliceRois;
    QTreeWidget * ListPolygonRois;
    QTreeWidget * ListBrushRois;
    QTreeWidget * ListPointRois;

    QList<QTreeWidget*> listOfPages;
    //QList<QList<int>*> listOfIndices;
    
    //QHash<medAbstractView*,ListRois> * viewsRoisMap;
    QHash<medAbstractView*,QList<medSeriesOfRoi*> *> * viewsRoisSeries;

    QList<PairInd> roisSelected;
    medAbstractRoi* roi;
    
    vtkSmartPointer<vtkSeedWidget> seedWidget;

    int currentPageIndex;

    toolBoxObserver * observer;
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
    d->ListAllRois = new QTreeWidget(this);
    d->ListAllRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    allLayout->addWidget(d->ListAllRois);

    QWidget * currentSliceTab = new QWidget(this);
    QVBoxLayout *currentSliceLayout = new QVBoxLayout(currentSliceTab);
    d->ListCurrentSliceRois = new QTreeWidget(this);
    d->ListCurrentSliceRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    currentSliceLayout->addWidget(d->ListCurrentSliceRois);
    
    QWidget * PolygonRoisTab = new QWidget(this);
    QVBoxLayout *polygonLayout = new QVBoxLayout(PolygonRoisTab);
    d->ListPolygonRois = new QTreeWidget(this);
    d->ListPolygonRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    polygonLayout->addWidget(d->ListPolygonRois);

    QWidget * brushRoisTab = new QWidget(this);
    QVBoxLayout *brushLayout = new QVBoxLayout(brushRoisTab);
    d->ListBrushRois = new QTreeWidget(this);
    d->ListBrushRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    brushLayout->addWidget(d->ListBrushRois);

    QWidget * pointRoisTab = new QWidget(this);
    QVBoxLayout *pointLayout = new QVBoxLayout(pointRoisTab);
    d->ListPointRois = new QTreeWidget(this);
    d->ListPointRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    pointLayout->addWidget(d->ListPointRois);

    d->layoutToolBoxTab->addTab(allRoisTab, tr("All"));
    d->layoutToolBoxTab->addTab(currentSliceTab, tr("Current Slice"));
    d->layoutToolBoxTab->addTab(PolygonRoisTab, tr("Bezier"));
    d->layoutToolBoxTab->addTab(brushRoisTab, tr("Brush"));

    d->listOfPages.append(d->ListAllRois);
    d->listOfPages.append(d->ListCurrentSliceRois);
    d->listOfPages.append(d->ListPolygonRois);
    d->listOfPages.append(d->ListBrushRois);
    d->listOfPages.append(d->ListPointRois);
    d->currentPageIndex = 0;
    saveCurrentPageIndex(0);

    d->viewsRoisSeries = new QHash<medAbstractView*,QList<medSeriesOfRoi*> *>();

    QPushButton *seedButton = new QPushButton("Select a point");
    seedButton->setCheckable(true);
    this->addWidget(d->toolBoxTab);
    this->addWidget(seedButton);
    connect(seedButton, SIGNAL(toggled(bool)), this, SLOT(seedMode(bool)));
    
    connect(d->layoutToolBoxTab,SIGNAL(currentChanged(int)),this,SLOT(saveCurrentPageIndex(int)));

    d->observer = toolBoxObserver::New();
    d->observer->setToolBox(this);
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
    
    d->observer->setView(static_cast<vtkImageView2D *>(d->currentView->getView2D()));

    connect(d->currentView,SIGNAL(sliceChanged(int,bool)),this,SLOT(updateDisplay()),Qt::UniqueConnection);
    updateDisplay();
}

void medRoiManagementToolBox::addRoi(medAbstractView * view, medAbstractRoi * roi, QString seriesName)
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


void medRoiManagementToolBox::updateDisplay()
{
    clearDisplay();
    if (!d->currentView)
        return;
    
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(d->currentView->getView2D());
    
    unsigned int currentSlice = view2d->GetSlice();
    unsigned char currentOrientation = view2d->GetViewOrientation();

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
                /*d->ListAllRois->insertTopLevelItem(i,item);*/
                d->ListAllRois->setItemWidget(item,0,widget);

                if (list->at(i)->getIdSlice()==currentSlice && list->at(i)->getOrientation()==currentOrientation)
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem(d->ListCurrentSliceRois);
                    medRoiItemWidget * widget = new medRoiItemWidget(QString::number(i+1) +  " - "  + list->at(i)->type() + " - " + list->at(i)->info(),PairInd(k,i));
                    connect(widget,SIGNAL(deleteWidget(PairInd)),this,SLOT(deleteRoi(PairInd)));
                    item->setSizeHint(0,widget->sizeHint());
                    d->ListCurrentSliceRois->insertTopLevelItem(i,item);
                    d->ListCurrentSliceRois->setItemWidget(item,0,widget);
                }
                if (list->at(i)->type()=="Polygon")
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem(d->ListPolygonRois);
                    medRoiItemWidget * widget = new medRoiItemWidget(QString::number(i+1) +  " - "  + list->at(i)->type() + " - " + list->at(i)->info(),PairInd(k,i));
                    connect(widget,SIGNAL(deleteWidget(PairInd)),this,SLOT(deleteRoi(PairInd)));
                    item->setSizeHint(0,widget->sizeHint());
                    d->ListPolygonRois->insertTopLevelItem(i,item);
                    d->ListPolygonRois->setItemWidget(item,0,widget);
                }

                if (list->at(i)->type()=="SeedPoint")
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem(d->ListPointRois);
                    medRoiItemWidget * widget = new medRoiItemWidget(QString::number(i+1) +  " - "  + list->at(i)->type() + " - " + list->at(i)->info(),PairInd(k,i));
                    connect(widget,SIGNAL(deleteWidget(PairInd)),this,SLOT(deleteRoi(PairInd)));
                    item->setSizeHint(0,widget->sizeHint());
                    d->ListPointRois->insertTopLevelItem(i,item);
                    d->ListPointRois->setItemWidget(item,0,widget);
                }
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

QHash<medAbstractView*,medRoiManagementToolBox::ListOfSeriesOfRois> * medRoiManagementToolBox::getSeriesOfRoi()
{
    return d->viewsRoisSeries;
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

void medRoiManagementToolBox::unselectRois()
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

void medRoiManagementToolBox::deleteRoi(PairInd indexes)
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

void medRoiManagementToolBox::seedMode(bool checked) // TODO if currentView changes need to change the renderer of the representation and interactor ...
{ // TODO should be activated via a toggle button if on seedwidget ON else seedWidget Off;
    if(!d->currentView)
        return;

    if (d->seedWidget)
        d->seedWidget->CompleteInteraction();

    d->seedWidget = NULL;

    if (!checked)
        return;
    
    vtkImageView2D * view2d = static_cast<vtkImageView2D *>(d->currentView->getView2D());

    // Create the representation
    vtkSmartPointer<vtkPointHandleRepresentation2D> handle = vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
    handle->GetProperty()->SetColor(1,0,0);
    vtkSmartPointer<vtkSeedRepresentation> rep = vtkSmartPointer<vtkSeedRepresentation>::New();
    rep->SetHandleRepresentation(handle);
    rep->SetRenderer(view2d->GetRenderer());

    // Seed widget
    d->seedWidget = vtkSmartPointer<vtkSeedWidget>::New();
    d->seedWidget->SetInteractor(view2d->GetInteractor());
    d->seedWidget->SetRepresentation(rep);

    d->seedWidget->AddObserver(vtkCommand::PlacePointEvent,d->observer); 
    d->seedWidget->On();
}

medAbstractView * medRoiManagementToolBox::getCurrentView()
{
    return d->currentView;
}

QList<medRoiManagementToolBox::PairInd> medRoiManagementToolBox::getSelectedRois()
{
    return d->roisSelected;
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