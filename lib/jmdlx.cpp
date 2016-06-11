/* -*- Mode: C++; fill-column: 80 -*- 
 *
 * $Id: jmdlx.cpp,v 1.1.1.1 2008/04/09 20:40:19 mark Exp $
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

#include "jmdlx.h"

#ifdef SETUP_DEBUG
#include <iostream>
#endif

JMD_DLX_NAMESPACE_BEGIN


matrix_one::matrix_one(col_hdr* col_, matrix_one* prev)
    : left(prev ? prev : this), right(prev ? prev->right : this),
      up(col_->up), down(col_), col(col_)
{
    up->down = down->up = right->left = left->right = this;
}

void ec_matrix::delete_column(matrix_one* col)
{
    // very similar to cover_column() but this guy does deleting and we didn't
    // want to slow down cover_column() (e.g., with a flag) for considerations
    // that only occur during startup
    matrix_one* next;
    col->column_unlink();
    for (matrix_one* column_one = col->down; column_one != col;
         column_one = next) {
        for (matrix_one* row_one = column_one->right; row_one != column_one;
             row_one = next) {
            row_one->row_unlink();
            --row_one->col->size;
            next = row_one->right;
            delete row_one;
        }
        next = column_one->down;
        delete column_one;
    }
}

void ec_matrix::prune_constraints() throw(ec_exception)
{
    ones_set delete_columns;

    try {
        std::pair<ones_set::iterator, bool> inserted;
        ones_vec::iterator it, endit = constraints.end();
        col_hdr* overconstrained = NULL;
        for (it = constraints.begin(); it != endit; ++it) {
            matrix_one* row_one = *it;
            inserted = delete_columns.insert(row_one->col);
            if (!inserted.second) {
                overconstrained = reinterpret_cast<col_hdr*>(*inserted.first);
                throw overconstrained;
            }
            for (matrix_one* row_next = row_one->right; row_one != row_next;
                 row_next = row_next->right) {
                inserted = delete_columns.insert(row_next->col);
                if (!inserted.second) {
                    overconstrained =
                        reinterpret_cast<col_hdr*>(*inserted.first);
                    throw overconstrained;
                }
            }
        }
    }
    catch(col_hdr* overconstrained) {
        std::ostringstream os("Overconstrained on column ", std::ios::app);
        os << overconstrained->name;
        ec_exception exc(os.str());
        throw exc;
    }

    ones_set::iterator it, endit = delete_columns.end();
    for (it = delete_columns.begin(); it != endit; ++it) {
        delete_column(*it);
    }

}

static void cleanup(col_hdr* proot)
{
    matrix_one* next_col, *next_elt;

    for (next_col = proot->right; next_col != proot;
         next_col = next_col->right) {
        for (next_elt = next_col->down; next_elt != next_col; ) {
            matrix_one* elt = next_elt;
            next_elt = next_elt->down;
#ifdef SETUP_DEBUG
            std::cout << elt << std::endl;
#endif
            delete elt;
        }
    }
}

ec_matrix::ec_matrix(col_hdr_array& cha, all_rows& rows, ec_callback& eccb_)
    throw(ec_exception)
    : root_lvalue("<root>"), root(&root_lvalue), eccb(eccb_)
{
    col_hdr* prev = root;
    col_hdr_array::iterator it, endit = cha.end();
    for (it = cha.begin(); it != endit; ++it) {
        it->append(prev);
        prev = &*it;
    }

    size_t i, row_count = rows.size();
    for (i = 0; i<row_count; ++i) {
        size_t j, row_elements = rows[i].col_indices.size();
        matrix_one* prev = NULL;
        for (j = 0; j<row_elements; ++j) {
            size_t column_idx = rows[i].col_indices[j];
            prev = new matrix_one(&cha[column_idx], prev);
            if ( !j && rows[i].constraint) {
                constraints.push_back(prev);
            }
            ++prev->col->size;
#ifdef SETUP_DEBUG
            std::cout << prev << std::endl;
#endif
        }
    }
    try {
        prune_constraints();
    }
    catch(ec_exception& ec) {
        cleanup(root);
        throw ec;
    }
}

ec_matrix::~ec_matrix()
{
    cleanup(root);
}

JMD_DLX_NAMESPACE_END

