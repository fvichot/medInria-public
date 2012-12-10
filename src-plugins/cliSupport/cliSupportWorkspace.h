// /////////////////////////////////////////////////////////////////
// Generated by medPluginGenerator
// /////////////////////////////////////////////////////////////////

#ifndef cliSupportWORKSPACE_H
#define cliSupportWORKSPACE_H

#include <QtCore>
#include <medWorkspace.h>

#include "cliSupportWorkspacePluginExport.h"

class cliSupportWorkspacePrivate;

class cliSupportWorkspace : public medWorkspace
{
    Q_OBJECT

public:
     cliSupportWorkspace(QWidget *parent = 0);
    ~cliSupportWorkspace(void);

    virtual void setupViewContainerStack();

    virtual QString identifier()  const;
    virtual QString description() const;

    static bool isUsable() {return true;}

public slots:
    /**
     * @brief Connects toolboxes to the current container
     *
     * @param name the container name
    */
    virtual void connectToolboxesToCurrentContainer(const QString &name);

private:
    cliSupportWorkspacePrivate *d;
};

#endif
