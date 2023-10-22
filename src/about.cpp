/*
 *
 *  Copyright (c) 2021
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "about.h"
#include "version.h"
#include "utility.h"

void about::enableAll()
{
}

void about::disableAll()
{
}

void about::resetMenu()
{
}

about::about( const Context& ctx ) : m_ctx( ctx )
{
	this->retranslateUi() ;
}

void about::retranslateUi()
{
	auto version   = QObject::tr( "Version" ) ;
	auto copyright = QObject::tr( "Copyright" ) ;
	auto license   = QObject::tr( "License" ) ;
	auto email     = QObject::tr( "Email" ) ;

	auto QtVersion = [ & ](){

		if( utility::platformIsLikeWindows() ){

			return QObject::tr( "Qt Version" ) + ": " QTVERSION "\n\n" ;
		}else{
			return QString() ;
		}
	}() ;

	auto banner1 = QObject::tr( "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version." ) ;
	auto banner2 = QObject::tr( "This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details." ) ;

	auto about = QString( "%1: %2\n\n%3%4: %5, Francis Banyikwa\n\n%6: mhogomchungu@gmail.com\n\n%7: GPLv2+\n\n" + banner1 + "\n\n" + banner2 ) ;

	auto vv = utility::aboutVersionInfo() ;

	auto m = about.arg( version,vv,QtVersion,copyright,COPYRIGHT,email,license ) ;

	m_ctx.Ui().TextLabelAbout->setText( m ) ;
}

void about::tabEntered()
{
}

void about::tabExited()
{
}

void about::exiting()
{
}

void about::init_done()
{
}
