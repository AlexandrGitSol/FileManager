#include "FileManager.h"
using namespace std;

FileManager::FileManager() :
	stop(false),
	recf(recursion_on),
	delf(del_dir_files),
	cref(cre_files),
	renf(ren_dir_files),
	regf(reg_off),
	rootf(root_on),
	danger_chars({ '\\','/',':','*','?','"','<','>','|' }),
	avaliable_flags({ "-rec", "-nrec", "-deld", "-delf", "-delfd", "-cref", "-cred", "-renf", "-rend", "-renfd", "-reg", "-nreg", "-root", "-nroot" })
{
	if (!exists("history.db")) {
		ofstream{ "history.db" };
	}
	sqlite3* db;
	sqlite3_open("history.db", &db);

	const char* sqlq =
		"CREATE TABLE IF NOT EXISTS history"
		"("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"command TEXT,"
		"param TEXT"
		");";

	char* errmsg = nullptr;
	sqlite3_exec(db, sqlq, nullptr, nullptr, &errmsg);
	if (errmsg) cerr << "Error: " << errmsg << endl << endl;

	sqlite3_close(db);
}

int callback_for_get_history(void* his, int num, char** vals, char** cols) {
	vector<vector<string>>* history = static_cast<vector<vector<string>>*>(his);
	vector<string> str;
	for (int i = 0; i < num; ++i) {
		str.push_back(vals[i]);
	}
	history->push_back(str);
	return 0;
}

vector<vector<string>> FileManager::get_history() {
	vector<vector<string>> history;

	sqlite3* db;
	sqlite3_open("history.db", &db);

	const char* sqlq =
		"SELECT command, param FROM history;";

	char* errmsg = nullptr;
	sqlite3_exec(db, sqlq, callback_for_get_history, &history, &errmsg);
	if (errmsg) cerr << "Error: " << errmsg << endl << endl;

	sqlite3_close(db);

	sqlite3_open("history.db", &db);

	sqlq = "INSERT INTO history (command, param) VALUES (\"history\", \"\");";

	char* errmsg2 = nullptr;
	sqlite3_exec(db, sqlq, nullptr, nullptr, &errmsg2);
	if (errmsg2) cerr << "Error: " << errmsg2 << endl;


	sqlite3_close(db);

	return history;
}

bool FileManager::del_history() {
	sqlite3* db;
	sqlite3_open("history.db", &db);

	const char* sqlq =
		"DELETE FROM history;";

	char* errmsg = nullptr;
	sqlite3_exec(db, sqlq, nullptr, nullptr, &errmsg);
	if (errmsg) {
		//cerr << "Error: " << errmsg << endl << endl;
		sqlite3_close(db);
		return false;
	}
	else {
		sqlite3_close(db);
		return true;
	}
}

bool FileManager::is_correct_flags_string(string flags_string) {
	string flag;
	int i = 0;
	while (i < flags_string.length()) {
		while (i < flags_string.length() && flags_string[i] != '-') {
			if (flags_string[i] != ' ') return false;
			i++;
		}
		flag = "";
		while (i < flags_string.length() && flags_string[i] != ' ') {
			flag += flags_string[i++];
		}
		if (find(avaliable_flags.begin(), avaliable_flags.end(), flag) == avaliable_flags.end() && flag != "") return false;
	}
	return true;
}

