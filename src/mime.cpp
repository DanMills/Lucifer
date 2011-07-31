#include <mime.h>

LaserMimeObject::LaserMimeObject(): QMimeData()
{
}

LaserMimeObject::~LaserMimeObject()
{
}

QStringList LaserMimeObject::mimeTypes()
{
    QStringList l;
    l.push_back("text/x-FrameSource");
    // l.push_back ("text/uri-list");
    return l;
}

// within a single application data will really be a LaserMimeObject which contains a 
// source ptr directly.
SourceImplPtr LaserMimeObject::getSource(const QMimeData *data)
{
    if (!data) {
	return SourceImplPtr();
    }
    SourceImplPtr sp;
    const LaserMimeObject *lo = qobject_cast<const LaserMimeObject*>(data);
    if (lo) { // a drag within the application
        sp = lo->source->clone();
    } else {
        // Drag in from outside....
        if (data->hasFormat("text/x-FrameSource")) {
            sp = FrameSource_impl::fromString(data->data("text/x-FrameSource").constData());
        } else {
            // if (data->hasFormat("text/uri-list")){
            //
            //}
        }
    }
    return sp;
}

bool LaserMimeObject::setFrame(SourceImplPtr sp)
{
    source = sp; // when copying within an application, use this
    setData("text/x-FrameSource",QByteArray(sp->toString().c_str()));
    return true;
}