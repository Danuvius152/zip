#include "compress.h"
#include <windows.h>
#define FILE_MENU_INFO 1
#define FILE_MENU_HELP 2
#define SELECT 3
#define COMPRESS 4
#define EXTRACT 5
#define FILE_MENU_EXIT 6
#define IDC_RADIO1 101
#define IDC_RADIO2 102
#define IDC_RADIO3 103
#define IDC_RADIO4 104
using namespace std;

// C++ Boxes

void menuItem1() {

  // Creates Window Box
  int menuItemBox1 = MessageBoxW(NULL, L"Danuvius\nVersion 1.0", L"Information",
                                 MB_ICONINFORMATION | MB_OK);
}

void menuItem2() {

  // Opens Link
  string openWebpage =
      string("start ").append("https://github.com/Danuvius152");
  system(openWebpage.c_str());
}

// login

int GeneralLogin(char username[30], char password[30]) {
  if (strcmp(username, "Tovape") == 0 && strcmp(password, "123") == 0) {
    return 0;
  } else {
    return 1;
  }
}
int FileDialog(char *path) {
  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFile = path;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  return GetOpenFileName(&ofn);
}
string path = "";
string opath = "";
string suffix = "";
char File[MAX_PATH] = {0};
int var = 0;
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

void AddMenus(HWND);
void AddContent(HWND);
// void loadImages();

HMENU hMenu;
HWND hMainwindow;
HWND hUsername;
HWND hPassword;
HWND hLoginstatus;
HWND hFilecontent;
HWND hLogo;
HBITMAP hLogoImage, hButtonImage;

// Background Window Colors
COLORREF color = RGB(0xFF, 0xFF, 0xFF);
HBRUSH backgroundColor = ::CreateSolidBrush(color);

// Main
int WINAPI WinMain(HINSTANCE box2, HINSTANCE hPrevInst, LPSTR args,
                   int ncmdshow) {
  // SetConsoleTitleA("Debugger");
  HWND hWnd = GetConsoleWindow();
  ShowWindow(hWnd, SW_HIDE);

  WNDCLASSW wc = {0};
  wc.hbrBackground = (HBRUSH)backgroundColor;
  wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
  wc.hInstance = box2;
  wc.lpszClassName = L"box2";
  wc.lpfnWndProc = WindowProcedure;
  if (!RegisterClassW(&wc)) {
    return 1;
  }

  // Getting Screen Resolution to place Screen in the center

  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);
  cout << "Screen resolution: " << screenWidth << "x" << screenHeight << "\n";

  // Window Creation
  hMainwindow =
      CreateWindowW(L"box2", L"Null", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                    (screenWidth - 500) / 2, (screenHeight - 500) / 2, 500, 300,
                    NULL, NULL, NULL, NULL);

  MSG msg = {0};

  while (GetMessage(&msg, NULL, NULL, NULL)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}

