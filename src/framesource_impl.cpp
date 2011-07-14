/*framesource_impl.cpp is part of lucifer a laser show controller.

Copyrignt 2011 Dan Mills <dmills@exponent.myzen.co.uk>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 dated June, 1991.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <assert.h>
#include <map>

#include "framesource_impl.h"
#include "log.h"

// The generator mapping
static std::map <std::string, SourceImplPtr (*)()> *framegen = NULL;

FrameSource_impl::FrameSource_impl(FrameSource_impl::FLAGS flags_, FrameSource_impl::POSSIBLE_CHILDREN pos_child, std::string unique_name)
{
    name = unique_name;
    possibleChildren = pos_child;
    flags = flags_;
    slog()->debugStream() << "Creating a framesource of type '" << unique_name << "' at " << this;
}

FrameSource_impl::~FrameSource_impl()
{
    slog()->debugStream() << "deleting a framesource type '" << name <<"' at " << this;
}

bool FrameSource_impl::addChild(SourceImplPtr child, int pos)
{
    assert (child);
    if ((numPossibleChildren() == MANY) ||
            ((numPossibleChildren() ==	ONE) && (children.size() == 0)) ||
            ((numPossibleChildren() == TWO) && (children.size() < 2))) {
        if (pos < 0) {
            children.push_back(child);
            slog()->debugStream() << "Appended child node to " << this << " at position " << children.size()-1;
            return true;
        }
        if (pos > children.size()) {
            slog ()->errorStream() << "Framesource_impl::Attempt to insert a child node past the end of the list";
            return false;
        }
        if (children.size() == 0) {
            children.push_back(child);
        } else {
            children.insert(children.begin()+pos,child);
        }
        return true;
    } else {
        slog()->errorStream() << "Framesource_impl::Attempted to add too many children to a '" << name <<"' at " << this;
        return false;
    }
}

std::string FrameSource_impl::getName() const
{
    return name;
}


SourceImplPtr FrameSource_impl::child(unsigned int pos) const
{
    assert (pos < children.size());
    return children[pos];
}

unsigned int FrameSource_impl::numChildren() const
{
    return children.size();
}

bool FrameSource_impl::deleteChild(unsigned int pos)
{
    assert (pos < children.size());
    children.erase(children.begin()+pos);
    return true;
}

void FrameSource_impl::setDescription(std::string des)
{
    description = des;
}

FrameSource_impl::POSSIBLE_CHILDREN FrameSource_impl::numPossibleChildren() const
{
    return possibleChildren;
}

SourceImplPtr FrameSource_impl::clone()
{
    SourceImplPtr fs = FrameSource_impl::newSource(name);
    assert (fs);
    copyDataTo (fs);
    // recursive clone the children
    for (unsigned int i=0; i < numChildren(); i++) {
        fs->addChild(child(i)->clone());
    }
    return fs;
}

void FrameSource_impl::saveFrames(QXmlStreamWriter* w)
{
    assert (w);
    w->writeStartElement(QString().fromStdString(name));
    w->writeAttribute("Description",QString().fromStdString(description));
    save(w);
    w->writeStartElement("Children");
    w->writeAttribute("Number", QString().number(numChildren()));
    for (unsigned int i=0; i < numChildren(); i++) {
        child(i)->saveFrames(w);
    }
    w->writeEndElement();//Children
    w->writeEndElement();//This
}

SourceImplPtr FrameSource_impl::loadFrames(QXmlStreamReader* e)
{
    std::string oname;
    SourceImplPtr fs;
    assert (e);
    while ((!e->atEnd()) && (e->tokenType() != QXmlStreamReader::StartElement))
    {
        e->readNext();
    }
    oname = e->name().toString().toStdString();
    slog()->info(std::string("Loading object called : ") + oname);
    SourceImplPtr f = FrameSource_impl::newSource(oname);
    if (f) {
        fs = f;
    } else {
        slog()->error(std::string("Attempt to load unknown object type : ")+oname);
        e->readNextStartElement();
        return fs;
    }
    if (fs) {
        fs->setDescription(e->attributes().value("Description").toString().toStdString());
        // load the configuration data for this node
        fs->load (e);
        e->readNextStartElement();
        while ((!e->atEnd()) && (e->tokenType() != QXmlStreamReader::StartElement))
        {
            e->readNext();
        }
        if (e->name().toString() == "Children") {
            int nc = e->attributes().value("Number").toString().toInt();
            e->readNextStartElement();// cursor on first child or next element
            // depending on number of children
            if (nc) {
                slog()->infoStream() << fs <<" Loading " << nc << " children";
                for (int i=0; i < nc; i++) {
                    while ((!e->atEnd()) && (e->tokenType() != QXmlStreamReader::StartElement)) {
                        e->readNext();
                    }
                    fs->addChild(loadFrames(e));
                    e->readNextStartElement();
                }
            }
        } else {
            slog()->errorStream() << "Malformed file (Missing child section)";
        }
    }
    return fs;
}

void FrameSource_impl::registerFrameGen (const std::string name, SourceImplPtr (*generator)())
{
    assert (generator);
    if (!framegen) {
        framegen = new std::map<std::string,SourceImplPtr(*)()>;
        framegen->clear();
        slog()->infoStream() <<"Created framesource generator mapping :" << framegen;
    }
    if (framegen->find(name) != framegen->end()) {
        slog()->errorStream() << "Attemped to register a non unique frame generator function : " << name;
        assert (0);
    } else {
        (*framegen)[name] = generator;
        slog()->infoStream() <<"Registered factory for " << name <<" objects : " << (void*)generator;
    }
}

SourceImplPtr FrameSource_impl::newSource(const std::string name)
{
    if (!framegen) {
        slog()->errorStream() << "Frame generator mapping was NULL";
        return SourceImplPtr();
    }
    std::map<std::string,SourceImplPtr(*)()>::iterator i;
    i = framegen->find(name);
    if (i != framegen->end()) {
        SourceImplPtr (*fg)() = i->second;
        return fg();
    } else {
        slog()->errorStream() << "Could not find a generator for a " << name << " framesource";
        return SourceImplPtr();
    }
}

std::vector<std::string> FrameSource_impl::enemerateFrameGenTypes()
{
    std::vector<std::string> s;
    if (framegen) {
        for (std::map<std::string,SourceImplPtr (*)()>::iterator it = framegen->begin();
                it != framegen->end(); it++) {
            s.push_back(it->first);
        }
    }
    return s;
}

bool FrameSource_impl::exists (const std::string s)
{
    if (!framegen) {
        return false;
    }
    return (framegen->find(s) != framegen->end());
}


PlaybackImplPtr FrameSource_impl::createPlayback()
{
    PlaybackImplPtr p = newPlayback();
    for (unsigned int i=0; i < numChildren(); i++) {
        p->addChild(child(i)->createPlayback());
    }
    return p;
}

std::string FrameSource_impl::getDescription()
{
    return description;
}

SourceImplPtr FrameSource_impl::fromString(const std::string s)
{
    QXmlStreamReader *r = new QXmlStreamReader (QString (s.c_str()));
    r->readNextStartElement();
    SourceImplPtr f = FrameSource_impl::loadFrames(r);
    delete r;
    return f;
}

std::string FrameSource_impl::toString()
{
    QString text;
    QXmlStreamWriter *w = new QXmlStreamWriter(&text);
    saveFrames(w);
    delete w;
    return text.toStdString();
}

Playback_impl::Playback_impl()
{
}

Playback_impl::~Playback_impl()
{
}

bool Playback_impl::addChild(PlaybackImplPtr child, int pos)
{
    if (pos<0) {
        children.push_back(child);
        return true;
    }
    if (pos < children.size()) {
        children.insert(children.begin()+pos,child);
        return true;
    }
    if (children.size() == pos) {
        children.push_back(child);
        return true;
    }
    slog ()->errorStream() << "Playback_impl::Attempted to add child at invalid index";
    return false;
}

PlaybackImplPtr Playback_impl::child(unsigned int pos)
{
    if (pos < children.size()) {
        return children[pos];
    }
    return PlaybackImplPtr();
}

FrameGui::~FrameGui()
{
}


