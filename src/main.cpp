#include "FileManager.h"

int main()
{
    //system("chcp 65001 > nul");
    //SetConsoleOutputCP(CP_UTF8);
    //SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "Ru");

    FileManager filemanager;
    FileManagerUI filemanagerui;
    FileManagerBK filemanagerbk;

    filemanagerui.ui_asking();

    return 0;
}