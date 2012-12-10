#ifndef cliSupportTOOLBOX_H
#define cliSupportTOOLBOX_H

#include "medToolBox.h"

class dtkAbstractView;
class dtkAbstractData;
class cliSupportToolBoxPrivate;
class medDataIndex;

class cliSupportToolBox : public medToolBox
{
    Q_OBJECT
public:
     cliSupportToolBox(QWidget *parent);
    ~cliSupportToolBox();


    virtual void setData(dtkAbstractData *data);

     static QString name();
     static QString identifier();
     static QString description();

public slots:
     virtual void update (dtkAbstractView *view);

protected slots:


    virtual void clear (void);

private:

    cliSupportToolBoxPrivate *d;

};


#endif
