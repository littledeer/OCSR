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

struct Segment
  {
  int left, right;				// l > r means no segment

  Segment( const int l = 1, const int r = 0 ) throw()
    : left( l ), right( r ) {}

  void add_point( const int col ) throw();
  void add_segment( const Segment & seg ) throw();

  bool valid() const throw() { return ( left <= right ); }
  bool includes( const Segment & seg ) const throw()
    { return ( seg.valid() && left <= seg.left && seg.right <= right ); }
  bool includes( const int col ) const throw()
    { return ( left <= col && col <= right ); }

  int distance( const Segment & seg ) const throw();
  int distance( const int col ) const throw();
  };


class Mask : public Rectangle
  {
  std::vector< Segment > data;			// segment in each line

public:
      // Creates a rectangular Mask
  Mask( const Rectangle & re )
    : Rectangle( re ), data( height(), Segment( re.left(), re.right() ) ) {}
  Mask( const int l, const int t, const int r, const int b )
    : Rectangle( l, t, r, b ), data( height(), Segment( l, r ) ) {}

  using Rectangle::left;
  using Rectangle::top;
  using Rectangle::right;
  using Rectangle::bottom;
  using Rectangle::height;

  int left ( const int row ) const throw();
  int right( const int row ) const throw();

  void top   ( const int t );
  void bottom( const int b );
  void height( const int h ) { bottom( top() + h - 1 ); }

  void add_mask( const Mask & m );
  void add_point( const int row, const int col );
  void add_rectangle( const Rectangle & re );

  bool includes( const Rectangle & re ) const throw();
  bool includes( const int row, const int col ) const throw();

  int distance( const Rectangle & re ) const throw();
  int distance( const int row, const int col ) const throw();
  };
