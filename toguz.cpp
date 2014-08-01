#include <iostream>

using namespace std;

const static size_t K = 9; // pits
const static size_t N = 9; // balls per pit
const static size_t D = 8; // minimax search depth depth 

struct Board {
    typedef char balls_t;
    typedef char socket_t;

    balls_t sockets[2 * K];
    socket_t tuzdeks[2]; // socket idx of player's tuzdek (on the opponent's side)
    balls_t kaznas[2];

    Board() {
        for (auto i = 0; i < 2 * K; ++i) {
            sockets[i] = N;
        }
        tuzdeks[0] = tuzdeks[1] = -1;
        kaznas[0] = kaznas[1] = 0;
    }

    socket_t playSocket(const socket_t p) {
        if (sockets[p] == 1) {
            sockets[p] = 0;
            sockets[idx(p + 1)] += 1;
            return idx(p + 1);
        }
        for (socket_t i = p + 1; i < p + sockets[p]; ++i) {
            sockets[idx(i)] += 1;
        }
        socket_t result = idx(p + sockets[p] - 1);
        sockets[p] = 1;
        return result;
    }

    bool tuzdekPossible(const socket_t s, const size_t player) const {
        return sockets[s] == 3
                && (s / K == (1 - player)) // opposite side
                && ((s + 1) % K) != 0 // not the last
                && tuzdeks[player] == -1 // not having tuzdek
                && (tuzdeks[1 - player] == -1 || tuzdeks[1 - player] % K != (s % K)); // not symmetrical tuzdek
    }

    void accountSocket(const socket_t s, const size_t player) {
        if (s / K == (1 - player) // correct side
                && sockets[s] % 2 == 0) { // or even
            kaznas[player] += sockets[s];
            sockets[s] = 0;
        }
        for (auto playerIt = 0; playerIt < 2; ++playerIt) {
            if (tuzdeks[playerIt] != -1) {
                kaznas[playerIt] += sockets[tuzdeks[playerIt]];
                sockets[tuzdeks[playerIt]] = 0;
            }
        }
    }

    void pli(const socket_t s, const bool tuzdek, const size_t player) {
        auto target = playSocket(s);
        if (tuzdek) {
            tuzdeks[player] = target;
        }
        accountSocket(target, player);
    }

    Board rotate() const {
        Board result(*this);
        result.kaznas[0] = kaznas[1];
        result.kaznas[1] = kaznas[0];

        result.tuzdeks[0] = tuzdeks[1];
        result.tuzdeks[1] = tuzdeks[0];

        for (auto i = 0; i < K; ++i) {
            result.sockets[i] = sockets[K + i];
            result.sockets[K + i] = sockets[i];
        }
        return result;
    }

    static socket_t idx(const socket_t s) {
        return s % (2 * K);
    }
};

ostream& operator<<(ostream& os, const Board& board) {
    for (char player = 1; player >= 0; --player) {
        os << static_cast<int> (player) << ":\t";
        for (auto i = 0; i < K; ++i) {
            Board::socket_t idx = player ? (2 * K - i - 1) : i;
            os << " " << static_cast<int> (board.sockets[idx]);
            if (board.tuzdeks[(1 - player)] == (idx)) {
                os << "*";
            }
            os << "\t";
        }
        os << "Kazna: " << static_cast<int> (board.kaznas[player]);
        os << endl;
    }
    os << "\t";
    for (auto i = 0; i < K; ++i) {
        os << "-" << (i + 1) << "-\t";
    }
    os << endl;
}

int minimax(Board &board, size_t depth, size_t player, Board::socket_t &move, bool &tuzdek) {
    if (depth == 0 || board.kaznas[0] > K * N || board.kaznas[1] > K * N) {
        move = -1;
        return board.kaznas[0];
    }

    Board::socket_t dummyMove;
    bool dummyTuzdek;

    int sign = (player == 0) ? 1 : -1;
    int bestValue = -127 * sign;
    bool played = false;
    for (Board::socket_t i = player * K; i < player * K + K; ++i) {
        Board localBoard(board);
        if (localBoard.sockets[i] == 0) {
            continue;
        }
        played = true;
        auto target = localBoard.playSocket(i);
        int withTuzdek = -127 * sign;
        if (localBoard.tuzdekPossible(target, player)) {
            Board tuzdekBoard(localBoard);
            tuzdekBoard.tuzdeks[player] = target;
            tuzdekBoard.accountSocket(target, player);

            withTuzdek = minimax(tuzdekBoard, depth - 1, 1 - player, dummyMove, dummyTuzdek);
        }

        localBoard.accountSocket(target, player);
        int withoutTuzdek = minimax(localBoard, depth - 1, 1 - player, dummyMove, dummyTuzdek);


        if (sign * withoutTuzdek > sign * bestValue) {
            move = i;
            bestValue = withoutTuzdek;
        }
        if (sign * withTuzdek > sign * bestValue) {
            tuzdek = true;
            bestValue = withTuzdek;
        } else {
            tuzdek = false;
        }
    }

    if (played) {
        return bestValue;
    } else {
        move = -1;
        cout << "!!!!!" << endl
                << board << endl << "!!!!!" << endl;
        return board.kaznas[0];
    }
}

void busy() {
    cout << "thinking..." << endl;
    //cout.flush();
}

void unbusy() {
    //    cout << "rrrrrrrrrrrr\r";
    //    cout.flush();
}

int main(int argc, char **argv) {
    Board board;
    int value;
    Board::socket_t move;
    bool tuzdek;

    char answer;
    cout << "Do you want to begin (Y/N)? ";
    cin >> answer;
    bool firstComp = (answer != 'Y');

    while (true) {
        if (firstComp) {
            busy();
            minimax(board, D, 0, move, tuzdek);
            board.pli(move, tuzdek, 0);
            unbusy();
            firstComp = false;
        }

        int playerMove;
        cout << board.rotate();

        if (board.kaznas[1] > K * N) {
            cout << "You win." << endl;
            break;
        } else if (board.kaznas[0] > K * N) {
            cout << "You lose." << endl;
            break;
        }

        cout << "Your move: ";
        cin >> playerMove;
        //playerMove = std::rand() % 9 + 1;
        Board::socket_t pMove = playerMove + K - 1;
        if (board.sockets[pMove] == 0) {
            cout << "Cannot play at " << playerMove << endl;
            continue;
        }

        auto target = board.playSocket(pMove);
        if (board.tuzdekPossible(target, 1)) {
            cout << "Create tuzdek at opponent's " << (static_cast<int> (target) + 1) << " (Y/N) ";
            cin >> answer;
            if (answer == 'Y') {
                board.tuzdeks[1] = target;
            }
        }
        board.accountSocket(target, 1);

        busy();
        auto value = minimax(board, D, 0, move, tuzdek);
        if (move == -1) {
            cout << "Game ended." << endl;
            // TODO determine winner from kaznas
            break;
        }
        board.pli(move, tuzdek, 0);
        cout << "Opponent's move: " << (static_cast<int> (move) + 1) << " (worst outcome: " << value << ")" << endl;
        unbusy();
    }



    return 0;
}
