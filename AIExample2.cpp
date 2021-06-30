#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <set>
#include <cstdlib>
#include <ctime>

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

int initial_player;

class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
    Point spots;
    int state_val = 0;
    std::vector<OthelloBoard> next_boards;
    int value_board[8][8] = {
        700, -35, 28, 14, 14, 28, -35, 700,
        -35, -70,-14,  3,  3,-14, -70, -35,
         28, -14, 14,  7,  7, 14, -14,  28,
         14,   3,  7, -7, -7,  7,   3,  14,
         14,   3,  7, -7, -7,  7,   3,  14,
         28, -14, 14,  7,  7, 14, -14,  28,
        -35, -70,-14,  3,  3,-14, -70, -35,
        700, -35, 28, 14, 14, 28, -35, 700
    };
private:
    int get_next_player(int player) const {
        // 1 -> 2, 2 -> 1.
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                // other side is cur_player's disc.
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    } 
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            // cur_player's disc -> don't flip.
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                // cur_player's disc found on the other side -> start flipping.
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    // update each player's count.
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                // opponet's disc -> store in disc.
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard() {
        reset();
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        disc_count[EMPTY] = 0;
        disc_count[BLACK] = 0;
        disc_count[WHITE] = 0;
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        // invalid move -> lose.
        if(!is_spot_valid(p)) {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Neither player can move -> Game ends.
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    }

    int get_state_val()
    {
        int val = 0;
        int player = board[spots.x][spots.y];
        int mod = (player == initial_player) ? 1 : -1;
        // disc number.
        if (disc_count[0] > 25) val += 1.4 * (disc_count[initial_player] - disc_count[3 - initial_player]);
        else if (disc_count[0] > 5) val += 3.5 * (disc_count[initial_player] - disc_count[3 - initial_player]);
        else val += 14 * (disc_count[initial_player] - disc_count[3 - initial_player]);
        // valid move count.
        if (mod == 1 && disc_count[0] > 5) val += - 14 * next_valid_spots.size();
        //else val += -mod * next_valid_spots.size();
        // position.
        for (int i = 0; i < SIZE; i++) for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == initial_player) val += value_board[i][j];
            else if (board[i][j] == 3 - initial_player) val -= value_board[i][j];
        }
        
        if (mod == 1) {
            if (board[0][0] != player && board[0][7] != player) {
                if (board[0][1] == player && board[0][2] == player && board[0][3] == player && board[0][4] == player && board[0][5] == player && board[0][6] == player) val -= mod * 140;
                else if ((board[0][1] == player && board[0][2] == player && is_spot_valid(Point{0, 3})) || (board[0][5] == player && board[0][6] == player && is_spot_valid(Point{0, 4}))) val -= mod * 70;
            }
            if (board[7][0] != player && board[7][7] != player) {
                if (board[7][1] == player && board[7][2] == player && board[7][3] == player && board[7][4] == player && board[7][5] == player && board[7][6] == player) val -= mod * 140;
                else if ((board[7][1] == player && board[7][2] == player && is_spot_valid(Point{7, 3})) || (board[7][5] == player && board[7][6] == player && is_spot_valid(Point{7, 4}))) val -= mod * 70;
            }
            if (board[0][0] != player && board[7][0] != player) {
                if (board[1][0] == player && board[2][0] == player && board[3][0] == player && board[4][0] == player && board[5][0] == player && board[6][0] == player) val -= mod * 140;
                else if ((board[1][0] == player && board[2][0] == player && is_spot_valid(Point{3, 0})) || (board[5][0] == player && board[6][0] == player && is_spot_valid(Point{4, 0}))) val -= mod * 70;
            }
            if (board[0][7] != player && board[7][7] != player) {
                if (board[1][7] == player && board[2][7] == player && board[3][7] == player && board[4][7] == player && board[5][7] == player && board[6][7] == player) val -= mod * 140;
                else if ((board[1][7] == player && board[2][7] == player && is_spot_valid(Point{3, 7})) || (board[5][7] == player && board[6][7] == player && is_spot_valid(Point{4, 7}))) val -= mod * 70;
            }
            if (board[0][0] != player && board[7][7] != player) {
                if (board[1][1] == player && board[2][2] == player && board[3][3] == player && board[4][4] == player && board[5][5] == player && board[6][6] == player) val -= mod * 140;
                else if ((board[1][1] == player && board[2][2] == player) || (board[5][5] == player && board[6][6] == player)) val -= mod * 70;        
            }
            if (board[0][7] != player && board[7][0] != player) {
                if (board[1][6] == player && board[2][5] == player && board[3][4] == player && board[4][3] == player && board[5][2] == player && board[6][1] == player) val -= mod * 140;
                else if ((board[1][6] == player && board[2][5] == player) || (board[5][2] == player && board[6][1] == player)) val -= mod * 70;
            }
            
            // spots.
            if (0 < spots.y && spots.y < 7 && (spots.x == 0 || spots.x == 7)) {
                if (board[spots.x][spots.y-1] == 3 - player ^ board[spots.x][spots.y-1] == 3 - player) val -= mod * 14;
                else val += mod * 14;
            }
            else if (0 < spots.x && spots.x < 7 && (spots.y == 0 || spots.y == 7)) {
                if (board[spots.x-1][spots.y] == 3 - player ^ board[spots.x-1][spots.y-1] == 3 - player) val -= mod * 14;
                else val += mod * 14;
            }
        }

        for (int i = 0; i < SIZE; i++) {
            if (board[i][0] == player && board[i][7] == player && board[i][1] == player && board[i][2] == player && board[i][3] == player && board[i][4] == player && board[i][5] == player && board[i][6] == player) {
                val += mod * 70;
                if (i == 0 || i == 7) val += mod * 140;
            }
            if (board[0][i] == player && board[7][i] == player && board[1][i] == player && board[2][i] == player && board[3][i] == player && board[4][i] == player && board[5][i] == player && board[6][i] == player) {
                val += mod * 70;
                if (i == 0 || i == 7) val += mod * 140;
            }
        }
        if (board[0][0] == player && board[7][7] == player && board[1][1] == player && board[2][2] == player && board[3][3] == player && board[4][4] == player && board[5][5] == player && board[6][6] == player) val += mod * 140;
        if (board[0][7] == player && board[7][0] == player && board[1][6] == player && board[2][5] == player && board[3][4] == player && board[4][3] == player && board[5][2] == player && board[6][1] == player) val += mod * 140;

        return val;
    }

    void extend()
    {
        for (auto spots : next_valid_spots) {
            OthelloBoard next;
            for (int i = 0; i < SIZE; i++) for (int j = 0; j < SIZE; j++) next.board[i][j] = board[i][j];
            for (int i = 0; i < 3; i++) next.disc_count[i] = disc_count[i];
            next.cur_player = this->cur_player;
            next.done = false;
            next.winner = -1;
            bool dum = next.put_disc(spots);
            next.spots = spots;
            next.state_val = next.get_state_val();
            this->next_boards.push_back(next);
        }
    }
};

