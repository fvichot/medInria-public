#include "medDataFuture.h"

class medDataFutureData : public QSharedData {
public:
};

medDataFuture::medDataFuture() : data(new medDataFutureData)
{
}

medDataFuture::medDataFuture(const medDataFuture &rhs) : data(rhs.data)
{
}

medDataFuture &medDataFuture::operator=(const medDataFuture &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

medDataFuture::~medDataFuture()
{
}
