#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include<vector>
#include <climits>
using namespace std;
struct Point {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
    Point operator+(const Point& p) const
    {
        int new_x=x+p.x;
        int new_y=y+p.y;
        return Point(new_x,new_y);
    }
    bool operator==(const Point& p) const
    {
        if(x!=p.x) return false;
        else if(y!=p.y) return false;
        else return true;
    }
    bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
    
};
int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream& fout);
int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
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
private:
    int get_next_player(int player) const {
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
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard() {
        reset();
    }
    OthelloBoard(std::array<std::array<int, SIZE>, SIZE> curboard,int idx):cur_player(idx)
    {
        disc_count[WHITE]=disc_count[BLACK]=disc_count[EMPTY]=0;
        for(int i=0;i<8;i++)
        {
            for(int j=0;j<8;j++)
            {
                board[i][j]=curboard[i][j];
                if(board[i][j]==0) disc_count[EMPTY]++;
                else if(board[i][j]==1) disc_count[BLACK]++;
                else if(board[i][j]==2) disc_count[WHITE]++;
            }
        }
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8*8-4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
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
                // Game ends
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
    int miniMax(int cur_depth,int depth,int alpha,int beta,bool maxplayer);
};

//GOAL:搶角(0,0) (7,7) (0,7) (7,0)
//搶邊(0,x) (x,0) (7,x) (x,7) >>以能與角連結為優先
//注意 不要下X位 (1,1) (1,6) (6,1) (6,6) 
// 跟C位(0,1) (1,0) (6,0) (1,7) (7,6) (6,7) (0,6) (7,1)
void write_valid_spot(std::ofstream& fout)
{
    int n_valid_spots = next_valid_spots.size();
    srand(time(NULL));
    OthelloBoard curState(board,player);
    vector<int> curStateval;
  /*  if(curState.disc_count[1]+curState.disc_count[2]<10)
    {
    }*/
    for(int depth=0;depth<8;depth++)
    {
        curStateval.resize(n_valid_spots,0);
        for(int i=0;i<n_valid_spots;i++)
        {
            Point x=next_valid_spots[i];
            OthelloBoard temp=curState;
            temp.put_disc(x);//curplayer change
            curStateval[i]=temp.miniMax(0,depth,INT_MIN,INT_MAX,true);
        }
        if(player==curState.BLACK)
        {
            int max=INT_MIN;
            for(int i=0;i<n_valid_spots;i++)
            {
                if(curStateval[i]>max)
                {
                    max=curStateval[i];
                    fout<<next_valid_spots[i].x<<" "<<next_valid_spots[i].y<<endl;
                    fout.flush();
                }
            }
        }
        else if(player==curState.WHITE)
        {
            int min=INT_MAX;
            for(int i=0;i<n_valid_spots;i++)
            {
                if(curStateval[i]<min)
                {
                    min=curStateval[i];
                    fout<<next_valid_spots[i].x<<" "<<next_valid_spots[i].y<<endl;
                    fout.flush();
                    
                }
            }
        }
    }
    
}
//min 修改 beta
//max 修改 alpha
int OthelloBoard::miniMax(int cur_depth,int depth,int alpha,int beta,bool maxplayer)
{
    if(cur_depth==depth || cur_depth==0)
    {
        int total=0;
        int good_C_num=0;
        int bad_C_num=0;   //good 對black而言
        int X_num=0;
        int good_X_num=0;
        int bad_X_num=0;
        int corner_num=0;
        int side_num=0;
        int mobility_num=0;
        int more_num=0;
        int mid_num=0;
        int better_side_num=0;
        next_valid_spots=get_valid_spots();
        for(int i=0;i<next_valid_spots.size();i++)
        {
            //corner
            if( next_valid_spots[i]==Point(0,0) ||next_valid_spots[i]==Point(7,0) ||next_valid_spots[i]==Point(0,7) ||next_valid_spots[i]==Point(7,7) )
            {
                corner_num++;
            }
            //C 
            else if(next_valid_spots[i]==Point(7,6)||next_valid_spots[i]==Point(6,7))
            {
                if(board[7][7]==BLACK) good_C_num++;
                else bad_C_num++;
            }
            else if(next_valid_spots[i]==Point(0,1)||next_valid_spots[i]==Point(1,0))
            {
                if(board[0][0]==BLACK) good_C_num++;
                else bad_C_num++;
            }
            else if(next_valid_spots[i]==Point(6,0)||next_valid_spots[i]==Point(7,1))
            {
                if(board[7][0]==BLACK) good_C_num++;
                else bad_C_num++;
            }
            else if(next_valid_spots[i]==Point(0,6)||next_valid_spots[i]==Point(1,7))
            {
                if(board[0][7]==BLACK) good_C_num++;
                else bad_C_num++;
            }
            //X
            else if(next_valid_spots[i]==Point(1,1))
            {
                if(board[0][0]==BLACK) good_X_num++;
                else bad_X_num++;
            }
            else if(next_valid_spots[i]==Point(1,6))
            {
                if(board[0][7]==BLACK) good_X_num++;
                else bad_X_num++;
            }
             else if(next_valid_spots[i]==Point(6,1))
            {
                if(board[0][7]==BLACK) good_X_num++;
                else bad_X_num++;
            }
             else if(next_valid_spots[i]==Point(6,6))
            {
                if(board[7][7]==BLACK) good_X_num++;
                else bad_X_num++;
            }
            //four side
            else if(next_valid_spots[i].x==0 && (next_valid_spots[i]!=Point(0,1) ||next_valid_spots[i]!=Point(0,6) ) )
            {
                //if(next_valid_spots[i].y==2 || next_valid_spots[i].y==5) better_side_num++;
                side_num++;
            }
            else if(next_valid_spots[i].x==7 && (next_valid_spots[i]!=Point(7,6) ||next_valid_spots[i]!=Point(7,1) ) )
            {
                //if(next_valid_spots[i].y==2 || next_valid_spots[i].y==5) better_side_num++;
                side_num++;
            }
            else if(next_valid_spots[i].y==7 && (next_valid_spots[i]!=Point(6,7) ||next_valid_spots[i]!=Point(1,7) ) )
            {
                //if(next_valid_spots[i].x==2 || next_valid_spots[i].x==5) better_side_num++;
                 side_num++;
            }
             else if(next_valid_spots[i].y==0 && (next_valid_spots[i]!=Point(1,0) ||next_valid_spots[i]!=Point(6,0) ) )
            {
                //if(next_valid_spots[i].x==2 || next_valid_spots[i].x==5) better_side_num++;
                side_num++;
            }
            
           
        }
        //mobility
        if(disc_count[BLACK]+disc_count[WHITE]<40)
        {
            mobility_num=(next_valid_spots.size())*2;
            more_num=disc_count[BLACK]-disc_count[WHITE];
            //mid_num*=5;
        }
        //black num vs white num
        else if(disc_count[BLACK]+disc_count[WHITE]>=40)
        {
            mobility_num=next_valid_spots.size();
            more_num=(disc_count[BLACK]-disc_count[WHITE])*2;
        }
       
        
        if(cur_player==BLACK)
        {
            total+=corner_num*800-bad_X_num*300-bad_C_num*300+mobility_num*30+good_C_num*250+good_X_num*200+side_num*90+more_num*40;
        }
        else if(cur_player==WHITE)
        {
            total-=corner_num*800-bad_X_num*300-bad_C_num*300+mobility_num*30+good_C_num*250+good_X_num*200+side_num*90+more_num*40;
        }
        return total;
    }
    if(cur_player==BLACK)
    {
        next_valid_spots=get_valid_spots();
        int val=INT_MIN;
        for(auto i: next_valid_spots)
        {
            OthelloBoard tmp=*this;
            tmp.put_disc(i);
            val=max(val,miniMax(cur_depth+1,depth,alpha,beta,false));
            alpha=max(alpha,val);
            if(alpha>=beta)
            {
                break;
            }
            return val;
        }
    }
    else if(cur_player==WHITE)
    {
        next_valid_spots=get_valid_spots();
        int val=INT_MAX;
        for(auto i:next_valid_spots)
        {
            OthelloBoard tmp=*this;
            tmp.put_disc(i);
            val=min(val,miniMax(cur_depth+1,depth,alpha,beta,true));
            beta=min(beta,val);
            if(beta<=alpha)
            {
                break;
            }
        }
        return val;
    }
}