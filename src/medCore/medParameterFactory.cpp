#include "medParameterFactory.h"

#include <QStringList>

//------------------ medParameterFactory::ParameterDetails -------------------------------------

struct medParameterDetails {
public:
       int id;
       QString name;
       medParameterFactory::Type type;
       union {
           double dRange[2];
           int iRange[2];
       };
       QStringList list; // can't go in the union... we need C++11
       QWidget * widget;

       bool compare(medParameterDetails * d);
};

bool medParameterDetails::compare(medParameterDetails * d)
{
    if (type != d->type) return false;
    if (name != d->name) return false;

    if (type == medParameterFactory::Double && ( ! qFuzzyCompare(dRange[0], d->dRange[0])
                                            ||   ! qFuzzyCompare(dRange[1], d->dRange[1])))
        return false;

    if (type == medParameterFactory::Double && (iRange[0] != d->iRange[0]
                                            ||  iRange[1] != d->iRange[1]))
        return false;

    if (type == medParameterFactory::List && list != d->list)
        return false;

    if (type == medParameterFactory::ColorList && list != d->list)
        return false;

    return true;
}

//------------------ medParameterWidget -------------------------------------

medParameterWidget::medParameterWidget(medParameterDetails *d, QWidget * parent)
    : QWidget(parent)
    , details(d)
{
}


QString medParameterWidget::name() const
{
    return details->name;
}


int medParameterWidget::id() const
{
    return details->id;
}

//------------------ medParameterBooleanWidget -------------------------------------

#include <QCheckBox>

medParameterBooleanWidget::medParameterBooleanWidget(medParameterDetails *d, QWidget * parent)
    : medParameterWidget(d, parent)
{
    QCheckBox * box = new QCheckBox(this);
}


// ------------------------------- medParameterFactory --------------------

medParameterFactory * medParameterFactory::factory = 0;

medParameterFactory * medParameterFactory::instance()
{
    if ( ! factory)
        factory = new medParameterFactory();
    return factory;
}

medParameterFactory::medParameterFactory()
    : QObject()
{
}


medParameterFactory::~medParameterFactory()
{
    qDeleteAll(parameters);
    parameters.clear();
}

int medParameterFactory::registerBooleanParameter(QString name)
{
    medParameterDetails * details = new medParameterDetails;
    details->type = medParameterFactory::Boolean;
    details->name = name;
    return findOrInsertParameter(details);
}

int medParameterFactory::registerDoubleParameter(QString name, double min, double max)
{
    medParameterDetails * details = new medParameterDetails;
    details->type = medParameterFactory::Double;
    details->name = name;
    details->dRange[0] = min;
    details->dRange[1] = max;
    return findOrInsertParameter(details);
}

int medParameterFactory::registerIntParameter(QString name, int min, int max)
{
    medParameterDetails * details = new medParameterDetails;
    details->type = medParameterFactory::Int;
    details->name = name;
    details->iRange[0] = min;
    details->iRange[1] = max;
    return findOrInsertParameter(details);
}

int medParameterFactory::registerListParameter(QString name, QStringList list)
{
    medParameterDetails * details = new medParameterDetails;
    details->type = medParameterFactory::List;
    details->name = name;
    details->list = list;
    return findOrInsertParameter(details);
}

int medParameterFactory::registerColorListParameter(QString name, QStringList colorList)
{
    medParameterDetails * details = new medParameterDetails;
    details->type = medParameterFactory::ColorList;
    details->name = name;
    details->list = colorList;
    return findOrInsertParameter(details);
}

QString medParameterFactory::parameterName(int id) const
{
    if ( ! parameters.contains(id)) return QString();
    return parameters.value(id)->name;
}

QWidget * medParameterFactory::widgetForParameter(int id, QWidget * parent)
{
    if ( ! parameters.contains(id)) return 0;

    medParameterDetails * details = parameters.value(id);
    switch(details->type)
    {
        case medParameterFactory::Boolean:

            break;
    }
    // unreachable
    return 0;
}

int medParameterFactory::findOrInsertParameter(medParameterDetails * details)
{
    for(ParameterHashType::iterator it = parameters.begin(); it != parameters.end(); ++it) {
        if (details->compare(it.value())) {
            delete details;
            return it.key();
        }
    }
    // Not found
    details->id = parameters.size();
    parameters.insert(details->id, details);
    return details->id;
}



