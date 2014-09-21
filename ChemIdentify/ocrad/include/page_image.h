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

struct OCRAD_Pixmap;
class Mask;
class Rational;
class Track;

class Page_image : public Rectangle		// left,top is always 0,0
  {
public:
  struct Error
    {
    const char * const msg;
    Error( const char * const s ) : msg( s ) {}
    };

private:
  std::vector< std::vector< unsigned char > > data;	// 256 level greymap
  unsigned char maxval_, threshold_;		// x > threshold == white

  void read_p1( FILE * const f, const bool invert );
  void read_p4( FILE * const f, const bool invert );
  void read_p2( FILE * const f, const bool invert );
  void read_p5( FILE * const f, const bool invert );
  void read_p3( FILE * const f, const bool invert );
  void read_p6( FILE * const f, const bool invert );

public:
  // Creates a Page_image from a pbm, pgm or ppm file
  Page_image( FILE * const f, const bool invert );

  // Creates a Page_image from a OCRAD_Pixmap
  Page_image( const OCRAD_Pixmap & image, const bool invert );

  // Creates a reduced Page_image
  Page_image( const Page_image & source, const int scale );

  using Rectangle::left;
  using Rectangle::top;
  using Rectangle::right;
  using Rectangle::bottom;
  using Rectangle::height;
  using Rectangle::width;
  void left  ( int ) { throw Error( "Page_image resize not allowed." ); }
  void top   ( int ) { left( 0 ); }
  void right ( int ) { left( 0 ); }
  void bottom( int ) { left( 0 ); }
  void height( int ) { left( 0 ); }
  void width ( int ) { left( 0 ); }

  bool get_bit( const int row, const int col ) const throw()
    { return data[row-top()][col-left()] <= threshold_; }
  bool get_bit( const int row, const int col, const unsigned char th ) const throw()
    { return data[row-top()][col-left()] <= th; }
  void set_bit( const int row, const int col, const bool bit ) throw()
    { data[row-top()][col-left()] = ( bit ? 0 : maxval_ ); }

  unsigned char maxval() const throw() { return maxval_; }
  unsigned char threshold() const throw() { return threshold_; }
  void threshold( const Rational & th );	// 0 <= th <= 1, else auto
  void threshold( const int th );		// 0 <= th <= 255, else auto

  bool cut( const Rational ltwh[4] );
  void draw_mask( const Mask & m ) throw();
  void draw_rectangle( const Rectangle & re ) throw();
  void draw_track( const Track & tr ) throw();
  bool save( FILE * const f, const char filetype ) const throw();
  bool scale( int n );
  void transform( const Transformation & t );
  };
