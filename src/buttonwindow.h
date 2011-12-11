/*buttonwindow.h is part of lucifer a laser show controller.

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

#ifndef BUTTON_WINDOW_INC
#define BUTTON_WINDOW_INC

#include <QtGui>
#include "buttongrid.h"

// The source selector window
#include "engine.h"
#include "outputview.h"

class ButtonWindow : public QMainWindow
{
    Q_OBJECT

public:
    ButtonWindow (EnginePtr e, QWidget *parent = NULL);
public slots:
    void loadFile(const QString &fn);
    bool saveFile(const QString &fn);

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void openFile();
    bool saveFile ();
    bool saveAsFile();
    void modified(); // A child has been modified, need to re-save?
    void saved (); // File saved. 
    void loaded(); // File loaded.
    void setFullScreen();
    void clearFullScreen();
    void importFiles ();
    void userKill();
    void userRestart();
    void displayIOSetup();
    
    void sourcesSizeChanged (size_t);
    void status(QString text, int time);
    void selectionModeData (int);
    void stepModeData (int);
    void headSelectionChanged (int);
protected:
    void keyPressEvent(QKeyEvent *event);
private:
    ButtonWindow();
    std::vector<ButtonGrid *> grids;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *setupMenu;
    QMenu *helpMenu;
    
    // File menu actions
    QAction * openAct;
    QAction * saveAct;
    QAction * saveAsAct;
    QAction * exitAct;
    QAction * importAct;
    QAction * startAct;
    // Command buttons
    QAction * fullScreenAct;
    QAction * windowScreenAct;
    QAction * blankLasersAct;
    // Setup menu options
    QAction * ioSetupAct;
    

    QTabWidget * tabs;
    QToolBox * effects;
    

    QComboBox * selectionMode;
    QComboBox * stepMode;

    bool unsaved;
    QString fileName;
    QString pathName;

    EnginePtr engine;

    OutputView *outputs[MAX_HEADS];

    bool saveAs();
    void setCurrentFile(const QString &fn);
    bool maybeSave();
    void loadSettings();
    void storeSettings();
    void makeActions();
    void loadFrame();

};
#endif
