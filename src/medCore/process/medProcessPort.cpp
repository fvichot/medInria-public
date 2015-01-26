#include <medProcessPort.h>


medProcessPort::medProcessPort(QString name)
{
    m_name = name;
}


medProcessPort::~medProcessPort()
{

}

QString medProcessPort::name() const
{
    return m_name;
}

void medProcessPort::retrieveContentFromPort(medProcessPort * port)
{
    m_content = port->m_content;
}

QVariant medProcessPort::content() const
{
    return m_content;
}

bool medProcessPort::setContent(const QVariant& value)
{
    m_content = value;
}
