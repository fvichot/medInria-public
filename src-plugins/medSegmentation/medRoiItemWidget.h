#pragma once

#include <QWidget>
#include <msegPluginExport.h>

class QPushButton;
class QLabel;
class QListWidget;

class MEDVIEWSEGMENTATIONPLUGIN_EXPORT medRoiItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit medRoiItemWidget(QString name,unsigned int index,QWidget * parent = 0);

    virtual ~medRoiItemWidget();

    unsigned int getIndex();

signals:
    void deleteWidget(unsigned int);

protected slots:
    void emitDeleteWidget();

private:
    QLabel * roiInfo;
    unsigned int index;
    unsigned int idSlice; // not sure if needed
    unsigned char orientation; // same
};