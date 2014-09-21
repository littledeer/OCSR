/*  GNU Ocrad - Optical Character Recognition program
    Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011
    Antonio Diaz Diaz.

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

class Character : public Rectangle
  {
public:
  struct Guess
    {
    int code;
    int value;
    Guess( const int c, const int v ) throw() : code( c ), value( v ) {}
    };

private:
  std::vector< Blob * > blobpv;		// the blobs forming this Character
  std::vector< Guess > gv;		// vector of possible char codes
					// and their associated values.
					// gv[0].code < 0 means further
					// processing is needed (merged chars)

  void recognize11( const Charset & charset, const Rectangle & charbox );
    void recognize110( const Charset & charset, const Rectangle & charbox );
    void recognize111( const Charset & charset, const Rectangle & charbox );
    void recognize112( const Rectangle & charbox );
  void recognize12( const Charset & charset, const Rectangle & charbox );
  void recognize13( const Charset & charset, const Rectangle & charbox );

public:
  Character( Blob * const p )
    : Rectangle( *p ), blobpv( 1, p ) {}

  Character( const Rectangle & re, int code, int value )
    : Rectangle( re ), gv( 1, Guess( code, value ) ) {}

  Character( const Character & c );
  Character & operator=( const Character & c );

  ~Character() throw();

  int area() const throw();
  const Blob & blob( const int i ) const throw();
  Blob & blob( const int i ) throw();
  int blobs() const throw() { return blobpv.size(); }
  Blob & main_blob() throw();

  void shift_blobp( Blob * const p );

  void add_guess( const int code, const int value )
    { gv.push_back( Guess( code, value ) ); }
  void clear_guesses() throw() { gv.clear(); }
  void insert_guess( const int i, const int code, const int value );
  void delete_guess( const int i ) throw();
  void only_guess( const int code, const int value )
    { gv.clear(); gv.push_back( Guess( code, value ) ); }
  bool set_merged_guess( const int code1, const int right1,
                         const int code2, const int blob_index );
  void swap_guesses( const int i, const int j ) throw();
  const Guess & guess( const int i ) const throw();
  int guesses() const throw() { return gv.size(); }
  bool maybe( const int code ) const throw();
//  bool maybe_digit() const throw();
//  bool maybe_letter() const throw();

  void join( Character & c );
  unsigned char byte_result() const throw();
  void print( const Control & control ) const throw();
  void dprint( const Control & control, const Rectangle & charbox,
               const bool graph, const bool recursive ) const throw();
  void xprint( const Control & control ) const throw();

  void recognize1( const Charset & charset, const Rectangle & charbox );
  void apply_filter( const Filter & filter );
  };
