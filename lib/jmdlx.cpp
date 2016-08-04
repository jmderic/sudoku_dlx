/* -*- Mode: C++; fill-column: 80 -*- */
/***************************************************************************
 *   Copyright (c) 2008, 2016 Mark Deric                                   *
 *   mark@dericnet.com                                                     *
 *                                                                         *
 * This work is offered under the the terms of the MIT License; see the    *
 * LICENSE file in the top directory of this distribution or on Github:    *
 *   https://github.com/jmderic/sudoku_dlx                                 *
 ***************************************************************************/

#include "jmdlx.h"
#include <sstream>

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
    for (i = 0; i<row_count; ++i) { // for each row in all_rows
        const row_spec& row = rows[i];
        bool constraint = row.constraint;
        matrix_one* prev = NULL;
        std::list<intz_t>::const_iterator it, endit=row.col_indices.end();
        for (it=row.col_indices.begin(); it!=endit; ++it) {
            // for each column index in which there is a 1 in this row
            intz_t col_idx = *it;
            prev = new matrix_one(i, col_specs_[col_idx].hdr_ptr, prev);
            heap_ones_.push_back(prev);
            if (constraint) {
                std::pair<ones_set::iterator, bool> inserted =
                    constraint_hdrs_.insert(prev->hdr_ptr);
                if (!inserted.second) {
                    // sudoku's not one, but is there a use case where this is
                    // not an error?
                    std::ostringstream os;
                    os << "Overconstrained on column " << col_idx;
                    throw std::runtime_error(os.str());
                }
            }
            ++col_specs_[col_idx].size;
        }
    }
    try {
        prune_constraints();
    }
    catch(std::exception& e) {
        cleanup();
        throw e;
    }
}

ec_matrix::~ec_matrix()
{
    cleanup();
}

JMD_DLX_NAMESPACE_END

