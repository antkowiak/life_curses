/*
 * life_curses.c - Game of Life implemented with the ncurses library.
 *
 * Written by Ryan Antkowiak 
 *
 * April 23, 2020
 *
 * Copyright (c) 2020, All rights reserved.
 *
 */

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>

#include "cmdline_options.h"
#include "rng.h"
#include "table.h"

class life_board
{
private:
    const size_t m_columns;
    const size_t m_rows;
    rda::table<bool> * m_cells;

    size_t count_neighbors(const size_t col, const size_t row)
    {
        size_t neighbors = 0;

        // if c or r underflows size_t, that is fine, since they are being
        //  compared with m_cols and m_rows.
        for (size_t c = col-1 ; c < col+2 ; ++c)     // adjacent column indexes
            for (size_t r = row-1 ; r < row+2 ; ++r) // adjacent row indxes
                if (! (c==col && r==row))         // don't consider the current cell
                    if (c < m_columns && r < m_rows) // range check
                        if (m_cells->get(c, r))   // check if cell is alive
                            ++neighbors;

        return neighbors;
    }

public:
    life_board(const size_t columns, const size_t rows)
    : m_columns(columns),
      m_rows(rows),
      m_cells(new rda::table<bool>(columns, rows, false))
    {
        randomize_board();
    }

    ~life_board()
    {
        delete m_cells;
    }

    void clear_board()
    {
        m_cells->clear();
    }

    void randomize_board()
    {
        for (size_t r = 0 ; r < m_rows ; ++r)
            for (size_t c = 0 ; c < m_columns ; ++c)
                m_cells->set(c,r, (RNG::Rand(2) == 0));
    }

    void advance_generation()
    {
        rda::table<bool> * next_generation = new rda::table<bool>(m_columns, m_rows, false);

        for (size_t col = 0 ; col < m_columns ; ++col)
        {
            for (size_t row = 0 ; row < m_rows ; ++row)
            {
                const bool old_state = m_cells->get(col, row);
                const size_t neighbors = count_neighbors(col, row);

                // Rules of the game
                if (old_state && (neighbors == 2 || neighbors == 3))
                {
                    next_generation->set(col, row, true);
                }
                else if ((!old_state) && (neighbors == 3))
                {
                    next_generation->set(col, row, true);
                }
            }
        }

        std::swap(m_cells, next_generation);
        delete next_generation;
    }

    void draw(WINDOW * pWindow) const
    {
        clear();

        for (size_t row = 0 ; row < m_rows ; ++row)
        {
            std::string row_text;

            for (size_t col = 0 ; col < m_columns ; ++col)
            {
                if (m_cells->get(col, row))
                    row_text += "*";
                else
                    row_text += " ";
            }

            mvwprintw(pWindow, row, 0, row_text.c_str());
        }

        wmove(pWindow, 0, 0);
    }
    
};

void play_life(const size_t columns, const size_t rows, const size_t delay_ms, const size_t generations)
{
    initscr();
    noecho();
    WINDOW * pWindow = newwin(rows, columns, 0, 0);

    life_board board(columns, rows);

    for (size_t gen = 0 ; gen < generations ; ++gen)
    {
        board.draw(pWindow);
        board.advance_generation();
        wrefresh(pWindow);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    delwin(pWindow);
    endwin();
}

void show_help()
{
    std::cout << "Usage: life_curses [OPTION]..." << std::endl;
    std::cout << "Displays a simulation of John Conway's Game of Life" << std::endl;
    std::cout << std::endl;
    std::cout << "-d delay         Specifies the delay (in ms) between each generation." << std::endl;
    std::cout << "                 Default is 100 ms." << std::endl;
    std::cout << std::endl;
    std::cout << "-g generations   Specifies the number of generations to simulate." << std::endl;
    std::cout << "                 Default is 5000." << std::endl;
    std::cout << std::endl;
    std::cout << "-h height        Specifies the height of the game board. Default is 24." << std::endl;
    std::cout << std::endl;
    std::cout << "-w width         Specifies the width of the game board. Default is 80." << std::endl;
    std::cout << std::endl;
    std::cout << "-?               Display help." << std::endl;
    std::cout << std::endl;

    exit(EXIT_SUCCESS);
}

void parse_cmd_line(int argc, const char * argv[], size_t& width, size_t& height, size_t& delay, size_t& generations)
{
    using cmdline_options = rda::cmdline_options;
    using option = rda::cmdline_options::option;
    using option_type = rda::cmdline_options::option_type;
    using option_value_num = rda::cmdline_options::option_value_num;
    std::vector<option> options;

    options.push_back(option(option_type::OT_SHORT, option_value_num::OVN_ONE, "w"));
    options.push_back(option(option_type::OT_SHORT, option_value_num::OVN_ONE, "h"));
    options.push_back(option(option_type::OT_SHORT, option_value_num::OVN_ONE, "d"));
    options.push_back(option(option_type::OT_SHORT, option_value_num::OVN_ONE, "g"));
    options.push_back(option(option_type::OT_SHORT, option_value_num::OVN_NONE, "?"));

    cmdline_options cmd(options);
    cmd.parse(argc, argv);

    if (!cmd.unclaimed.empty())
        show_help();

    if (cmd.options[0].present) // "w"
    {
        if (cmd.options[0].values.size() != 1)
            show_help();
        else
            width = static_cast<size_t>(atoi(cmd.options[0].values[0].c_str()));
    }

    if (cmd.options[1].present) // "h"
    {
        if (cmd.options[1].values.size() != 1)
            show_help();
        else
            height = static_cast<size_t>(atoi(cmd.options[1].values[0].c_str()));
    }

    if (cmd.options[2].present) // "d"
    {
        if (cmd.options[2].values.size() != 1)
            show_help();
        else
            delay = static_cast<size_t>(atoi(cmd.options[2].values[0].c_str()));
    }

    if (cmd.options[3].present) // "g"
    {
        if (cmd.options[3].values.size() != 1)
            show_help();
        else
            generations = static_cast<size_t>(atoi(cmd.options[3].values[0].c_str()));
    }

    if (cmd.options[4].present) // "?"
        show_help();

    if (width == 0 || height == 0 || delay == 0 || generations == 0)
        show_help();
}

int main(int argc, const char * argv[])
{
    size_t width = 80;
    size_t height = 24;
    size_t delay = 100;
    size_t generations = 5000;

    parse_cmd_line(argc, argv, width, height, delay, generations);

    play_life(width, height, delay, generations);

    return EXIT_SUCCESS;
}


