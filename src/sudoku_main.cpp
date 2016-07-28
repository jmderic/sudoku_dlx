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

class square_formatter
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

class state_rearranger
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

class state_printer
{
public:
    void operator() (const std::string& sq) {
        std::cout << sq << std::endl;
    }
};

static void render_state(std::vector<std::string> sv,
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
{
public:
    frequency_pair operator() (const counting_pair& cp) {
        frequency_pair fp;
        fp.first = cp.second;
        fp.second = cp.first;
        return fp;
    }
};

class cell_frequency
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

class solver_callback : public jmd::dlx::ec_callback
{
public:
    solver_callback() : cf(cm), solution_count(0) {}
    virtual void collect_state(std::vector<std::string> sv) {
        // std::cout << "_jmd collect_state: " << sv << std::endl;
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

        sudoku_squares sqs(taken);
        jmd::dlx::col_hdr_array cha = sqs.create_columns();
        jmd::dlx::all_rows rows = sqs.create_rows();
        solver_callback scb;

        jmd::dlx::ec_matrix ecm(cha, rows, scb);
        ecm.search();
        scb.dump_frequency();
    }
    catch(jmd::dlx::ec_exception& ec) {
        std::cerr << "Exiting on exception " << ec.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
