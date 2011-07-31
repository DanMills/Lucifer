#ifndef MIME_INCL
#define MIME_INCL

#include <qmimedata.h>
#include "framesource_impl.h"
#include "framesource.h"

/// \brief a shim for drag and drop of laser frames, this should work both between applications and within a single app.
class LaserMimeObject : public QMimeData
{
Q_OBJECT  
public:
    LaserMimeObject();
    virtual ~LaserMimeObject();

    static QStringList mimeTypes ();
    bool setFrame (SourceImplPtr sp);
    static SourceImplPtr getSource(const QMimeData* data);
private:
    SourceImplPtr source;
};

#endif