// Handling Window Properties (Being Open/Closed/Sized etc)

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  // Default
  default:
    return DefWindowProcW(hWnd, msg, wp, lp);
  // Closed
  case WM_DESTROY:
    PostQuitMessage(0);
    cout << "Window Closed\n";
    break;
  // Being Moved
  case WM_MOVING:
    // cout << "Moving Window\n";
    break;
  // Being Created
  case WM_NCCREATE:
    // Load Images
    // loadImages();
    // We can now create the menus for our program in functions.cpp
    AddMenus(hWnd);
    // Adding Content
    AddContent(hWnd);
    // Set Window Title
    SetWindowTextW(hWnd, L"My Program");
    break;
  // Menu Items Behaviour
  case WM_COMMAND:
    switch (wp) {
    // Login function
    case SELECT:
      if (FileDialog(File))
        puts(File);
      path = string(File);
      suffix = path.substr(path.find_last_of('.') + 1);
      SetWindowText(hLoginstatus, path.c_str());
      break;
    case COMPRESS:
      if (path == "") {
        MessageBoxA(hWnd, "Please choose a file!", "Wait!",
                    MB_OKCANCEL | MB_ICONEXCLAMATION);
        break;
      }
      if (var == 1) {
        opath = path + ".hzp";
        compress_file_huffman(path, opath);
        MessageBoxA(hWnd, "\tDone!", "Help", MB_OKCANCEL | MB_ICONINFORMATION);
      } else if (var == 2) {
        opath = path + ".lzw";
        compress_file_lzw(path);
        MessageBoxA(hWnd, "\tDone!", "Help", MB_OKCANCEL | MB_ICONINFORMATION);
      } else if (var == 3) {
        opath = path + ".lz77";
        FILE *in = fopen(path.c_str(), "rb");
        FILE *out = fopen(opath.c_str(), "wb");
        lz77_encode(in, out);
        MessageBoxA(hWnd, "\tDone!", "Help", MB_OKCANCEL | MB_ICONINFORMATION);
      } else if (var == 4) {
        opath = path + ".lzss";
        FILE *in = fopen(path.c_str(), "rb");
        FILE *out = fopen(opath.c_str(), "wb");
        lzss_encode(in, out);
        MessageBoxA(hWnd, "\tDone!", "Help", MB_OKCANCEL | MB_ICONINFORMATION);
      } else {
        MessageBoxA(hWnd, "Please pick an appropriate compression algorithm!",
                    "Wait!", MB_OKCANCEL | MB_ICONEXCLAMATION);
      }
      break;
    case EXTRACT:
      if (path == "") {
        MessageBoxA(hWnd, "Please choose a file!", "Wait!",
                    MB_OKCANCEL | MB_ICONEXCLAMATION);
        break;
      }
      opath = path + ".out";
      if (var == 1) {
        int res = extract_file_huffman(path, opath);
        if (res == 0)
          MessageBoxA(hWnd, "\tDone!", "Help",
                      MB_OKCANCEL | MB_ICONINFORMATION);
        else
          MessageBoxA(hWnd, "Please choose a suitable compression format!",
                      "Wait!", MB_OKCANCEL | MB_ICONEXCLAMATION);
      } else if (var == 2) {
        if (suffix != "lzw") {
          MessageBoxA(hWnd, "Please choose a suitable compression format!",
                      "Wait!", MB_OKCANCEL | MB_ICONEXCLAMATION);
          break;
        }
        extract_file_lzw(path);
        MessageBoxA(hWnd, "\tDone!", "Help", MB_OKCANCEL | MB_ICONINFORMATION);
      } else if (var == 3) {
        if (suffix != "lz77") {
          MessageBoxA(hWnd, "Please choose a suitable compression format!",
                      "Wait!", MB_OKCANCEL | MB_ICONEXCLAMATION);
          break;
        }
        FILE *in = fopen(path.c_str(), "rb");
        FILE *out = fopen(opath.c_str(), "wb");
        lz77_decode(in, out);
        MessageBoxA(hWnd, "\tDone!", "Help", MB_OKCANCEL | MB_ICONINFORMATION);
      } else if (var == 4) {
        if (suffix != "lzss") {
          MessageBoxA(hWnd, "Please choose a suitable compression format!",
                      "Wait!", MB_OKCANCEL | MB_ICONEXCLAMATION);
          break;
        }
        FILE *in = fopen(path.c_str(), "rb");
        FILE *out = fopen(opath.c_str(), "wb");
        lzss_decode(in, out);
        MessageBoxA(hWnd, "\tDone!", "Help", MB_OKCANCEL | MB_ICONINFORMATION);
      } else {
        MessageBoxA(hWnd, "Please pick an appropriate compression algorithm!",
                    "Wait!", MB_OKCANCEL | MB_ICONEXCLAMATION);
      }
      break;
    case IDC_RADIO1:
      var = 1;
      break;
    case IDC_RADIO2:
      var = 2;
      break;
    case IDC_RADIO3:
      var = 3;
      break;
    case IDC_RADIO4:
      var = 4;
      break;
    // Handling Menu Buttons
    case FILE_MENU_INFO:
      menuItem1();
      break;
    case FILE_MENU_HELP:
      MessageBeep(MB_OK); // Message Sound Effect
      menuItem2();
      break;
    case FILE_MENU_EXIT:
      DestroyWindow(hWnd);
      break;
    }
  }
}

// Creating Menu Bars

void AddMenus(HWND hWnd) {
  hMenu = CreateMenu();

  HMENU hSubmenu = CreateMenu();
  AppendMenu(hSubmenu, MF_STRING, 7, "Submenu");

  // File Menu
  HMENU hFileMenu = CreateMenu();
  AppendMenu(hFileMenu, MF_STRING, FILE_MENU_EXIT, "Exit");

  AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");
  AppendMenu(hMenu, MF_STRING, 2, "Info");
  AppendMenu(hMenu, MF_STRING, 3, "Help");

  SetMenu(hWnd, hMenu);
}

// Adding Content

void AddContent(HWND hWnd) {
  // Username Field

  CreateWindow(TEXT("button"), TEXT("Huffman"),
               WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 50, 20, 100, 30,
               hWnd, (HMENU)IDC_RADIO1, NULL, NULL);
  CreateWindow(TEXT("button"), TEXT("LZW"),
               WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 50, 60, 100, 30,
               hWnd, (HMENU)IDC_RADIO2, NULL, NULL);
  CreateWindow(TEXT("button"), TEXT("LZ77"),
               WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 50, 100, 100, 30,
               hWnd, (HMENU)IDC_RADIO3, NULL, NULL);
  CreateWindow(TEXT("button"), TEXT("LZSS"),
               WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 50, 140, 100, 30,
               hWnd, (HMENU)IDC_RADIO4, NULL, NULL);

  CreateWindowW(L"Button", L"选择文件",
                WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER, 190, 40, 198, 30,
                hWnd, (HMENU)SELECT, NULL, NULL);

  CreateWindowW(L"Button", L"压缩",
                WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER, 190, 80, 198, 30,
                hWnd, (HMENU)COMPRESS, NULL, NULL);

  CreateWindowW(L"Button", L"解压",
                WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER, 190, 120, 198,
                30, hWnd, (HMENU)EXTRACT, NULL, NULL);

  // Show Login Status
  hLoginstatus = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
                               50, 180, 350, 20, hWnd, NULL, NULL, NULL);
}

#ifdef DEBUG

int main() {
  string path_ = string("C:/Users/ASUS/Desktop/hzip/1.txt");
  string opath_ = path_ + ".hzp";
  compress_file_huffman(path_, opath_);
  extract_file_huffman(opath_, string("C:/Users/ASUS/Desktop/hzip/1.out"));
  return 0;
}
#endif