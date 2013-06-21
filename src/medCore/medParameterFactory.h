#ifndef MEDPARAMETERFACTORY_H
#define MEDPARAMETERFACTORY_H

#include <QObject>
#include <QWidget>
#include <QMap>

class medParameterWidget;
struct medParameterDetails;

enum medParameterType {
    Boolean,
    Double,
    Int,
    List,
    ColorList
};

class medParametrizedObject : public QObject
{
    Q_OBJECT
public:
    medParametrizedObject();
    virtual ~medParametrizedObject();

    bool hasParameter(int id);

    void setMotherfuckingParameter();
    get
};

class medParameterFactory : public QObject
{
    Q_OBJECT
public:

    static medParameterFactory * instance();

    int registerBooleanParameter(QString name);
    int registerDoubleParameter(QString name, double min, double max);
    int registerIntParameter(QString name, int min, int max);
    int registerListParameter(QString name, QStringList list);
    int registerColorListParameter(QString name, QStringList colorList);

    QString parameterName(int id) const;

    QWidget * widgetForParameter(int id, QWidget * parent = 0);

private:
    medParameterFactory();
    ~medParameterFactory();

    static medParameterFactory * factory;

    int findOrInsertParameter(medParameterDetails * details);

    typedef QMap<int, medParameterDetails*> ParameterHashType;
    ParameterHashType parameters;
};


class medParameterWidget : public QWidget
{
    Q_OBJECT
public:
    medParameterWidget(medParameterDetails * d, QWidget * parent = 0);

    QString name() const;
    int id() const;

    virtual medParameterWidget * clone() = 0;
protected:
    medParameterDetails * details;
};


class medParameterBooleanWidget : public medParameterWidget
{
    Q_OBJECT
public:
    medParameterBooleanWidget(medParameterDetails * d, QWidget * parent = 0);

    void setValue(bool value);
    bool value();

signals:
    void valueChanged(bool value);
};

#endif // MEDPARAMETERFACTORY_H
