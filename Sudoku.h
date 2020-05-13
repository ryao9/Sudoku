#ifndef SUDOKU_H
#define SUDOKU_H

#include <set>
#include <vector>
#include <utility>
#include <string>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <fstream>

class Sudoku {
public:
	typedef std::vector<std::vector<int>> two_dim_int_vector;

	Sudoku(const two_dim_int_vector &board_in) :
		size(board_in.size()), square_size(sqrt(board_in.size())), board(board_in) {
		set_board_choices();
	}

	Sudoku(std::string file, int size_in) :
		size(size_in), square_size(sqrt(size_in)) {
		std::ifstream fin;
		fin.open(file);

		if (fin.is_open()) {
			for (int j = 0; j < size; ++j) {
				std::vector<int> nums;
				char num[1];

				for (int i = 0; i < size; ++i) {
					fin >> num[0];
					nums.push_back(atoi(num));
				}
				board.push_back(nums);
			}
			fin.close();
		}
		set_board_choices();
	}

	/*
	You can swap any row of three 3x3 blocks with any other row. 
	You can swap any column of three 3x3 blocks with another column. 
	Within each block row or block column you can swap single rows and single columns. 
	Finally you can permute the numbers so there are different numbers in the filled positions as long as the permutation is consistent across the whole board.
	*/
	void generate() {

	}

	// Looks for unfilled location with least possible choices of numbers
	// Fills location with first possible number then updates board
	// Repeat process until solved
	// If an invalid move is attempted, goes back one step to previous location and tries next number
	void solve() {
		std::vector<int> indices(1, 0);

		// While board is not solved
		while (num_zeroes != 0) {
			// Find first unfilled location with least possible choices
			int least_choices_index = 0;
			for (; !board_choices[least_choices_index].size(); ++least_choices_index);
			choices &first_choices = board_choices[least_choices_index].back();

			// If board is in invalid state undoes what was previously filled in
			if (!least_choices_index || indices.back() >= first_choices.size()) {
				indices.pop_back();
				++indices.back();
				reverse();
				continue;
			}

			// Row and column of location to be filled
			int row_updated = first_choices.row;
			int column_updated = first_choices.column;

			// Fills in board at location
			std::set<int>::iterator itr = first_choices.nums.begin();
			for (int i = 0; i < indices.back(); ++i, ++itr);
			board[row_updated][column_updated] = *(itr);
			--num_zeroes;

			// Update board information
			changed_choices.push_back(changes(row_updated, column_updated, board[row_updated][column_updated]));
			first_choices.nums.erase(itr);
			changed_choices.back().choices_added.push_back(first_choices);
			board_choices[least_choices_index].pop_back();
			update_board_choices(row_updated, column_updated);
			indices.push_back(0);
		}
	}

	bool solved() {
		std::vector<std::set<int>> used_nums_in_rows(size);
		std::vector<std::set<int>> used_nums_in_columns(size);
		std::vector<std::set<int>> used_nums_in_squares(size);
		std::set<int> all_choices;

		set_all_choices(all_choices);
		populate_used_nums(used_nums_in_rows, used_nums_in_columns, used_nums_in_squares);

		return contains_all_choices(used_nums_in_rows, all_choices) && contains_all_choices(used_nums_in_columns, all_choices) && contains_all_choices(used_nums_in_squares, all_choices);
	}

