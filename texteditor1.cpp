#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

void setColor(const string& style) {
    cout << style;
}
void resetColor() {
    cout << "\033[0m"; 
}

#define COL_NORMAL   setColor("\033[38;5;15m")       // White Text
#define COL_TITLE    setColor("\033[38;5;15;48;5;18m") // White Text on Dark Blue Background
#define COL_MENU     setColor("\033[38;5;0;48;5;44m")  // Black Text on Dark Cyan Background
#define COL_SELECTED setColor("\033[38;5;0;48;5;11m")  // Black Text on Yellow Background
#define COL_STATUS   setColor("\033[38;5;0;48;5;7m")   // Black Text on Gray Background
#define COL_ERROR    setColor("\033[38;5;9m")        // Bright Red Text
#define COL_SUCCESS  setColor("\033[38;5;10m")       // Bright Green Text
#define COL_LINENO   setColor("\033[38;5;14m")       // Cyan Text
#define COL_PROMPT   setColor("\033[38;5;11m")       // Yellow Text

void clearScreen() { 
  
    cout << "\033[2J\033[1;1H"; 
}


void printRep(char c, int n) {
    for (int i = 0; i < n; i++) cout << c;
}


int consoleWidth() {
    return 80; 
}


struct LineNode {
    string    text;
    LineNode* prev;
    LineNode* next;
    LineNode(const string& t = "")
        : text(t), prev(nullptr), next(nullptr) {}
};


class TextBuffer {
public:
    LineNode* head;
    LineNode* tail;
    int       totalLines;

    TextBuffer() {
        head       = new LineNode("");
        tail       = head;
        totalLines = 1;
    }

    ~TextBuffer() {
        clearAll();
        delete head;
    }

    // Saari lines clear karo
    void clearAll() {
        LineNode* cur = head->next;
        while (cur) {
            LineNode* tmp = cur->next;
            delete cur;
            cur = tmp;
        }
        head->text = "";
        head->next = nullptr;
        tail       = head;
        totalLines = 1;
    }


    LineNode* getNode(int n) const {
        if (n < 1) return nullptr;
        LineNode* cur = head;
        for (int i = 1; i < n && cur; i++) cur = cur->next;
        return cur;
    }

    void insertAfter(int n, const string& text = "") {
        LineNode* cur = getNode(n);
        if (!cur) return;
        LineNode* nn  = new LineNode(text);
        nn->next      = cur->next;
        nn->prev      = cur;
        if (cur->next) cur->next->prev = nn;
        else           tail = nn;
        cur->next = nn;
        totalLines++;
    }


    void deleteLine(int n) {
        if (totalLines == 1) { head->text = ""; return; }
        LineNode* cur = getNode(n);
        if (!cur) return;
        if (cur->prev) cur->prev->next = cur->next;
        else           head = cur->next;
        if (cur->next) cur->next->prev = cur->prev;
        else           tail = cur->prev;
        delete cur;
        totalLines--;
    }

    vector<string> toVector() const {
        vector<string> v;
        LineNode* cur = head;
        while (cur) { v.push_back(cur->text); cur = cur->next; }
        return v;
    }


    void fromVector(const vector<string>& v) {
        clearAll();
        if (v.empty()) return;
        head->text = v[0];
        for (int i = 1; i < (int)v.size(); i++)
            insertAfter(i, v[i]);
    }
};

struct EditorState {
    TextBuffer     buf;
    string         fileName    = "Untitled.txt";
    bool           modified    = false;
    bool           wordWrap    = true;
    int            curLine     = 1;
    int            scrollTop   = 0;   
    string         fontName    = "Courier New";
    int            fontSize    = 12;
    vector<string> recentFiles;       
    int            pageSize    = 15;  // VS code compatibility ke liye ideal size
} ed;

