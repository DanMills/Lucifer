/*main.cpp is part of lucifer a laser show controller.

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

/* $Id: main.cpp 19 2010-07-03 16:53:57Z dmills $ */

/*! \mainpage Lucifer Index Page
*
* \section intro_sec Introduction
*	Lucifer is a laser show control program written basically to target Linux and other Unix like systems,
* It may also be possible to compile on windows with only minor changes.
*
* The underlying metaphor for Lucifer is a tree structure consisting of sequencers, modifiers and frame sources,
* where leaf nodes actually provide the point data and higher level sequencers and modifiers do things to it before
* finally passing it to the output devices.
*
* \section intro_sec The frame Window
*
* \section install_sec Installation
*
* \subsection step1 Step 1: Opening the box
*
* etc...
*/







#include <string>
#include <iostream>
#include <unistd.h>

#include "buttonwindow.h"
#include "log.h"

static const std::string usage(" \
lucifer [-option] [-option]... [filename.lsf] [filename.ild(a)]\n\
\n\
Options are: \n\
  -l logfile sets the location of the system log file to logfile\n\
  -v log level sets the level of logging, valid options are \n\
    DEBUG (Very verbose), these files get big.\n\
    INFO (default), logs startup, what files are loaded, plus any errors.\n\
    ERROR, only log errors.\n\
  -n Do not reload the last used configuration.\n\
  -h Display this help screen then exit.\n\
  -p Display a list of loadable plugins and drivers then exit.\n\
If a filename is specified it will be loaded and if more then \none is specified they will all be loaded.\n\
Files are loaded into unused slots in the main grid window on a first \ncome basis.\n");

int main (int argc, char **argv)
{
    bool listPlugs = false;
    bool noReload = false;
    std::string logname;
    std::string loglevel("INFO");

    int c;
    opterr = 0;
    // FIXME GNU SPECIFIC
    char * wd = getcwd (NULL,0);
    logname = std::string(wd) + "lucifer.log";
    free (wd);
    while ((c = getopt (argc, argv, "l:v:nhp")) != -1) {
        switch (c) {
        case 'h':
            std::cout << usage;
            exit (0);
            break;
        case 'p':
            listPlugs = true;
            break;
        case 'n':
            noReload = true;
            break;
        case 'l':
            logname=std::string(optarg);
            break;
        case 'v':
            loglevel = std::string(optarg);
            std::cerr <<  loglevel <<std::endl;

            break;
        case '?':
            if (optopt == 'l') {
                std::cerr <<  "Option -l requires a filename argument." << std::endl;
                exit(1);
            } else if (optopt == 'v') {
                std::cerr <<  "Option -v requires a log level argument." << std::endl;
                exit(1);
            }	else if (isprint (optopt)) {
                std::cerr << "Unknown option " << optopt << std::endl;
                exit (1);
            } else {
                std::cerr << "Unknown option character" <<std::endl;
                exit (1);
            }
        default:
            abort ();
        }
    }
    setLogFileName(logname);
    // Figure out the logger options
    if (loglevel == std::string("DEBUG")) {
        slog()->setPriority(log4cpp::Priority::DEBUG);
    } else if (loglevel == std::string("INFO")) {
        slog()->setPriority(log4cpp::Priority::INFO);
    } else if (loglevel == std::string("ERROR")) {
        slog()->setPriority(log4cpp::Priority::ERROR);
    } else {
        std::cerr <<"Invalid logging level requested, choices are \n\
			DEBUG, INFO, ERROR.\n";
        exit (1);
    }

    std::vector<std::string> filenames;
    //The non option arguments are file names
    for (int index = optind; index < argc; index++) {
        std::string name(argv[index]);
        filenames.push_back(name);
    }
    QApplication app(argc, argv);
    // do the setup for using the QSettings methods to store geomentry and the like
    QCoreApplication::setOrganizationName("Exponential Software");
    QCoreApplication::setOrganizationDomain("exponent.myzen.co.uk");
    QCoreApplication::setApplicationName("Lucifer");
    slog()->info("Starting lucifer");
    ButtonWindow grid;
		for (unsigned int i=0; i < filenames.size(); i++){
			std::string fn = filenames[i];
			grid.loadFile (QString().fromStdString(fn));
		}
    return app.exec();
    return 0;
}


