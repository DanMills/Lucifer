/*framesource_impl.h is part of lucifer a laser show controller.

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
/// FrameSources should derive from this virtual base class not from framesource.h
/// As this will ensure the availability of the appropriate boost shared pointers.

#ifndef FRAMESOURCE_IMPL
#define FRAMESOURCE_IMPL
#include <string>
#include <vector>
#include <QtGui>
#include "frame.h"
#include <boost/shared_ptr.hpp>
#include <qxmlstream.h>

class FrameSource_impl;
class Playback_impl;
class FrameGui;

typedef boost::shared_ptr<FrameSource_impl> SourceImplPtr;
typedef boost::shared_ptr<Playback_impl> PlaybackImplPtr;

/// Derive from this to produce frame generators or effects, overload nextFrame to
/// produce whatever frame is appropriate (direct copy from a child is possible), and
/// overload newPlayback to produce an object derived from Playback_impl to hold the per
/// playback state of this object.
/// Register a factory for the new object type and you are done.

class FrameSource_impl
{
public:
    enum FLAGS {
        NOTHING=0,
        MAKES_FRAMES=1,///< Object generates laser frame data.
	EFFECT=2, /// List this in the effects section, has at least one child that produces frames that this then modifies.
    };
    /// Can this object support child nodes.
    enum POSSIBLE_CHILDREN {
        NONE, ///< No, it is always a leaf node.
        ONE, ///< Yes, but only one.
        TWO, ///< One or two, but no more.
        MANY ///< As many as you like.
    };
    FrameSource_impl(enum FLAGS flags, enum POSSIBLE_CHILDREN pos_child, std::string unique_name);
    virtual ~FrameSource_impl();

    /// Returns a FramePtr if a valid frame is available or NULL if done
    /// pb is the playback we are using.
    /// NB will throw an assertion if pb is not the one for this object.
    virtual FramePtr nextFrame(PlaybackImplPtr pb) = 0;
    /// Returns the number of frames that can be generated by this node.
    virtual size_t frames () = 0;
    /// Returns the current frame number within this node.
    /// @param p is the Playback that data should be produced for.
    virtual size_t pos(PlaybackImplPtr p) = 0;
    /// Reset the playback state.
    /// @param p is the Playback that should be reset.
    /// Derive from this to produce frame generators or effects, overload nextFrame to
    /// produce whatever frame is appropriate (direct copy from a child is possible), and
    /// overload newPlayback to produce an object derived from Playback to hold the per
    /// playback state of this object
    virtual void reset(PlaybackImplPtr p) = 0;


    /// Non virtual functions common to all framesources

    /// Create a new playback tree structure from this node and all its children
    PlaybackImplPtr createPlayback();

    /// Access a child of this object.
    /// @param pos is the index of the child to be returned, range is [0,numChildren()-1].
    SourceImplPtr child(unsigned int pos) const;
    /// Add a child node to this object (also creating copies of all playbacks in the parent).
    /// @param child is a pointer to the the child node derived from this class.
    /// @param pos is the index where the child should be inserted.
    bool addChild(SourceImplPtr child, int pos = -1);
    /// Delete a child node and all of its subtree.
    /// @param pos is the position of the child to delete.
    bool deleteChild (unsigned int pos);
    /// @returns the number of direct children this object has
    unsigned int numChildren() const;
    /// @returns the allowable number of children // NONE, ONE, TWO or MANY
    enum POSSIBLE_CHILDREN numPossibleChildren () const;
    /// Set the description of this object to string des
    std::string getDescription() const;
    void setDescription (std::string des);
    /// File IO
    static SourceImplPtr loadFrames (QXmlStreamReader* e);
    void saveFrames(QXmlStreamWriter* w);
    /// IO to and from strings
    std::string toString();
    static SourceImplPtr fromString(const std::string s);

    /// Copying trees of these things
    SourceImplPtr clone();

    /// Functions to manage the generation of new frame objects.

    /// Call this to register your framesource with the system
    /// @param name is the globally unique name of your class and must match that found in
    /// the name field of the object derived from FrameSource.
    /// @param generator is the static generator function that will return a pointer to your class.
    static void registerFrameGen (const std::string name, SourceImplPtr (*generator)());
    /// This is the way most of the system gets new FrameSources of whatever type.
    /// @param name is the name of the desired FrameSource derived object.
    static SourceImplPtr newSource (const std::string name);
    /// Returns a listing of all frame generator types known to the system.
    static std::vector<std::string> enemerateFrameGenTypes();
    /// Check if framesource type s is valid.
    static bool exists (const std::string s);

    /// GUI creation method
    virtual FrameGui * controls (QWidget *parent) = 0;
    std::string getName() const;

protected:
    virtual void save (QXmlStreamWriter *w) = 0;
    virtual void load (QXmlStreamReader *r) = 0;

private:
    FrameSource_impl();
    /// Create a new object derived from a Playback object that has the relevant per instance data
    /// for this playback type.
    virtual PlaybackImplPtr newPlayback() = 0;
    /// Reimplement this to be something which copies the stored data from this framesource to
    /// f which will allready have the apropriate type.
    virtual  void copyDataTo (SourceImplPtr f) const = 0;
    // A unique identifier used in saving and loading these objects
    std::string name;
    std::string description;
    POSSIBLE_CHILDREN possibleChildren;
    FLAGS flags;
    std::vector <SourceImplPtr> children;
};

/// Specialise this to produce an object that stores all the per playback instance data your frame generator needs.
/// This should NOT be used for data that is common to every time a specific object is asked for a frame
/// which should be in a specialisation of a FrameSource_impl instead.

class Playback_impl
{
public:
    Playback_impl();
    virtual ~Playback_impl();
    PlaybackImplPtr child (unsigned int pos);
    bool addChild (PlaybackImplPtr child, int pos=-1);
    unsigned int numChildren() const
    {
      return children.size();
    }
private:
    std::vector<PlaybackImplPtr> children;
};


class FrameGui : public QGroupBox
{
    Q_OBJECT
public:
    FrameGui (QWidget * parent) : QGroupBox (parent) {};
    virtual ~FrameGui();

    virtual const QIcon * icon() = 0;
signals:
    void graphicsChanged();

};

#endif
