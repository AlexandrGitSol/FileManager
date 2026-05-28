
//добавить флаг чтобы функции не трогали корневую папку
//столбец параметры можно улучшать
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <locale>
#include <algorithm>
#include <iomanip>
#include "../libs/sqlite3.h"

#ifdef _WIN64
    #include <windows.h>
#endif //_WIN64

#include <algorithm>
using namespace std::filesystem;

int callback_for_get_history(void* his, int num, char** vals, char** cols);

class FileManager {
protected:
    enum Flag {
        recursion_on,
        recursion_off,
        del_dir,
        del_files,
        del_dir_files,
        cre_files,
        cre_dir,
        ren_files,
        ren_dir,
        ren_dir_files,
        reg_on,
        reg_off,
        root_on,
        root_off
    };

    Flag recf;
    Flag delf;
    Flag cref;
    Flag renf;
    Flag regf;
    Flag rootf;
    std::vector<char> danger_chars;
    std::vector<std::string> avaliable_flags;

    bool stop;

    bool is_correct_flags_string(std::string flags_string);

    virtual bool flags_parser(std::string all_flags);

    bool del(path path, std::vector<std::string>& ext, std::vector<std::string>& exeptions, bool first_call = true);

    bool ren(path path, std::vector<std::string>& ext, std::vector<std::string>& exeptions, std::string name, bool first_call = true);

    bool cre(path path, std::string name, int count_f, bool first_call = true);

    bool checker(std::string name, std::vector<std::string>& del_list);

    bool have_danger_characters(std::string name);

    virtual std::vector<path> fin(path path, std::vector<std::string>& ext, std::vector<std::string>& exeptions);

    void helper_fin(path pathv, std::vector<std::string>& ext, std::vector<std::string>& exeptions, std::vector<path>& all_paths);

    bool del_history();

    virtual std::vector<std::vector<std::string>> get_history();

public:

    FileManager();

};

class FileManagerUI : public FileManager {
protected:
    bool flags_parser(std::string all_flags) override;
    std::vector<path> fin(path path, std::vector<std::string>& ext, std::vector<std::string>& exeptions) override;
    std::vector<std::vector<std::string>> get_history() override;
public:
    void ui_asking();
};

class FileManagerBK : public FileManager {
public:
    using FileManager::checker; //повысили уровень доступа до public
    using FileManager::ren;
    using FileManager::del;
    using FileManager::del_history;
    using FileManager::get_history;
    using FileManager::cre;
    using FileManager::fin;
    using FileManager::have_danger_characters;
    using FileManager::is_correct_flags_string;
    using FileManager::flags_parser;
};

#endif //FILEMANAGER_H