void addRecent(const string& f) {
    auto it = find(ed.recentFiles.begin(), ed.recentFiles.end(), f);
    if (it != ed.recentFiles.end()) ed.recentFiles.erase(it);
    ed.recentFiles.insert(ed.recentFiles.begin(), f);
    if ((int)ed.recentFiles.size() > 5) ed.recentFiles.resize(5);
}



void drawTitleBar() {
    int w = consoleWidth();
    COL_TITLE;
    cout << " DSA Text Editor  |  " << ed.fileName << (ed.modified ? " [*]" : "    ");
    string right = "BS CS F24 Afternoon ";
    int spaces = w - 21 - (int)ed.fileName.size() - 4 - (int)right.size();
    if (spaces > 0) printRep(' ', spaces);
    cout << right;
    resetColor();
    cout << "\n";
}

void drawMenuBar() {
    int w = consoleWidth();
    COL_MENU;
    string bar = "  [F]ile   [E]dit   [O]ptions   [V]iew   [H]elp  ";
    cout << bar;
    int rem = w - (int)bar.size();
    if (rem > 0) printRep(' ', rem);
    resetColor();
    cout << "\n";
    COL_NORMAL;
    printRep('-', w);
    resetColor();
    cout << "\n";
}

void drawDocument() {
    auto  lines = ed.buf.toVector();
    int   total = (int)lines.size();
    int   w     = consoleWidth();

    if (ed.curLine - 1 < ed.scrollTop)
        ed.scrollTop = ed.curLine - 1;
    if (ed.curLine - 1 >= ed.scrollTop + ed.pageSize)
        ed.scrollTop = ed.curLine - 1 - ed.pageSize + 1;

    for (int row = 0; row < ed.pageSize; row++) {
        int idx = ed.scrollTop + row;   
        int lineNo = idx + 1;

        COL_LINENO;
        cout << "  " << lineNo;
        if (lineNo < 10)   cout << "   ";
        else if (lineNo < 100) cout << "  ";
        else               cout << " ";
        resetColor();

        if (lineNo == ed.curLine) {
            COL_SELECTED;
            cout << ">";
            resetColor();
        } else {
            cout << " ";
        }
        cout << " ";

        if (idx < total) {
            string txt = lines[idx];
            int maxW = w - 9;
            if (ed.wordWrap && (int)txt.size() > maxW)
                txt = txt.substr(0, maxW) + "~";
            cout << txt;
        }
        cout << "\n";
    }
}

void drawStatusBar(const string& msg = "") {
    int w = consoleWidth();
    COL_STATUS;
    string s;
    if (!msg.empty()) {
        s = "  " + msg;
    } else {
        s = "  Ln:" + to_string(ed.curLine)
          + "  |  Lines:" + to_string(ed.buf.totalLines)
          + "  |  Font:" + ed.fontName
          + " " + to_string(ed.fontSize) + "pt"
          + "  |  Wrap:" + (ed.wordWrap ? "ON" : "OFF")
          + "  |  " + (ed.modified ? "[Unsaved]" : "[Saved]");
    }
    cout << s;
    int rem = w - (int)s.size();
    if (rem > 0) printRep(' ', rem);
    resetColor();
    cout << "\n";
}

void drawHelp() {
    COL_NORMAL;
    cout << "\n";
    COL_LINENO;
    cout << "  Shortcuts: ";
    resetColor();
    cout << "F=File  E=Edit  O=Options  V=View  H=Help  Q=Quit\n";
    COL_LINENO;
    cout << "  In editor: ";
    resetColor();
    cout << "Type normally | ENTER=new line\n";
}

void redraw(const string& statusMsg = "") {
    clearScreen();
    drawTitleBar();
    drawMenuBar();
    drawDocument();
    drawStatusBar(statusMsg);
    drawHelp();
}


string getLine(const string& prompt, const string& def = "") {
    COL_PROMPT;
    cout << "\n  " << prompt;
    if (!def.empty()) cout << " [" << def << "]";
    cout << ": ";
    resetColor();
    COL_NORMAL;
    string s;
    getline(cin, s);
    resetColor();
    if (s.empty() && !def.empty()) return def;
    return s;
}

