/* -*- Mode: C++; fill-column: 80 -*- 
 *
 * $Id: sudoku_squares.h,v 1.1.1.1 2008/04/09 20:40:19 mark Exp $
 *
 ***************************************************************************
 *   Copyright (C) 2008, 2016 by Mark Deric                                *
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

#define SIDE_COUNT 9
#define SUB_SIDES  3

class sudoku_square
{
public:
    // members row, col, num, sub are 1-based, _not_ zero based
    sudoku_square(char row, char col, char num)
            : row_(row), col_(col), num_(num) { }

    sudoku_square(const std::string& str_spec)
    {
        num_ = -1; // setup to throw below unless the incoming string is OK
        // e.g., "721" => 7 in row 2 col 1
        if (str_spec.size()==3) {
            num_ = str_spec[0] - '0'; // numerical value of char: '5' - '0' = 5
            row_ = str_spec[1] - '0';
            col_ = str_spec[2] - '0';
        }
        if ( !((0 < num_ && num_ <= SIDE_COUNT) && (0 < row_ && row_ <= SIDE_COUNT)
               && (0 < col_ && col_ <= SIDE_COUNT)) ) {
            std::ostringstream os("Invalid square specifier \"", std::ios::app);
            os << str_spec << "\"";
            throw jmd::dlx::ec_exception(os.str());
        }
    }

    std::vector<size_t> get_ec_columns() {
        // ec_column indices are zero based (even though their names are 1
        // based)
        std::vector<size_t> stv;
        size_t idx;
        char sub = (((row_-1)/SUB_SIDES)*SUB_SIDES)+((col_-1)/SUB_SIDES)+1;
        idx = (0*SIDE_COUNT*SIDE_COUNT) + (num_-1)*SIDE_COUNT+(row_-1);
        stv.push_back(idx);
        idx = (1*SIDE_COUNT*SIDE_COUNT) + (num_-1)*SIDE_COUNT+(col_-1);
        stv.push_back(idx);
        idx = (2*SIDE_COUNT*SIDE_COUNT) + (num_-1)*SIDE_COUNT+(sub-1);
        stv.push_back(idx);
        idx = (3*SIDE_COUNT*SIDE_COUNT) + (row_-1)*SIDE_COUNT+(col_-1);
        stv.push_back(idx);
        return stv;
    }

    bool operator<(const sudoku_square& rhs) const {
        // ordering is by row, col, number
        if ( row_ != rhs.row_ )
            return row_ < rhs.row_;
        if ( col_ != rhs.col_ )
            return col_ < rhs.col_;
        if ( num_ != rhs.num_ )
            return num_ < rhs.num_;
        return false;
    }

private:
    char row_;
    char col_;
    char num_;
#ifdef ROW_SETUP_DEBUG
    friend std::ostream& operator<<(std::ostream& os, const sudoku_square& ssq);
#endif
};

#ifdef ROW_SETUP_DEBUG
inline std::ostream& operator<<(std::ostream& os, const sudoku_square& ssq) {
    os << 'r' << (int) ssq.row_ << 'c' << (int) ssq.col_ << '-'
        << (int) ssq.num_;
    return os;
}
#ifdef JMD_HIDE
inline std::ostream& operator<<(std::ostream& os,
                                const std::vector<size_t>& stv) {
    std::vector<size_t>::const_iterator it, endit=stv.end();
    for (it=stv.begin(); it != endit; ++it)
        os << *it << ((it+1!=endit)?", ":"");
    return os;
}
#endif //JMD_HIDE
#endif

// put in a more general place? template for vector of any insertable type
template<typename T>
inline std::ostream& operator<<(std::ostream& os,
                                const std::vector<T>& stv) {
    typename std::vector<T>::const_iterator it, endit=stv.end();
    for (it=stv.begin(); it != endit; ++it)
        os << *it << ((it+1!=endit)?", ":"");
    return os;
}

typedef std::set<sudoku_square> square_set;

// all the choices of squares are rows on the ec_matrix; this sets up the rows
// and columns
class sudoku_squares
{
public:
    sudoku_squares(const square_set& squares_taken)
        : squares_taken_(squares_taken) {}
    jmd::dlx::col_hdr_array create_columns() {
        static const char criteria[] = { 'r', 'c', 's', 'n' };
        jmd::dlx::col_hdr_array cha;

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
                    jmd::dlx::col_hdr ch(os.str());
                    cha.push_back(ch);
                }
            }
        }
        return cha;
    }

    jmd::dlx::all_rows create_rows() {
        jmd::dlx::all_rows rows;
        square_set::const_iterator endit=squares_taken_.end();
#ifdef ROW_SETUP_DEBUG
        square_set::const_iterator it;
        for(it = squares_taken_.begin(); it != endit; ++it)
            std::cout << "squares_taken_: " << *it << std::endl;
#endif

        for (size_t i=1; i <= SIDE_COUNT; ++i) {
            for (size_t j=1; j <= SIDE_COUNT; ++j) {
                for (size_t k=1; k <= SIDE_COUNT; ++k) {
                    sudoku_square sq(i, j, k);
                    std::vector<size_t> stv = sq.get_ec_columns();
                    square_set::const_iterator it = squares_taken_.find(sq);
                    jmd::dlx::row_spec rs(stv, it != endit);
#ifdef ROW_SETUP_DEBUG
                    std::cout << sq << " " << stv << ((it != endit) ? "*" : "")
                        << std::endl;
#endif
                    rows.push_back(rs);
                }
            }
        }
        return rows;
    }

protected:
    const square_set squares_taken_;
};

#endif // not defined _SUDOKU_SQUARES_H
