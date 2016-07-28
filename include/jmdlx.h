/* -*- Mode: C++; fill-column: 80 -*- 
 *
 * $Id: jmdlx.h,v 1.1.1.1 2008/04/09 20:40:19 mark Exp $
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

#ifndef _JMDLX_H
#define _JMDLX_H 1

#define JMD_NAMESPACE_BEGIN namespace jmd {
#define JMD_NAMESPACE_END }

#define JMD_DLX_NAMESPACE_BEGIN JMD_NAMESPACE_BEGIN namespace dlx {
#define JMD_DLX_NAMESPACE_END } JMD_NAMESPACE_END

#include <cstddef>  // size_t
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <stdexcept>
#include <climits>

JMD_DLX_NAMESPACE_BEGIN

class ec_exception : public std::runtime_error
{
public:
    ec_exception(const std::string& what) : std::runtime_error(what) {}
};

class col_hdr;

class matrix_one
{
public:
    matrix_one()
        : left(this), right(this), up(this), down(this), col(NULL) {}
    matrix_one(const matrix_one& src)
        : left(this), right(this), up(this), down(this), col(NULL) {}
    matrix_one(col_hdr* col_, matrix_one* prev);

    void column_unlink() {
        right->left = left;
        left->right = right;
    }
    void row_unlink() {
        down->up = up;
        up->down = down;
    }
    void column_relink() {
        right->left = left->right = this;
    }
    void row_relink() {
        down->up = up->down = this;
    }

    matrix_one* left;
    matrix_one* right;
    matrix_one* up;
    matrix_one* down;
    col_hdr* col;
};

class col_hdr : public matrix_one
{
public:
    col_hdr(std::string name_) : size(0), name(name_) {}
    void append(col_hdr* prev) {
        left = prev;
        right = prev->right;
        right->left = left->right = this;
    }

    size_t size;
    std::string name;
};

struct row_spec
{
    row_spec(size_t* begin, size_t* end, bool constraint = false)
        : constraint(constraint), col_indices(begin, end) {}
    row_spec(std::vector<size_t>& stv, bool constraint = false)
        : constraint(constraint), col_indices(stv) {}
    bool constraint;
    std::vector<size_t> col_indices;
};

typedef std::vector<col_hdr> col_hdr_array;
typedef std::vector<matrix_one*> ones_vec;
typedef std::set<matrix_one*> ones_set;
typedef std::vector<row_spec> all_rows;

class ec_callback
{
public:
    // bool return values for start_depth() and have_result() are true if the
    // consumer wants the harvest the state via a call to collect_state()
    virtual bool start_depth(size_t k) { return false; }
    virtual bool have_result(bool& quit_searching) {
        quit_searching = false;
        return true;
    }
    //only fn used by subclass solver_callback; others for runtime analysis &
    //control (see removed creator_callback and reconsider during algo review)
    virtual void collect_state(std::vector<std::string> sv) = 0;
    virtual void end_depth(size_t k, bool& quit_searching) { }
};

class ec_matrix
{
public:
    ec_matrix(col_hdr_array& cha, all_rows& rows, ec_callback& eccb_)
        throw(ec_exception);
    ~ec_matrix();

    void search() {
        search(0);
    }

protected:
    void prune_constraints();
    // helper for prune_constraints()
    void delete_column(matrix_one* col);

    col_hdr* best_column() {
        size_t min_ones = UINT_MAX;
        col_hdr* best = NULL;

        // use reinterpret_cast as the cheapest re-cast 'cause we know we're
        // right ?? Is there better??
        for (col_hdr* col = reinterpret_cast<col_hdr*>(root->right);
             col != root; col = reinterpret_cast<col_hdr*>(col->right)) {
            if ( col->size < min_ones ) {
                min_ones = col->size;
                best = col;
            }
        }
        return best;
    }

    void cover_column(col_hdr* col) {
        col->column_unlink();
        for (matrix_one* column_one = col->down; column_one != col;
             column_one = column_one->down) {
            for (matrix_one* row_one = column_one->right; row_one != column_one;
                 row_one = row_one->right) {
                row_one->row_unlink();
                --row_one->col->size;
            }
        }
    }

    void uncover_column(col_hdr* col) {
        for (matrix_one* column_one = col->up; column_one != col;
             column_one = column_one->up) {
            for (matrix_one* row_one = column_one->left; row_one != column_one;
                 row_one = row_one->left) {
                ++row_one->col->size;
                row_one->row_relink();
            }
        }
        col->column_relink();
    }

    void advertise_state(size_t k) {
        std::vector<std::string> sv;
        for (size_t i=0; i<k; ++i) {
            matrix_one* row_one = O_vector[i];
            std::ostringstream os(std::ios::app);
            do {
                os << row_one->col->name;
                if ( (row_one = row_one->right) != O_vector[i] ) {
                    os << " ";
                }
                else {
                    sv.push_back(os.str());
                    break;
                }
            } while (true);
        }
        eccb.collect_state(sv);
    }

    bool search(size_t k) {
        bool quit_searching = false;
        if (root->right == root) {
            if ( eccb.have_result(quit_searching) ) {
                advertise_state(k);
            }
        }
        else {
            if ( eccb.start_depth(k) ) {
                advertise_state(k);
            }
            col_hdr* col = best_column();
            cover_column(col);
            for (matrix_one* column_one = col->down;
                 column_one != col && !quit_searching;
                 column_one = column_one->down) {
                if (O_vector.size()<k+1)
                    O_vector.resize(k+1);
                O_vector[k] = column_one;
                for (matrix_one* row_one = column_one->right; row_one != column_one;
                     row_one = row_one->right) {
                    cover_column(row_one->col);
                }
                quit_searching = search(k+1);
                for (matrix_one* row_one = column_one->left; row_one != column_one;
                     row_one = row_one->left) {
                    uncover_column(row_one->col);
                }
            }
            uncover_column(col);
            eccb.end_depth(k, quit_searching);
        }
        return quit_searching;
    }

    col_hdr root_lvalue;
    col_hdr* root;
    ones_vec O_vector;
    ec_callback& eccb;
    ones_vec constraints;
};


JMD_DLX_NAMESPACE_END

#endif // not defined _JMDLX_H
