life_curses : life_curses.cpp table.h rng.h Makefile
	g++ -std=c++14 life_curses.cpp -lncurses -o life_curses -Wall -Wextra -Wpedantic

clean :
	\rm -f life_curses

