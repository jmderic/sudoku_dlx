/* -*- Mode: C++; fill-column: 80 -*- 
 *
 * $Id: sudoku_main.cpp,v 1.2 2008/04/25 19:05:28 mark Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <map>

#include "sudoku_squares.h"

static void output_solution(std::ostream& os,
                            sudoku_game& game,
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
                std::ostringstream os("Duplicate squares specified: ",
                                      std::ios::app);
                os << argv[i];
                jmd::dlx::ec_exception exc(os.str());
                throw exc;
            }            
        }

        sudoku_game game(taken);
        jmd::dlx::all_rows rows = game.create_rows();
        solver_callback scb(std::cout, game);

        jmd::dlx::ec_matrix ecm(game.get_col_count(), rows, scb);
        ecm.search();
        scb.dump_summary();
    }
    catch(jmd::dlx::ec_exception& ec) {
        std::cerr << "Exiting on exception " << ec.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