	void input() {
		while (num_zeroes != 0) {
			int row;
			int column;
			int value;
			bool request_reverse;

			std::cout << "\nCurrent board:\n\n";
			print_board();

			std::cout << "Do you want to reverse the previous action? (1 to reverse, 0 to not reverse) ";
			std::cin >> request_reverse;
			if (request_reverse) {
				reverse();
				continue;
			}

			std::cout << "Enter a row (1 to " << size << " ) ";
			std::cin >> row;
			std::cout << "Enter a column (1 to " << size << " ) ";
			std::cin >> column;
			std::cout << "Enter a value (1 to " << size << " ) ";
			std::cin >> value;

			if (!in_range(row) || !in_range(column) || !in_range(value) || board[--row][--column]) {
				std::cout << "\nInvalid input: invalid coordinates or value\n";
				continue;
			}

			std::vector<choices>::iterator choices_at_r_c = find_choices(row, column);
			if ((*choices_at_r_c).nums.erase(value) != 1) {
				std::cout << "\nInvalid input: value already used\n";
				continue;
			}

			board[row][column] = value;
			--num_zeroes;
			
			changed_choices.push_back(changes(row, column, value));		
			changed_choices.back().choices_added.push_back(*choices_at_r_c);
			board_choices[(*choices_at_r_c).size() + 1].erase(choices_at_r_c);
			update_board_choices(row, column);
		}

		std::cout << "\nCurrent board:\n\n";
		print_board();
		std::cout << "Puzzle solved!\n";
	}

	int sum_first_three() {
		return board[0][0] * 100 + board[0][1] * 10 + board[0][2];
	}

	void print_board() {
		std::cout << '\n';
		for (int r = 0; r < size; ++r) {
			if (r && r % square_size == 0) {
				std::cout << std::string(square_size * 4 * square_size, '-') << '\n';
			}
			for (int c = 0; c < size; ++c) {
				if (c && c % square_size == 0) {
					std::cout << "  |";
				}
				std::cout << std::setw(3) << board[r][c];
			}
			std::cout << '\n';
		}
		std::cout << "\n\n";
	}

	// Prints possible numbers at each unfilled location
	void print_board_choices() {
		for (auto &i : board_choices) {
			for (auto &j : i) {
				std::cout << "Row " << j.row << " Column " << j.column << "  ";
				print_set(j.nums);
				std::cout << "\n";
			}
		}
	}

	int get_size() {
		return size;
	}

	int get_square_size() {
		return square_size;
	}

private:
	// Stores location on board and possible numbers at that location
	struct choices {
		int row;
		int column;
		std::set<int> nums;

		choices() {}

		choices(int row_in, int column_in, int size_in, std::set<int> nums_in) :
			row(row_in), column(column_in), nums(nums_in) {}

		int size() {
			return nums.size();
		}
	};

	// Stores location filled, value filled at location, and change in other locations from filling
	struct changes {
		int row;
		int column;
		int value_removed;
		// change to choices * for faster copying (?)
		std::vector<choices> choices_added;

		changes() {}

		changes(int row_in, int column_in, int value_removed_in) :
			row(row_in), column(column_in), value_removed(value_removed_in) {}
	};

	typedef std::vector<choices>::iterator choices_itr;
	typedef std::vector<std::vector<choices>> two_dim_choices_vector;

	const int size;
	const int square_size;
	int num_zeroes;
	two_dim_int_vector board;
	two_dim_choices_vector board_choices = two_dim_choices_vector(size + 1);
	std::vector<changes> changed_choices;

	void set_board_choices() {
		std::vector<std::set<int>> used_nums_in_rows(size);
		std::vector<std::set<int>> used_nums_in_columns(size);
		std::vector<std::set<int>> used_nums_in_squares(size);
		std::set<int> all_choices;

		num_zeroes = 0;

		set_all_choices(all_choices);
		populate_used_nums(used_nums_in_rows, used_nums_in_columns, used_nums_in_squares);

		for (int r = 0; r < size; ++r) {
			for (int c = 0; c < size; ++c) {
				if (!board[r][c]) {
					add_choices(r, c, all_choices, used_nums_in_rows, used_nums_in_columns, used_nums_in_squares);
					++num_zeroes;
				}
			}
		}
	}

	void set_all_choices(std::set<int> &all_choices) {
		std::vector<int> choices;
		for (int i = 1; i <= size; ++i) {
			choices.push_back(i);
		}
		all_choices = std::set<int>(choices.begin(), choices.end());
	}