void pressEnter(const string& msg = "Enter dabao jari rakhne ke liye...") {
    COL_PROMPT;
    cout << "\n  " << msg;
    resetColor();
    cin.ignore(10000, '\n');
}

bool confirm(const string& msg) {
    COL_ERROR;
    cout << "\n  *** " << msg << " (y/n): ";
    resetColor();
    string s;
    getline(cin, s);
    return (s == "y" || s == "Y");
}

void showError(const string& msg) {
    COL_ERROR;
    cout << "\n  [ERROR] " << msg << "\n";
    resetColor();
    pressEnter();
}

void showSuccess(const string& msg) {
    COL_SUCCESS;
    cout << "\n  [OK] " << msg << "\n";
    resetColor();
}

bool isValidFileName(const string& name) {
    if (name.empty()) return false;
    string invalid = "\\/:*?\"<>|";
    for (char c : name) {
        if (invalid.find(c) != string::npos) return false;
    }
    return true;
}


void doNew() {
    if (ed.modified) {
        if (!confirm("Unsaved changes hain! Phir bhi new file banayein?"))
            return;
    }
    ed.buf.clearAll();
    ed.fileName  = "Untitled.txt";
    ed.modified  = false;
    ed.curLine   = 1;
    ed.scrollTop = 0;
    redraw("New file create ho gaya.");
}

bool saveToFile(const string& fname) {
    if (!isValidFileName(fname)) {
        showError("File name mein invalid characters hain.");
        return false;
    }
    ofstream f(fname);
    if (!f.is_open()) {
        showError("File save nahi ho saki.");
        return false;
    }
    auto lines = ed.buf.toVector();
    for (auto& l : lines) f << l << "\n";
    f.close();
    ed.modified = false;
    ed.fileName = fname;
    addRecent(fname);
    return true;
}

void doSave() {
    if (ed.fileName == "Untitled.txt") {
        string name = getLine("File ka naam daalo (e.g. myfile.txt)");
        if (name.empty() || !isValidFileName(name)) {
            showError("Invalid File Name!");
            return;
        }
        if (name.find('.') == string::npos) name += ".txt";
        ed.fileName = name;
    }
    if (saveToFile(ed.fileName)) redraw("File saved: " + ed.fileName);
}

void doSaveAs() {
    string name = getLine("Naya file naam daalo", ed.fileName);
    if (name.empty() || !isValidFileName(name)) {
        showError("Invalid File Name!");
        return;
    }
    if (name.find('.') == string::npos) name += ".txt";
    if (saveToFile(name)) redraw("Saved As: " + name);
}

void doOpen() {
    if (ed.modified) {
        if (!confirm("Unsaved changes jayenge! File open karein?")) return;
    }
    string fname = getLine("File ka path daalo");
    ifstream test(fname);
    if (!test.is_open()) {
        showError("File nahi mili!");
        return;
    }
    vector<string> lines;
    string line;
    while (getline(test, line)) lines.push_back(line);
    test.close();
    if (lines.empty()) lines.push_back("");
    ed.buf.fromVector(lines);
    ed.fileName  = fname;
    ed.modified  = false;
    ed.curLine   = 1;
    ed.scrollTop = 0;
    addRecent(fname);
    redraw("Opened: " + fname);
}

void doRecentFiles() {
    clearScreen();
    COL_TITLE; cout << "\n  === Recent Files ===\n\n"; resetColor();
    if (ed.recentFiles.empty()) { showError("Koi recent file nahi hai."); return; }
    for (int i = 0; i < (int)ed.recentFiles.size(); i++) {
        COL_LINENO; cout << "  " << (i + 1) << ". ";
        COL_NORMAL; cout << ed.recentFiles[i] << "\n";
    }
    string inp = getLine("Number daalo (0 to go back)");
    if (inp == "0" || inp.empty()) { redraw(); return; }
    int sel = stoi(inp);
    if (sel >= 1 && sel <= (int)ed.recentFiles.size()) {
        string fname = ed.recentFiles[sel - 1];
        ifstream f(fname);
        if (f.is_open()) {
            vector<string> lines; string line;
            while (getline(f, line)) lines.push_back(line);
            f.close();
            ed.buf.fromVector(lines);
            ed.fileName = fname;
            redraw("Opened: " + fname);
        }
    }
}

