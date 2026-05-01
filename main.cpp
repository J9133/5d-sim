#include <iostream>
#include <vector>
#include <iomanip>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
#include <string>
#include <map>
#include <array>
using namespace std;

int L = 64;
const int B = 3;
int cros = 1;
string cros_v = "010";
map<int, string> names = {
        {0, "000"},
        {1, " P "},
        {2, " B "},
        {3, " I "},
        {4, " E "},
        {5, " Bo"}
};

map<int, int> ox = {
    {2, 0},
    {5, 1}
};

long long totalBlocks = 1;
vector<unsigned char> world;

void initvoid(){
    for(int i = 0; i < 5; i++){
        totalBlocks *= L;
    }
    world.resize(totalBlocks * B, 0);
}

long long getIndex(int x, int y, int z, int w, int v){
    return x +
       y * L +
       z * L * L +
       w * L * L * L +
       v * L * L * L * L;
}

void setbyte(int x, int y, int z, int w, int v, int ByteId, unsigned char value){
    long long idx = getIndex(x, y, z, w, v);
    world[idx * B + ByteId] = value;
}

long long readbyte(int x, int y, int z, int w, int v, int ByteId){
    long long idx = getIndex(x, y, z, w, v);
    return world[idx * B + ByteId];
}

void printWorld(int SHOW_X, int SHOW_Y, int SHOW_Z, int SHOW_W, int SHOW_V){
    for(int v = 0; v < SHOW_V; v++){
        for(int z = 0; z < SHOW_Z; z++){
            for(int y = 0; y < SHOW_Y; y++){
                for(int w = 0; w < SHOW_W; w++){
                    if(w > 0) cout << "  |  ";
                    for(int x = 0; x < SHOW_X; x++){
                        cout << setw(3) << setfill('0') << (int)(unsigned char)readbyte(x,y,z,w,v, 0) << " ";
                    }
                }
                cout << endl;
            }
            if(z < SHOW_Z - 1) cout << endl;
        }
        if(v < SHOW_V - 1){
            cout << endl << string((L*13), '-') << endl << endl;
        }
    }
}

array<int, 5> printSlice(int fix_x, int fix_y, int fix_z, int fix_w, int fix_v, int dims, int fix_xp, int fix_yp, int fix_zp, int fix_wp, int fix_vp){
    array<int, 5> retT = {};
    for(int v = (dims >= 5 ? 0 : fix_v); v < (dims >= 5 ? L : fix_v + 1); v++){
        for(int w = (dims >= 4 ? 0 : fix_w); w < (dims >= 4 ? L : fix_w + 1); w++){
            for(int y = (dims >= 3 ? 0 : fix_y); y < (dims >= 3 ? L : fix_y + 1); y++){
                for(int z = (dims >= 2 ? 0 : fix_z); z < (dims >= 2 ? L : fix_z + 1); z++){
                    for(int x = 0; x < L; x++){
                        //cout << setw(3) << setfill('0') << (int)(unsigned char)readbyte(x,y,z,w,v, 0) << " ";
                        
                        if      (cros == 1  && x == fix_xp+1 && y == fix_yp   && z == fix_zp   && w == fix_wp   && v == fix_vp)   { cout << cros_v; retT = {x,   y,   z,   w,   v}; }
                        else if (cros == 2  && x == fix_xp-1 && y == fix_yp   && z == fix_zp   && w == fix_wp   && v == fix_vp)   { cout << cros_v; retT = {x,   y,   z,   w,   v}; }
                        else if (cros == 3  && x == fix_xp   && y == fix_yp+1 && z == fix_zp   && w == fix_wp   && v == fix_vp)   { cout << cros_v; retT = {x,   y,   z,   w,   v}; }
                        else if (cros == 4  && x == fix_xp   && y == fix_yp-1 && z == fix_zp   && w == fix_wp   && v == fix_vp)   { cout << cros_v; retT = {x,   y,   z,   w,   v}; }
                        else if (cros == 5  && x == fix_xp   && y == fix_yp   && z == fix_zp+1 && w == fix_wp   && v == fix_vp)   { cout << cros_v; retT = {x,   y,   z,   w,   v}; }
                        else if (cros == 6  && x == fix_xp   && y == fix_yp   && z == fix_zp-1 && w == fix_wp   && v == fix_vp)   { cout << cros_v; retT = {x,   y,   z,   w,   v}; }
                        else if (cros == 7  && x == fix_xp   && y == fix_yp   && z == fix_zp   && w == fix_wp+1 && v == fix_vp)   { cout << cros_v; retT = {x,   y,   z,   w,   v}; }
                        else if (cros == 8  && x == fix_xp   && y == fix_yp   && z == fix_zp   && w == fix_wp-1 && v == fix_vp)   { cout << cros_v; retT = {x,   y,   z,   w,   v}; }
                        else if (cros == 9  && x == fix_xp   && y == fix_yp   && z == fix_zp   && w == fix_wp   && v == fix_vp+1) { cout << cros_v; retT = {x,   y,   z,   w,   v}; }
                        else if (cros == 10 && x == fix_xp   && y == fix_yp   && z == fix_zp   && w == fix_wp   && v == fix_vp-1) { cout << cros_v; retT = {x,   y,   z,   w,   v}; }
                        else{
                            cout << names[readbyte(x,y,z,w,v, 0)];
                        }

                    }
                    if(dims >= 2) cout << endl;
                }
                if(dims >= 3) cout << endl;
            }
            if(dims >= 4) cout << string(L*4, '-') << endl;
        }
        if(dims >= 5) cout << string(L*4, '=') << endl;
    }
    return retT;
}

