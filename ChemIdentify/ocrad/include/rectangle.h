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

class Rectangle
  {
  int left_, top_, right_, bottom_;

public:
  Rectangle( const int l, const int t, const int r, const int b ) throw();

  void left  ( const int l ) throw();
  void top   ( const int t ) throw();
  void right ( const int r ) throw();
  void bottom( const int b ) throw();
  void height( const int h ) throw();
  void width ( const int w ) throw();
  void add_point( const int row, const int col ) throw();
  void add_rectangle( const Rectangle & re ) throw();
  void enlarge( const int scale ) throw();
  void move( const int row, const int col ) throw();

  int left()    const throw() { return left_;   }
  int top()     const throw() { return top_;    }
  int right()   const throw() { return right_;  }
  int bottom()  const throw() { return bottom_; }
  int height()  const throw() { return bottom_ - top_ + 1; }
  int width()   const throw() { return right_ - left_ + 1; }
  int size()    const throw() { return height() * width(); }
  int hcenter() const throw() { return ( left_ + right_ ) / 2; }
  int vcenter() const throw() { return ( top_ + bottom_ ) / 2; }
  int hpos( const int p ) const throw()
    { return left_ + ( ( ( right_ - left_ ) * p ) / 100 ); }
  int vpos( const int p ) const throw()
    { return top_ + ( ( ( bottom_ - top_ ) * p ) / 100 ); }

  bool operator==( const Rectangle & re ) const throw()
    { return ( left_ == re.left_ && top_ == re.top_ && right_ == re.right_ && bottom_ == re.bottom_ ); }
  bool operator!=( const Rectangle & re ) const throw() { return !( *this == re ); }

  bool includes( const Rectangle & re ) const throw();
  bool includes( const int row, const int col ) const throw();
  bool strictly_includes( const Rectangle & re ) const throw();
  bool strictly_includes( const int row, const int col ) const throw();
  bool includes_hcenter( const Rectangle & re ) const throw();
  bool includes_vcenter( const Rectangle & re ) const throw();
  bool h_includes( const Rectangle & re ) const throw();
  bool h_includes( const int col ) const throw();
  bool v_includes( const Rectangle & re ) const throw();
  bool v_includes( const int row ) const throw();
  bool h_overlaps( const Rectangle & re ) const throw();
  bool v_overlaps( const Rectangle & re ) const throw();
  int  v_overlap_percent( const Rectangle & re ) const throw();
  bool is_hcentred_in( const Rectangle & re ) const throw();
  bool is_vcentred_in( const Rectangle & re ) const throw();
  bool precedes( const Rectangle & re ) const throw();
  bool h_precedes( const Rectangle & re ) const throw();
  bool v_precedes( const Rectangle & re ) const throw();

  int distance( const Rectangle & re ) const throw();
  int distance( const int row, const int col ) const throw();
  int h_distance( const Rectangle & re ) const throw();
  int h_distance( const int col ) const throw();
  int v_distance( const Rectangle & re ) const throw();
  int v_distance( const int row ) const throw();

  static int hypoti( const int c1, const int c2 ) throw();
  };
