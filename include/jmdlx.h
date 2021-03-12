/* -*- Mode: C++; fill-column: 80 -*- */
/***************************************************************************
 *   Copyright (c) 2008, 2016 Mark Deric                                   *
 *   mark@dericnet.com                                                     *
 *                                                                         *
 * This work is offered under the the terms of the MIT License; see the    *
 * LICENSE file in the top directory of this distribution or on Github:    *
 *   https://github.com/jmderic/sudoku_dlx                                 *
 ***************************************************************************/
#ifndef _JMDLX_H
#define _JMDLX_H 1

#define JMD_DLX_NAMESPACE_BEGIN namespace jmd { namespace dlx {
#define JMD_DLX_NAMESPACE_END }}

#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <list>
#include <stdexcept>
#include <climits> // LONG_MAX

// Settled on these for the exact cover matrix's row and column indices.
// Considered size_t, ssize_t, ptrdiff_t and <boost/cstdint.hpp>.  Go with
// signed everywhere.
typedef long int intz_t;
#define INTZ_MAX LONG_MAX

JMD_DLX_NAMESPACE_BEGIN

// a matrix_one instance represents a 1 in a matrix row -- hdr_ptr points to the
// column header and rc_idx is the row number
// secondarily but importantly, represents a column header -- hdr_ptr is NULL
// and rc_idx is the column number
// thirdly, represents the column header root which is positioned to the left of
// the 0th column header -- hdr_ptr is this and rc_idx is -1
class matrix_one
{
public:
    // inserting constructor: creates and inserts at the specified location
    matrix_one(intz_t rc_idx, matrix_one* hdr_ptr, matrix_one* prev)
            : left(prev ? prev : this), right(prev ? prev->right : this),
              up(hdr_ptr?hdr_ptr->up:this), down(hdr_ptr?hdr_ptr:this),
              hdr_ptr(hdr_ptr), rc_idx(rc_idx)
    {
        up->down = down->up = right->left = left->right = this;
    }
private:
    // ctor for the column header root
    matrix_one() : left(this), right(this), up(this), down(this),
                   hdr_ptr(this), rc_idx(-1) {}

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
    matrix_one* hdr_ptr;
    intz_t rc_idx;

    friend class ec_matrix;
};

class matrix_one_store
{
public:
    matrix_one* add_one(intz_t rc_idx, matrix_one* hdr_ptr, matrix_one* prev) {
        matrix_one* new_ptr = new matrix_one(rc_idx, hdr_ptr, prev);
        ones_.push_back(new_ptr);
        return new_ptr;
    }
    ~matrix_one_store() {
        std::list<matrix_one*>::const_iterator it, endit=ones_.end();
        for (it=ones_.begin(); it!=endit; ++it)
            delete *it;
    }
private:
    std::list<matrix_one*> ones_;
};

struct col_spec
{
    col_spec() : size(0), hdr_ptr(NULL) {}
    intz_t size;
    matrix_one* hdr_ptr;
};

struct row_spec
{
    row_spec(std::list<intz_t>& cols, bool constraint = false)
        : constraint(constraint), col_indices(cols) {}
    bool constraint;
    std::list<intz_t> col_indices; // for columns with 1's
};

typedef std::vector<row_spec> all_rows;

// callbacks to control the exact cover engine and get solutions from it
class ec_callback
{
public:
    // returns true if the consumer wants to harvest the answer via a call to
    // get_search_path(); quit_searching exits search giving no more answers
    virtual bool harvest_result(bool& /*quit_searching*/) {
        return true;
    }
    virtual void get_search_path(const std::vector<intz_t>& row_list) = 0;
};

