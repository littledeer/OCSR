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

class Bitmap : public Rectangle
  {
  std::vector< std::vector< unsigned char > > data;	// faster than bool

public:
      // Creates a blank Bitmap
  Bitmap( const int l, const int t, const int r, const int b );

      // Creates a Bitmap from part of another Bitmap
  Bitmap( const Bitmap & source, const Rectangle & re );

  using Rectangle::left;
  using Rectangle::top;
  using Rectangle::right;
  using Rectangle::bottom;
  using Rectangle::height;
  using Rectangle::width;
  void left  ( const int l );
  void top   ( const int t );
  void right ( const int r );
  void bottom( const int b );
  void height( const int h ) { bottom( top() + h - 1 ); }
  void width ( const int w ) { right( left() + w - 1 ); }

  void add_bitmap( const Bitmap & bm );
  void add_point( const int row, const int col );
  void add_rectangle( const Rectangle & re );
  bool adjust_height();
  bool adjust_width();

  bool get_bit( const int row, const int col ) const throw()
    { return data[row-top()][col-left()]; }
  void set_bit( const int row, const int col, const bool bit ) throw()
    { data[row-top()][col-left()] = bit; }

  int area() const throw();			// 'area' means filled area
  int area_octagon() const throw();
  int size_octagon() const throw();
  int seek_left  ( const int row, const int col, const bool black = true ) const throw();
  int seek_top   ( const int row, const int col, const bool black = true ) const throw();
  int seek_right ( const int row, const int col, const bool black = true ) const throw();
  int seek_bottom( const int row, const int col, const bool black = true ) const throw();
  bool escape_left  ( int row, int col ) const throw();
  bool escape_top   ( int row, int col ) const throw();
  bool escape_right ( int row, int col ) const throw();
  bool escape_bottom( int row, int col ) const throw();
  int  follow_top   ( int row, int col ) const;
  int  follow_bottom( int row, int col ) const;
  bool top_hook   ( int *hdiff ) const;
  bool bottom_hook( int *hdiff ) const;
  };
