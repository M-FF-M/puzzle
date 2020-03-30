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

int solcnt = 0, actposcheck = 0;
long long totpos = 0, checkedpos = 0, lastwpos = 0, addcnt[1000];
long long starttime = 0;
stringstream solstr;
stringstream soltxtstr;
map<int, string> color_map;
map<int, char> id_map;
const set<iii> EMPTY_SET;

long long get_ms() {
  return (long long) (chrono::system_clock::now().time_since_epoch() / chrono::milliseconds(1));
}

string to_num(long long num, int num_zeroes = 2) {
  stringstream tonst;
  tonst << num;
  string ret = tonst.str();
  while (ret.size() < num_zeroes)
    ret = "0" + ret;
  return ret;
}

string to_timestr(long long ms) {
  long long msecs = ms % 1000; ms /= 1000;
  long long secs = ms % 60; ms /= 60;
  long long mins = ms % 60; ms /= 60;
  return to_num(ms, 1) + ":" + to_num(mins) + ":" + to_num(secs) + "." + to_num(msecs, 3);
}

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

  board(ifstream& fin) {
    fin >> width >> height;
    content = new int*[width];
    for (int i = 0; i < width; i++)
      content[i] = new int[height];
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
  }

  board(ifstream& fin, int w, int h) {
    width = w; height = h;
    content = new int*[width];
    for (int i = 0; i < width; i++)
      content[i] = new int[height];
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
  }

  bool operator< (const board& b) const {
    return true;
  }

  void clear() {
    for (int i = 0; i < width; i++)
      delete[] content[i];
    delete[] content;
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

  void updatefixed(board& b) {
    if (b.fixed.find(id) == b.fixed.end())
      fixed = false;
    else
      fixed = true;
  }
};

pair<double, double> rot45(int x, int y) {
  double x2 = 0.7071068 * x - 0.7071068 * y;
  double y2 = 0.7071068 * x + 0.7071068 * y;
  return pair<double, double>(x2, y2);
}

void print_state(stringstream& sstr, const board& b, double ox, double oy, double scale) {
  sstr << fixed << setprecision(3);
  for (int x = 0; x < b.width; x++) {
    for (int y = 0; y < b.height; y++) {
      pair<double, double> nc = rot45(x, y); double x2 = nc.first;  double y2 = nc.second;
      if (b.content[x][y] >= 0) {
        sstr << "<circle cx=\"" << (ox + x2 * scale * 30.0) << "\" cy=\"" << (oy + y2 * scale * 30.0) << "\" r=\"" << (scale * 15.0) << "\" fill=\"#333333\" />" << endl;
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
              height = 6.0;
            }
            pair<double, double> nc_a = rot45((x + x2) * 30.0 - width, (y + y2) * 30.0 + height); double x_a = nc_a.first;  double y_a = nc_a.second;
            pair<double, double> nc_b = rot45((x + x2) * 30.0 + width, (y + y2) * 30.0 + height); double x_b = nc_b.first;  double y_b = nc_b.second;
            pair<double, double> nc_c = rot45((x + x2) * 30.0 + width, (y + y2) * 30.0 - height); double x_c = nc_c.first;  double y_c = nc_c.second;
            pair<double, double> nc_d = rot45((x + x2) * 30.0 - width, (y + y2) * 30.0 - height); double x_d = nc_d.first;  double y_d = nc_d.second;
            sstr << "<path d=\"M" << (ox + 0.5 * x_a * scale) << " " << (oy + 0.5 * y_a * scale) << " L"
              << (ox + 0.5 * x_b * scale) << " " << (oy + 0.5 * y_b * scale) << " L"
              << (ox + 0.5 * x_c * scale) << " " << (oy + 0.5 * y_c * scale) << " L"
              << (ox + 0.5 * x_d * scale) << " " << (oy + 0.5 * y_d * scale) << " L"
              << (ox + 0.5 * x_a * scale) << " " << (oy + 0.5 * y_a * scale) << " Z\" fill=\"" << color_map[b.content[x][y]] << "\"/>" << endl;
          }
        }
      }
    }
  }
  for (int x = 0; x < b.width; x++) {
    for (int y = 0; y < b.height; y++) {
      pair<double, double> nc = rot45(x, y); double x2 = nc.first;  double y2 = nc.second;
      if (b.content[x][y] > 0) {
        sstr << "<circle cx=\"" << (ox + x2 * scale * 30.0) << "\" cy=\"" << (oy + y2 * scale * 30.0) << "\" r=\"" << (scale * 11.0) << "\" fill=\"" << color_map[b.content[x][y]] << "\" />" << endl;
        sstr << "<text x=\"" << (ox + x2 * scale * 30.0) << "\" y=\"" << (oy + y2 * scale * 30.0) << "\" text-anchor=\"middle\" alignment-baseline=\"middle\" font-size=\"" << scale << "em\" font-family=\"Arial, Helvetica, sans-serif\">" << id_map[b.content[x][y]] << "</text>" << endl;
      }
    }
  }
}

