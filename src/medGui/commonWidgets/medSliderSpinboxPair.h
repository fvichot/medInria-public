/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "medGuiExport.h"
#include <QtGui>

class MEDGUI_EXPORT medSliderSpinboxPair: public QWidget {

    Q_OBJECT

public:

    medSliderSpinboxPair(QWidget* parent=0): QWidget(parent) {
        slider  = new QSlider(Qt::Horizontal,this);
        spinbox = new QSpinBox(this);

        slider->setTracking(false);

        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0,0,0,0);
        layout->addWidget(slider);
        layout->addWidget(spinbox);

        connect(slider, SIGNAL(valueChanged(int)),this,SLOT(onValueChanged()));
        connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(updateSpinbox(int)));
        connect(spinbox,SIGNAL(editingFinished()),this,SLOT(onValueChanged()));
    }

    void setMinimum(const int min) {
        slider->setMinimum(min);
        spinbox->setMinimum(min);
    }

    void setMaximum(const int max) {
        slider->setMaximum(max);
        spinbox->setMaximum(max);
    }

    int value() const { return slider->value(); }

    void setSpinBoxSuffix(QString suffix) { spinbox->setSuffix(suffix); }

signals:

    void valueChanged(int);

public slots:

    void setValue(int value) {
        slider->blockSignals(true);
        spinbox->blockSignals(true);
        slider->setValue(value);
        spinbox->setValue(value);
        slider->blockSignals(false);
        spinbox->blockSignals(false);
        emit(valueChanged(value));
    }

    void setEnabled(bool boolean){
        slider->setEnabled(boolean);
        spinbox->setEnabled(boolean);
    }

    void updateSpinbox(int value)
    {
        spinbox->blockSignals(true);
        spinbox->setValue(value);
        spinbox->blockSignals(false);
    }

protected slots:

    void onValueChanged() {
        //editingFinished is emitted when we press Enter AND when the spinBox loses focus
        if (sender()==spinbox && !spinbox->hasFocus()) //we ignore the latter
            return;
        slider->blockSignals(true);
        spinbox->blockSignals(true);
        if (sender()==slider)
            spinbox->setValue(slider->value());
        else
            slider->setValue(spinbox->value());
        slider->blockSignals(false);
        spinbox->blockSignals(false);
        emit(valueChanged(slider->value()));
    }

private:

    QSlider*  slider;
    QSpinBox* spinbox;
};
