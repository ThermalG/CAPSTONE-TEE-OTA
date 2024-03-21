#include <random>
#include "capstone.h"

int SIZE = 15;
int DIFFICULTY = 1; // depth of minimax search: EXCEEDING 6 NOT RECOMMENDED
int PLAYER = USER;  // set first player
int moves = 0;
double t = 0.0;     // average response time by computer
std::vector<std::vector<int>> board(SIZE, std::vector<int>(SIZE));

bool peripheral(const std::vector<std::vector<int>> &, int, int, int);

std::pair<int, int> userGetter() {
	int row, col;
    char c;
	while (true) {  // till legal input
		std::cout << PLAY << "PICK COLUMN: ";   // of course, you can enter column & row together, e.g. h8 as center
        std::cin >> c;
        col = static_cast<char>(std::toupper(c)) - 'A';
		if (!std::isalpha(c) || col < 0 || col >= SIZE) { // warn & reenter on void inputs
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << INFO << RED << "INVALID INDEX" << RESET;
        } else break;
	}
    while(true) {
        std::cout << PLAY << "PICK ROW: ";
        std::cin >> row;
        if (std::cin.peek() != '\n' || row < 1 || row > SIZE || board[row - 1][col] != 0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << INFO << RED << "INVALID INDEX" << RESET;
        } else break;
    }
	return std::make_pair(row - 1, col);    // zero-based indexing
}

/**
 Function to verify if a move is near occupied cell(s)
 * @param b - the board to check
 * @param r, c - given move
 * @param dist - how near is "near" (including diagonals)
 */
bool peripheral(const std::vector<std::vector<int>> &b, int r, int c, int dist = 2) {
    for (int i = -dist; i <= dist; i++) {
        for (int j = -dist; j <= dist; j++) {
            int probe_r = r + i;
            int probe_c = c + j;
            if (probe_r >= 0 && probe_r < SIZE && probe_c >= 0 && probe_c < SIZE && b[probe_r][probe_c] != 0) {
                return true;
            }
        }
    }
    return false;
}

std::vector<int> minimax(std::vector<std::vector<int>> &b, int alpha, int beta, int depth, int p) { // NOLINT
	if (depth == 0 || depth >= (SIZE * SIZE) - moves) { // evaluate current grid in every direction
        int scores = 0;
        std::vector<int> elements(5);
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (j < SIZE - 4) {                 // horizontal
                    for (int k = 0; k < 5; k++) {
                        elements[k] = b[i][j + k];
                    }
                    scores += heuristic(elements);
                }
                if (i < SIZE - 4) {                 // vertical
                    for (int k = 0; k < 5; k++) {
                        elements[k] = b[i + k][j];
                    }
                    scores += heuristic(elements);
                }
                if (i < SIZE - 4 && j < SIZE - 4) { // anti-diagonal
                    for (int k = 0; k < 5; k++) {
                        elements[k] = b[i + k][j + k];
                    }
                    scores += heuristic(elements);
                }
                if (i >= 4 && j < SIZE - 4) {       // diagonal
                    for (int k = 0; k < 5; k++) {
                        elements[k] = b[i - k][j + k];
                    }
                    scores += heuristic(elements);
                }
            }
        }
		return std::vector<int>{scores, -1, -1};
	}
    std::vector<int> optima = {p == COMP ? INT_MIN : INT_MAX, -1, -1};  // maximizing computer while minimizing user
    if (win(-p)) {
        return optima;  // coerce poorest score to avoid the move
    }
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (b[i][j] == 0 && peripheral(b, i, j)) {  // only go down when cell unoccupied and near occupied
                std::vector<std::vector<int>> replica(SIZE, std::vector<int>(SIZE));
                for (int r = 0; r < SIZE; r++) {
                    for (int c = 0; c < SIZE; c++) {
                        replica[r][c] = b[r][c];
                    }
                }
                replica[i][j] = p;
                int score = minimax(replica, alpha, beta, depth - 1, -p)[0];
                if (p == COMP) {
                    if (score > optima[0]) {
                        optima = {score, i, j};
                    }
                    alpha = std::max(alpha, optima[0]);
                } else {
                    if (score < optima[0]) {
                        optima = {score, i, j};
                    }
                    beta = std::min(beta, optima[0]);
                }
                if (alpha >= beta) {    // branch pruned
                    break;
                }
            }
        }
    }
    return optima;
}

