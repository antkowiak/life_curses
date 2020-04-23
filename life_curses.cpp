/*
 * life_curses.c - Game of Life implemented with the ncurses library.
 *
 * Written by Ryan Antkowiak (antkowiak@gmail.com)
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

#include <string>
#include <chrono>
#include <thread>

#include "rng.h"
#include "table.h"

class life_board
{
private:
    const size_t m_columns;
    const size_t m_rows;
    table<bool> * m_cells;

    size_t count_neighbors(const int col, const int row)
    {
        size_t neighbors = 0;

        for (int c = col-1 ; c < col+2 ; ++c)     // adjacent column indexes
            for (int r = row-1 ; r < row+2 ; ++r) // adjacent row indxes
                if (! (c==col && r==row))         // don't consider the current cell
                    if (c >=0 && c < m_columns && r >= 0 && r < m_rows) // range check
                        if (m_cells->get(c, r))   // check if cell is alive
                            ++neighbors;

        return neighbors;
    }

public:
    life_board(const size_t columns, const size_t rows)
    : m_columns(columns),
      m_rows(rows),
      m_cells(new table<bool>(columns, rows, false))
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
        table<bool> * next_generation = new table<bool>(m_columns, m_rows, false);

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

void play_life(const size_t columns, const size_t rows, const size_t delay_ms)
{
    initscr();
    noecho();
    WINDOW * pWindow = newwin(rows, columns, 0, 0);

    life_board board(columns, rows);

    while (true)
    {
        board.draw(pWindow);
        board.advance_generation();
        wrefresh(pWindow);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    delwin(pWindow);
    endwin();
}

int main(int argc, char * argv[])
{
    play_life(80, 24, 100);

    return EXIT_SUCCESS;
}


