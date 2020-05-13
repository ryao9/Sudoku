#include "Sudoku.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int main() {
	Sudoku test1("sudoku_ai.txt", 9);
	test1.print_board();
	test1.solve();
	cout << test1.solved() << endl;
	test1.print_board();

	Sudoku test2("sudoku15.txt", 9);
	test2.print_board();
	test2.solve();
	cout << test2.solved() << endl;
	test2.print_board();
}