/// OPTIMIZATION POSSIBLE
int heuristic(std::vector<int> &v) {
    int favour{0}, neutral{0}, hazard{0}, scores{0};
	for (int i : v) {
        neutral += (i == 0) ? 1 : 0;
		favour += (i == COMP) ? 1 : 0;
        hazard += (i == USER || i == COMP) ? 1 : 0;
	}
    hazard -= favour;
    if (favour == 5) {
        scores += 65536;
    } else if (favour == 4 && neutral == 1) {
        scores += 16384;
    } else if (favour == 3 && neutral == 2) {
        scores += 4096;
    } else if (favour == 2 && neutral == 3) {
        scores += 1024;
    } else if (favour == 1 && neutral == 4) {
        scores += 256;
    } else if (hazard == 1 && neutral == 4) {
        scores -= 512;
    } else if (hazard == 2 && neutral == 3) {
        scores -= 2048;
    } else if (hazard == 3 && neutral == 2) {
        scores -= 8192;
    } else if (hazard == 4 && neutral == 1) {
        scores -= 32768;
    } else if (hazard == 5) {
        scores -= 65537;
    }
    return scores;
}

bool aligned(int i, int j, int di, int dj, int p) {
    for (int k = 0; k < 5; k++) {
        if (board[i + k * di][j + k * dj] != p) {
            return false;
        }
    }
    return true;
}

bool win(int p) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if ((j + 4 < SIZE && aligned(i, j, 0, 1, p)) ||                   // horizontal
                (i + 4 < SIZE && aligned(i, j, 1, 0, p)) ||                   // vertical
                (i + 4 < SIZE && j > 3 && aligned(i, j, 1, -1, p)) ||         // diagonal
                (i + 4 < SIZE && j + 4 < SIZE && aligned(i, j, 1, 1, p))) { // anti-diagonal
                return true;
            }
        }
    }
    return false;
}

int main() {
    for (int i = SIZE; i > 0; i--) { // grid initialization
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << i;
        std::string s = ss.str();
        std::cout << s << "   ";
        for (int j = 0; j < SIZE; j++) {
            std::cout << "+   ";
        }
        std::cout << std::endl << std::endl;
    }
    std::cout << "     ";
    for (int i = 0; i < SIZE; i++) {
        std::cout << char(i + 65) << "   ";
    }
    int row, col;
	while (moves < SIZE * SIZE) {       // game loop
        bool P = PLAYER == USER;
        if (P) {
            std::tie(row, col) = userGetter();
        } else {
            if (moves == 0) {           // coerce standardized opening
                row = int(SIZE / 2);
                col = int(SIZE / 2);
            } else if (moves == 1) {    // early randomization around the first user move
                std::mt19937 gen(std::random_device{}());
                std::uniform_int_distribution<int> dis(-1, 1);
                int delta_r, delta_c;
                do {                    // ensure not overwriting user's move
                    delta_r = dis(gen);
                    delta_c = dis(gen);
                } while (delta_r == 0 && delta_c == 0);
                row += delta_r;
                col += delta_c;
            } else {
                std::cout << PLAY << "COMPUTER EVALUATING...";
                auto start = std::chrono::high_resolution_clock::now();
                std::vector<int> coord = minimax(board, INT_MIN, INT_MAX, DIFFICULTY, COMP);
                auto end = std::chrono::high_resolution_clock::now();
                t += std::chrono::duration<double, std::milli>(end - start).count();
                row = coord[1];
                col = coord[2];
            }
        }
        board[row][col] = PLAYER;
        std::cout << INFO << "AVG EVAL " << t / int((++moves + P) / 2) << " ms\x1b[" << 2 * (SIZE - row) - 3
                  << ";" << 4 * col + 6 << "H" << (P ? YELLOW "O" : BLUE "X") << RESET;
        if (win(PLAYER)) {
            std::cout << PLAY << GREEN << (P ? "WON" : "LOST") << RESET << std::endl;
            break;
        }
        PLAYER *= -1;
	}
	if (moves == SIZE * SIZE) {
		std::cout << PLAY << GREEN << "DRAW" << RESET << std::endl;
	}
    system("pause");
    return 0;
}