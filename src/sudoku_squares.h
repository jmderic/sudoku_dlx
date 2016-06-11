/* -*- Mode: C++; fill-column: 80 -*- 
 *
 * $Id: sudoku_squares.h,v 1.1.1.1 2008/04/09 20:40:19 mark Exp $
 *
 ***************************************************************************
 *   Copyright (C) 2008 by Mark Deric                                      *
 *   mark@dericnet.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _SUDOKU_SQUARES_H
#define _SUDOKU_SQUARES_H 1

#include "jmdlx.h"

#if defined(COLUMN_SETUP_DEBUG) || defined(ROW_SETUP_DEBUG)
#include <iostream>
#endif

JMD_DLX_NAMESPACE_BEGIN

#define SIDE_COUNT 9
#define SUB_SIDES  3

struct sudoku_square
{
    // members 1-based, _not_ zero based
    sudoku_square(char num_, char row_, char col_)
        : num(num_), row(row_), col(col_),
          sub( (((row-1)/SUB_SIDES)*SUB_SIDES)+((col-1)/SUB_SIDES)+1 )
    { }

    sudoku_square(std::string str_spec) throw(ec_exception)
    {
        std::stringstream ss;
        size_t val;
        ss << str_spec;
        ss >> val;
        num = val/100;
        val -= num*100;
        row = val/10;
        val -= row*10;
        col = val;
        if ( !((0 < num && num <= SIDE_COUNT) && (0 < row && row <= SIDE_COUNT)
               && (0 < col && col <= SIDE_COUNT)) ) {
            std::ostringstream os("Invalid square specifier \"", std::ios::app);
            os << str_spec << "\"";
            JMD::DLX::ec_exception exc(os.str());
            throw exc;
        }
        sub = (((row-1)/SUB_SIDES)*SUB_SIDES)+((col-1)/SUB_SIDES)+1;
    }

    size_t_vec get_ec_columns() {
        // ec_column indices are zero based (even though their names are 1
        // based)
        size_t_vec stv;
        size_t idx;
        idx = (0*SIDE_COUNT*SIDE_COUNT) + (num-1)*SIDE_COUNT+(row-1);
        stv.push_back(idx);
        idx = (1*SIDE_COUNT*SIDE_COUNT) + (num-1)*SIDE_COUNT+(col-1);
        stv.push_back(idx);
        idx = (2*SIDE_COUNT*SIDE_COUNT) + (num-1)*SIDE_COUNT+(sub-1);
        stv.push_back(idx);
        idx = (3*SIDE_COUNT*SIDE_COUNT) + (row-1)*SIDE_COUNT+(col-1);
        stv.push_back(idx);
        return stv;
    }

    char num;
    char row;
    char col;
    char sub;
};

struct sudoku_square_less
    : public std::binary_function<sudoku_square, sudoku_square, bool> 
{
  bool operator()(const sudoku_square& __x, const sudoku_square& __y) const {
      if ( __x.num < __y.num )
          return true;
      if ( __x.num == __y.num && __x.row < __y.row )
          return true;
      if ( __x.num == __y.num && __x.row == __y.row && __x.col < __y.col )
          return true;
      
      return false;
  }
};

typedef std::set<sudoku_square, sudoku_square_less> square_set;

// could be called sudoku row since all the choices of squares are rows on the
// ec_matrix
class sudoku_squares
{
public:
    sudoku_squares(square_set& squares_taken_)
        : squares_taken(squares_taken_) {}
    col_hdr_array create_columns() {
        static const char criteria[] = { 'r', 'c', 's', 'n' };
        col_hdr_array cha;

        for (size_t i=0; i < sizeof(criteria)/sizeof(criteria[0]); ++i) {
            for (size_t j=1; j <= SIDE_COUNT; ++j) {
                for (size_t k=1; k <= SIDE_COUNT; ++k) {
                    std::ostringstream os(std::ios::app);
                    if ( criteria[i] != 'n' ) {
                        os << j << criteria[i] << k;
                    }
                    else {
                        os << "r" << j << "c" << k;
                    }
#ifdef COLUMN_SETUP_DEBUG
                    std::cout << os.str() << std::endl;
#endif
                    col_hdr ch(os.str());
                    cha.push_back(ch);
                }
            }
        }
        return cha;
    }

    all_rows create_rows() {
        all_rows rows;

        for (size_t i=1; i <= SIDE_COUNT; ++i) {
            for (size_t j=1; j <= SIDE_COUNT; ++j) {
                for (size_t k=1; k <= SIDE_COUNT; ++k) {
                    sudoku_square sq(i, j, k);
                    size_t_vec stv = sq.get_ec_columns();
                    square_set::iterator it = squares_taken.find(sq),
                        endit = squares_taken.end();

                    row_spec rs(stv, it != endit);
#ifdef ROW_SETUP_DEBUG
                    std::cout << (int) sq.num << (int) sq.row << (int) sq.col
                              << (int) sq.sub << " "
                              << stv[0] << ", " << stv[1] << ", "
                              << stv[2] << ", " << stv[3]
                              << ((it != endit) ? "*" : "") << std::endl;
#endif
                    rows.push_back(rs);
                }
            }
        }
        return rows;
    }

protected:
    square_set squares_taken;
};


JMD_DLX_NAMESPACE_END

#endif // not defined _SUDOKU_SQUARES_H
