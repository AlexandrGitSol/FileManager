#include "FileManager.h"
using namespace std;

vector<vector<string>> FileManagerUI::get_history() {
	vector<vector<string>> history;

	sqlite3* db;
	sqlite3_open("history.db", &db);

	const char* sqlq =
		"SELECT command, param FROM history;";

	char* errmsg = nullptr;
	sqlite3_exec(db, sqlq, callback_for_get_history, &history, &errmsg);
	if (errmsg) cerr << "Error: " << errmsg << endl << endl;

	sqlite3_close(db);

	if (!history.empty()) {
		cout << "История:\n";
		for (int i = 0; i < history.size(); ++i) {
			for (int j = 0; j < history[0].size(); ++j) {
				cout << setw(4) << "";
				cout << history[i][j] << ' ';
			}
			cout << endl;
		}
	}
	else {
		cout << "История команд пустая.";
	}
	cout << "\n\n";


	sqlite3_open("history.db", &db);

	sqlq = "INSERT INTO history (command, param) VALUES (\"history\", \"\");";

	char* errmsg2 = nullptr;
	sqlite3_exec(db, sqlq, nullptr, nullptr, &errmsg2);
	if (errmsg2) cerr << "Error: " << errmsg2 << endl;


	sqlite3_close(db);

	return history;
}

bool FileManagerUI::flags_parser(string all_flags) {
	if (!is_correct_flags_string(all_flags)) {
		cout << "Ошибка чтения флагов.\n" << endl;
		return false;
	}
	string flag;
	int i = 0;
	while (i < all_flags.length()) {
		while (i < all_flags.length() && all_flags[i] != '-') {
			i++;
		}
		flag = "";
		while (i < all_flags.length() && all_flags[i] != ' ') {
			flag += all_flags[i++];
		}
		if (flag == "-rec") recf = recursion_on;
		else if (flag == "-nrec") recf = recursion_off;
		else if (flag == "-deld") delf = del_dir;
		else if (flag == "-delf") delf = del_files;
		else if (flag == "-delfd") delf = del_dir_files;
		else if (flag == "-cref") cref = cre_files;
		else if (flag == "-cred") cref = cre_dir;
		else if (flag == "-renf") renf = ren_files;
		else if (flag == "-rend") renf = ren_dir;
		else if (flag == "-renfd") renf = ren_dir_files;
		else if (flag == "-reg") regf = reg_on;
		else if (flag == "-nreg") regf = reg_off;
		else if (flag == "-root") rootf = root_on;
		else if (flag == "-nroot") rootf = root_off;
	}
	cout << "Флаги успешно установлены.\n" << endl;
	return true;
}

vector<path> FileManagerUI::fin(path pathv, vector<string>& ext, vector<string>& exeptions) {
	if (exists(pathv)) {
		vector<path> all_paths;
		helper_fin(pathv, ext, exeptions, all_paths);

		vector<path> ans;

		sort(all_paths.begin(), all_paths.end());

		for (path p : all_paths) {
			if (checker(p.filename().string(), ext) && !checker(p.filename().string(), exeptions)) {
				ans.push_back(p);
			}
		}

		if (ans.empty()) {
			cout << "Файлов/Папок с такой маской не найдено.\n" << endl;
		}
		else {
			cout << "Найдено (" << ans.size() << "):" << endl;
			for (path p : ans) {
				cout << p.string() << endl;
			}
		}

		cout << endl;

		sqlite3* db;

		sqlite3_open("history.db", &db);

		const char* sqlq = "INSERT INTO history (command) VALUES (\"find\");";

		char* errmsg = nullptr;
		sqlite3_exec(db, sqlq, nullptr, nullptr, &errmsg);
		if (errmsg) cerr << "Error: " << errmsg << endl;


		sqlite3_close(db);

		return ans;
	}
	return vector<path>();
}