struct OthelloCmp
{
    bool operator() (const OthelloBoard a, const OthelloBoard b) {
        return a.state_val > b.state_val;
    }
};

const int SIZE = 8;
int Depth = 6;
OthelloBoard initialBoard;
Point action;
bool found_action = false;

void read_board(std::ifstream& fin) {
    fin >> initialBoard.cur_player;
    initial_player = initialBoard.cur_player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> initialBoard.board[i][j];
            if (initialBoard.board[i][j] == 0) initialBoard.disc_count[0]++;
            else if (initialBoard.board[i][j] == 1) initialBoard.disc_count[1]++;
            else if (initialBoard.board[i][j] == 2) initialBoard.disc_count[2]++;
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    for (int i = 0; i < n_valid_spots; i++) {
        Point nw;
        fin >> nw.x >> nw.y;
        initialBoard.next_valid_spots.push_back(nw); 
    }
}

std::ofstream debug("debug.txt");

int alphabeta(OthelloBoard b, int depth, int alpha, int beta, bool maximizing_player, std::ofstream& fout)
{
    if (depth == 0 || b.done == true || b.winner != -1) {
        return b.state_val;
    }
    b.extend();
    // Black.
    if (maximizing_player) {
        int val = -100000000;
        for (auto next : b.next_boards) {
            int tmp = alphabeta(next, depth - 1, alpha, beta, false, fout);
            if (depth == Depth) {
                if (val < tmp) {
                    action = next.spots;
                    debug << "state_val: " << next.state_val;
                    debug << ", alphabeta: " << tmp;
                    debug << ", found: " << action.x << " " << action.y << std::endl;
                    fout << action.x << " " << action.y << std::endl;
                    fout.flush();
                    found_action = true;
                }
            }
            if (val < tmp) val = tmp;
            alpha = std::max(alpha, val);
            if (alpha >= beta) break;
        }
        return val;
    }
    // White.
    else {
        int val = 100000000;
        for (auto next : b.next_boards) {
            val = std::min(val, alphabeta(next, depth - 1, alpha, beta, true, fout));
            beta = std::min(beta, val);
            if (alpha <= beta) break;
        }
        return val;
    }
}

void write_valid_spot(std::ofstream& fout) {
    debug << "initial_player: " << initial_player << std::endl;
    int target = alphabeta(initialBoard, Depth, -1000000000, 1000000000, true, fout);
    debug << "finished\n";
    // Remember to flush the output to ensure the last action is written to file.
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    debug.close();
    fin.close();
    fout.close();
    return 0;
}
