life_curses : life_curses.cpp table.h rng.h
	g++ -std=c++14 life_curses.cpp -lncurses -o life_curses

clean :
	\rm -f life_curses

