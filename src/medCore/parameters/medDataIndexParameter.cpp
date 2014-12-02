/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medDataIndexParameter.h>
#include <medDataIndex.h>
#include <medDropSite.h>

#include <QLabel>

class medDataListParameterPrivate
{
public:
    medDataIndex value;
    medDropSite* dropSite;
    QString text;

    ~medDataListParameterPrivate()
    {
        delete dropSite;
    }
};

medDataIndexParameter::medDataIndexParameter(QString name, QObject* parent):
    medAbstractParameter(name, parent),
    d(new medDataListParameterPrivate)
{
    d->value = medDataIndex();
}

medDataIndexParameter::~medDataIndexParameter()
{
    delete d;
}

void medDataIndexParameter::setValue(medDataIndex value)
{
    if(d->value == value)
        return;

    d->value = value;

    //  update intern widget
    this->blockInternWidgetsSignals(true);
    this->updateInternWigets();
    this->blockInternWidgetsSignals(false);

    emit valueChanged(d->value);
}

void medDataIndexParameter::updateInternWigets()
{
    if(d->dropSite)
        d->dropSite->setValue(d->value);
}

void medDataIndexParameter::clear()
{
    this->setValue(medDataIndex());
}

medDataIndex medDataIndexParameter::value() const
{
    return d->value;
}

QWidget* medDataIndexParameter::getWidget()
{
    return this->getDropSite();
}

void medDataIndexParameter::setText(const QString &text)
{
    d->text = text;
    if(d->dropSite)
        d->dropSite->setText(text);
}

medDropSite* medDataIndexParameter::getDropSite()
{
    if(!d->dropSite)
    {
        d->dropSite = new medDropSite;
        d->dropSite->setText(d->text);
    }

    return d->dropSite;
}

void medDataIndexParameter::removeInternDropSite()
{
    this->removeFromInternWidgets(d->dropSite);
    d->dropSite = NULL;
}
