/*
 *
 *  Copyright (c) 2022
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

#include "versionInfo.h"
#include "context.hpp"
#include "tabmanager.h"
#include "mainwindow.h"

versionInfo::versionInfo( Ui::MainWindow&,const Context& ctx ) :
	m_ctx( ctx ),
	m_network( m_ctx.network() ),
	m_checkForUpdates( m_ctx.Settings().checkForUpdates() )
{
}

void versionInfo::checkEnginesUpdates( const std::vector< engines::engine >& engines,bool na ) const
{
	class meaw : public versionInfo::idone
	{
	public:
		meaw( const Context& t ) : m_ctx( t )
		{
		}
		void operator()() override
		{
			m_ctx.TabManager().init_done() ;
		}
	private:
		const Context& m_ctx ;
	} ;

	this->check( { { engines,utility::sequentialID() },
		     { m_ctx.Settings().showVersionInfoWhenStarting(),false },
		     { util::types::type_identity< meaw >(),m_ctx },na } ) ;
}

void versionInfo::log( const QString& msg,int id ) const
{
	m_ctx.logger().add( msg,id ) ;
}

void versionInfo::done( versionInfo::extensionVersionInfo vInfo ) const
{
	QStringList m ;

	QStringList mm ;

	QString s = "%1: %2\n%3: %4\n%5: %6\n" ;

	auto nt = QObject::tr( "Engine Name" ) ;
	auto it = QObject::tr( "Installed Version" ) ;
	auto lt = QObject::tr( "Latest Version" ) ;

	vInfo.report( [ & ]( const QString& name,const QString& iv,const QString& lv ){

		mm.append( name ) ;
		m.append( s.arg( nt,name,it,iv,lt,lv ) ) ;
	} ) ;

	if( m.size() ){

		m_ctx.mainWindow().setTitle( QObject::tr( "There Is An Update For " ) + mm.join( ", " ) ) ;

		auto s = QObject::tr( "Update Found" ) ;

		m_ctx.logger().add( s + "\n" + m.join( "\n" ),utility::sequentialID() ) ;
	}
}

void versionInfo::done( printVinfo vinfo ) const
{
	if( vinfo.hasNext() ){

		this->check( vinfo.next() ) ;
	}else{
		m_ctx.TabManager().enableAll() ;

		vinfo.reportDone() ;
	}
}

void versionInfo::updatesResult( versionInfo::cEnginesUpdates m,const utils::qprocess::outPut& r ) const
{
	if( r.success() ){

		const auto& engine = m.vInfo.engine() ;

		util::version iv = engine.setVersionString( r.stdOut ) ;

		auto infov = m.vInfo.move() ;

		infov.append( engine.name(),std::move( iv ),std::move( m.lv.version ) ) ;

		if( infov.hasNext() ){

			return this->checkForEnginesUpdates( infov.next() ) ;
		}else{
			this->done( infov.move() ) ;
		}
	}else{
		if( m.vInfo.hasNext() ){

			return this->checkForEnginesUpdates( m.vInfo.next() ) ;
		}else{
			this->done( m.vInfo.move() ) ;
		}
	}
}

void versionInfo::checkForEnginesUpdates( versionInfo::extensionVersionInfo vInfo,
					  const utils::network::reply& reply ) const
{
	auto lv = vInfo.engine().versionInfoFromGithub( utility::networkReply( m_ctx,reply ).data() ) ;

	if( !lv.version.valid() ){

		if( vInfo.hasNext() ){

			return this->checkForEnginesUpdates( vInfo.next() ) ;
		}else{
			return this->done( vInfo.move() ) ;
		}
	}

	const auto& engine = vInfo.engine() ;

	engines::engine::exeArgs::cmd cmd( engine.exePath(),{ engine.versionArgument() } ) ;

	auto mm = QProcess::ProcessChannelMode::MergedChannels ;

	utility::setPermissions( cmd.exe() ) ;

	cEnginesUpdates ceu{ vInfo.move(),lv.move() } ;

	utils::qprocess::run( cmd.exe(),cmd.args(),mm,ceu.move(),this,&versionInfo::updatesResult ) ;
}

void versionInfo::checkForEnginesUpdates( versionInfo::extensionVersionInfo vInfo ) const
{
	const auto& engine = vInfo.engine() ;

	const auto& url = engine.downloadUrl() ;

	if( url.isEmpty() ){

		if( vInfo.hasNext() ){

			return this->checkForEnginesUpdates( vInfo.next() ) ;
		}else{
			return this->done( vInfo.move() ) ;
		}
	}

	if( engine.name().contains( "yt-dlp" ) && engine.name() != "yt-dlp" ){

		if( vInfo.hasNext() ){

			return this->checkForEnginesUpdates( vInfo.next() ) ;
		}else{
			return this->done( vInfo.move() ) ;
		}
	}

	m_network.get( url,[ this,vInfo = vInfo.move() ]( const utils::network::reply& reply ){

		this->checkForEnginesUpdates( vInfo.move(),reply ) ;
	} ) ;
}

void versionInfo::check( versionInfo::printVinfo vinfo ) const
{
	const auto& engine = vinfo.engine() ;

	auto m = vinfo.setAfterDownloading( false ) ;

	if( engine.validDownloadUrl() && networkAccess::hasNetworkSupport() ){

		if( engine.backendExists() ){

			if( m || vinfo.show() ){

				this->printVersion( vinfo.move() ) ;
			}else{
				this->done( vinfo.move() ) ;
			}
		}else{
			if( vinfo.networkAvailable() ){

				auto ee = vinfo.showVersionInfo() ;

				m_network.download( this->wrap( vinfo.move() ),ee ) ;
			}else{
				this->done( vinfo.move() ) ;
			}
		}
	}else{
		if( engine.backendExists() ){

			if( vinfo.show() || m ){

				this->printVersion( vinfo.move() ) ;
			}else{
				this->done( vinfo.move() ) ;
			}
		}else{
			if( vinfo.show() ){

				auto m = QObject::tr( "Failed to find version information, make sure \"%1\" is installed and works properly" ).arg( engine.name() ) ;

				this->log( m,vinfo.iter().id() ) ;
			}else{
				this->done( vinfo.move() ) ;
			}
		}
	}
}

void versionInfo::checkForUpdates() const
{
	auto url = "https://api.github.com/repos/mhogomchungu/media-downloader/releases/latest" ;

	m_network.get( url,[ this ]( const utils::network::reply& reply ){

		utility::networkReply nreply( m_ctx,reply ) ;

		if( reply.success() ){

			QJsonParseError err ;

			auto e = QJsonDocument::fromJson( nreply.data(),&err ) ;

			if( err.error == QJsonParseError::NoError ){

				auto lv = e.object().value( "tag_name" ).toString() ;
				auto iv = utility::runningVersionOfMediaDownloader() ;

				versionInfo::extensionVersionInfo vInfo = m_ctx.Engines().getEnginesIterator() ;

				vInfo.append( m_ctx.appName(),iv,lv ) ;

				this->checkForEnginesUpdates( vInfo.move() ) ;
			}else{
				m_ctx.logger().add( err.errorString(),utility::sequentialID() ) ;

				this->checkForEnginesUpdates( m_ctx.Engines().getEnginesIterator() ) ;
			}
		}
	} ) ;
}

void versionInfo::checkMediaDownloaderUpdate( int id,
					      const QByteArray& data,
					      const std::vector< engines::engine >& engines,
					      bool hasNetworkAccess ) const
{
	QJsonParseError err ;

	auto e = QJsonDocument::fromJson( data,&err ) ;

	if( err.error == QJsonParseError::NoError ){

		auto lvs = e.object().value( "tag_name" ).toString() ;

		util::version lv = lvs  ;
		util::version iv = utility::runningVersionOfMediaDownloader() ;

		versionInfo::extensionVersionInfo vInfo = m_ctx.Engines().getEnginesIterator() ;

		vInfo.append( m_ctx.appName(),iv,lv ) ;

		if( lv.valid() && iv < lv ){

			this->log( QObject::tr( "Newest Version Is %1, Updating" ).arg( lvs ),id ) ;

			class meaw : public networkAccess::status
			{
			public:
				meaw( const std::vector< engines::engine >& m,
				      const versionInfo& v,
				      bool hasNetworkAccess,
				      int id ) :
					m_engines( m ),
					m_parent( v ),
					m_hasNetworkAccess( hasNetworkAccess ),
					m_id( id )
				{
				}
				void done()
				{
					m_parent.checkEnginesUpdates( m_engines,m_hasNetworkAccess ) ;
				}
				int id()
				{
					return m_id ;
				}
			private:
				const std::vector< engines::engine >& m_engines ;
				const versionInfo& m_parent ;
				bool m_hasNetworkAccess ;
				int m_id ;
			} ;

			networkAccess::Status s{ util::types::type_identity< meaw >(),engines,*this,hasNetworkAccess,id } ;

			m_network.updateMediaDownloader( s.move() ) ;
		}else{
			this->checkEnginesUpdates( engines,hasNetworkAccess ) ;
		}
	}else{
		m_ctx.logger().add( err.errorString(),id ) ;

		this->checkEnginesUpdates( engines,hasNetworkAccess ) ;
	}
}

void versionInfo::checkMediaDownloaderUpdate( const std::vector< engines::engine >& engines ) const
{
	if( !m_ctx.Settings().showVersionInfoWhenStarting() || utility::platformIsNOTWindows() ){

		return this->checkEnginesUpdates( engines,true ) ;
	}

	m_ctx.TabManager().disableAll() ;

	auto id = utility::sequentialID() ;

	this->log( QObject::tr( "Checking installed version of %1" ).arg( m_ctx.appName() ),id ) ;

	this->log( QObject::tr( "Found version: %1" ).arg( utility::runningVersionOfMediaDownloader() ),id ) ;

	if( !m_checkForUpdates ){

		return this->checkEnginesUpdates( engines,true ) ;
	}

	auto url = "https://api.github.com/repos/mhogomchungu/media-downloader/releases/latest" ;

	m_network.get( url,[ this,id,&engines ]( const utils::network::reply& reply ){

		utility::networkReply nreply( m_ctx,reply ) ;

		if( reply.success() ){

			this->checkMediaDownloaderUpdate( id,nreply.data(),engines,true ) ;
		}else{
			this->checkEnginesUpdates( engines,false ) ;
		}
	} ) ;
}

networkAccess::iterator versionInfo::wrap( printVinfo m ) const
{
	class meaw : public networkAccess::iter
	{
	public:
		meaw( printVinfo m ) : m_vInfo( std::move( m ) )
		{
		}
		const engines::engine& engine() override
		{
			return m_vInfo.engine() ;
		}
		bool hasNext() override
		{
			return m_vInfo.hasNext() ;
		}
		void moveToNext() override
		{
			m_vInfo = m_vInfo.next() ;
		}
		void reportDone() override
		{
			m_vInfo.reportDone() ;
		}
		void failed() override
		{
			m_vInfo.failed() ;
		}
		const engines::Iterator& itr() override
		{
			return m_vInfo.iter() ;
		}
	private:
		printVinfo m_vInfo ;
	};

	return { util::types::type_identity< meaw >(),std::move( m ) } ;
}

void versionInfo::printVersion( versionInfo::printVinfo vInfo ) const
{
	m_ctx.TabManager().disableAll() ;

	const auto& engine = vInfo.engine() ;

	auto id = utility::sequentialID() ;

	this->log( QObject::tr( "Checking installed version of %1" ).arg( engine.name() ),id ) ;

	if( engine.name().contains( "yt-dlp" ) && engine.name() != "yt-dlp" ){

		const auto& e = m_ctx.Engines().getEngineByName( "yt-dlp" ) ;

		if( e.has_value() ){

			const auto& version = e.value().versionInfo() ;

			if( version.valid() ){

				this->log( QObject::tr( "Found version: %1" ).arg( version.toString() ),id ) ;

				return this->done( std::move( vInfo ) ) ;
			}
		}
	}

	engines::engine::exeArgs::cmd cmd( engine.exePath(),{ engine.versionArgument() } ) ;

	if( !m_ctx.debug().isEmpty() ){

		auto exe = "cmd: \"" + cmd.exe() + "\"" ;

		for( const auto& it : cmd.args() ){

			exe += " \"" + it + "\"" ;
		}

		m_ctx.logger().add( exe,id ) ;
	}

	auto mm = QProcess::ProcessChannelMode::MergedChannels ;

	utility::setPermissions( cmd.exe() ) ;

	versionInfo::pVInfo v{ vInfo.move(),id } ;

	utils::qprocess::run( cmd.exe(),cmd.args(),mm,v.move(),this,&versionInfo::printVersionP ) ;
}

void versionInfo::printVersionP( versionInfo::pVInfo pvInfo,const utils::qprocess::outPut& r ) const
{
	const auto& engine = pvInfo.engine() ;

	if( r.success() ){

		auto m = engine.setVersionString( r.stdOut ) ;

		this->log( QObject::tr( "Found version: %1" ).arg( m ),pvInfo.id() ) ;

		const auto& url = engine.downloadUrl() ;

		const auto& vInfo = pvInfo.printVinfo() ;

		if( !url.isEmpty() && m_checkForUpdates && vInfo.networkAvailable() ){

			return m_network.get( url,pvInfo.move(),this,&versionInfo::printVersionN ) ;
		}
	}else{
		auto m = QObject::tr( "Failed to find version information, make sure \"%1\" is installed and works properly" ) ;

		this->log( m.arg( engine.name() ),pvInfo.id() ) ;

		engine.setBroken() ;
	}

	this->done( pvInfo.movePrintVinfo() ) ;
}

void versionInfo::printVersionN( versionInfo::pVInfo pvInfo,const utils::network::reply& reply ) const
{
	const auto& engine = pvInfo.engine() ;

	auto ss = utility::networkReply( m_ctx,reply ).data() ;

	const auto& versionOnline = engine.versionInfoFromGithub( ss ) ;

	const auto& installedVersion = engine.versionInfo() ;

	const auto& version = versionOnline.version ;

	if( version.valid() && installedVersion.valid() && installedVersion < version ){

		auto m = versionOnline.stringVersion ;

		this->log( QObject::tr( "Newest Version Is %1, Updating" ).arg( m ),pvInfo.id() ) ;

		auto ee = pvInfo.printVinfo().showVersionInfo() ;

		m_network.download( this->wrap( pvInfo.movePrintVinfo() ),ee ) ;
	}else{
		m_ctx.TabManager().enableAll() ;

		this->done( pvInfo.movePrintVinfo() ) ;
	}
}

versionInfo::idone::~idone()
{
}