bool FileManager::flags_parser(string all_flags) {
	if (!is_correct_flags_string(all_flags)) {
		//cout << "Ошибка чтения флагов.\n" << endl;
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
	return true;
	//cout << "Флаги успешно установлены.\n" << endl;
}

bool FileManager::del(path path_, vector<string>& ext, vector<string>& exeptions, bool first_call) {
	if (exists(path_)) {
		if (!checker(path_.filename().string(), exeptions) && checker(path_.filename().string(), ext)) {
			if (rootf == root_on && first_call && (delf == del_dir || delf == del_dir_files) && is_directory(path_)) {
				remove_all(path_);
				return true;
			}
			else if ((delf == del_files || delf == del_dir_files) && !is_directory(path_)) {
				remove(path_);
				return true;
			}
		}
		if (first_call) {
			sqlite3* db;

			sqlite3_open("history.db", &db);
			const string preq = "INSERT INTO history (command, param) VALUES (\"delete\", \"" + path_.string() + "\");";

			const char* sqlq = preq.c_str();

			char* errmsg = nullptr;
			sqlite3_exec(db, sqlq, nullptr, nullptr, &errmsg);
			if (errmsg) cerr << "Error: " << errmsg << endl;


			sqlite3_close(db);
		}
		first_call = false;
		if (!checker(path_.filename().string(), exeptions)) {
			for (auto& it : directory_iterator(path_)) {
				if (is_directory(it.path())) {
					if (!checker(it.path().filename().string(), exeptions) && checker(it.path().filename().string(), ext)) {
						if (delf == del_dir || delf == del_dir_files) remove_all(it.path());
					}
					else if (recf == recursion_on) {
						del(it.path(), ext, exeptions, first_call);
						//thread th([&](){ del(it.path(), ext, exeptions); });
						//th.detach();
					}
				}
				else {
					if (!checker(it.path().filename().string(), exeptions) && checker(it.path().filename().string(), ext)) {
						if (delf == del_dir_files || delf == del_files) remove(it.path());
					}
				}
			}
		}
		return true;
	}
	return false;
}

//тут баг - при переименовании файлов в тоже имя происходит конфликт между ними и num получает лишнии номера
//еще баг - нет рекурсивного обхода
bool FileManager::ren(path path, vector<string>& ext, vector<string>& exeptions, string name, bool first_call) {
	if (exists(path)) {
		if (!checker(path.filename().string(), exeptions) && checker(path.filename().string(), ext)) {
			if ((rootf == root_on || (rootf == root_off && !first_call)) && (is_directory(path) && (renf == ren_dir || renf == ren_dir_files)) || (!is_directory(path) && (renf == ren_files || renf == ren_dir_files))) {
				//тут
				//if (rootf == root_on || (rootf == root_off && !first_call)) rename(path, path.parent_path().string() + '\\' + name);
				#ifdef _WIN64
					rename(path, path.parent_path().string() + '\\' + name);
					path = path.parent_path().string() + '\\' + name;
				#elif __linux__
					rename(path, path.parent_path().string() + '/' + name);
					path = path.parent_path().string() + '/' + name;
				#endif
			}
		}
		if (first_call) {
			sqlite3* db;

			sqlite3_open("history.db", &db);

			const string preq = "INSERT INTO history (command, param) VALUES (\"rename\", \"" + path.string() + "\");";

			const char* sqlq = preq.c_str();

			char* errmsg = nullptr;
			sqlite3_exec(db, sqlq, nullptr, nullptr, &errmsg);
			if (errmsg) cerr << "Error: " << errmsg << endl;


			sqlite3_close(db);
		}
		//this_first_call - исправление бага (см. create)
		first_call = false;
		if (!checker(path.filename().string(), exeptions)) {
			int count_files = 0;
			int count_folders = 0;
			for (auto& it : directory_iterator(path)) {
				if (!checker(it.path().filename().string(), exeptions) && checker(it.path().filename().string(), ext)) {
					if ((renf == ren_files) || (renf == ren_dir_files) && !is_directory(it.path())) ++count_files;
					else if ((renf == ren_dir) || (renf == ren_dir_files) && is_directory(it.path())) ++count_folders;
				}
			}
			int num = 1;
			for (auto& it : directory_iterator(path)) {
				if (count_files + count_folders <= 1) {
					if (is_directory(it.path())) {
						string new_name = it.path().parent_path().string();
						if (!checker(it.path().filename().string(), exeptions) && checker(it.path().filename().string(), ext)) {
							#ifdef _WIN64
								new_name += '\\' + name;
							#elif __linux__
								new_name += '/' + name;
							#endif
						}
						else {
							new_name = it.path().string();
						}
						rename(it.path(), new_name);
						if (recf == recursion_on && (rootf == root_on || (rootf == root_off && !first_call))) {
							ren(new_name, ext, exeptions, name, first_call);
							//thread th([&]() { ren(new_name, ext, exeptions, name); });
							//th.detach();
						}
					}
					else {
						if (!checker(it.path().filename().string(), exeptions) && checker(it.path().filename().string(), ext)) {
							if (renf == ren_dir_files || renf == ren_files) rename(it.path(), it.path().parent_path().string() + '\\' + name);
						}
					}
				}
				else {
					string new_name = it.path().string();
					if (!checker(it.path().filename().string(), exeptions) && checker(it.path().filename().string(), ext)) {
						bool first_dot;
						do {
							#ifdef _WIN64
								new_name = it.path().parent_path().string() + "\\";
							#elif __linux__
								new_name = it.path().parent_path().string() + "/";
							#endif
							first_dot = true;
							for (int i = 0; i < name.length(); i++) {
								if (name[i] != '.') {
									new_name += name[i];
								}
								else if (first_dot) {
									new_name += to_string(num++) + name[i];
									first_dot = false;
								}
							}
							if (first_dot) new_name += to_string(num++);
						} while (exists(new_name));
						if (is_directory(it.path()) && (renf == ren_dir || renf == ren_dir_files) || !is_directory(it.path()) && (renf == ren_files || renf == ren_dir_files)) rename(it.path(), new_name);
					}
					if (is_directory(new_name) && !checker(it.path().filename().string(), exeptions)) {
						if (recf == recursion_on && (rootf == root_on || (rootf == root_off && !first_call))) {
							ren(new_name, ext, exeptions, name, first_call);
							//thread th([&]() { ren(new_name, ext, exeptions, name); });
							//th.detach();
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}

bool FileManager::cre(path path, string name, int count_f, bool first_call) {
	if (exists(path)) {
		if (count_f < 1) return false;
		bool this_first_call = first_call;

		if (first_call) {
			sqlite3* db;

			sqlite3_open("history.db", &db);
			const string preq = "INSERT INTO history (command, param) VALUES (\"create\", \"" + path.string() + "\");";

			const char* sqlq = preq.c_str();

			char* errmsg = nullptr;
			sqlite3_exec(db, sqlq, nullptr, nullptr, &errmsg);
			if (errmsg) cerr << "Error: " << errmsg << endl;


			sqlite3_close(db);
		}

		first_call = false;
		if (recf == recursion_on) {
			for (auto& it : directory_iterator(path)) {
				if (is_directory(it.path())) {
					cre(it.path(), name, count_f, first_call);
					//thread th([&]() { cre(it.path(), name, count_f); });
					//th.detach();
				}
			}
		}
		if (rootf == root_on || (rootf == root_off && !this_first_call)) {
			if (count_f == 1) {
				if (cref == cre_files){
					#ifdef _WIN64
						ofstream{ path.string() + '\\' + name };
					#elif __linux__
						ofstream{ path.string() + '/' + name };
					#endif
				}
				else if (cref == cre_dir){
					#ifdef _WIN64
					    create_directory(path.string() + '\\' + name);
					#elif __linux__
					    create_directory(path.string() + '/' + name);
					#endif
				}
			}
			else {
				int num = 1;
				for (int i = 0; i < count_f; i++) {
					string new_name;
					bool first_dot;
					do {
						#ifdef _WIN64
							new_name = path.string() + "\\";
						#elif __linux__
							new_name = path.string() + "/";
						#endif
						first_dot = true;
						for (int i = 0; i < name.length(); i++) {
							if (name[i] != '.') {
								new_name += name[i];
							}
							else if (first_dot) {
								new_name += to_string(num++) + name[i];
								first_dot = false;
							}
						}
						if (first_dot) new_name += to_string(num++);
					} while (exists(new_name));
					if (cref == cre_files) ofstream{ new_name };
					else if (cref == cre_dir) create_directory(new_name);
				}
			}
		}
		return true;
	}
	return false;
}

//Введите список файлов для переименования(если список закончен, то введите '.') :
//	Gg *
//	.
//	Введите список ключевых слов для исключений, это файлы, которые не будут переименованы(если список закончен, то введите '.') :
//	.
//	Переименовано.
//Итог: папка Gg осталась

//Исправлено (99%)

bool FileManager::checker(string name, vector<string>& str_list) {
	if (regf == reg_off) {
		transform(name.begin(), name.end(), name.begin(), [](char c) { return tolower((int)c); });
		for (int i = 0; i < str_list.size(); i++) {
			transform(str_list[i].begin(), str_list[i].end(), str_list[i].begin(), [](char c) { return tolower((int)c); });
		}
	}
	for (int i = 0; i < str_list.size(); i++) {
		int first = 0, second = 0;
		bool ret_false = false;
		while (first < name.length() && second < str_list[i].length()) {
			if (str_list[i][second] != '*') {
				if (str_list[i][second] != name[first]) {
					ret_false = true;
				}
				++first;
				++second;
			}
			else {
				while (second < str_list[i].length() && str_list[i][second] == '*') {
					++second;
				}
				if (second >= str_list[i].length()) return true;
				while (first < name.length() && second < str_list[i].length() && str_list[i][second] != name[first]) {
					++first;
				}
				if (first >= name.length()) ret_false = true;
			}
		}
		if (first < name.length() || second < str_list[i].length()) ret_false = true;

		if (!ret_false) return true;
	}
	return false;
}


//bool FileManager::checker(string name, vector<string>& str_list) {
//	if (regf == reg_off) {
//		transform(name.begin(), name.end(), name.begin(), [](char c) { return tolower((int)c); });
//		for (int i = 0; i < str_list.size(); i++) {
//			transform(str_list[i].begin(), str_list[i].end(), str_list[i].begin(), [](char c) { return tolower((int)c); });
//		}
//	}
//	for (int i = 0; i < str_list.size(); i++) {
//		int left = 0, right = 0;
//		int l = 0, rs = 0;
//		bool flag = true;
//		bool ending = false;
//		int last_rs = 0;
//		bool starting = false;
//		bool flag_for_starting = false;
//		bool flag_for_ending = true;
//		while (right < str_list[i].length()) {
//			while (right < str_list[i].length() && str_list[i][right] != '*') {
//				right++;
//			}
//			if (right == 0) starting = true;
//			if (right == 0) l++;
//			if (right == str_list[i].length() - 1) ending = true;
//			string sub = str_list[i].substr(left, right - left);
//			rs = sub.size();
//			while (rs + l < name.length() && name.substr(l, rs) != sub) {
//				l++;
//			}
//			last_rs = rs;
//			if (name.substr(l, rs) != sub) {
//				right = str_list[i].length();
//				flag = false;
//			}
//			else {
//				if (starting && (l != 0) || !starting && (l == 0)) flag_for_starting = true;
//				//тут добавили флаг
//				if (right >= str_list[i].length() - 1 && !ending && ((l + rs - 1) < name.length() - 1)) flag_for_ending = false;
//				left = right + 1;
//				right = left;
//				rs = 0;
//			}
//		}
//		if (flag_for_ending && flag_for_starting && flag && ((ending && ((l + last_rs) < (name.length()))) || (!ending && ((l + last_rs) >= (name.length() - 1))))) return true;
//	}
//	return false;
//}

bool FileManager::have_danger_characters(string name) {
	for (char ch : danger_chars) {
		if (find(name.begin(), name.end(), ch) != name.end()) return true;
	}
	return false;
}

vector<path> FileManager::fin(path pathv, vector<string>& ext, vector<string>& exeptions) {
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
		
		sqlite3* db;

		sqlite3_open("history.db", &db);

		const string preq = "INSERT INTO history (command, param) VALUES (\"find\", \"" + pathv.string() + "\");";

		const char* sqlq = preq.c_str();

		char* errmsg = nullptr;
		sqlite3_exec(db, sqlq, nullptr, nullptr, &errmsg);
		if (errmsg) cerr << "Error: " << errmsg << endl;


		sqlite3_close(db);

		return ans;
	}
	return vector<path>();
}

void FileManager::helper_fin(path pathv, vector<string>& ext, vector<string>& exeptions, vector<path>& all_paths) {
	for (auto it : directory_iterator(pathv)) {
		all_paths.push_back(it.path().string());
		if (is_directory(it.path())) {
			helper_fin(it.path(), ext, exeptions, all_paths);
		}
	}
}