/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medAbstractProcess.h>

#include <medAbstractParameter.h>
#include <medToolBox.h>
#include <medTriggerParameter.h>
#include <medAbstractJob.h>
#include <medJobManager.h>
#include <medViewContainer.h>
#include <medViewContainerSplitter.h>
#include <medAbstractData.h>
#include <medMetaDataKeys.h>
#include <medAbstractLayeredView.h>
#include <medDataManager.h>

medRunnableProcess::medRunnableProcess(medAbstractProcess* process, QString name): medAbstractJob(name)
{
    m_process = process;

    connect(m_process, SIGNAL(progressed(int)), this, SIGNAL(progressed(int)));
    connect(m_process, SIGNAL(success()), this, SIGNAL(success()));
    connect(m_process, SIGNAL(failure()), this, SIGNAL(failure()));
    connect(m_process, SIGNAL(canceled()), this, SIGNAL(cancelled()));
    connect(m_process, SIGNAL(showError(QString, uint)), this, SIGNAL(showError(QString, uint)));
}

void medRunnableProcess::internalRun()
{
    m_process->start();
}

void medRunnableProcess::cancel()
{
    m_process->cancel();
}


class medAbstractProcessPrivate
{
  public:
    QList<medProcessPort*> inputs;
    QList<medProcessPort*> outputs;
    QPointer<medToolBox> toolbox;
    QPointer<QWidget> parameterWidget;
    medTriggerParameter* runParameter;
    QPointer<medViewContainerSplitter> viewContainerSplitter;
    QHash <medProcessPort*, medViewContainer*> containerForInputPort;
    QHash <medProcessPort*, medViewContainer*> containerForOutputPort;
};


medAbstractProcess::medAbstractProcess(medAbstractProcess * parent):dtkAbstractProcess(parent), d(new medAbstractProcessPrivate)
{
    d->toolbox = NULL;
    d->parameterWidget = NULL;
    d->runParameter = new medTriggerParameter("Run", this);
    d->runParameter->setButtonText("Run");

    connect(this, SIGNAL(success()), this, SLOT(handleOutputs()));
}

medAbstractProcess::~medAbstractProcess()
{
    delete d;
}


QList<medProcessPort*> medAbstractProcess::inputs() const
{
   return d->inputs;
}

QList<medProcessPort*> medAbstractProcess::outputs() const
{
    return d->outputs;
}

medProcessPort* medAbstractProcess::inputPort(QString name) const
{
    medProcessPort* res = NULL;
    foreach(medProcessPort* port, this->inputs())
    {
        if(port->name() == name)
        {
            res = port;
            break;
        }
    }
    return res;
}

void medAbstractProcess::appendInput(medProcessPort *input)
{
    d->inputs.append(input);
}

void medAbstractProcess::appendOutput(medProcessPort *output)
{
    d->outputs.append(output);
}

void medAbstractProcess::setInputValue(const QVariant& data, unsigned int portNumber)
{
    if(portNumber >= (unsigned int)this->inputs().size())
        return;

    medProcessPort* port = this->inputs().at(portNumber);
    port->setContent(data);
    updateContainer(port);
}

const QVariant& medAbstractProcess::inputValue(unsigned int portNumber)
{
    if(portNumber >= (unsigned int)this->inputs().size())
        return QVariant();

    return this->inputs().at(portNumber)->content();
}


const QVariant& medAbstractProcess::outputValue(unsigned int portNumber)
{
    if(portNumber >= (unsigned int)this->outputs().size())
        return QVariant();

    return this->outputs().at(portNumber)->content();
}


medAbstractParameter* medAbstractProcess::parameter(QString parameterName)
{
    foreach(medAbstractParameter *param, this->parameters())
    {
        if(param->name() == parameterName)
            return param;
    }

    return NULL;
}

medToolBox* medAbstractProcess::toolbox()
{
    if(d->toolbox.isNull())
    {
        d->toolbox = new medToolBox;
        d->toolbox->addWidget(this->parameterWidget());
        d->toolbox->addWidget(d->runParameter->getPushButton());
    }
    return d->toolbox;
}


QWidget* medAbstractProcess::parameterWidget()
{
    if(d->parameterWidget.isNull())
    {
        d->parameterWidget = new QWidget;
        QFormLayout *layout = new QFormLayout(d->parameterWidget);
        foreach(medAbstractParameter *param, this->parameters())
            layout->addRow(param->getLabel(), param->getWidget());
    }
    return d->parameterWidget;
}

medTriggerParameter* medAbstractProcess::runParameter() const
{
    return d->runParameter;
}

void setupNewPortContainer(medViewContainer * container, QString name)
{
    container->setClosingMode(medViewContainer::CLOSE_VIEW_ONLY);
    container->setDefaultWidget(new QLabel(name));
    container->setUserSplittable(false);
    container->setMultiLayered(false);
}


