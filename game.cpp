
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

// Chess include.
#include "game.hpp"

// Qt include.
#include <QVariant>
#include <QString>
#include <QMetaObject>


namespace Chess {

//
// Error
//

Error::Error()
	:	std::runtime_error( "error" )
{
}

Error::~Error() noexcept
{
}


//
// Game
//

Game::Game( QObject * root, Board & board, Signals & s )
	:	m_board( board )
	,	m_root( root )
	,	m_boardObject( 0 )
	,	m_turnColor( Figure::White )
	,	m_selected( 0 )
	,	m_selectedX( 0 )
	,	m_selectedY( 0 )
	,	m_isChess( false )
	,	m_signals( s )
{
	m_boardObject = m_root->findChild< QObject* > ( QLatin1String( "board" ) );

	if( !m_boardObject )
		throw Error();

	connect( m_boardObject, SIGNAL( clicked( int, int ) ),
		this, SLOT( clicked( int, int ) ) );
	connect( m_boardObject, SIGNAL( hovered( int, int ) ),
		this, SLOT( hovered( int, int ) ) );
	connect( m_boardObject, SIGNAL( newGame() ),
		this, SLOT( newGame() ) );
}

Game::~Game()
{
}

void
Game::markCellsForMove( int x, int y, int dx, int dy, Move::Distance d,
	Figure * figure, Board & tmpBoard )
{
	x += dx;
	y += dy;

	int move = 0;

	if( d == Move::Any || d == Move::TwoFirstTime )
	{
		while( x >= 0 && x < 8 && y >= 0 && y < 8 )
		{
			if( !m_board.figures()[ y ][ x ] )
			{
				if( !isCheckAfterMove( x, y, figure, tmpBoard ) )
				{
					m_board.markBlue( x, y );

					m_possibleMoves.append( x * 10 + y );
				}
			}
			else
				break;

			x += dx;
			y += dy;
			++move;

			if( d == Move::TwoFirstTime && figure->isFirstMoveDone() )
				break;

			if( d == Move::TwoFirstTime && move == 2 )
				break;
		}
	}
	else if( x >= 0 && x < 8 && y >= 0 && y < 8 )
	{
		if( !m_board.figures()[ y ][ x ] )
		{
			if( !isCheckAfterMove( x, y, figure, tmpBoard ) )
			{
				m_board.markBlue( x, y );

				m_possibleMoves.append( x * 10 + y );
			}
		}
	}
}

void
Game::markCellForHit( int x, int y, int dx, int dy, Move::Distance d,
	Figure * figure, Board & tmpBoard )
{
	x += dx;
	y += dy;

	if( d == Move::Any )
	{
		while( x >= 0 && x < 8 && y >= 0 && y < 8 )
		{
			if( m_board.figures()[ y ][ x ] )
			{
				if( figure->color() != m_board.figures()[ y ][ x ]->color() )
				{
					if( !isCheckAfterMove( x, y, figure, tmpBoard ) )
					{
						m_board.markRed( x, y );

						m_possibleMoves.append( x * 10 + y );
					}
				}

				break;
			}

			x += dx;
			y += dy;
		}
	}
	else if( x >= 0 && x < 8 && y >= 0 && y < 8 )
	{
		if( m_board.figures()[ y ][ x ] &&
			figure->color() != m_board.figures()[ y ][ x ]->color() )
		{
			if( !isCheckAfterMove( x, y, figure, tmpBoard ) )
			{
				m_board.markRed( x, y );

				m_possibleMoves.append( x * 10 + y );
			}
		}
	}
}

void
Game::clearCellsColor()
{
	m_board.clearBlueRed();
}

void
Game::firstClick( int x, int y )
{
	Figure * figure = m_board.figures()[ y ][ x ];

	if( figure != 0 )
	{
		if( figure->color() == m_turnColor )
		{
			m_selected = figure;
			m_selectedX = x;
			m_selectedY = y;

			m_board.markBlue( x, y );

			const Figure::Moves moves = figure->moves();

			Board tmpBoard = m_board;

			// For each possibe move.
			for( int i = 0; i < 5; ++i )
			{
				for( int j = 0; j < 5; ++j )
				{
					int dx = 0;
					int dy = 0;

					if( figure->color() == Figure::White )
					{
						dx = j - 2;
						dy = i - 2;
					}
					else
					{
						dx = 2 - j;
						dy = 2 - i;
					}

					// Moves highlight.
					if( moves[ i ][ j ].types().testFlag( Move::Movement ) )
					{
						markCellsForMove( x, y, dx, dy,
							moves[ i ][ j ].dist(), figure, tmpBoard );
					}

					// Castling highlight.
					if( figure->type() == Figure::KingFigure &&
						!figure->isFirstMoveDone() && !m_isChess )
					{
						if( !m_board.figures()[ y ][ x - 1 ] &&
							!m_board.figures()[ y ][ x - 2 ] &&
							!m_board.figures()[ y ][ x - 3 ] &&
							m_board.figures()[ y ][ x - 4 ] &&
							!m_board.figures()[ y ][ x - 4 ]->isFirstMoveDone() )
						{
							m_possibleMoves.append( ( x - 3 ) * 10 + y );

							m_board.markBlue( x - 3, y );
						}

						if( !m_board.figures()[ y ][ x + 1 ] &&
							!m_board.figures()[ y ][ x + 2 ] &&
							m_board.figures()[ y ][ x + 3 ] &&
							!m_board.figures()[ y ][ x + 3 ]->isFirstMoveDone() )
						{
							m_possibleMoves.append( ( x + 2 ) * 10 + y );

							m_board.markBlue( x + 2, y );
						}
					}

					// Hit highlight.
					if( moves[ i ][ j ].types().testFlag( Move::Hit ) )
					{
						markCellForHit( x, y, dx, dy,
							moves[ i ][ j ].dist(), figure, tmpBoard );
					}
				}
			}
		}
	}
}

bool
Game::secondClick( int x, int y )
{
	if( m_possibleMoves.contains( x * 10 + y ) )
	{
		m_board.move( m_selectedX, m_selectedY, x, y );

		handleCastling( x, y, m_selected, m_board );

		m_selected->firstMoveDone();
		m_possibleMoves.clear();

		markTurnLabel();

		if( checkCheck() && isCheckMate() )
		{
			m_turnColor = Figure::White;
			m_isChess = false;
			m_possibleMoves.clear();
			m_selectedX = -1;
			m_selectedY = -1;

			emit m_signals.checkmate();
		}

		m_selected = 0;

		return true;
	}
	else
	{
		m_selected = 0;

		clearCellsColor();

		checkCheck();

		firstClick( x, y );

		return false;
	}
}

void
Game::markTurnLabel()
{
	if( m_turnColor == Figure::White )
	{
		m_turnColor = Figure::Black;

		QObject * turn = m_root->findChild< QObject* > (
			QLatin1String( "turn" ) );

		if( turn )
			turn->setProperty( "text", tr( "Black" ) );
	}
	else
	{
		m_turnColor = Figure::White;

		QObject * turn = m_root->findChild< QObject* > (
			QLatin1String( "turn" ) );

		if( turn )
			turn->setProperty( "text", tr( "White" ) );
	}
}

void
Game::handleCastling( int x, int y,
	Figure * figure, Board & board ) const
{
	Q_UNUSED( y )

	// Castling.
	if( figure->type() == Figure::KingFigure &&
		!figure->isFirstMoveDone() )
	{
		switch( figure->color() )
		{
			case Figure::White :
			{
				if( x == 1 )
					board.move( 0, 7, 2, 7 );
				else if( x == 6 )
					board.move( 7, 7, 5, 7 );
			}
				break;

			case Figure::Black :
			{
				if( x == 1 )
					board.move( 0, 0, 2, 0 );
				else if( x == 6 )
					board.move( 7, 0, 5, 0 );
			}
				break;

			default :
				break;
		}
	}
}

bool
Game::checkCheck()
{
	King * king = ( m_turnColor == Figure::Black ? m_board.blackKing() :
		m_board.whiteKing() );

	bool res = false;

	m_isChess = false;

	// For each figure.
	for( int x = 0; x < 8; ++x )
	{
		for( int y = 0; y < 8; ++y )
		{
			if( m_board.figures()[ y ][ x ] &&
				m_board.figures()[ y ][ x ]->color() != m_turnColor )
					if( markCheck( king, m_board.figures()[ y ][ x ] ) )
						res = true;
		}
	}

	return res;
}

bool
Game::markCheck( King * king, Figure * figure )
{
	// For each possible move.
	for( int i = 0; i < 5; ++i )
	{
		for( int j = 0; j < 5; ++j )
		{
			if( figure->moves()[ i ][ j ].types().testFlag( Move::Hit ) )
			{
				int dx = 0;
				int dy = 0;

				if( figure->color() == Figure::White )
				{
					dx = j - 2;
					dy = i - 2;
				}
				else
				{
					dx = 2 - j;
					dy = 2 - i;
				}

				QList< QPair< int, int > > turns;
				turns.append( qMakePair( figure->x(), figure->y() ) );

				int x = figure->x() + dx;
				int y = figure->y() + dy;

				turns.append( qMakePair( x, y ) );

				if( figure->moves()[ i ][ j ].dist() == Move::Any )
				{
					while( x >= 0 && x < 8 && y >= 0 && y < 8 )
					{
						if( m_board.figures()[ y ][ x ] == king )
						{
							m_isChess = true;

							for( auto & p : qAsConst( turns ) )
								m_board.markRed( p.first, p.second );

							return true;
						}
						else if( m_board.figures()[ y ][ x ] &&
							m_board.figures()[ y ][ x ]->type() != Figure::KnightFigure )
								break;

						x += dx;
						y += dy;

						turns.append( qMakePair( x, y ) );
					}
				}
				else if( figure->moves()[ i ][ j ].dist() != Move::No &&
					x >= 0 && x < 8 && y >= 0 && y < 8 )
				{
					if( m_board.figures()[ y ][ x ] == king )
					{
						m_isChess = true;

						for( auto & p : qAsConst( turns ) )
							m_board.markRed( p.first, p.second );

						return true;
					}
				}
			}
		}
	}

	return false;
}

bool
Game::isCheckAfterMove( int x, int y, Figure * figure, Board & tmpBoard ) const
{
	Figure * f = tmpBoard.figure( figure->index() );

	const int oldX = f->x();
	const int oldY = f->y();

	bool res = false;

	Figure * oldFigure = tmpBoard.figures()[ y ][ x ];

	tmpBoard.move( oldX, oldY, x, y );

	handleCastling( x, y, f, tmpBoard );

	King * king = ( f->color() == Figure::White ? tmpBoard.whiteKing() :
		tmpBoard.blackKing() );

	// For each figure.
	for( int x = 0; x < 8; ++x )
	{
		for( int y = 0; y < 8; ++y )
		{
			if( tmpBoard.figures()[ y ][ x ] &&
				tmpBoard.figures()[ y ][ x ]->color() != f->color() )
			{
				Figure * tmpFigure = tmpBoard.figures()[ y ][ x ];

				// For each possible move.
				for( int i = 0; i < 5; ++i )
				{
					for( int j = 0; j < 5; ++j )
					{
						if( tmpFigure->moves()[ i ][ j ].types().testFlag( Move::Hit ) )
						{
							int dx = 0;
							int dy = 0;

							if( tmpFigure->color() == Figure::White )
							{
								dx = j - 2;
								dy = i - 2;
							}
							else
							{
								dx = 2 - j;
								dy = 2 - i;
							}

							int x = tmpFigure->x() + dx;
							int y = tmpFigure->y() + dy;

							if( tmpFigure->moves()[ i ][ j ].dist() == Move::Any )
							{
								while( x >= 0 && x < 8 && y >= 0 && y < 8 )
								{
									if( tmpBoard.figures()[ y ][ x ] == king )
									{
										res = true;

										break;
									}
									else if( tmpBoard.figures()[ y ][ x ] &&
										tmpBoard.figures()[ y ][ x ]->type() !=
											Figure::KnightFigure )
												break;

									x += dx;
									y += dy;
								}
							}
							else if( x >= 0 && x < 8 && y >= 0 && y < 8 )
							{
								if( tmpBoard.figures()[ y ][ x ] == king )
									res = true;
							}
						}

						if( res )
							break;
					}

					if( res )
						break;
				}

				if( res )
					break;
			}
		}

		if( res )
			break;
	}

	tmpBoard.move( x, y, oldX, oldY );

	if( oldFigure )
		tmpBoard.figures()[ oldFigure->y() ][ oldFigure->x() ] = oldFigure;

	return res;
}

bool
Game::isCheckMate()
{
	Board tmpBoard = m_board;

	// For each figure.
	for( int x = 0; x < 8; ++x )
	{
		for( int y = 0; y < 8; ++y )
		{
			if( tmpBoard.figures()[ y ][ x ] &&
				tmpBoard.figures()[ y ][ x ]->color() == m_turnColor )
			{
				Figure * tmpFigure = tmpBoard.figures()[ y ][ x ];

				// For each possible move.
				for( int i = 0; i < 5; ++i )
				{
					for( int j = 0; j < 5; ++j )
					{
						int dx = 0;
						int dy = 0;

						if( tmpFigure->color() == Figure::White )
						{
							dx = j - 2;
							dy = i - 2;
						}
						else
						{
							dx = 2 - j;
							dy = 2 - i;
						}

						int x = tmpFigure->x() + dx;
						int y = tmpFigure->y() + dy;

						int move = 0;

						Move::Distance d = tmpFigure->moves()[ i ][ j ].dist();

						if( d == Move::Any || d == Move::TwoFirstTime )
						{
							while( x >= 0 && x < 8 && y >= 0 && y < 8 )
							{
								++move;

								if( tmpBoard.figures()[ y ][ x ] &&
									( !tmpFigure->moves()[ i ][ j ]
										.types().testFlag( Move::Hit ) ||
									tmpBoard.figures()[ y ][ x ]->color() == m_turnColor ) )
										break;

								const bool firstMoveDone =
									tmpFigure->isFirstMoveDone();

								if( !isCheckAfterMove( x, y, tmpFigure, tmpBoard ) )
									return false;

								if( d == Move::TwoFirstTime )
								{
									if( firstMoveDone )
										break;
									else if( move == 2 )
										break;
								}

								x += dx;
								y += dy;
							}
						}
						else if( tmpFigure->moves()[ i ][ j ].dist() != Move::No &&
							x >= 0 && x < 8 && y >= 0 && y < 8 )
						{
							if( tmpBoard.figures()[ y ][ x ] &&
								( !tmpFigure->moves()[ i ][ j ]
									.types().testFlag( Move::Hit ) ||
								tmpBoard.figures()[ y ][ x ]->color() == m_turnColor ) )
									continue;

							if( !isCheckAfterMove( x, y, tmpFigure, tmpBoard ) )
								return false;
						}
					}
				}
			}
		}
	}

	return true;
}

bool
Game::isStaleMate()
{
	return false;
}

void
Game::clicked( int x, int y )
{
	if( !m_selected )
		firstClick( x, y );
	else
	{
		clearCellsColor();

		if( secondClick( x, y ) && isStaleMate() )
		{

		}
	}
}

void
Game::hovered( int x, int y )
{
	Q_UNUSED( x )
	Q_UNUSED( y )
}

void
Game::newGame()
{
	m_board.newGame();
	m_board.update();
}

} /* namespace Chess */
