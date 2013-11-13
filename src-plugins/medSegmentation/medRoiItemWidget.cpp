#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
//#include <QStyle>
//#include <QStyleOption>
#include <QApplication>

#include <medRoiItemWidget.h>

medRoiItemWidget::medRoiItemWidget(QString name,PairInd indexes,QWidget * parent)
    : QWidget(parent)
{

    QHBoxLayout* layout = new QHBoxLayout(this);
    QPushButton * deleteButton = new QPushButton("delete",this);
    QIcon deleteIcon;
    // Set the off icon to the delete icon used on thumbnail (red cross)
    //deleteIcon.addPixmap(QPixmap(Path), QIcon::Normal, QIcon::On);
    //
    //
    //
    //deleteIcon.addPixmap(pix, QIcon::Normal, QIcon::Off);
    //deleteButton->setFocusPolicy(Qt::NoFocus);
    //deleteButton->setIcon(thumbnailIcon);
    //deleteButton->setIconSize(QSize(22,22));
    
    roiInfo = new QLabel(name, this);
    
    //layout->setContentsMargins(0,0,0,0);
    // add an icon showing the type of the ROI just before the roiInfo
    layout->addWidget(roiInfo);
    layout->addStretch();
    layout->addWidget(deleteButton);

    this->indexes = indexes;
    
    connect(deleteButton, SIGNAL(clicked(bool)),this,SLOT(emitDeleteWidget()));
}

medRoiItemWidget::~medRoiItemWidget()
{
}

//QSize medRoiItemWidget::sizeHint()
//{
//    QSize size = QWidget::sizeHint();
//    size.setHeight(25);
//    return size;
//}

medRoiItemWidget::PairInd medRoiItemWidget::getIndex()
{
    return indexes;
}

void medRoiItemWidget::emitDeleteWidget()
{
    emit deleteWidget(indexes);
}