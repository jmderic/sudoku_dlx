/* Copyright 2021 J. Mark Deric */

#include <algorithm>
#include <vector>
#include <sstream>
#include <cstdlib>
#include "jmdlx.h"
#include "sudoku_squares.h"
#include "gtest/gtest.h"


static void output_solution(std::ostream& os, sudoku_game& game,
                            const std::vector<intz_t>& row_list)
{
    square_set ordered_rows;
    std::transform(row_list.begin(), row_list.end(),
                   std::inserter(ordered_rows, ordered_rows.end()),
                   std::bind1st(std::mem_fun
                                (&sudoku_game::get_square_array), &game));
    square_set::const_iterator it, nextit, endit=ordered_rows.end();
    for (it=ordered_rows.begin(); it != endit; ++it) {
        nextit = it;
        os << *it << ((++nextit!=endit)?",":"");
    }
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

// =============================================================================

class sudoku_dlx_test : public ::testing::Test {
 protected:
    using Args = vector<std::string>;
    void RunCase(const Args& args, const std::string& expected) {
        square_set taken;
        std::pair<square_set::iterator, bool> inserted;

        for (const string& s : args) {
            sudoku_square sq(s.c_str());
            inserted = taken.insert(sq);
            if (!inserted.second) {
                std::ostringstream os;
                os << "Duplicate squares specified: " << argv[i];
                throw std::runtime_error(os.str());
            }            
        }

        sudoku_game game(taken);
        jmd::dlx::all_rows rows = game.create_rows();
        std::ostringstream oss;
        solver_callback scb(oss, game);

        jmd::dlx::ec_matrix ecm(game.get_col_count(), rows, scb);
        ecm.search();
        ASSERT_EQ(oss.str(), expected);
        //scb.dump_summary();
    }
};

TEST_F(sudoku_dlx_test, HeadlineTest) {
    Args args{"312","914","715","822","626","528","136","142","543","747","649","351","752","858","559","961","663","167","268","474","282","784","688","895","996","798"};  // NOLINT
    const std::string expected{"r1c1-5,r1c3-4,r1c6-8,r1c7-6,r1c8-1,r1c9-2,r2c1-1,r2c3-9,r2c4-3,r2c5-2,r2c7-4,r2c9-7,r3c1-2,r3c2-6,r3c3-7,r3c4-5,r3c5-4,r3c7-3,r3c8-9,r3c9-8,r4c1-8,r4c4-2,r4c5-9,r4c6-3,r4c8-4,r5c3-2,r5c4-6,r5c5-1,r5c6-4,r5c7-9,r6c2-4,r6c4-8,r6c5-5,r6c6-7,r6c9-3,r7c1-7,r7c2-9,r7c3-8,r7c5-6,r7c6-2,r7c7-5,r7c8-3,r7c9-1,r8c1-4,r8c3-1,r8c5-3,r8c6-5,r8c7-8,r8c9-9,r9c1-6,r9c2-5,r9c3-3,r9c4-1,r9c7-2,r9c9-4"}  // NOLINT
    RunCase(bldgs, expected);
}
