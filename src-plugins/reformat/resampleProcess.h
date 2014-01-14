#pragma once
#include <dtkCore/dtkAbstractProcess.h>
#include <reformatPluginExport.h>

class resampleProcessPrivate;
class REFORMATPLUGIN_EXPORT resampleProcess : public dtkAbstractProcess
{
    Q_OBJECT
public:
    resampleProcess(void);
    virtual ~resampleProcess(void);
    virtual QString description(void) const;
    static bool registered(void);

    public slots:
        //! Input data to the plugin is set through here
        void setInput(dtkAbstractData *data);
        //! Parameters are set through here, channel allows to handle multiple parameters
        void setParameter(double data, int channel);
        //! Method to actually start the filter
        int update(void);
        //! The output will be available through here
        dtkAbstractData *output(void);
private:
    template <class ImageType> void resample(const char * str);
    resampleProcessPrivate *d;
};
dtkAbstractProcess *createResampleProcess();
