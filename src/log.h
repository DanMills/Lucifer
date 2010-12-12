// Shim on top of log4cpp logging library

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

#ifndef LOG_INCL
#define LOG_INCL

#include	<log4cpp/Category.hh>
#include	<log4cpp/FileAppender.hh>
#include	<log4cpp/BasicLayout.hh>

class Logger
{
public:
    Logger(std::string filename);
    ~Logger();
    log4cpp::Category * cat;
    void setFileName (std::string filename);
private:
    log4cpp::Appender * app;
    log4cpp::Layout * layout;

    Logger();
};
void setLogFileName (std::string fn);
log4cpp::Category * slog();

#endif
