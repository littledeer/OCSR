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

class Page_image;
class Rational;

class Textline : public Track
  {
  int big_initials_;
  mutable std::vector< Character * > cpv;

public:
  Textline() : big_initials_( 0 ) {}
  Textline( const Textline & tl );
  Textline & operator=( const Textline & tl );
  ~Textline() throw();
  void set_track();
  void verify_big_initials() throw();

  int big_initials() const throw() { return big_initials_; }
  Character & character( const int i ) const throw();
  Character * character_at( const int col ) const throw();
  int characters() const throw() { return cpv.size(); }
  Rectangle charbox( const Character & c ) const throw();
  int width() const throw()
    { return cpv.size() ? cpv.back()->right() - cpv.front()->left() : 0; }

  void delete_character( const int i ) throw();
  int  shift_characterp( Character * const p, const bool big = false );
  bool insert_space( const int i, const bool tab = false );
  void insert_spaces();
  void join( Textline & tl );

  int mean_height() const throw();
  Rational mean_width() const throw();
  Rational mean_gap_width( const int first = 0, int last = -1 ) const throw();
  int mean_hcenter() const throw();
  int mean_vcenter() const throw();

  void print( const Control & control ) const throw();
  void dprint( const Control & control, const bool graph,
               const bool recursive ) const throw();
  void xprint( const Control & control ) const throw();
  void cmark( Page_image & page_image ) const throw();

  void recognize1( const Charset & charset ) const;
  void recognize2( const Charset & charset );
  void apply_filter( const Filter & filter );
  };