void doClose() {
    if (ed.modified && confirm("Save karna chahte ho pehle?")) doSave();
    ed.buf.clearAll();
    ed.fileName  = "Untitled.txt";
    ed.modified  = false;
    ed.curLine   = 1;
    redraw("File close ho gayi.");
}

void doExit() {
    if (ed.modified && confirm("Save karna chahte ho exit se pehle?")) doSave();
    COL_SUCCESS; cout << "\n  Editor band ho raha hai. Allah Hafiz!\n\n"; resetColor();
    exit(0);
}



void doTypeText() {
    clearScreen();
    COL_TITLE; cout << "\n  === Text Type Karo ===\n"; resetColor();
    cout << "  (Khali line chhod ke ENTER dabao — type khatam hoga)\n\n";
    int insertAt = ed.curLine;
    while (true) {
        COL_LINENO; cout << "  Ln " << insertAt << " > "; resetColor();
        string line; getline(cin, line);
        if (line.empty()) break;
        LineNode* node = ed.buf.getNode(insertAt);
        if (node && node->text.empty()) node->text = line;
        else ed.buf.insertAfter(insertAt, line);
        insertAt++;
        ed.modified = true;
    }
    ed.curLine = max(1, insertAt - 1);
    redraw("Text add ho gaya.");
}

void doEditText() {
    clearScreen();
    COL_TITLE; cout << "\n  === Text Edit ===\n\n"; resetColor();
    auto lines = ed.buf.toVector();
    int lineNo = ed.curLine;
    LineNode* node = ed.buf.getNode(lineNo);
    if (!node) return;
    cout << "  Current Line " << lineNo << ": " << node->text << "\n\n";
    cout << "    1. Text replace karo\n    2. Line clear karo\n    3. Line delete karo\n    0. Cancel\n";
    string op = getLine("Option");
    if (op == "1") {
        node->text = getLine("Naya text", node->text);
        ed.modified = true;
    } else if (op == "2") {
        node->text = "";
        ed.modified = true;
    } else if (op == "3") {
        ed.buf.deleteLine(lineNo);
        if (ed.curLine > ed.buf.totalLines) ed.curLine = ed.buf.totalLines;
        ed.modified = true;
    }
    redraw();
}

void doFindReplace() {
    clearScreen();
    COL_TITLE; cout << "\n  === Find & Replace ===\n\n"; resetColor();
    string findStr = getLine("Find text daalo");
    if (findStr.empty()) return;
    string replStr = getLine("Replace with");
    auto lines = ed.buf.toVector();
    int found = 0;
    for (int i = 0; i < (int)lines.size(); i++) {
        size_t pos = lines[i].find(findStr);
        if (pos != string::npos) {
            found++;
            if (!replStr.empty()) lines[i].replace(pos, findStr.size(), replStr);
        }
    }
    if (found > 0 && !replStr.empty()) {
        ed.buf.fromVector(lines);
        ed.modified = true;
        redraw(to_string(found) + " replacements done.");
    } else {
        redraw("Matches found: " + to_string(found));
    }
}

void doSelectAll() {
    clearScreen();
    COL_TITLE; cout << "\n  === Select All ===\n\n"; resetColor();
    auto lines = ed.buf.toVector();
    for(auto& l : lines) cout << "  " << l << "\n";
    pressEnter();
    redraw();
}

