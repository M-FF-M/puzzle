#include <bits/stdc++.h>

#define GET_X first.first
#define GET_Y first.second
#define GET_NUM second
#define GET_NUM_B second.first
#define GET_PIECE second.second

using namespace std;

typedef pair<int, int> ii;
typedef pair<ii, int> iii;
typedef pair<ii, ii> iiii;

int solcnt = 0;
long long totpos = 0, checkedpos = 0, lastwpos = 0, addcnt[1000];
stringstream solstr;
map<int, string> color_map;
map<int, char> id_map;
const set<iii> EMPTY_SET;

vector<ii> rotate_blocks(const vector<ii>& blocks, int rotby) {
  vector<ii> ret;
  for (int i = 0; i < blocks.size(); i++) {
    ii n(blocks[i].first, blocks[i].second);
    if (rotby == 1) {
      n.first = -blocks[i].second; n.second = blocks[i].first;
    } else if (rotby == 2) {
      n.first = -blocks[i].first; n.second = -blocks[i].second;
    } else if (rotby == 3) {
      n.first = blocks[i].second; n.second = -blocks[i].first;
    }
    ret.push_back(n);
  }
  return ret;
}

vector<ii> mirror_blocks(const vector<ii>& blocks) {
  vector<ii> ret;
  for (int i = 0; i < blocks.size(); i++) {
    ii n(blocks[i].first, -blocks[i].second);
    ret.push_back(n);
  }
  return ret;
}

struct board {
  int width, height;
  int** content;
  set<int> fixed;

  set<iiii>** posmap;
  set<iiii>** cposmap;

  board(string inp_file) {
    ifstream fin(inp_file);
    fin >> width >> height;
    content = new int*[width];
    posmap = new set<iiii>*[width];
    cposmap = new set<iiii>*[width];
    for (int i = 0; i < width; i++) {
      content[i] = new int[height];
      posmap[i] = new set<iiii>[height];
      cposmap[i] = new set<iiii>[height];
    }
    char c;
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        fin >> c;
        if (c == '.') content[i][j] = 0;
        else if (c == '#') content[i][j] = -1;
        else {
          content[i][j] = 1 + ((int) (c - 'A'));
          fixed.insert(content[i][j]);
        }
      }
    }
    fin.close();
  }

  void resetpos() {
    for (int j = 0; j < height; j++)
      for (int i = 0; i < width; i++)
        cposmap[i][j] = posmap[i][j];
  }

  ~board() {
    for (int i = 0; i < width; i++) {
      delete[] content[i];
      delete[] posmap[i];
      delete[] cposmap[i];
    }
    delete[] content;
    delete[] posmap;
    delete[] cposmap;
  }
};

struct piece {
  vector<ii> blocks;
  string color;
  char id_char;
  int id;
  bool req_mirror;
  bool req_180rot;
  bool fixed = false;

  vector< vector<ii> > all_orient;
  set<iii> pos;
  set<iii> cpos;

  piece(ifstream& fin) {
    int num_blocks, x, y;
    char mirror, rot180;
    fin >> num_blocks >> mirror >> rot180 >> id_char;
    id = 1 + ((int) (id_char - 'A'));
    if (mirror == 'y') req_mirror = true;
    else req_mirror = false;
    if (rot180 == 'y') req_180rot = true;
    else req_180rot = false;
    blocks.push_back(ii(0, 0));
    for (int i = 0; i < num_blocks - 1; i++) {
      fin >> x >> y;
      blocks.push_back(ii(x, y));
    }
    fin >> color;

    int rot_num = 4;
    if (!req_180rot) rot_num = 2;
    for (int i = 0; i < rot_num; i++) {
      all_orient.push_back(rotate_blocks(blocks, i));
      if (req_mirror)
        all_orient.push_back(mirror_blocks(all_orient.back()));
    }
    id_map[id] = id_char;
    color_map[id] = color;
  }

  void fillpos(board& b) {
    pos.clear();
    if (b.fixed.find(id) == b.fixed.end()) {
      for (int x = 0; x < b.width; x++) {
        for (int y = 0; y < b.height; y++) {
          if (b.content[x][y] == 0) {
            for (int i = 0; i < all_orient.size(); i++) {
              bool can_place = true;
              for (int j = 0; j < all_orient[i].size() && can_place; j++) {
                int x2 = x + all_orient[i][j].first;
                int y2 = y + all_orient[i][j].second;
                if (x2 < 0 || y2 < 0 || x2 >= b.width || y2 >= b.height || b.content[x2][y2] != 0)
                  can_place = false;
              }
              if (can_place) {
                pos.insert(iii(ii(x, y), i));
                for (int j = 0; j < all_orient[i].size() && can_place; j++) {
                  int x2 = x + all_orient[i][j].first;
                  int y2 = y + all_orient[i][j].second;
                  b.posmap[x2][y2].insert(iiii(ii(x, y), ii(i, id)));
                }
              }
            }
          }
        }
      }
    } else {
      fixed = true;
    }
    cpos = pos;
  }

