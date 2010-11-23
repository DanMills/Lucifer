/*framesource.h is part of lucifer a laser show controller.

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
/* $Id: framesource.h 22 2010-07-24 15:01:51Z dmills $ */

#ifndef FRAMESOURCE_INC
#define FRAMESOURCE_INC

#include <QtGui>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

#include "point.h"
#include "frame.h"


///FrameSources derive from this to produce specialised data for their own use.

/// Specialise this to produce an object that stores all the per playback instance data your frame generator needs.
/// This should NOT be used for data that is common to every time a specific object is asked for a frame.

class PlaybackData
{
public:
    PlaybackData() {};
    virtual ~PlaybackData() {};
};

typedef boost::shared_ptr<PlaybackData> PlaybackDataPtr;

typedef  int Playback;

class FrameGui;
class FrameSource;

typedef boost::shared_ptr<FrameSource> FrameSourcePtr;
typedef boost::shared_ptr<const FrameSource> ConstFrameSourcePtr;

/// This is the underlying virtual base class for frame manipulators of all types.

/// A show consists of a tree of objects derived from these things where the top ones data (Playback)
/// method is called repeatedly to obtain a series of frames.
/// Objects derived from this should return a null FramePtr when they have no further frames available.

class FrameSource
{
public:
    /// Frame generator IO types flags, bitwise or together as appropriate.
    enum FLAGS {
        NOTHING=0,
        MAKES_FRAMES=1,///< Object generates laser frame data.
        MAKES_DMX=2, ///< Object generates DMX512 control data.
        MAKES_MIDI=4, ///< Object generates MIDI.
        MAKES_GPIO=8, ///< Object generates Generap Purpose Outputs.
        USES_GPI=16,	///< Object listens to General purpose input.
        USES_MIDI=32, ///< Object listens to MIDI messages.
        USES_DMX=64,	///< Object listens to DMX data.
        USES_FRAMES=128 ///< Object listeds to externally supplied frames.
    };
    /// Can this object support child nodes.
    enum POSSIBLE_CHILDREN {
        NONE, ///< No, it is always a leaf node.
        ONE, ///< Yes, but only one.
        TWO, ///< One or two, but no more.
        MANY ///< As many as you like.
    };
    /// Unique name used to identify the type of object when loading from file.
    /// Must not contain spaces as that confuses the XML parser.
    std::string name;
    /// Optional descriptive text
    std::string description;
    /// Call this from the initialiser list of your effects constructor.
    FrameSource (enum FLAGS flags, enum POSSIBLE_CHILDREN pos_child);
    virtual ~FrameSource ();
    /// This returns either a FramePtr to your generated frame or a NULL FramePtr if
    /// you are done.
    /// @param p is the Playback that data should be produced for.
    virtual FramePtr data(Playback p) = 0;
    /// Returns the number of frames that can be generated by this node.
    virtual size_t frames () = 0;
    /// Returns the current frame number within this node.
    /// @param p is the Playback that data should be produced for.
    virtual size_t pos(Playback p) = 0;
    /// Reset the playback state.
    /// @param p is the Playback that should be reset.
    virtual void reset(Playback p) = 0;
		/// Reimplement this to be something which copies the stored data from this framesouce to
		/// f which will allready have the apropriate type.
		virtual  void copyDataTo (FrameSourcePtr f) const = 0;

    /// This is overloaded to create whatever private per playback data is needed.
    /// The object returned from this should be capable of being dynamic cast to your
    /// PlaybackData derived class.
    virtual PlaybackDataPtr createPlaybackData () = 0;

    /// This maps from a Playback to a stored instance of the PlaybackData derived object
    /// produced by createPlaybackData().
    PlaybackData * getPlayback (Playback p);

    /// Create a new unique playback, this eventually recursively calls the createPlaybackData() data
    /// method in the derived class as well as in any children so that the entire tree has the new
    /// playback.
    Playback createPlayback ();
    /// Recursively delete the playback from all nodes in this (sub)tree.
    /// @param p is the Playback that should be deleted.
    void deletePlayback(Playback p);
    /// Create a specific playback ID in this node and all children.
    /// Will fail an assert if this allready exists.
    /// This is used internally to propagate a new playback to all child nodes.
    /// @param p is the Playback that should be created.
    void createPlayback (Playback p);
    /// Erase all playbacks in this (sub) tree, mainly used when merging two trees.
    void erasePlaybacks();

		/// returns a copy of the source tree.
		virtual FrameSourcePtr clone() const;

    /// Access a child of this object.
    /// @param pos is the index of the child to be returned, range is [0,numChildren()-1].
    FrameSourcePtr child(unsigned int pos) const;
    /// Add a child node to this object (also creating copies of all playbacks in the parent).
    /// @param child is a pointer to the the child node derived from this class.
    /// @param pos is the index where the child should be inserted.
    bool addChild(FrameSourcePtr child, unsigned int pos);
    /// Add a child node to the end of the vector of child sources.
    /// @param child is a pointer to the the child node derived from this class.
    bool addChild(FrameSourcePtr child);
    /// Delete a child node and all of its subtree.
    /// @param pos is the position of the child to delete.
    bool deleteChild (unsigned int pos);
    /// Delete all child nodes.
    void eraseChildren();
    /// @returns the number of direct children this object has
    unsigned int numChildren() const;
    /// @returns the allowable number of children // NONE, ONE, TWO or MANY
    enum POSSIBLE_CHILDREN numPossibleChildren () const;

    virtual FrameGui * controls (QWidget *parent) = 0;

    static FrameSourcePtr loadFrames (QXmlStreamReader* e);
    void saveFrames(QXmlStreamWriter* w);
    static FrameSourcePtr fromString (std::string &s);
    std::string toString();
    /// Call this to register your framesource with the system
    /// @param name is the globally unique name of your class and must match that found in
    /// the name field of the object derived from FrameSource.
    /// @param generator is the static generator function that will return a pointer to your class.
    static void registerFrameGen (const std::string name, FrameSourcePtr (*generator)());
    /// This is the way most of the system gets new FrameSources of whatever type.
    /// @param name is the name of the desired FrameSource derived object.
    static FrameSourcePtr newSource (const std::string name);
    /// Returns a listing of all frame generator types known to the system.
    static std::vector<std::string> enemerateFrameGenTypes();
    /// Check if framesource type s is valid.
    static bool exists (const std::string s);

protected:
    // Overload these to load and save your setup
    virtual void save (QXmlStreamWriter *w) = 0;
    virtual void load (QXmlStreamReader *e) = 0;
private:
    FrameSource();
    // Used to create playback handles
    static Playback getUniqueHandle ();
    std::vector<FrameSourcePtr> children_;
    enum POSSIBLE_CHILDREN pos_children_;
    enum FLAGS flags_;
    std::map <Playback, PlaybackDataPtr> playbacks_;
};

class FrameGui : public QGroupBox
{
public:
    FrameGui (QWidget * parent) : QGroupBox (parent) {};
    virtual ~FrameGui();

    virtual const QIcon * icon() = 0;
};


#endif