void doToggleWrap() { ed.wordWrap = !ed.wordWrap; redraw(); }
void doPageUp() { ed.curLine = max(1, ed.curLine - ed.pageSize); redraw(); }
void doPageDown() { ed.curLine = min(ed.buf.totalLines, ed.curLine + ed.pageSize); redraw(); }
void doHome() { ed.curLine = 1; ed.scrollTop = 0; redraw(); }
void doEnd() { ed.curLine = ed.buf.totalLines; redraw(); }
void doGoToLine() {
    string inp = getLine("Line number daalo");
    if (inp.empty()) return;
    int n = stoi(inp);
    if (n >= 1 && n <= ed.buf.totalLines) { ed.curLine = n; redraw(); }
}


void showFileMenu() {
    clearScreen();
    COL_TITLE; cout << "\n  === FILE MENU ===\n\n"; resetColor();
    cout << "  1. New File\n  2. Open File\n  3. Recent Files\n  4. Save\n  5. Save As\n  6. Close File\n  7. Exit\n  0. Back\n\n";
    string ch = getLine("Option");
    if (ch == "1") doNew(); else if (ch == "2") doOpen(); else if (ch == "3") doRecentFiles();
    else if (ch == "4") doSave(); else if (ch == "5") doSaveAs(); else if (ch == "6") doClose();
    else if (ch == "7") doExit(); else redraw();
}

void showEditMenu() {
    clearScreen();
    COL_TITLE; cout << "\n  === EDIT MENU ===\n\n"; resetColor();
    cout << "  1. Type Text\n  2. Edit/Delete Line\n  3. Find & Replace\n  4. Select All\n  0. Back\n\n";
    string ch = getLine("Option");
    if (ch == "1") doTypeText(); else if (ch == "2") doEditText(); else if (ch == "3") doFindReplace();
    else if (ch == "4") doSelectAll(); else redraw();
}

void showOptionsMenu() {
    clearScreen();
    COL_TITLE; cout << "\n  === OPTIONS MENU ===\n\n"; resetColor();
    cout << "  1. Toggle Word Wrap\n  0. Back\n\n";
    string ch = getLine("Option");
    if (ch == "1") doToggleWrap(); else redraw();
}

void showViewMenu() {
    clearScreen();
    COL_TITLE; cout << "\n  === VIEW MENU ===\n\n"; resetColor();
    cout << "  1. Page Up\n  2. Page Down\n  3. Home\n  4. End\n  5. Go to Line\n  0. Back\n\n";
    string ch = getLine("Option");
    if (ch == "1") doPageUp(); else if (ch == "2") doPageDown(); else if (ch == "3") doHome();
    else if (ch == "4") doEnd(); else if (ch == "5") doGoToLine(); else redraw();
}

void showHelpMenu() {
    clearScreen();
    COL_TITLE; cout << "\n  === HELP ===\n\n"; resetColor();
    cout << "  Data Structure: Doubly Linked List\n  Keys:\n  F = File Menu | E = Edit Menu | O = Options | V = View | Q = Quit\n";
    pressEnter();
    redraw();
}


int main() {
  
    ed.buf.getNode(1)->text = "=== HMS Text Editor ===";
    ed.buf.insertAfter(1, "F = File Menu   |   E = Edit Menu");
    ed.buf.insertAfter(2, "O = Options     |   V = View Menu");
    ed.buf.insertAfter(3, "H = Help        |   Q = Quit");
    ed.modified = false;

    redraw("Editor ready! Menu ke liye F/E/O/V/H dabao.");

    while (true) {
        COL_PROMPT; cout << "\n  Menu > "; resetColor();
        string ch; getline(cin, ch);
        if (ch.empty()) { redraw(); continue; }
        if (ch.size() == 1) {
            char c = tolower(ch[0]);
            if      (c == 'f') showFileMenu();
            else if (c == 'e') showEditMenu();
            else if (c == 'o') showOptionsMenu();
            else if (c == 'v') showViewMenu();
            else if (c == 'h') showHelpMenu();
            else if (c == 'q') doExit();
            else { showError("Invalid Key!"); redraw(); }
        } else {
            showError("Sirf ek character daalo!"); redraw();
        }
    }
    return 0;
}