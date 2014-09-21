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

class Features
  {
  const Blob & b;		// Blob to witch these features belong
  mutable int hbars_, vbars_;
  mutable bool hscan_valid, vscan_valid;
  mutable std::vector< Rectangle > hbar_, vbar_;
  mutable std::vector< int > hscan_, vscan_;

public:
  mutable Profile lp, tp, rp, bp, hp, wp;

  Features( const Blob & b_ );

//  const Blob & blob() const throw() { return b; }

  const Rectangle & hbar( const int i ) const throw() { return hbar_[i]; }
  const Rectangle & vbar( const int i ) const throw() { return vbar_[i]; }
  int hbars() const;
  int vbars() const;

  const std::vector< int > & hscan() const;
  const std::vector< int > & vscan() const;

  int test_235Esz( const Charset & charset ) const;
  int test_49ARegpq( const Rectangle & charbox ) const;
  int test_4ADQao( const Charset & charset, const Rectangle & charbox ) const;
  int test_6abd( const Charset & charset ) const;
  int test_CEFIJLlT( const Charset & charset ) const;
  int test_c() const;
  int test_frst( const Rectangle & charbox ) const;
  int test_G() const;
  int test_HKMNUuvwYy( const Rectangle & charbox ) const;
  int test_hknwx( const Rectangle & charbox ) const;
  int test_s_cedilla() const;

  bool test_comma() const;
  int test_easy( const Rectangle & charbox ) const;
  int test_line( const Rectangle & charbox ) const;
  int test_solid( const Rectangle & charbox ) const;
  int test_misc( const Rectangle & charbox ) const;
  };