  void resetpos() {
    cpos = pos;
  }

  bool operator< (const piece& b) {
    return cpos.size() < b.cpos.size();
  }
};

void print_state(stringstream& sstr, const board& b, double ox, double oy, double scale) {
  sstr << fixed << setprecision(3);
  for (int x = 0; x < b.width; x++) {
    for (int y = 0; y < b.height; y++) {
      if (b.content[x][y] >= 0) {
        sstr << "<circle cx=\"" << (ox + x * scale * 30.0) << "\" cy=\"" << (oy + y * scale * 30.0) << "\" r=\"" << (scale * 15.0) << "\" fill=\"#333333\" />" << endl;
      }
    }
  }
  int dirs[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
  for (int x = 0; x < b.width; x++) {
    for (int y = 0; y < b.height; y++) {
      if (b.content[x][y] > 0) {
        for (int i = 0; i < 4; i++) {
          int x2 = x + dirs[i][0];
          int y2 = y + dirs[i][1];
          if (x2 >= 0 && y2 >= 0 && x2 < b.width && y2 < b.height && b.content[x2][y2] == b.content[x][y]) {
            double width = 6.0;
            double height = 12.0;
            if (abs(x - x2) == 1) {
              width = 12.0;
              height = 8.0;
            }
            sstr << "<rect x=\"" << (ox + 0.5 * ((x + x2) * 30.0 - width) * scale) << "\" y=\"" << (oy + 0.5 * ((y + y2) * 30.0 - height) * scale ) << "\" width=\"" << (scale * width) << "\" height=\"" << (scale * height) << "\" fill=\"" << color_map[b.content[x][y]] << "\" />" << endl;
          }
        }
      }
    }
  }
  for (int x = 0; x < b.width; x++) {
    for (int y = 0; y < b.height; y++) {
      if (b.content[x][y] > 0) {
        sstr << "<circle cx=\"" << (ox + x * scale * 30.0) << "\" cy=\"" << (oy + y * scale * 30.0) << "\" r=\"" << (scale * 11.0) << "\" fill=\"" << color_map[b.content[x][y]] << "\" />" << endl;
        sstr << "<text x=\"" << (ox + x * scale * 30.0) << "\" y=\"" << (oy + y * scale * 30.0) << "\" text-anchor=\"middle\" alignment-baseline=\"middle\" font-size=\"" << scale << "em\" font-family=\"Arial, Helvetica, sans-serif\">" << id_map[b.content[x][y]] << "</text>" << endl;
      }
    }
  }
}

bool rec_solve(bool find_all_sol, board& b, vector<piece>& pieces, map<int, int>& piece_map, int piece_idx, set<iii>::iterator& pos) {
  if (piece_idx >= pieces.size()) {
    print_state(solstr, b, 20.0 + solcnt * (10.0 + b.width * 30.0), 30.0 + b.height * 30.0, 1.0);
    solcnt++;
    checkedpos++;
    return true;
  }
  piece& cpiece = pieces[piece_idx];
  // set<iii>::iterator pos = cpiece.cpos.begin();
  // advance(pos, pos_idx);
  if (pos == cpiece.cpos.end()) return false;

  if (pos == cpiece.cpos.begin() && pieces[piece_idx].cpos.size() < pieces[piece_idx].pos.size())
    checkedpos += (pieces[piece_idx].pos.size() - pieces[piece_idx].cpos.size()) * addcnt[piece_idx + 1];

  int x = (*pos).GET_X;
  int y = (*pos).GET_Y;
  int idx = (*pos).GET_NUM;
  const vector<ii>& blocks = cpiece.all_orient[idx];

  bool solvable = true, solved = false;
  set<iiii> erased;

  for (int j = 0; j < blocks.size(); j++) {
    int x2 = x + blocks[j].first;
    int y2 = y + blocks[j].second;
    b.content[x2][y2] = cpiece.id;
    for (const iiii& item : b.cposmap[x2][y2]) {
      if (piece_map[item.GET_PIECE] > piece_idx) {
        iii pitem = iii(ii(item.GET_X, item.GET_Y), item.GET_NUM_B);
        if (pieces[ piece_map[item.GET_PIECE] ].cpos.find(pitem) != pieces[ piece_map[item.GET_PIECE] ].cpos.end()) {
          pieces[ piece_map[item.GET_PIECE] ].cpos.erase(pitem);
          if (pieces[ piece_map[item.GET_PIECE] ].cpos.size() == 0)
            solvable = false;
          erased.insert(item);
        }
      }
    }
  }

  if (solvable) {
    set<iii>::iterator iter = pieces[piece_idx + 1].cpos.begin();
    solved = rec_solve(find_all_sol, b, pieces, piece_map, piece_idx + 1, iter);
  } else {
    checkedpos += addcnt[piece_idx + 1];
  }
  
  for (int j = 0; j < blocks.size(); j++) {
    int x2 = x + blocks[j].first;
    int y2 = y + blocks[j].second;
    b.content[x2][y2] = 0;
  }
  for (const iiii& item : erased) {
    pieces[ piece_map[item.GET_PIECE] ].cpos.insert( iii(ii(item.GET_X, item.GET_Y), item.GET_NUM_B) );
  }

  if (checkedpos - lastwpos > 1000) {
    cout << "\rProgress: " << fixed << setprecision(1) << (100.0 * checkedpos / totpos) << "%";
    if (solcnt == 1) cout << " (1 solution)";
    else if (solcnt > 1) cout << " (" << solcnt << " solutions)";
    lastwpos = checkedpos;
  }

  if (!solved || find_all_sol)
    return rec_solve(find_all_sol, b, pieces, piece_map, piece_idx, ++pos) || solved;
  else
    return solved;
}

void solve(bool find_all_sol, board& b, vector<piece>& pieces, map<int, int>& piece_map, string out_file) {
  solcnt = 0;
  solstr.clear();
  addcnt[pieces.size()] = 1;
  totpos = 1; checkedpos = 0; lastwpos = 0;
  for (int i = pieces.size() - 1; i >= 0; i--)
    addcnt[i] = pieces[i].cpos.size() * addcnt[i + 1];
  cout << endl;
  for (int i = 0; i < pieces.size(); i++) {
    cout << "Piece " << pieces[i].id_char << " : " << pieces[i].pos.size() << " possible placements" << endl;
    totpos *= pieces[i].cpos.size();
  }
  cout << endl;
  set<iii>::iterator iter = pieces[0].cpos.begin();
  bool foundsol = rec_solve(find_all_sol, b, pieces, piece_map, 0, iter);
  cout << "\rCalculation completed. Checked " << checkedpos << " out of " << totpos << " positions (" << fixed << setprecision(1) << (100.0 * checkedpos / totpos) << "%)." << endl;
  if (find_all_sol) {
    if (foundsol) {
      if (solcnt == 1)
        cout << "Found 1 solution!" << endl;
      else
        cout << "Found " << solcnt << " solutions!" << endl;
    } else
      cout << "Unsolvable!" << endl;
  } else {
    if (foundsol)
      cout << "Found solution!" << endl;
    else
      cout << "Unsolvable!" << endl;
  }
  ofstream fout(out_file);
  fout << fixed << setprecision(3) << endl;
  fout << "<svg version=\"1.1\" baseProfile=\"full\" width=\"" << (max(1, solcnt) * (10.0 + b.width * 30.0)) << "\" height=\"" << ((1 + min(1, solcnt)) * (10.0 + b.height * 30.0)) << "\" xmlns=\"http://www.w3.org/2000/svg\">" << endl;
  stringstream cstate;
  print_state(cstate, b, 20, 20, 1);
  fout << cstate.str();
  fout << solstr.str();
  fout << "</svg>" << endl;
  fout.close();
}

int main(int argc, char** argv) {
  string piece_in = "pieces.txt";
  string puzzle_in = "board.txt";
  bool find_all_sol = false;
  if (argc >= 2) puzzle_in = argv[1];
  if (argc >= 3) piece_in = argv[2];
  if (argc >= 4) {
    if (argv[3][0] == 'y') find_all_sol = true;
  }
  string puzzle_out = puzzle_in.substr(0, puzzle_in.size() - 4) + "-sol.svg";
  cout << "Puzzle solver" << endl << endl;
  cout << "1st argument: board input file -- " << puzzle_in << endl;
  cout << "2nd argument: piece input file -- " << piece_in << endl;
  cout << "3rd argument: whether to find all solutions (y) or just any solution (n) -- " << (find_all_sol ? "y" : "n") << endl;
  cout << "output file -- " << puzzle_out << endl;

  board b(puzzle_in);
  vector<piece> pieces;
  vector<piece> pieces_movable;
  vector<piece> pieces_fixed;
  ifstream fin(piece_in);
  int num_pieces;
  fin >> num_pieces;
  for (int i = 0; i < num_pieces; i++) {
    piece p(fin);
    pieces.push_back(p);
  }
  fin.close();

  map<int, int> mov_map;
  for (int i = 0; i < num_pieces; i++) {
    pieces[i].fillpos(b);
    if (pieces[i].fixed) pieces_fixed.push_back(pieces[i]);
    else pieces_movable.push_back(pieces[i]);
  }
  b.resetpos();
  sort(pieces_movable.begin(), pieces_movable.end());
  for (int i = 0; i < pieces_movable.size(); i++)
    mov_map[pieces_movable[i].id] = i;
  solve(find_all_sol, b, pieces_movable, mov_map, puzzle_out);
}