medViewContainerSplitter* medAbstractProcess::viewContainerSplitter()
{
    if( ! d->viewContainerSplitter.isNull())
        return d->viewContainerSplitter;

    d->viewContainerSplitter = new medViewContainerSplitter;

    // Create input containers
    medViewContainer* inputContainer = NULL;
    medViewContainer* outputContainer = NULL;
    for(int i = 0 ; i <  d->inputs.size(); ++i) {
        medProcessPort * port = d->inputs.at(i);
        if (i == 0) {
            inputContainer = new medViewContainer();
            d->viewContainerSplitter->addViewContainer(inputContainer);
            if ( ! d->outputs.isEmpty()) {
                outputContainer = inputContainer->splitHorizontally();
            }
        } else {
            inputContainer = inputContainer->splitVertically();
        }
        medAbstractData* data = dynamic_cast<medAbstractData*>(port->content().value<QObject*>());
        if (data) {
            inputContainer->addData(data);
        }

        setupNewPortContainer(inputContainer, port->name());
        d->containerForInputPort[port] = inputContainer;

        connect(inputContainer, SIGNAL(viewContentChanged()), this, SLOT(handleInput()));
        connect(inputContainer, SIGNAL(viewRemoved()), this, SLOT(handleInput()));
    }

    // Create output containers
    for(int i = 0 ; i <  d->outputs.size(); ++i) {
        medProcessPort * port = d->outputs.at(i);
        if (i > 0) {
            outputContainer = outputContainer->splitVertically();
        }
        medAbstractData* data = dynamic_cast<medAbstractData*>(port->content().value<QObject*>());
        if (data) {
            outputContainer->addData(data);
        }

        setupNewPortContainer(outputContainer, port->name());
        d->containerForOutputPort[port] = outputContainer;

        connect(outputContainer, SIGNAL(viewContentChanged()), this, SLOT(handleOutputs()));
        connect(outputContainer, SIGNAL(viewRemoved()), this, SLOT(handleOutputs()));
    }

    d->viewContainerSplitter->adjustContainersSize();

    return d->viewContainerSplitter;
}



int medAbstractProcess::start()
{
    int ret = this->update();
    if (ret == EXIT_SUCCESS)
    {
        emit success();
    }
    else if (ret == EXIT_FAILURE)
        emit failure();

    return ret;
}

void medAbstractProcess::handleInput()
{
    medViewContainer *container = dynamic_cast<medViewContainer *>(QObject::sender());
    if(!container)
        return;

    medAbstractData *inputData;
    //TODO - RDE / GPE - We have to deal with medAbstractView as well.
    medAbstractLayeredView *view = dynamic_cast<medAbstractLayeredView *>(container->view());
    if(!view)
        inputData = NULL;
    else
        inputData = view->layerData(view->currentLayer());

    d->containerForInputPort.key(container)->setContent(QVariant::fromValue(inputData));


}

void medAbstractProcess::handleOutputs()
{
    foreach(medProcessPort* port, d->containerForOutputPort.keys())
    {
        medAbstractData* ouputData = dynamic_cast<medAbstractData*>(port->content().value<QObject*>());
        if(!ouputData)
            return;

        medAbstractData* inputData = dynamic_cast<medAbstractData*>(d->containerForInputPort.keys()[0]->content().value<QObject*>());
        if(!inputData)
            return;

        if(!ouputData->hasMetaData(medMetaDataKeys::SeriesDescription.key()))
        {
            QString newSeriesDescription = inputData->metadata(medMetaDataKeys::SeriesDescription.key());
            newSeriesDescription += "_" + this->name();
            ouputData->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
        }
        foreach(QString metaData, inputData->metaDataList())
            if (!ouputData->hasMetaData(metaData))
                ouputData->addMetaData(metaData, inputData->metaDataValues(metaData));

        foreach(QString property, inputData->propertyList())
            ouputData->addProperty(property, inputData->propertyValues(property));

        QString generatedID = QUuid::createUuid().toString().replace("{","").replace("}","");
        ouputData->setMetaData(medMetaDataKeys::SeriesID.key(), generatedID);

        medDataManager::instance()->importData(ouputData);
        if(d->containerForOutputPort.value(port))
            d->containerForOutputPort.value(port)->addData(ouputData);
    }
}

void medAbstractProcess::updateContainer(medProcessPort *inputDataPort)
{
    if(!inputDataPort)
        return;

    if(d->containerForInputPort.value(inputDataPort))
    {
        medAbstractData* data = dynamic_cast<medAbstractData*>(inputDataPort->content().value<QObject*>());
        d->containerForInputPort.value(inputDataPort)->addData(data);
    }
}

void medAbstractProcess::retrieveInputs(const medAbstractProcess *other)
{
    foreach(medProcessPort *otherProcessPort, other->inputs())
    {
        medProcessPort *port = this->inputPort(otherProcessPort->name());
        if(port)
        {
            port->retrieveContentFromPort(otherProcessPort);

            if(d->inputs.contains(port))
            {
                updateContainer(port);
            }
        }
    }
}