class ec_matrix
{
public:
    ec_matrix(intz_t col_count, const all_rows& rows, ec_callback& eccb)
            : quit_searching_(false), col_specs_(col_count), eccb_(eccb)
    {
        intz_t i;
        matrix_one* prev = &root_;
        // initialize the columns
        for (i= 0; i<col_count; ++i) {
            matrix_one* next = store_.add_one(i, NULL, prev);
            col_specs_[i].hdr_ptr = next;
            prev = next;
        }
        // initialize the rows
        intz_t row_count = rows.size();
        ones_set constraint_hdrs;
        for (i = 0; i<row_count; ++i) { // for each row in all_rows
            const row_spec& row = rows[i];
            bool constraint = row.constraint;
            matrix_one* prev = NULL;
            std::list<intz_t>::const_iterator it, endit=row.col_indices.end();
            for (it=row.col_indices.begin(); it!=endit; ++it) {
                // for each column index in which there is a 1 in this row
                intz_t col_idx = *it;
                prev = store_.add_one(i, col_specs_[col_idx].hdr_ptr, prev);
                if (constraint) {
                    std::pair<ones_set::iterator, bool> inserted =
                        constraint_hdrs.insert(prev->hdr_ptr);
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
        // prune the constraints
        ones_set::iterator it, endit=constraint_hdrs.end();
        for (it=constraint_hdrs.begin(); it!=endit; ++it) {
            cover_column(*it);
        }
    }

    void search() {
        search(0);
    }

protected:
    typedef std::set<matrix_one*> ones_set;

    matrix_one* best_column() {
        intz_t col_ones, min_ones = INTZ_MAX;
        matrix_one* best = NULL;

        for (matrix_one* col=root_.right; col!=&root_; col=col->right) {
            if ( (col_ones=col_specs_[col->rc_idx].size) < min_ones ) {
                min_ones = col_ones;
                best = col;
            }
        }
        return best;  // never returns NULL; called when >=1 column uncovered
    }

    // cover/uncover_column: oneYs are in the column and are the markers for
    // each row we want to detach/attach; oneXs are in the same row as the outer
    // loop oneY.  oneYs' up/down are left alone; oneXs' up/down are modified
    void cover_column(matrix_one* col) {
        col->column_unlink();
        for (matrix_one* oneY=col->down; oneY!=col; oneY=oneY->down) {
            for (matrix_one* oneX=oneY->right; oneX!=oneY; oneX=oneX->right) {
                oneX->row_unlink();
                --col_specs_[oneX->hdr_ptr->rc_idx].size;
            }
        }
    }
    void uncover_column(matrix_one* col) {
        for (matrix_one* oneY=col->up; oneY!=col; oneY=oneY->up) {
            for (matrix_one* oneX=oneY->left; oneX!=oneY; oneX=oneX->left) {
                ++col_specs_[oneX->hdr_ptr->rc_idx].size;
                oneX->row_relink();
            }
        }
        col->column_relink();
    }

    void search(std::vector<intz_t>::size_type k) {
        if (root_.right == &root_) {
            // no columns left uncovered; this is a solution
            if ( eccb_.harvest_result(quit_searching_) ) { // callback wants it
                // furnish callback with the search path via a vector slice
                std::vector<intz_t>::const_iterator it = search_path_.begin();
                std::vector<intz_t> row_list(it, it+k);
                eccb_.get_search_path(row_list);
            }
        }
        else {
            matrix_one* best_hdr = best_column();
            cover_column(best_hdr);
            for (matrix_one* trial_row=best_hdr->down;
                 trial_row!=best_hdr && !quit_searching_;
                 trial_row=trial_row->down) {
                if (search_path_.size()<k+1)
                    search_path_.resize(k+1);
                search_path_[k] = trial_row->rc_idx;
                for (matrix_one* tr_col=trial_row->right; tr_col!=trial_row;
                     tr_col=tr_col->right) {
                    cover_column(tr_col->hdr_ptr);
                }
                search(k+1);
                for (matrix_one* tr_col=trial_row->left; tr_col!=trial_row;
                     tr_col=tr_col->left) {
                    uncover_column(tr_col->hdr_ptr);
                }
            }
            uncover_column(best_hdr);
        }
    }

    matrix_one root_;
    matrix_one_store store_;
    bool quit_searching_;
    std::vector<col_spec> col_specs_;
    std::vector<intz_t> search_path_;
    ec_callback& eccb_;
};


JMD_DLX_NAMESPACE_END

#endif // not defined _JMDLX_H
