/*Copyrignt 2010 Dan Mills <dmills@exponent.myzen.co.uk>

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

/* $Id: log.cpp 16 2010-06-26 22:35:55Z dmills $ */

#include "log.h"

void Logger::setFileName (std::string filename)
{
    delete app;
    app = new log4cpp::FileAppender("FileAppender",filename);
    app->setLayout (layout);
    cat->setAppender(app);
}



Logger::Logger(std::string filename)
{
    app = new log4cpp::FileAppender("FileAppender",filename);

    layout = new log4cpp::BasicLayout();
    cat = &log4cpp::Category::getInstance("");
    app->setLayout(layout);
    cat->setAdditivity(false);
    cat->setAppender(app);
    cat->setPriority(log4cpp::Priority::DEBUG);
};

Logger::~Logger()
{
    delete cat;
    delete app;
}

static Logger *l = NULL;

log4cpp::Category * slog()
{
    if (!l) {
        l = new Logger (std::string("lucifer.log"));
    }
    return l->cat;
}

void setLogFileName (std::string fn)
{
    if (!l) {
        l = new Logger (fn);
    } else l->setFileName(fn);
}

