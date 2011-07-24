/* editparams.cpp is part of lucifer a laser show controller.

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

#include <QtGui>
#include <boost/make_shared.hpp>
#include "editparams.h"
#include <assert.h>
#include "framesequencer.h"
#include "log.h"

ShowTreeWidget::ShowTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    // Set up the drag and drop handling
    setColumnCount(2);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
}

bool ShowTreeWidget::dropMimeData ( QTreeWidgetItem * p, int index, const QMimeData * data, Qt::DropAction)
{
    ShowTreeWidgetItem *parent = (ShowTreeWidgetItem *) p;
		if (!parent) { // drop onto the root
			return false;
		}
		slog()->infoStream()<<"Dropping onto editor";
		if ((parent->data->numPossibleChildren() == 1) && (parent->data->numChildren() ==1)){
			slog()->infoStream() << "Parent allows maximum of 1 child so we need to add a sequencer";
			// We must add a sequencer to allow more child nodes.
			SourceImplPtr sequence = boost::make_shared<FrameSequencer>();
			// Splice the child as the new child of the sequencer
			ShowTreeWidgetItem *child = (ShowTreeWidgetItem*) parent->takeChild(0);
			SourceImplPtr t = child->data;
			child->data->deleteChild(0);
			sequence->addChild(t);
			parent->data->deleteChild(0);
			// Hopefully sequence is the only thing containing a reference to the old framesource now
			parent->populateTree(sequence);
			//sequence->addChild(child);
			// and the sequencer becomes the new child.
			//parent->data->deleteChild(0);
			//parent->removeChild(0);
			//sequence->data->addChild(child->data());
			//parent->populateTree(sequence);
			//parent->addChild(child);
			//parent->data =
			//parent = sequence;
			// Parent is now the sequencer and can have multiple children.
		}
    if ((parent->data->numPossibleChildren() == FrameSource_impl::MANY) ||
            ((parent->data->numPossibleChildren() == FrameSource_impl::TWO) && (parent->data->numChildren() <= 1)) ||
            ((parent->data->numPossibleChildren() == FrameSource_impl::ONE) && (parent->data->numChildren() == 0))) {
        slog()->infoStream()<<"Drop is acceptable";
        // We can accept this drop
        std::string d((char *) data->data("Text/FrameSource").constData());
        SourceImplPtr f = FrameSource_impl::fromString(d);
        if (f) {
            ShowTreeWidgetItem *n = new ShowTreeWidgetItem;
            n->populateTree (f);
            parent->insertChild(index,n);
            parent->data->addChild(f,index);
        }
        return true;
    }
    slog ()->infoStream()<<"Parent had too many children";
    return false;
}

QStringList ShowTreeWidget::mimeTypes () const
{
    QStringList l;
    l.append(QString("Text/FrameSource"));
    return l;
}

QMimeData * ShowTreeWidget::mimeData (const QList<QTreeWidgetItem *>  items ) const
{
    slog()->infoStream()<<"Started dragging from editor";
    QMimeData *mimeData = new QMimeData;
		ShowTreeWidgetItem *it = (ShowTreeWidgetItem*)items.first();
    std::string text = it->data->toString();
    // now setup the mime type to hold this data
    QByteArray arr (text.c_str(),text.size());
    mimeData->setData("Text/FrameSource", arr);
    return mimeData;
}

void ShowTreeWidgetItem::populateTree(SourceImplPtr f)
{
    assert (f);    
    setText(0,QString().fromStdString(f->getName()));
    FrameGui * c = f->controls(NULL);
    if (c){
	const QIcon * i = c->icon();
	setIcon(0,*i);
	delete i;
	delete c;
    }
    data = f;
    ShowTreeWidgetItem *op = NULL;
    for (unsigned int i = 0; i < f->numChildren(); i ++) {
        ShowTreeWidgetItem *cp = new ShowTreeWidgetItem (this,op);
        op = cp;
        cp->populateTree(f->child(i));
    }
}

ParameterEditor::ParameterEditor(QWidget* parent) :
        QDialog (parent)
{
    setAutoFillBackground(true);
    setGeometry(0,0,600,600);
    pixmapLabel = new QLabel(this);
    setWindowTitle("Editor");
    controls = new QWidget (this);
    tree = new ShowTreeWidget (this);
    tree->setIconSize(QSize(48,48));
    tree->setHeaderLabel("Source");
    connect (tree,SIGNAL(itemClicked(QTreeWidgetItem *, int)),this,SLOT(itemClickedData(QTreeWidgetItem *, int)));
    connect (tree,SIGNAL(itemSelectionChanged()),this,SLOT(selectionChangedData()));
    root = NULL;
    available = new QListWidget (this);
    std::vector<std::string> fn = FrameSource_impl::enemerateFrameGenTypes();
    for (unsigned int i=0; i < fn.size(); i++){
	QListWidgetItem *it = new QListWidgetItem(tr(fn[i].c_str()), available);     
    }
    hbox = new QHBoxLayout(this);
    hbox->addWidget(tree);
    hbox->addWidget (controls);
    hbox->addWidget(available);
    load (SourceImplPtr());
    setLayout(hbox);
    show ();
}

ParameterEditor::~ParameterEditor()
{
    fs.reset();
    delete root;
}

void ParameterEditor::load(SourceImplPtr f)
{
    if (!f) return;
    fs = f;//->clone();
    if (root) delete root;
    root = NULL;
    tree->clear();
    //pixmapLabel->setPixmap(fs->icon()->pixmap(pixmapLabel->size()));
    if (fs) {
        setWindowTitle(QString().fromStdString(fs->getName()));
    } else {
        setWindowTitle("Editor");
    }
    //FrameRootPtr r = boost::make_shared<FrameRoot>();
		//if (fs){
		//r->addChild(fs);
		//}
    root = populateTree (root,fs);
    tree->addTopLevelItem(root);
    hbox->removeWidget(controls);
    controls->deleteLater();
		if (fs){
			controls = fs->controls(this);
		}
    hbox->addWidget(controls);
    update();
}

// Recursive build of a treewidget data structure
ShowTreeWidgetItem * ParameterEditor::populateTree(ShowTreeWidgetItem *p, SourceImplPtr f)
{
    assert (f);
    if (!p) {
        p = new ShowTreeWidgetItem;
	setStyleSheet("QTreeWidget::item{ height: 50px;}");
    }
    p->populateTree(f);
    return p;
}

void ParameterEditor::itemClickedData (QTreeWidgetItem *item, int)
{
    ShowTreeWidgetItem *tw = static_cast<ShowTreeWidgetItem*>(item);
    assert (tw);
    SourceImplPtr f = tw->data;
		if (f){
			fs = f;
			controls->hide();
			hbox->removeWidget(controls);
			controls->deleteLater();
			controls = fs->controls(this);
			hbox->addWidget(controls);
			controls->show();
			update();
		}
}

void ParameterEditor::selectionChangedData()
{
    QList<QTreeWidgetItem *> selected = tree->selectedItems();
    if (selected.size()) {
        itemClickedData(selected[0],0);
    }
}
