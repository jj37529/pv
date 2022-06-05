#include <stdio.h>
#ifdef __linux__
#include <unistd.h>
#include <termios.h>
#else
#include <windows.h>
#include <conio.h>
#include "getopt.h"
#endif
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <list>
#include <vector>
using namespace std;
#ifdef __linux__
void clrscr()
{
    cout << "\033[2J\033[1;1H";
}

unsigned char getch(void)
{
    char buf = 0;
    struct termios old = { 0 };
    fflush(stdout);
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    //printf("%c\n", buf);
    return buf;
}
void SetCursorPos(int XPos, int YPos)
{
    printf("\033[%d;%dH", YPos + 1, XPos + 1);
}
#else
int getch(void) {
    return _getch();
}
    void clrscr()
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        SMALL_RECT scrollRect;
        COORD scrollTarget;
        CHAR_INFO fill;

        // Get the number of character cells in the current buffer.
        if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        {
            return;
        }

        // Scroll the rectangle of the entire buffer.
        scrollRect.Left = 0;
        scrollRect.Top = 0;
        scrollRect.Right = csbi.dwSize.X;
        scrollRect.Bottom = csbi.dwSize.Y;

        // Scroll it upwards off the top of the buffer with a magnitude of the entire height.
        scrollTarget.X = 0;
        scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);

        // Fill with empty spaces with the buffer's default text attribute.
        fill.Char.UnicodeChar = TEXT(' ');
        fill.Attributes = csbi.wAttributes;

        // Do the scroll
        ScrollConsoleScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE), &scrollRect, NULL, scrollTarget, &fill);

        // Move the cursor to the top left corner too.
        csbi.dwCursorPosition.X = 0;
        csbi.dwCursorPosition.Y = 0;

        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), csbi.dwCursorPosition);
    }
#endif//#ifdef __linux__
char* log_fname;
ifstream g_logfs;
typedef struct {
    streampos pos;
    int       line_cnt;
}log_fram;
vector<log_fram> g_framelist;
int g_log_line_cnt = 0;
int g_log_frame_cnt = 0;
int g_log_frame_idx = 0;

void show_frame(int no) {
    log_fram fram_info;
    streampos tell_pos;
    string line;
    int i;
    char str_line[1024];
    clrscr();
    SetCursorPos(0, 0);
    printf("%d/%d\n", no, g_framelist.size());
    g_logfs.open(log_fname);
    fram_info = g_framelist[no];
    g_logfs.seekg(fram_info.pos);
    tell_pos = g_logfs.tellg();
    printf("line_cnt: %d ,seek pos=%d , tell pos=%d\n", fram_info.line_cnt, fram_info.pos, tell_pos);
    i = 0;
    while (i < fram_info.line_cnt) {
        //cout<< line<<endl;
        g_logfs.getline(str_line, 1024);
        //getline(g_logfs,line);
        printf("%s\n", str_line);
        i++;
    }
    g_logfs.close();
}

int main(int argc, char* argv[])
{
    bool b_log_ok = 0, b_exit = 0;
    int opt;
    unsigned char key;

    string line;

    while ((opt = getopt(argc, argv, "c:f:")) != -1) {
        switch (opt) {
        case 'c':
            //gCycleCntEn = 1;
            //gCycleCnt = atoi(optarg);
            break;
        case 'f':
            cout << "??" << endl;
            g_logfs.open(optarg, std::ifstream::in);
            if (g_logfs.fail()) {
                fprintf(stderr, "Usage: %s [-c cycle count]\n", argv[0]);
                return -1;
            }
            else {
                cout << "file opend!!" << endl;
            }
            break;
        default: /* '?' */
            cout << "?" << endl;
            g_logfs.open(optarg, std::ifstream::in);
            if (g_logfs.fail()) {
                fprintf(stderr, "Usage: %s [-c cycle count]\n", argv[0]);
                return -1;
            }
            else {
                cout << "file opend!!" << endl;
            }
        }
    }

    /* Do we have args? */
    if (argc > optind) {
        log_fram fram_info;
        char str_line[1024];
        g_logfs.open(argv[optind], std::ifstream::binary);
        if (g_logfs.fail()) {
            cout << "file opend!!" << endl;
            return -1;
        };
        fram_info.pos = g_logfs.tellg();
        fram_info.line_cnt = 1;
        g_logfs.getline(str_line, 1024);
        g_logfs.getline(str_line, 1024);
        //getline(g_logfs,line);
        while (!g_logfs.fail()) {
            g_log_line_cnt++;
            fram_info.line_cnt++;
            if (str_line[0] == '@') {
                g_framelist.push_back(fram_info);
                g_log_frame_cnt++;
                fram_info.line_cnt = 0;
            }
            fram_info.pos = g_logfs.tellg();
            //printf("%s\n", str_line);
            g_logfs.getline(str_line, 1024);
        }
        //g_framelist.push_back(fram_info);
        //g_log_frame_cnt++;
        printf("frame cnt:%i\n", g_log_frame_cnt);
        b_log_ok = 1;
        g_logfs.close();
        log_fname = argv[optind];

        show_frame(0);
    }

    if (b_log_ok) {
        while (!b_exit) {
            switch (key = getch()) {
            case 'q':
                b_exit = 1;
                break;
#ifdef __linux__
            case 0x1b:
                switch (key = getch()) {
                case 0x5b:
                    switch (key = getch()) {
                    case 0x41:
                    case 0x44:
                        //printf("up");
                        if (g_log_frame_idx > 0) {
                            g_log_frame_idx--;
                        }
                        show_frame(g_log_frame_idx);
                        break;
                    case 0x42:
                    case 0x43:
                        if (g_log_frame_idx + 1 < g_framelist.size()) {
                            g_log_frame_idx++;
                        }
                        show_frame(g_log_frame_idx);
                        break;
                    case 0x48://HOME
                        g_log_frame_idx = 0;
                        show_frame(g_log_frame_idx);
                        break;
                    case 0x46://END
                        g_log_frame_idx = g_framelist.size() - 1;
                        show_frame(g_log_frame_idx);
                        break;
                    }
                    //printf("key:5b %x\n",key);
                    break;
                }
                break;
#else
            case 0xe0:
                switch (key = getch()) {
                case 0x4B://LEFT
                case 0x48://UP
                    if (g_log_frame_idx > 0) {
                        g_log_frame_idx--;
                    }
                    show_frame(g_log_frame_idx);
                    break;
                case 0x4d://RIGHT
                case 0x50://DOWN
                    if (g_log_frame_idx + 1 < g_framelist.size()) {
                        g_log_frame_idx++;
                    }
                    show_frame(g_log_frame_idx);
                    break;
                case 0x47://HOME
                    g_log_frame_idx = 0;
                    show_frame(g_log_frame_idx);
                    break;
                case 0x4F://END
                    g_log_frame_idx = g_framelist.size() - 1;
                    show_frame(g_log_frame_idx);
                    break;
                default:
                    printf("key:%x\n", key);
                    break;
                }
                break;
#endif
            default:
                printf("key:%x\n", key);
                break;
            }

        }
    }
    cout << "bye" << endl;
    return 0;
}