	void populate_used_nums(std::vector<std::set<int>> &rows, std::vector<std::set<int>> &columns, std::vector<std::set<int>> &squares) {
		for (int r = 0; r < size; ++r) {
			for (int c = 0; c < size; ++c) {
				if (board[r][c]) {
					rows[r].insert(board[r][c]);
					columns[c].insert(board[r][c]);
					squares[(c / square_size) + (r / square_size) * square_size].insert(board[r][c]);
				}
			}
		}
	}

	void add_choices(int row, int column, const std::set<int> &all_choices, std::vector<std::set<int>> &rows,
		std::vector<std::set<int>> &columns, std::vector<std::set<int>> &squares) {
		std::set<int> grid_choices = all_choices;

		for (int i : rows[row]) {
			grid_choices.erase(i);
		}
		for (int i : columns[column]) {
			grid_choices.erase(i);
		}
		for (int i : squares[(column / square_size) + (row / square_size) * square_size]) {
			grid_choices.erase(i);
		}

		struct choices grid(row, column, grid_choices.size(), grid_choices);
		board_choices[grid.size()].push_back(grid);
	}

	void reverse() {
		++num_zeroes;

		while (changed_choices.back().choices_added.size() > 1) {
			choices &back_choices = changed_choices.back().choices_added.back();
			back_choices.nums.insert(changed_choices.back().value_removed);
			board_choices[back_choices.nums.size()].push_back(back_choices);

			// alternatively just look in row before added back_choices
			choices_itr old_location = find_choices(back_choices.row, back_choices.column);
			board_choices[back_choices.nums.size() - 1].erase(old_location);
			changed_choices.back().choices_added.pop_back();
		}

		board[changed_choices.back().row][changed_choices.back().column] = 0;
		choices &back_choices = changed_choices.back().choices_added.back();
		back_choices.nums.insert(changed_choices.back().value_removed);
		board_choices[back_choices.nums.size()].push_back(back_choices);

		changed_choices.pop_back();
	}

	void update_board_choices(int row, int column) {
		int removed_value = board[row][column];

		for (int c = 1; c < size; ++c) {
			int current_column = (column + c) % size;
			if (!board[row][current_column]) {
				update_current_choices(row, current_column, removed_value);
			}

		}
		for (int r = 1; r < size; ++r) {
			int current_row = (row + r) % size;
			if (!board[current_row][column]) {
				update_current_choices(current_row, column, removed_value);
			}
		}
		for (int r = 0; r < size; ++r) {
			for (int c = 0; c < size; ++c) {
				int current_row = (row + r) % square_size + square_size * (row / square_size);
				int current_column = (column + c) % square_size + square_size * (column / square_size);
				if (!board[current_row][current_column]) {
					update_current_choices(current_row, current_column, removed_value);
				}
			}
		}
	}

	void update_current_choices(int row, int column, int value) {
		choices_itr choices_location = find_choices(row, column);
		if ((*choices_location).nums.erase(value) == 1) {
			board_choices[(*choices_location).size()].push_back(*choices_location);
			changed_choices.back().choices_added.push_back(*choices_location);
			board_choices[(*choices_location).size() + 1].erase(choices_location);
			return;
		}
	}

	choices_itr find_choices(int row, int column) {
		for (two_dim_choices_vector::iterator i = board_choices.begin(); i != board_choices.end(); ++i) {
			for (choices_itr j = (*i).begin(); j != (*i).end(); ++j) {
				if ((*j).row == row && (*j).column == column) {
					return j;
				}
			}
		}
		return choices_itr();
	}

	bool contains_all_choices(const std::vector<std::set<int>> &search_container, const std::set<int> &all_choices) {
		for (std::set<int> i : search_container) {
			if (i != all_choices) {
				return false;
			}
		}
		return true;
	}

	bool in_range(int value) {
		return value >= 1 && value <= size;
	}

	void print_set(const std::set<int> &s) {
		for (int i : s) {
			std::cout << std::setw(2) << i;
		}
	}
};

#endif