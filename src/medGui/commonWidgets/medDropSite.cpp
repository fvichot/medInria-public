/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <dtkCore/dtkGlobal.h>

#include <medDataIndex.h>

#include "medDropSite.h"

#include <QtGui>

class medDropSitePrivate
{
public:
    QStringList acceptedTypes;
    medDataIndex index;
    bool canAutomaticallyChangeAppereance;
};

medDropSite::medDropSite(QWidget *parent) : QLabel(parent), d(new medDropSitePrivate)
{
    setAlignment(Qt::AlignCenter);
    setAcceptDrops(true);
    setBackgroundRole(QPalette::Base);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setText("Either drop an image from the database, or select a file by clicking here");
    setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    setWordWrap(true);
//    font().setPointSize(9);
    d->index = medDataIndex();
    d->canAutomaticallyChangeAppereance = true;
}

medDropSite::~medDropSite(void)
{
    delete d;

    d = NULL;
}

QSize medDropSite::sizeHint(void) const
{
    return QSize(128, 128);
}

void medDropSite::setCanAutomaticallyChangeAppereance(bool can)
{
    d->canAutomaticallyChangeAppereance = can;
}

void medDropSite::setAcceptedTypes(const QStringList & types)
{
    d->acceptedTypes = types;
}

QStringList medDropSite::acceptedTypes() const
{
    return d->acceptedTypes;
}

medDataIndex medDropSite::index(void) const
{
    return d->index;
}

void medDropSite::dragEnterEvent(QDragEnterEvent *event)
{
    setBackgroundRole(QPalette::Highlight);

    QString type;
    medDataIndex index( medDataIndex::readMimeData(event->mimeData()) );
    if (index.isValid()) {
//        type = medDataManaindex;
    }

    if (acceptedTypes().isEmpty() || acceptedTypes().contains(type))
        event->acceptProposedAction();
}

void medDropSite::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void medDropSite::dragLeaveEvent(QDragLeaveEvent *event)
{
    setBackgroundRole(QPalette::Base);

    event->accept();
}

void medDropSite::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (d->canAutomaticallyChangeAppereance && mimeData->hasImage()) {
        setPixmap(qvariant_cast<QPixmap>(mimeData->imageData()));
    }

    medDataIndex index( medDataIndex::readMimeData(mimeData) );
    if (index.isValid()) {
        d->index = index;
    }

    setBackgroundRole(QPalette::Base);

    event->acceptProposedAction();
    
    emit objectDropped(d->index);
}

void medDropSite::clear(){
    QLabel::clear();
    d->index = medDataIndex();
}

void medDropSite::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

//    // Optionally draw something (e.g. a tag) over the label in case it is a pixmap

//    if(!this->pixmap())
//        return;
//
//    QPainter painter;
//    painter.begin(this);
//    painter.setPen(Qt::white);
//    painter.drawText(event->rect(), "Overlay", QTextOption(Qt::AlignHCenter | Qt::AlignCenter));
//    painter.end();
}

void medDropSite::mousePressEvent(QMouseEvent* event)
{
    Qt::MouseButtons mouseButtons = event->buttons();
    if( mouseButtons & Qt::LeftButton )
        emit clicked();
}