void print_state_txt(stringstream& sstr, const board& b) {
  for (int y = 0; y < b.height; y++) {
    for (int x = 0; x < b.width; x++) {
      if (b.content[x][y] == -1) sstr << "#";
      else if (b.content[x][y] == 0) sstr << ".";
      else if (b.content[x][y] > 0) sstr << id_map[b.content[x][y]];
    }
    sstr << endl;
  }
  sstr << endl;
}

void print_problem(ostream& outs, const board& b, const board& bsol, int level, int num_sol, int time_req, int tpersol,
  int fixedpieces, int fixedblocks, double ox, double oy, bool insert_grad = false) {
    outs << fixed << setprecision(3);
    outs << "<defs>" << endl;
    double gradx = ox + 0.85 * b.width * 30.0 + 10; double gradwidth = 200.0;
    double grady = oy - 0.2 * b.height * 30.0 + 165;
    outs << "<linearGradient id=\"grad1\">" << endl;
    outs << "<stop offset=\"0%\" style=\"stop-color:#99cc00;stop-opacity:1\" />" << endl;
    outs << "<stop offset=\"50%\" style=\"stop-color:#ffcc00;stop-opacity:1\" />" << endl;
    outs << "<stop offset=\"100%\" style=\"stop-color:#cc0000;stop-opacity:1\" />" << endl;
    outs << "</linearGradient>" << endl;
    outs << "</defs>" << endl;
    outs << "<rect x=\"" << gradx << "\" y=\"" << grady << "\" width=\"" << gradwidth << "\" height=\"30\"  fill=\"url(#grad1)\" />" << endl;
    
    outs << "<rect x=\"" << (gradx + level / 1000.0 * gradwidth - 1) << "\" y=\"" << grady << "\" width=\"2\" height=\"30\"  fill=\"black\" />" << endl;
    outs << "<path d=\"M" << (gradx + level / 1000.0 * gradwidth) << " " << (grady - 5) << " L"
      << (gradx + level / 1000.0 * gradwidth - 10) << " " << (grady - 15) << " L"
      << (gradx + level / 1000.0 * gradwidth + 10) << " " << (grady - 15) << " L"
      << (gradx + level / 1000.0 * gradwidth) << " " << (grady - 5) << " Z\" fill=\"black\" />" << endl;
    outs << "<text x=\"" << (gradx) << "\" y=\"" << (grady - 35)
      << "\" text-anchor=\"start\" alignment-baseline=\"middle\" font-size=\"1em\" font-family=\"Arial, Helvetica, sans-serif\">"
      << "Difficulty</text>" << endl;
    outs << "<text x=\"" << (gradx) << "\" y=\"" << (grady + 50)
      << "\" text-anchor=\"start\" alignment-baseline=\"middle\" font-size=\"1em\" font-family=\"Arial, Helvetica, sans-serif\""
      << (level <= 333 ? " font-weight=\"bold\"" : "") << ">"
      << "Easy</text>" << endl;
    outs << "<text x=\"" << (gradx + 0.5 * gradwidth) << "\" y=\"" << (grady + 50)
      << "\" text-anchor=\"middle\" alignment-baseline=\"middle\" font-size=\"1em\" font-family=\"Arial, Helvetica, sans-serif\""
      << (level > 333 && level <= 666 ? " font-weight=\"bold\"" : "") << ">"
      << "Medium</text>" << endl;
    outs << "<text x=\"" << (gradx + gradwidth) << "\" y=\"" << (grady + 50)
      << "\" text-anchor=\"end\" alignment-baseline=\"middle\" font-size=\"1em\" font-family=\"Arial, Helvetica, sans-serif\""
      << (level >= 667 ? " font-weight=\"bold\"" : "") << ">"
      << "Hard</text>" << endl;

    stringstream cstate;
    print_state(cstate, b, ox + 0.4 * b.width * 30.0 + 10, oy - 0.2 * b.height * 30.0 + 40, 1);
    outs << cstate.str(); cstate.clear();
    print_state(cstate, bsol, ox + 0.4 * b.width * 30.0 + 10, oy + 0.8 * b.height * 30.0 + 60, 1);
    outs << cstate.str();
    outs << "<text x=\"" << (ox + 0.4 * b.width * 30.0 - 88) << "\" y=\"" << (oy - 0.2 * b.height * 30.0 + 85) << "\" text-anchor=\"start\" alignment-baseline=\"middle\" font-size=\"1.5em\" font-family=\"Arial, Helvetica, sans-serif\">Problem</text>" << endl;
    outs << "<text x=\"" << (ox + 0.4 * b.width * 30.0 - 88) << "\" y=\"" << (oy + 0.8 * b.height * 30.0 + 105) << "\" text-anchor=\"start\" alignment-baseline=\"middle\" font-size=\"1.5em\" font-family=\"Arial, Helvetica, sans-serif\">Solution</text>" << endl;
    vector<string> print_lines;
    print_lines.push_back(((string) "") + to_num(num_sol, 0) + " solution" + (num_sol == 1 ? "" : "s") + " in total");
    print_lines.push_back(((string) "Difficulty: ") + to_num(level, 0));
    print_lines.push_back(((string) "Computation time: ") + to_timestr(time_req) + " in total");
    print_lines.push_back(((string) "Computation time: ") + to_timestr(tpersol) + " per solution");
    print_lines.push_back(((string) "Fixed pieces: ") + to_num(fixedpieces, 0));
    print_lines.push_back(((string) "Fixed blocks: ") + to_num(fixedblocks, 0));
    for (int j = 0; j < print_lines.size(); j++) {
      outs << "<text x=\"" << (ox + 0.85 * b.width * 30.0 + 10) << "\" y=\"" << (oy + 0.8 * b.height * 30.0 + 155 + 25.0 * j)
        << "\" text-anchor=\"start\" alignment-baseline=\"middle\" font-size=\"1em\" font-family=\"Arial, Helvetica, sans-serif\">"
        << print_lines[j] << "</text>" << endl;
    }
}

