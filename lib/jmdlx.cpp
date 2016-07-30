/* -*- Mode: C++; fill-column: 80 -*- 
 *
 * $Id: jmdlx.cpp,v 1.1.1.1 2008/04/09 20:40:19 mark Exp $
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

#include "jmdlx.h"
#include <sstream>


#ifdef SETUP_DEBUG
#include <iostream>
#endif

JMD_DLX_NAMESPACE_BEGIN

matrix_one::matrix_one(intz_t rc_idx, matrix_one* hdr_ptr, matrix_one* prev)
    : left(prev ? prev : this), right(prev ? prev->right : this),
      up(hdr_ptr?hdr_ptr->up:this), down(hdr_ptr?hdr_ptr:this),
      hdr_ptr(hdr_ptr), rc_idx(rc_idx)
{
    up->down = down->up = right->left = left->right = this;
}

void ec_matrix::prune_constraints()
{
    ones_set::iterator it, endit=constraint_hdrs_.end();
    for (it=constraint_hdrs_.begin(); it!=endit; ++it) {
        cover_column(*it);
    }
}

void ec_matrix::cleanup()
{
    std::list<matrix_one*>::const_iterator it, endit=heap_ones_.end();
    for (it=heap_ones_.begin(); it!=endit; ++it)
        delete *it;
}

ec_matrix::ec_matrix(intz_t col_count, const all_rows& rows, ec_callback& eccb)
        : quit_searching_(false), col_specs_(col_count), eccb_(eccb)
{
    intz_t i;
    matrix_one* prev = &root_;
    for (i= 0; i<col_count; ++i) {
        matrix_one* next = new matrix_one(i, NULL, prev);
        heap_ones_.push_back(next);
        col_specs_[i].hdr_ptr = next;
        prev = next;
    }

    intz_t row_count = rows.size();
    for (i = 0; i<row_count; ++i) {
        intz_t j, row_elements = rows[i].col_indices.size();
        bool constraint = rows[i].constraint;
        matrix_one* prev = NULL;
        for (j = 0; j<row_elements; ++j) {
            intz_t col_idx = rows[i].col_indices[j];
            prev = new matrix_one(i, col_specs_[col_idx].hdr_ptr, prev);
            heap_ones_.push_back(prev);
            if (constraint) {
                std::pair<ones_set::iterator, bool> inserted =
                    constraint_hdrs_.insert(prev->hdr_ptr);
                if (!inserted.second) {
                    std::ostringstream os;
                    os << "Overconstrained on column " << col_idx;
                    ec_exception exc(os.str());
                    throw exc;
                }
            }
            ++col_specs_[col_idx].size;
        }
    }
    try {
        prune_constraints();
    }
    catch(ec_exception& ec) {
        cleanup();
        throw ec;
    }
}

ec_matrix::~ec_matrix()
{
    cleanup();
}

JMD_DLX_NAMESPACE_END