void FileManagerUI::ui_asking() {
	while (!stop) {
		cout << "Создать файлы [1]\nПереименовать файлы [2]\nУдалить список файлов/папок [3]\nНайти список файлов/папок [4]\nЗадать флаги [5]\nПосмотреть информацию о флагах [6]\nИстория [7]\nЗакрыть [8]\nВыберите команду: ";
		string com;
		getline(cin, com);
		if (com == "1") {
			string path, d = "", name, repath = "";
			cout << "Введите путь: ";
			getline(cin, path);
			for (int c = 0; c < path.length(); ++c) {
				if (path[c] != '"') repath += path[c];
			}
			path = repath;
			if (exists(path)) {
				cout << "Введите имя создаваемого объекта: ";
				getline(cin, name);
				if (!have_danger_characters(name)) {
					cout << "Введите количество создаваемых файлов: ";
					getline(cin, d);
					bool right_count = true;
					for (char x : d) {
						if (!isdigit(x)) right_count = false;
					}
					if (right_count) {
						cre(path, name, stoi(d));
						cout << "Создано.\n" << endl;
					}
					else {
						cout << "Принимаются только числа!" << endl;
					}
				}
				else {
					cout << "Имя имеет недопустимые символы: '\\', '/', ':', '*', '?', '\"', '<', '>', '|'.\n" << endl;
				}
			}
			else {
				cout << "Такого пути не существует\n" << endl;
			}
		}
		else if (com == "2") {
			string path, d = "", repath = "", new_name = "";
			vector<string> ren_vec;
			vector<string> exeptions;
			cout << "Введите путь: ";
			getline(cin, path);
			for (int c = 0; c < path.length(); ++c) {
				if (path[c] != '"') repath += path[c];
			}
			path = repath;
			if (exists(path)) {
				//тут ошибка
				cout << "Введите новое имя объекта/объектов: ";
				getline(cin, new_name);
				if (!have_danger_characters(new_name)) {
					cout << "Введите список файлов для переименования (если список закончен, то введите '.'): " << endl;
					do {
						if (d != ".") {
							getline(cin, d);
							ren_vec.push_back(d);
						}
					} while (d != ".");
					d = "";
					cout << "Введите список ключевых слов для исключений, это файлы, которые не будут переименованы (если список закончен, то введите '.'): " << endl;
					do {
						if (d != ".") {
							getline(cin, d);
							exeptions.push_back(d);
						}
					} while (d != ".");

					cout << "Вы уверены, что хотите ПЕРЕИМЕНОВАТЬ? ";
					if (recf == recursion_on) cout << "РЕКУРСИЯ включена! ";
					else cout << "РЕКУРСИЯ выключена. ";
					cout << "[Yes/No]: ";
					getline(cin, d);
					transform(d.begin(), d.end(), d.begin(), [](unsigned char c) { return tolower(c); });
					if (d == "yes") {
						if (ren(path, ren_vec, exeptions, new_name)) cout << "Переименовано.\n\n";
						else cout << "Произошла неизвестная ошибка!\n\n";
					}
					else {
						cout << "Операция отменена.\n\n";
					}
				}
				else {
					cout << "Имя имеет недопустимые символы: '\\', '/', ':', '*', '?', '\"', '<', '>', '|'.\n" << endl;
				}
			}
			else {
				cout << "Такого пути не существует\n" << endl;
			}
		}
		else if (com == "3") {
			string path, d = "", repath = "";
			vector<string> del_vec;
			vector<string> exeptions;
			cout << "Введите путь: ";
			getline(cin, path);
			for (int c = 0; c < path.length(); ++c) {
				if (path[c] != '"') repath += path[c];
			}
			path = repath;
			if (exists(path)) {
				//тут ошибка
				cout << "Введите список ключевых слов для удаления (если список закончен, то введите '.'): " << endl;
				do {
					if (d != ".") {
						getline(cin, d);
						del_vec.push_back(d);
					}
				} while (d != ".");
				d = "";
				cout << "Введите список ключевых слов для исключений, это файлы, которые не будут удалены (если список закончен, то введите '.'): " << endl;
				do {
					if (d != ".") {
						getline(cin, d);
						exeptions.push_back(d);
					}
				} while (d != ".");

				cout << "Вы уверены, что хотите УДАЛИТЬ? ";
				if (recf == recursion_on) cout << "РЕКУРСИЯ включена! ";
				else cout << "РЕКУРСИЯ выключена. ";
				cout << "[Yes/No]: ";
				getline(cin, d);
				transform(d.begin(), d.end(), d.begin(), [](unsigned char c) { return tolower(c); });
				if (d == "yes") {
					if (del(path, del_vec, exeptions)) cout << "Удалено.\n\n";
					else cout << "Произошла неизвестная ошибка!\n\n";
				}
				else {
					cout << "Операция отменена.\n\n";
				}
			}
			else {
				cout << "Такого пути не существует\n" << endl;
			}
		}
		else if (com == "4") {
			string path, d = "", repath = "";
			vector<string> fin_vec, exeptions;
			cout << "Введите путь: ";
			getline(cin, path);
			for (int c = 0; c < path.length(); ++c) {
				if (path[c] != '"') repath += path[c];
			}
			path = repath;
			if (exists(path)) {
				cout << "Введите список ключевых слов для поиска (если список закончен, то введите '.'): " << endl;
				do {
					if (d != ".") {
						getline(cin, d);
						fin_vec.push_back(d);
					}
				} while (d != ".");
				d = "";
				cout << "Введите список ключевых слов для исключений, это файлы, которые не будут найдены (если список закончен, то введите '.'): " << endl;
				do {
					if (d != ".") {
						getline(cin, d);
						exeptions.push_back(d);
					}
				} while (d != ".");
				fin(path, fin_vec, exeptions);
			}
			else {
				cout << "Такого пути не существует\n" << endl;
			}
		}
		else if (com == "5") {
			string parsing_str;
			cout << "Введите флаги: ";
			getline(cin, parsing_str);
			flags_parser(parsing_str);
		}
		else if (com == "6") {
			string d;
			cout << "Выбор:\nИнфо [1]\nФлаги [2]\nВвод: ";
			getline(cin, d);
			if (d == "1") {
				cout << "Информация о флагах: \n" << endl;

				cout << "-rec -> включает рекусривный обход всех вложенных папок для каждой функции" << endl;
				cout << "-nrec -> выключает" << endl;
				cout << endl;

				cout << "-deld -> теперь удаляются только directories" << endl;
				cout << "-delf -> удаляются только files" << endl;
				cout << "-delfd -> удаляются directories и files" << endl;
				cout << endl;

				cout << "-cref -> создаются files" << endl;
				cout << "-cref -> создаются directories" << endl;
				cout << endl;

				cout << "-rend -> переименовывает только directories" << endl;
				cout << "-renf -> переименовывает только files" << endl;
				cout << "-renfd -> переименовывает files и directories" << endl;
				cout << endl;

				cout << "-reg -> учитывается регистр при проверках" << endl;
				cout << "-nreg -> не учитывается" << endl;
				cout << endl;

				cout << "-root -> функции переименования, создания и удаления работают с корневой папкой" << endl;
				cout << "-nroot -> функции переименования и удаления не затрагивают корневую папку, даже если она попала в список выбранных шаблонов, а функция создания не создает ничего в корневой папке" << endl;
				cout << endl;
			}
			else if (d == "2") {
				cout << "Флаги: \n" << endl;

				if (recf == recursion_on) cout << "-rec: on" << endl;
				else cout << "-rec: off" << endl;
				if (recf == recursion_off) cout << "-nrec: on" << endl;
				else cout << "-nrec: off\n";
				cout << endl;

				if (delf == del_dir) cout << "-deld: on" << endl;
				else cout << "-deld: off" << endl;
				if (delf == del_files) cout << "-delf: on" << endl;
				else cout << "-delf: off" << endl;
				if (delf == del_dir_files) cout << "-delfd: on" << endl;
				else cout << "-delfd: off\n";
				cout << endl;

				if (cref == cre_files) cout << "-cref: on" << endl;
				else cout << "-cref: off" << endl;
				if (cref == cre_dir) cout << "-cref: on" << endl;
				else cout << "-cred: off\n";
				cout << endl;

				if (renf == ren_dir) cout << "-rend: on" << endl;
				else cout << "-rend: off" << endl;
				if (renf == ren_files) cout << "-renf: on" << endl;
				else cout << "-renf: off" << endl;
				if (renf == ren_dir_files) cout << "-renfd: on" << endl;
				else cout << "-renfd: off\n";
				cout << endl;

				if (regf == reg_on) cout << "-reg: on" << endl;
				else cout << "-reg: off" << endl;
				if (regf == reg_off) cout << "-nreg: on" << endl;
				else cout << "-nreg: off\n";
				cout << endl;

				if (regf == root_on) cout << "-root: on" << endl;
				else cout << "-root: off" << endl;
				if (regf == root_off) cout << "-nroot: on" << endl;
				else cout << "-nroot: off\n";
				cout << endl;
			}
			else cout << "Такой команды нет.\n" << endl;
		}
		else if (com == "7") {
			string d;
			cout << "Выбор:\nПоказать [1]\nОчистить [2]\nВвод: ";
			getline(cin, d);
			if (d == "1") {
				get_history();
			}
			else if (d == "2") {
				cout << "Вы уверены, что хотите очистить историю команд? [Yes/No]: ";
				getline(cin, d);
				transform(d.begin(), d.end(), d.begin(), [](unsigned char c) { return tolower(c); });
				if (d == "yes") {
					if (del_history()) cout << "История очищена.\n\n";
					else cout << "Произошла неизвестная ошибка!\n\n";
				}
				else {
					cout << "Операция отменена.\n\n";
				}
			}
			else {
				cout << "Выбрана неверная команда!\n\n";
			}
		}
		else if (com == "8") {
			stop = true;
		}
		else {
			cout << "Такой команды не существует!\n" << endl;
		}
	}
	cout << "Работа завершена.\n" << endl;
	cin.ignore(); //очистка буфера
	cin.get(); //ждёт нажатия Enter
}