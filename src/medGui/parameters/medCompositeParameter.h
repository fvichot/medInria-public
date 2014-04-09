/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medAbstractParameter.h>
#include <QVariant>

#include <medGuiExport.h>

class medCompositeParameterPrivate;

class MEDGUI_EXPORT medCompositeParameter : public medAbstractParameter
{
    Q_OBJECT

public:
    medCompositeParameter(QString name = "Unknow composite parameter", QObject* parent = 0);
    virtual ~medCompositeParameter();

    virtual QWidget* getWidget();

    void addVariant(QString name, QVariant variant,
                    QVariant min = QVariant(), QVariant max = QVariant(), QVariant step = QVariant());

    virtual QList<QVariant> value() const;

public slots:
    virtual void setValue(const QList<QVariant> );

protected:
    virtual void updateInternWigets();

signals:
    void valueChanged(const QList<QVariant>& );

private slots:
    void updateValue(bool value);
    void updateValue(int value);
    void updateValue(double value);

private:
    medCompositeParameterPrivate* d;
};