char _getch() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    read(STDIN_FILENO, &ch, 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void add_item(int x, int y, int z, int w, int v, int b1 = 0, int b2 = 0, int b3 = 0){
    setbyte(x,y,z,w,v,0,b1);
    setbyte(x,y,z,w,v,1,b2);
    setbyte(x,y,z,w,v,2,b3);
}

array<unsigned char, 3> read_item(int x, int y, int z, int w, int v){
    return {(unsigned char)readbyte(x,y,z,w,v,0),
            (unsigned char)readbyte(x,y,z,w,v,1),
            (unsigned char)readbyte(x,y,z,w,v,2)
    };
}

int get_next_p(int v, int vb){
    return (2 * v) - vb;
}

int main() {
    vector<int> collBs = {2, 5};
    vector<int> itemsBs = {3};
    vector<int> EBs = {4};
    vector<array<unsigned char, 3>> inv = {};
    cout << "5D Simulator Engine Started!" << endl;
    initvoid(); 

    int px = 1;
    int py = 1;
    int pz = 1;
    int pw = 1;
    int pv = 1;

    int Bpx = 1;
    int Bpy = 1;
    int Bpz = 1;
    int Bpw = 1;
    int Bpv = 1;
    
    int Dpx = 1;
    int Dpy = 1;
    int Dpz = 1;
    int Dpw = 1;
    int Dpv = 1;

    int bpx = 1;
    int bpy = 1;
    int bpz = 1;
    int bpw = 1;
    int bpv = 1;

    int inv_size = 2;
    int bxl = 0;

    array<int, 5> retT = {2,1,1,1,1};
    
    setbyte(Bpx, Bpy, Bpz, Bpw, Bpv, 0, 0);
    setbyte(px, py, pz, pw, pv, 0, 1);
    add_item(2,2,2,1,1, 2);
    add_item(3,3,2,1,1, 5);
    add_item(1,2,3,1,1, 4, 2);
    
    while (true){
        Bpx = px;
        Bpy = py;
        Bpz = pz;
        Bpw = pw;
        Bpv = pv;
        
        char c = _getch();
        int move = 0;

        bool invF = false;
        bool invF_AnI = false;

        string messag = "";

        if (true) {
            if (c == 's') {
                if (pz < L - 1) {
                    pz += 1;
                }
                move = 1;
            } else if (c == 'w') {
                if (pz > 0) {
                    pz -= 1;
                }
                move = 2;
            } else if (c == 'd') {
                if (px < L - 1) {
                    px += 1;
                }
                move = 3;
            } else if (c == 'a') {
                if (px > 0) {
                    px -= 1;
                }
                move = 4;
            } else if (c == 'e') {
                if (py < L - 1) {
                    py -= 1;
                }
                move = 5;
            } else if (c == 'q') {
                if (py > 0) {
                    py += 1;
                }
                move = 6;
            } else if (c == 'x') {
                if (pw < L - 1) {
                    pw += 1;
                }
                move = 7;
            } else if (c == 'z') {
                if (pw > 0) {
                    pw -= 1;
                }
                move = 8;
            } else if (c == 't') {
                if (pv < L - 1) {
                    pv += 1;
                }
                move = 9;
            } else if (c == 'r') {
                if (pv > 0) {
                    pv -= 1;
                }
                move = 10;
            }
            if (c == 'k') { cros = 5; }
            else if (c == 'i') { cros = 6; }
            else if (c == 'l') { cros = 1; }
            else if (c == 'j') { cros = 2; }
            else if (c == 'o') { cros = 3; }
            else if (c == 'u') { cros = 4; }
            else if (c == 'm') { cros = 7; }
            else if (c == 'n') { cros = 8; }
            else if (c == 'y') { cros = 9; }
            else if (c == 'h') { cros = 10; }
        }
        
        int Neblock = readbyte(px, py, pz, pw, pv, 0);
        int NeblockE = readbyte(px, py, pz, pw, pv, 1);
        auto T = find(collBs.begin(), collBs.end(), Neblock);
        auto Ti = find(itemsBs.begin(), itemsBs.end(), Neblock);
        auto Te = find(EBs.begin(), EBs.end(), Neblock);

        if (Te != EBs.end()){
            if (NeblockE == 1){
                inv_size += 2;
                add_item(px,py,pz,pw,pv,0,0,0);
                messag = "new enhancement: " + to_string(NeblockE);
            }else if (NeblockE == 2){
                bxl += 5;
                add_item(px,py,pz,pw,pv,0,0,0);
                messag = "new enhancement: " + to_string(NeblockE);
            }
        }

        if (c == 'b') {
            bpx = retT[0];
            bpy = retT[1];
            bpz = retT[2];
            bpw = retT[3];
            bpv = retT[4];
            
            if (bpx >= 0 && bpx < L && 
                bpy >= 0 && bpy < L && 
                bpz >= 0 && bpz < L && 
                bpw >= 0 && bpw < L && 
                bpv >= 0 && bpv < L) {
                    
                int nbb0 = readbyte(bpx, bpy, bpz, bpw, bpv, 0);
                auto Tb = find(collBs.begin(), collBs.end(), nbb0);
                int blox = ox[nbb0];

                if (nbb0 != 0 && Tb != collBs.end() && bxl >= blox){
                    array<unsigned char, 3> invi = read_item(bpx, bpy, bpz, bpw, bpv);
                    array<unsigned char, 3> inviT = {};
                    inviT[1] = invi[0];
                    inviT[0] = 3;
                    inviT[2] = 0;
                    if (inv.size() < inv_size){
                        inv.push_back(inviT);
                        add_item(bpx, bpy, bpz, bpw, bpv, 0, 0, 0);
                    }else{
                        add_item(bpx, bpy, bpz, bpw, bpv, invi[0], invi[1], invi[2]);
                    }
                }
            }
            cout << "\033[2J\033[H";
            cout << "pw:" << pw << ", pv:" << pv << endl;
            cout << "inv: ";
            for (auto& item : inv) { cout << (int)item[0] << ", "; }
            cout << endl;
            retT = printSlice(px, py, pz, pw, pv, 3, px, py, pz, pw, pv);
            continue;
        }

        if (c == 'v' && inv.size() >= 1) {
            bpx = retT[0];
            bpy = retT[1];
            bpz = retT[2];
            bpw = retT[3];
            bpv = retT[4];
            
            if (bpx >= 0 && bpx < L && 
                bpy >= 0 && bpy < L && 
                bpz >= 0 && bpz < L && 
                bpw >= 0 && bpw < L && 
                bpv >= 0 && bpv < L) {
                    
                int nbb0 = readbyte(bpx, bpy, bpz, bpw, bpv, 0);


                array<unsigned char, 3> iteminvnback = {inv.back()[0], inv.back()[1], inv.back()[2]};
                int ibb0 = iteminvnback[1];
                auto Tv = find(collBs.begin(), collBs.end(), ibb0);

                if (nbb0 == 0 && Tv != collBs.end()) {
                    add_item(bpx, bpy, bpz, bpw, bpv, iteminvnback[1], 0, 0);
                    inv.pop_back();
                }
            } else {
                messag = "Cannot build outside the world!";
            }
            cout << "\033[2J\033[H";
            cout << "pw:" << pw << ", pv:" << pv << endl;
            cout << "inv: ";
            for (auto& item : inv) { cout << (int)item[0] << ", "; }
            cout << endl;
            retT = printSlice(px, py, pz, pw, pv, 3, px, py, pz, pw, pv);

            if (messag != "") {
                cout << messag << endl;
            }
            continue;
            
        }

        if (c == 'c' && inv.size() >= 1) {
            Dpx = retT[0];
            Dpy = retT[1];
            Dpz = retT[2];
            Dpw = retT[3];
            Dpv = retT[4];

            if (Dpx >= 0 && Dpx < L && 
                Dpy >= 0 && Dpy < L && 
                Dpz >= 0 && Dpz < L && 
                Dpw >= 0 && Dpw < L && 
                Dpv >= 0 && Dpv < L) {
                    
                int nbb0 = readbyte(Dpx, Dpy, Dpz, Dpw, Dpv, 0);
                
                if (nbb0 == 0) {
                    array<unsigned char, 3> iteminvnback = {inv.back()[0], inv.back()[1], inv.back()[2]};
                    inv.pop_back();
                    add_item(Dpx, Dpy, Dpz, Dpw, Dpv, iteminvnback[0], iteminvnback[1], iteminvnback[2]);
                }
            } else {
                messag = "Cannot place item outside the world!";
            }
            cout << "\033[2J\033[H";
            cout << "pw:" << pw << ", pv:" << pv << endl;
            cout << "inv: ";
            for (auto& item : inv) { cout << (int)item[0] << ", "; }
            cout << endl;
            retT = printSlice(px, py, pz, pw, pv, 3, px, py, pz, pw, pv);

            if (messag != "") {
                cout << messag << endl;
            }
            continue;
        }

        if (inv.size() >= inv_size){
            invF = true;
        }if (Ti != itemsBs.end()){
            if (invF == false){
                inv.push_back({(unsigned char)Neblock, (unsigned char)readbyte(px,py,pz,pw,pv,1), (unsigned char)readbyte(px,py,pz,pw,pv,2)});
                messag = "new item: " + to_string(Neblock);
            }else{
                invF_AnI = true;
                messag = "inv Full";
            }
        }

        if (T == collBs.end() && invF_AnI == false){
            setbyte(Bpx, Bpy, Bpz, Bpw, Bpv, 0, 0);
            setbyte(px, py, pz, pw, pv, 0, 1);
            cout << "\033[2J\033[H";
            cout << "pw:" << pw << ", pv:" << pv << endl;
            cout << "inv: ";
            for (auto& item : inv) {cout << (int)item[0] << ", ";}
            cout << endl;
            retT = printSlice(px, py, pz, pw, pv, 3, px, py, pz, pw, pv);
        }else{
            px = Bpx;
            py = Bpy;
            pz = Bpz;
            pw = Bpw;
            pv = Bpv;
        }

        if (messag != ""){
            cout << messag << endl;
        }
        
    }
    
    // cout << "Simulation done! Blocks: " << totalBlocks << endl;
    // cout << "RAM used: " << (totalBlocks * B) / (1024*1024) << " MB" << endl;
    cin.get();
    return 0;
}