/* -*- Mode: C++; fill-column: 80 -*- 
 *
 * $Id: sudoku_main.cpp,v 1.2 2008/04/25 19:05:28 mark Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <map>

#include "sudoku_squares.h"

class square_formatter : public std::unary_function<const char&, void>
{
public:
    square_formatter() : formatted(formatted_l), new_token(true),
                         next(formatted) {
        formatted[4] = '-';
        formatted[6] = 0;
    }
    void operator() (const char& letter) {
        if ( new_token ) {
            if ( letter == 'r' ) {
                *next++ = letter;
            }
            else {
                formatted[5] = letter;
            }
            new_token = false;
        }
        else if( formatted < next && next < &formatted[4] ) {
            *next++ = letter;
        }
        else if ( letter == ' ' ) {
            new_token = true;
        }
    }
    char* get_formatted() { return formatted; }
private:
    // r1c1-1
    // 0123456
    char formatted_l[7];
    // need the indirection provided by formatted so the system
    // written copy ctor (called by for_each) points the copy
    // constructed instance to the first instance's formatted_l array
    char* formatted;
    bool new_token;
    char* next;
};

class state_rearranger : public std::unary_function<const std::string&, void>
{
public:
    state_rearranger(std::set<std::string>& ordered_output_)
        : ordered_output(ordered_output_) {}
    void operator() (const std::string& sq) {
        square_formatter sf;
        std::for_each(sq.begin(), sq.end(), sf);
        ordered_output.insert(sf.get_formatted());
    }
private:
    std::set<std::string>& ordered_output;
};

class state_printer : public std::unary_function<const std::string&, void>
{
public:
    //state_printer() {}
    void operator() (const std::string& sq) {
        std::cout << sq << std::endl;
    }
};

static void render_state(JMD::DLX::string_vec sv,
                         std::string state_description,
                         std::set<std::string>& ordered_output)
{
    std::for_each(sv.begin(), sv.end(),
                  state_rearranger(ordered_output));
    std::cout << state_description << std::endl;
    std::for_each(ordered_output.begin(), ordered_output.end(),
                  state_printer());
}

typedef std::pair<std::string, size_t> counting_pair;
typedef std::map<std::string, size_t> counting_map;
typedef std::pair<size_t, std::string> frequency_pair;
typedef std::multimap<size_t, std::string> frequency_map;

// generally useful ... should be a pair_swap template
class frequency_transformer
    : public std::unary_function<counting_pair, frequency_pair>
{
public:
    frequency_pair operator() (const counting_pair& cp) {
        frequency_pair fp;
        fp.first = cp.second;
        fp.second = cp.first;
        return fp;
    }
};

class cell_frequency : public std::unary_function<const std::string&, void>
{
public:
    cell_frequency(counting_map& cm_) : cm(cm_) {}
    void operator() (const std::string& sq) {
        std::pair<counting_map::iterator, bool> inserted;
        inserted = cm.insert(counting_pair(sq, 1));
        if ( !inserted.second ) {
            ++inserted.first->second;
        }
    }

    frequency_map get_frequency() {
        frequency_map fm;
        std::transform(cm.begin(), cm.end(), std::inserter(fm, fm.begin()),
                       frequency_transformer());
        return fm;
    }

protected:
    counting_map& cm;
};

class solver_callback : public JMD::DLX::ec_callback
{
public:
    solver_callback() : cf(cm), solution_count(0) {}
    virtual void collect_state(JMD::DLX::string_vec sv) {
        std::set<std::string> ordered_output;
        render_state(sv, "solution:", ordered_output);
        for_each(ordered_output.begin(), ordered_output.end(), cf);
        ++solution_count;
    }
    void dump_frequency() {
        if ( solution_count > 1 ) {
            frequency_map fm = cf.get_frequency();
            frequency_map::iterator it, endit = fm.end();
            for ( it = fm.begin(); it != endit; ++it) {
                std::cout << it->second << " " << it->first << std::endl;
            }
        }
    }
protected:
    counting_map cm;
    cell_frequency cf;
    size_t solution_count;
};

class creator_callback : public JMD::DLX::ec_callback
{
public:
    creator_callback() : depth_threshhold(50), result_count(-1) {}
    virtual bool start_depth(size_t k) {
        if ( k == depth_threshhold ) {
            std::cout << "v" << std::flush;
            result_count = 0;
            return true; // collect state
        }
        return false; // don't collect state
    }
    virtual bool have_result(bool& quit_searching) {
        quit_searching = false;
        if ( ++result_count == 1 ) {
            return true; // collect state
        }
        quit_searching = true;
        return false; // don't collect state
    }
    virtual void collect_state(JMD::DLX::string_vec sv) {
        if ( result_count == 0 ) {
            start = sv;
        }
        else { // result_count == 1
            solution = sv;
        }
    }
    virtual void end_depth(size_t k, bool& quit_searching) {
        if ( k == depth_threshhold ) {
            std::cout << "^" << result_count << std::flush;
            if ( result_count == 1 ) {
                std::set<std::string> ordered_output;
                render_state(start, "start:", ordered_output);
                ordered_output.clear();
                render_state(solution, "solution:", ordered_output);
                quit_searching = true;
            }
            else {
                quit_searching = false;
                // continue the search at the level above
                result_count = -1;
            }
        }
        // don't change quit_searching
    }

protected:
    size_t depth_threshhold;
    int result_count;
    JMD::DLX::string_vec start, solution;
};


int main(int argc, char *argv[])
{
    try {
        JMD::DLX::square_set taken;
        std::pair<JMD::DLX::square_set::iterator, bool> inserted;

        //std::cout << argc << std::endl;
        for (int i=1; i<argc; ++i ) {
            JMD::DLX::sudoku_square sq(argv[i]);
            inserted = taken.insert(sq);
            if (!inserted.second) {
                std::ostringstream os("Duplicate squares specified: ",
                                      std::ios::app);
                os << argv[i];
                JMD::DLX::ec_exception exc(os.str());
                throw exc;
            }            
        }

        JMD::DLX::sudoku_squares sqs(taken);
        JMD::DLX::col_hdr_array cha = sqs.create_columns();
        JMD::DLX::all_rows rows = sqs.create_rows();
        solver_callback scb;
        //creator_callback scb;

        JMD::DLX::ec_matrix ecm(cha, rows, scb);
        ecm.search();
        scb.dump_frequency();
    }
    catch(JMD::DLX::ec_exception& ec) {
        std::cerr << "Exitting on exception " << ec.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
