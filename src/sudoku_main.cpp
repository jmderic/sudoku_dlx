/* -*- Mode: C++; fill-column: 80 -*- */
/***************************************************************************
 *   Copyright (c) 2008, 2016 Mark Deric                                   *
 *   mark@dericnet.com                                                     *
 *                                                                         *
 * This work is offered under the the terms of the MIT License; see the    *
 * LICENSE file in the top directory of this distribution or on Github:    *
 *   https://github.com/jmderic/sudoku_dlx                                 *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "sudoku_squares.h"

static void output_solution(std::ostream& os, sudoku_game& game,
                            const std::vector<intz_t>& row_list)
{
    square_set ordered_rows;
    std::transform(row_list.begin(), row_list.end(),
                   std::inserter(ordered_rows, ordered_rows.end()),
                   std::bind1st(std::mem_fun
                                (&sudoku_game::get_square_array), &game));
    square_set::const_iterator it, nextit, endit=ordered_rows.end();
    os << "solution:\n";
    for (it=ordered_rows.begin(); it != endit; ++it) {
        nextit = it;
        os << *it << ((++nextit!=endit)?"\n":"");
    }
    os << std::endl;
}

class solver_callback : public jmd::dlx::ec_callback
{
public:
    solver_callback(std::ostream& os, sudoku_game& game)
            : os_(os), game_(game), max_solutions_(10), solution_count_(0) {}
    virtual bool harvest_result(bool& quit_searching) {
        if (++solution_count_ > max_solutions_) {
            quit_searching = true;
            return false;
        }
        return true;
    }
    virtual void get_search_path(const std::vector<intz_t>& row_list) {
        output_solution(os_, game_, row_list);
    }
    void dump_summary() {
        os_ << "Number of solutions: ";
        if (solution_count_ > max_solutions_)
            os_ << "more than " << max_solutions_ << " shown" << std::endl;
        else
            os_ << solution_count_ << std::endl;
    }
protected:
    std::ostream& os_;
    sudoku_game& game_;
    const intz_t max_solutions_;
    intz_t solution_count_;
};

int main(int argc, char *argv[])
{
    try {
        square_set taken;
        std::pair<square_set::iterator, bool> inserted;

        for (int i=1; i<argc; ++i ) {
            sudoku_square sq(argv[i]);
            inserted = taken.insert(sq);
            if (!inserted.second) {
                std::ostringstream os;
                os << "Duplicate squares specified: " << argv[i];
                throw std::runtime_error(os.str());
            }            
        }

        sudoku_game game(taken);
        jmd::dlx::all_rows rows = game.create_rows();
        solver_callback scb(std::cout, game);

        jmd::dlx::ec_matrix ecm(game.get_col_count(), rows, scb);
        ecm.search();
        scb.dump_summary();
    }
    catch(std::exception& e) {
        std::cerr << "Exiting on exception " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
