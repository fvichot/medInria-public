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
    typedef QPair<unsigned int,unsigned int> PairInd;

    explicit medRoiItemWidget(QString name,PairInd indexes,QWidget * parent = 0);

    virtual ~medRoiItemWidget();

    PairInd getIndex();

signals:
    void deleteWidget(PairInd);

protected slots:
    void emitDeleteWidget();

private:
    QLabel * roiInfo;
    PairInd indexes;
    unsigned int idSlice; // not sure if needed
    unsigned char orientation; // same
};