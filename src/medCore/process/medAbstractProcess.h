/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <dtkCore/dtkAbstractProcess.h>

#include <medProcessPort.h>
#include <medProcessInput.h>
#include <medProcessOutput.h>

#include <medAbstractData.h>
#include <medAbstractImageData.h>

#include <medCoreExport.h>
#include <medAbstractJob.h>

class medAbstractProcessPrivate;

class medAbstractParameter;
class medToolBox;
class medTriggerParameter;
class medViewContainerSplitter;


/**
 * Extending dtkAbstractProcess class to hold more specific information
 */
class MEDCORE_EXPORT medAbstractProcess : public dtkAbstractProcess
{
    Q_OBJECT

public:
    medAbstractProcess( medAbstractProcess * parent = NULL );
    virtual ~medAbstractProcess();

    QList<medProcessPort*> inputs() const;
    QList<medProcessPort*> outputs() const;

    medProcessPort* inputPort(QString name) const;
    medProcessPort* outputPort(QString name) const;

    void retrieveInputs(const medAbstractProcess *);

    void setInputValue(const QVariant&  data, unsigned int portNumber);
    const QVariant& inputValue(unsigned int portNumber);

    const QVariant& outputValue(unsigned int portNumber);

    virtual QList<medAbstractParameter*> parameters() = 0;
    medAbstractParameter* parameter(QString parameterName);

    virtual medToolBox* toolbox();
    virtual QWidget* parameterWidget();
    virtual medTriggerParameter* runParameter() const;
    virtual medViewContainerSplitter* viewContainerSplitter();

    virtual bool isInteractive() const = 0;

public slots:
    int start();

signals:
    void showError(QString message, unsigned int timeout = 5000);

protected:
    void appendInput(medProcessPort*);
    void appendOutput(medProcessPort*);

protected slots:
    virtual void handleInput();
    virtual void handleOutputs();

private:
    virtual int update () = 0;

    virtual void updateContainer(medProcessPort *);

private:
    using dtkAbstractProcess::onCanceled;
    using dtkAbstractProcess::read;
    using dtkAbstractProcess::write;
    using dtkAbstractProcess::setParameter;
    using dtkAbstractProcess::setInput;
    using dtkAbstractProcess::setData;
    //TODO rename our output methode
    //using dtkAbstractProcess::output;
    using dtkAbstractProcess::data;
    using dtkAbstractProcess::channelCount;

private:
    medAbstractProcessPrivate* d;
};


class medRunnableProcess: public medAbstractJob
{
    Q_OBJECT

private:
    medAbstractProcess* m_process;

public:
    medRunnableProcess(medAbstractProcess* process, QString name);

    virtual void internalRun();
    virtual void cancel();
};

