
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2016 Igor Mironchik

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Qt include.
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

// Chess include.
#include "game.hpp"
#include "board.hpp"
#include "signals.hpp"


int main( int argc, char ** argv )
{
	QGuiApplication app( argc, argv );
	QQmlApplicationEngine engine;
	Chess::Board board;
	Chess::Signals sigs;

	engine.rootContext()->setContextProperty( "chessBoard", &board );
	engine.rootContext()->setContextProperty( "game", &sigs );

	engine.load( QUrl( "qrc:/main.qml" ) );

	if( engine.rootObjects().isEmpty() )
		return -1;

	try {
		Chess::Game game( engine.rootObjects().first(), board, sigs );

		return app.exec();
	}
	catch( const Chess::Error & )
	{
		return -1;
	}
}
