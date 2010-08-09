/*framesource.cpp is part of lucifer a laser show controller.

Copyrignt 2010 Dan Mills <dmills@exponent.myzen.co.uk>

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

/* $Id: framesource.cpp 22 2010-07-24 15:01:51Z dmills $ */

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>

#include "frame.h"
#include "framesource.h"
#include "framesequencer.h"
#include "staticframe.h"
#include "log.h"


static std::map <std::string, FrameSourcePtr (*)()> *framegen = NULL;

void FrameSource::registerFrameGen (const std::string name, FrameSourcePtr (*generator)())
{
    assert (generator);
    if (!framegen) {
        framegen = new std::map<std::string,FrameSourcePtr(*)()>;
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

FrameSourcePtr FrameSource::newSource (const std::string name)
{
    assert (framegen);
    std::map<std::string,FrameSourcePtr(*)()>::iterator i;
    i = framegen->find(name);
    if (i != framegen->end()) {
        FrameSourcePtr (*fg)() = i->second;
        return fg();
    } else {
        slog()->errorStream() << "Could not find a generator for a " << name << " framesource";
        return FrameSourcePtr();
    }
}

std::vector<std::string> FrameSource::enemerateFrameGenTypes()
{
    std::vector<std::string> s;
    if (framegen) {
        for (std::map<std::string,FrameSourcePtr (*)()>::iterator it = framegen->begin();
                it != framegen->end(); it++) {
            s.push_back(it->first);
        }
    }
    return s;
}

FrameSource::FrameSource (FrameSource::FLAGS flags, FrameSource::POSSIBLE_CHILDREN pc)
{
    flags_ = flags;
    pos_children_ = pc;
    slog()->debugStream() << "Creating FrameSource : " << this << " Flags : " << flags;
}

FrameSource::~FrameSource ()
{
    slog()->debugStream() << "Deleting Framesource : " << this;
}

unsigned int FrameSource::getUniqueHandle()
{
    unsigned static int hdl = 0;
    return hdl++;
}

void FrameSource::saveFrames(QXmlStreamWriter *w)
{
    assert (w);
    w->writeStartElement(QString().fromStdString(name));
    save(w);
    for (unsigned int i=0; i < numChildren(); i++) {
        child(i)->saveFrames(w);
    }
    w->writeEndElement();
}

FrameSourcePtr FrameSource::loadFrames (QXmlStreamReader *e)
{
    std::string oname;
    FrameSourcePtr fs;
    assert (e);
    assert (e->tokenType() == QXmlStreamReader::StartElement);
    oname = e->name().toString().toStdString();
    slog()->debug(std::string("Loading object called : ") + oname);
    int col = e->columnNumber();
    FrameSourcePtr f = FrameSource::newSource(oname);
    if (f) {
        fs = f;
    } else {
        slog()->error(std::string("Attempt to load unknown object type : ")+oname);
        e->readNextStartElement();
        return fs;
    }
    if (fs) {
        fs->load (e);
        if (fs->numPossibleChildren() != NONE) {
            slog()->debugStream() << fs <<" Loading children";
            while ((!e->atEnd()) && (e->columnNumber() > col)) {
                while (!e->atEnd() && !e->isStartElement()) {
                    e->readNextStartElement();
                }
                if (e->columnNumber() <= col) {
                    return fs;
                }
                if (e->tokenType() == QXmlStreamReader::StartElement) {
                    slog()->debugStream() << fs <<" Loading child";
                    fs->addChild(loadFrames(e));
                } else {
                    e->readNextStartElement();
                }
            }
        }
    }
    return fs;
}

std::string FrameSource::toString()
{
    QString text;
    QXmlStreamWriter *w = new QXmlStreamWriter(&text);
    saveFrames(w);
    delete w;
    return text.toStdString();
}

FrameSourcePtr FrameSource::fromString (std::string &s)
{
    slog()->debugStream() << "Dropping :" << s;
    QXmlStreamReader *r = new QXmlStreamReader (QString (s.c_str()));
    r->readNextStartElement();
    FrameSourcePtr f = FrameSource::loadFrames(r);
    delete r;
    return f;
}

FrameSourcePtr FrameSource::child(unsigned int pos) {
    if (children_.size() > pos) {
        return children_[pos];
    } else {
        return FrameSourcePtr();
    }
}

bool FrameSource::addChild(FrameSourcePtr child, unsigned int pos)
{
    slog()->debugStream() << this << " Adding child at " << pos;
    if (pos <= children_.size()) {
        if ((pos_children_ == MANY) ||
                ((pos_children_ ==	ONE) && (children_.size() == 0)) ||
                ((pos_children_ == TWO) && (children_.size() < 2))) {
            if (pos == children_.size()) {
                children_.push_back(child);
            } else {
                children_.insert (children_.begin() + pos,child);
            }
            // make sure the newly added data source has a correct set of playbacks
            //child->erasePlaybacks(); ///TODO - FIXME Breaks editor
            std::map <Playback, PlaybackDataPtr>::iterator it;
            it = playbacks_.begin();
            while (it != playbacks_.end()) {
                child->createPlayback ((*it).first);
                ++it;
            }
            return true;
        } else {
            slog()->errorStream() << this <<" Attempt to add child to framesource that cannot support it";
        }
    } else {
        slog()->errorStream() << this << " Attempt to add child in invalid position";
    }
    return false;
}

bool FrameSource::addChild(FrameSourcePtr child)
{
    return addChild (child,children_.size());
}

Playback FrameSource::createPlayback ()
{
    Playback idx = getUniqueHandle ();
    createPlayback (idx);
    return idx;
}

void FrameSource::createPlayback (Playback p)
{
    assert (playbacks_.find (p) == playbacks_.end());
    slog()->debugStream() << this << " Creating playback : " << p;
    playbacks_[p] = createPlaybackData();
    // now do the children
    for (unsigned int i=0; i < numChildren(); i++) {
        child(i)->createPlayback(p);
    }
}

void FrameSource::deletePlayback(Playback p)
{
    assert (playbacks_.find (p) != playbacks_.end());
    slog()->debugStream() << this << " Deleting playback : " << p;
    playbacks_.erase (p);
    // now do the children
    for (unsigned int i=0; i < numChildren(); i++) {
        child(i)->deletePlayback(p);
    }
}

PlaybackData * FrameSource::getPlayback (Playback p)
{
    assert (playbacks_.find (p) != playbacks_.end());
    return &(*playbacks_.find(p)->second);
}

void FrameSource::erasePlaybacks()
{
    slog()->debugStream() << this << " Erasing all playbacks";
    playbacks_.clear();
    // now do the children
    for (unsigned int i=0; i < numChildren(); i++) {
        child(i)->erasePlaybacks();
    }
}

FrameSource::POSSIBLE_CHILDREN FrameSource::numPossibleChildren()
{
    return pos_children_;
}

bool FrameSource::deleteChild (unsigned int pos)
{
    if (pos < children_.size()) {
        children_.erase(children_.begin()+pos);
        return true;
    }
    return false;
}

void FrameSource::eraseChildren()
{
    children_.clear();
}

unsigned int FrameSource::numChildren()
{
    return children_.size();
}

FrameGui::~FrameGui()
{
}