typedef pair<board, board> probsol;
typedef pair<ii, probsol> lvlprob;

int main(int argc, char** argv) {
  char file_sep = '\\';
  string piece_in = "pieces.txt";
  if (argc >= 2) piece_in = argv[1];
  bool list_in = false;
  if (argc >= 3) {
    if (argv[2][0] == 'y') list_in = true;
  }
  cout << "Puzzle solution visualizer" << endl << endl;
  cout << "1st argument: piece input file -- " << piece_in << endl;
  cout << "2nd argument: whether to use a list input file (y) or just a single file (n) -- " << (list_in ? "y" : "n") << endl;
  int in_num = 1;
  vector<string> in_files;
  if (!list_in) {
    if (argc >= 4) in_files.push_back(argv[3]);
    else in_files.push_back("board.txt");
    cout << "3rd argument: board input file -- " << in_files[0] << endl;
  } else {
    if (argc < 4) {
      cout << "3rd argument: list input file required!" << endl;
      return 1;
    } else {
      string list_in_file = argv[3];
      cout << "3rd argument: list input file -- " << list_in_file << endl;
      ifstream lin(list_in_file);
      lin >> in_num;
      for (int i = 0; i < in_num; i++) {
        string puzzle_in;
        lin >> puzzle_in;
        puzzle_in = "puzzles" + (file_sep + ("solutions" + (file_sep + (puzzle_in + "-sol.txt"))));
        in_files.push_back(puzzle_in);
      }
      lin.close();
    }
  }
  
  vector<piece> pieces;
  ifstream fin(piece_in);
  int num_pieces;
  fin >> num_pieces;
  for (int i = 0; i < num_pieces; i++) {
    piece p(fin);
    pieces.push_back(p);
  }
  fin.close();

  cout << endl;
  vector<ii> allprobs;
  vector<iii> infos;
  vector<board> bs;
  vector< vector<board> > bsols;
  int maxtps = 0, mintps = 1000000000;
  for (int i = 0; i < in_num; i++) {
    string puzzle_in = in_files[i];
    string puzzle_out = puzzle_in.substr(0, puzzle_in.size() - 4) + ".svg";
    cout << "File " << puzzle_in << " / output file " << puzzle_out << endl;

    ifstream fin2(puzzle_in);
    int fixedblocks = 0, fixedpieces = 0;
    board b(fin2);
    bs.push_back(b);
    for (int j = 0; j < num_pieces; j++) {
      pieces[j].updatefixed(b);
      if (pieces[j].fixed)
        fixedblocks += pieces[j].blocks.size(), ++fixedpieces;
    }
    vector<board> solutions;
    int num_sol, time_req;
    fin2 >> time_req >> num_sol;
    for (int j = 0; j < num_sol; j++) {
      board bsol(fin2, b.width, b.height);
      solutions.push_back(bsol);
    }
    bsols.push_back(solutions);
    fin2.close();

    int tpersol = (int) round(1.0 * time_req / num_sol);
    int level = 100 + max(0, min(900, (int) round(900.0 * (1.0 - pow(0.97, tpersol / 1000.0))) ));
    if (tpersol < 400) level = max(0, min(100, (int) round(100.0 * (1.0 - fixedblocks / 50.0)) ));
    maxtps = max(tpersol, maxtps);
    mintps = min(tpersol, mintps);
    cout << "  Computation time: " << to_timestr(tpersol) << " per solution / difficulty level: " << level << endl;
    cout << "  Fixed blocks: " << fixedblocks << endl;

    ofstream fout(puzzle_out);
    fout << fixed << setprecision(3);
    fout << "<svg version=\"1.1\" baseProfile=\"full\" width=\"" << (10.0 + b.width * 30.0 + 300.0) << "\" height=\"" << (2 * (10.0 + b.height * 30.0) + 20.0) << "\" xmlns=\"http://www.w3.org/2000/svg\">" << endl;
    print_problem(fout, b, solutions[0], level, num_sol, time_req, tpersol, fixedpieces, fixedblocks, 0, 0, true);
    fout << "</svg>" << endl;
    fout.close();

    infos.push_back(iii(ii(num_sol, time_req), tpersol));
    allprobs.push_back(ii(level, i));
  }

  sort(allprobs.begin(), allprobs.end());
  ofstream fout(((string) "puzzles") + file_sep + "solutions" + file_sep + "all.svg");
  board& b1 = bs[ allprobs[0].second ];
  double svgwidth = 10.0 + b1.width * 30.0 + 300.0;
  double svgheight = 2 * (10.0 + b1.height * 30.0) + 20.0;
  int cutoff = 24;
  int numcols = 5;
  int numrows = (int) ceil(1.0 * (allprobs.size() - cutoff) / numcols);
  fout << fixed << setprecision(3);
  fout << "<svg version=\"1.1\" baseProfile=\"full\" width=\"" << (10.0 + svgwidth) * numcols << "\" height=\"" << (10.0 + svgheight) * numrows << "\" xmlns=\"http://www.w3.org/2000/svg\">" << endl;
  for (int i = cutoff; i < allprobs.size(); i++) {
    board& b = bs[ allprobs[i].second ];
    board& bsol = bsols[ allprobs[i].second ][0];
    int fixedblocks = 0, fixedpieces = 0;
    for (int j = 0; j < num_pieces; j++) {
      pieces[j].updatefixed(b);
      if (pieces[j].fixed)
        fixedblocks += pieces[j].blocks.size(), ++fixedpieces;
    }
    int num_sol = infos[ allprobs[i].second ].first.first, time_req = infos[ allprobs[i].second ].first.second, tpersol = infos[ allprobs[i].second ].second;
    
    int col = (i - cutoff) % numcols;
    int row = (i - cutoff) / numcols;
    fout << "<rect x=\"" << (10.0 + svgwidth) * col + 5.0 << "\" y=\"" << (10.0 + svgheight) * row + 5.0 << "\" width=\"" << svgwidth << "\" height=\"" << svgheight << "\" fill=\"#e6e6e6\" />" << endl;
    print_problem(fout, b, bsol, allprobs[i].first, num_sol, time_req, tpersol, fixedpieces, fixedblocks, (10.0 + svgwidth) * col + 5.0, (10.0 + svgheight) * row + 5.0, true);
  }
  fout << "</svg>" << endl;
  fout.close();

  for (int i = 0; i < bs.size(); i++) {
    bs[i].clear();
    for (int j = 0; j < bsols[i].size(); j++) {
      bsols[i][j].clear();
    }
  }

  cout << "Minimum time per solution: " << to_timestr(mintps) << " / " << mintps << " ms" << endl;
  cout << "Maximum time per solution: " << to_timestr(maxtps) << " / " << maxtps << " ms" << endl;
}