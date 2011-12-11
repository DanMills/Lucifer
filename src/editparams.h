/*editparams.h is part of lucifer a laser show controller.

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


#ifndef EDIT_PARAMS
#define EDIT_PARAMS

#include <QtGui>
#include "framesource.h"
#include "displayframe.h"
#include "mime.h"

class ShowTreeWidgetItem;

class ShowTreeWidget : public QTreeWidget
{
public:
    ShowTreeWidget(QWidget* parent = 0);
    virtual ~ShowTreeWidget() {};
protected:
    bool dropMimeData ( QTreeWidgetItem * parent, int index, const QMimeData * data, Qt::DropAction action);
    QStringList mimeTypes () const;
    QMimeData * mimeData ( const QList<QTreeWidgetItem *>  items ) const;
};

class ShowTreeWidgetItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT
public:
    ShowTreeWidgetItem(int type = Type) :
            QTreeWidgetItem(type), icon(NULL) {};
    ShowTreeWidgetItem(const QStringList& strings, int type = Type) :
            QTreeWidgetItem(strings,type), icon(NULL) {};
    ShowTreeWidgetItem(QTreeWidget* view, int type = Type) :
            QTreeWidgetItem(view,type), icon(NULL) {};
    ShowTreeWidgetItem(QTreeWidget* view, const QStringList& strings, int type = Type) :
            QTreeWidgetItem(view,strings,type), icon(NULL) {};
    ShowTreeWidgetItem(QTreeWidget* view, QTreeWidgetItem* after, int type = Type) :
            QTreeWidgetItem(view,after,type), icon(NULL) {};
    ShowTreeWidgetItem(QTreeWidgetItem* parent, int type = Type) :
            QTreeWidgetItem(parent,type), icon(NULL) {};
    ShowTreeWidgetItem(QTreeWidgetItem* parent, const QStringList& strings, int type = Type) :
            QTreeWidgetItem(parent,strings,type),icon(NULL) {};
    ShowTreeWidgetItem(QTreeWidgetItem* parent, QTreeWidgetItem* after, int type = Type) :
            QTreeWidgetItem(parent,after,type),icon(NULL) {};
    ShowTreeWidgetItem(const QTreeWidgetItem& other) :
            QTreeWidgetItem(other), icon(NULL) {};
    ~ShowTreeWidgetItem () {
        data.reset();
    }
    void populateTree(SourceImplPtr f);
    SourceImplPtr data;
    ShowTreeWidgetItem * locateData (SourceImplPtr d);

private:
    const QIcon * icon;
private slots:
    void setIcon ();

};

class NodeSelectorWidget : public QListWidget
{
    Q_OBJECT
public:
    NodeSelectorWidget (QWidget *parent = NULL);
    virtual ~NodeSelectorWidget();
protected:
    virtual LaserMimeObject * mimeData ( const QList<QListWidgetItem *> items ) const;
    virtual QStringList mimeTypes () const;
    virtual Qt::DropActions supportedDropActions () const;
};




class ParameterEditor : public QDialog
{
    Q_OBJECT
public:
    ParameterEditor (QWidget *parent);
    ~ParameterEditor ();
protected:
    virtual void closeEvent (QCloseEvent *event);

public slots:
    void load (SourceImplPtr f);
signals:
    void modified ();


private:
    DisplayFrame *display;
    // Used to animate the display
    PlaybackPtr playback;
    QTimer *displayTimer;
    
    QLabel *pixmapLabel;
    QGridLayout *controlLayout;
    QHBoxLayout * hbox;
    ShowTreeWidget * tree;
    NodeSelectorWidget * available;
    ShowTreeWidgetItem *root;
    SourceImplPtr fs;
    // This is the control pane loaded from 
    // the frame we are looking at
    QWidget *controlWidget;
    QPushButton *playbutton;
    QPushButton *stopbutton;

    void updateControls (SourceImplPtr p);  
    ShowTreeWidgetItem * populateTree(ShowTreeWidgetItem *p, SourceImplPtr f);
private slots:
    void itemClickedData (QTreeWidgetItem *item, int column);
    void selectionChangedData();
    void updateDisplay ();
    void playButtonData (bool);
    void stopButtonData (bool);

};

#endif
