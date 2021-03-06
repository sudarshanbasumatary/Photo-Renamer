// Photo Renamer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdio>
#include <string>
#include <cctype>

using namespace std;

struct fileData {
	string fdate, ftime, fname, fext;
	int num = 0;
};

inline string lower_case(string exts) {
	string ext = "";

	for (int i = 0, len = exts.length(); i < len; i++)
		ext += tolower(exts[i]);
	
	return ext;
}

bool get_file_names(vector <fileData> &fList) {
	ifstream fileList("flist", ios::in);

	string fdetails;
	if (fileList.is_open()) {
		short k = 0;
		//skipping volume info
		while (getline(fileList, fdetails)) {
			if (++k == 5)
				break;
		}
		//read file info
		while (getline(fileList, fdetails)) {
			string exts, fname, fdate, ftime;
			int flen = fdetails.length();

			//since it's a non-bare dir listing, string length will be always more than 39
			exts = fdetails.substr(flen - 3, 3);

			//only rename files starting with dsc_ or nik_ and having nef, jpg, mov file extensions
			if ((lower_case(exts) == "nef" || lower_case(exts) == "jpg" || lower_case(exts) == "mov") && (lower_case(fdetails.substr(39, 4)) == "dsc_" || lower_case(fdetails.substr(39, 4)) == "nik_" || lower_case(fdetails.substr(39, 3)) == "nik")) {
				fileData f;
				f.fdate = fdetails.substr(0, 10);
				f.ftime = fdetails.substr(12, 8);
				f.fname = fdetails.substr(39, flen - 39);
				f.fext = exts;
				fList.push_back(f);
			}
		}
		fileList.close();
		return false;
	}
	else {
		cout << "Could not open file!" << endl;
		return true;
	}
}

inline string format_date(const string date) {
	return date.substr(6, 4) + date.substr(0, 2) + date.substr(3, 2);
}

string format_time(string time) {
	string ft = time.substr(0, 2) + time.substr(3, 2);
	if (time[6] == 'P') {
		if (ft[1] < '8') {
			ft[0] += 1;
			ft[1] += 2;
		}
		else if (ft[1] == '8') {
			ft[0] = '2';
			ft[1] = '0';
		}
		else if (ft[1] == '9') {
			ft[0] = '2';
			ft[1] = '1';
		}
	}
	else if (ft[0] == 1 && ft[1] == 2)
		ft[0] = ft[1] = '0';

	return ft;
}

string format_num(int num) {
	string snum = "";
	while (num) {
		snum = (char)(num % 10 + '0') + snum;
		num /= 10;
	}
	while (snum.length() < 4)
		snum = '0' + snum;

	return snum;
}

inline bool file_test(const std::string& name) {
	if (FILE *file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	}
	else
		return false;
}

void rename_files(vector <fileData> &fList) {
	string new_fname;
	for (unsigned int i = 0; i < fList.size(); i++) {
		while (true) {
			if ((new_fname = "NIK_" + format_date(fList[i].fdate) + "_" + format_time(fList[i].ftime) + "_" + format_num(fList[i].num++) + "." + fList[i].fext) != fList[i].fname) {
				if (!file_test(new_fname)) {
					if (!rename(fList[i].fname.c_str(), new_fname.c_str()))
						cout << "Rename Success: " << new_fname << endl;
					else cout << "Rename Failed: " << fList[i].fname << " " << new_fname << endl;
					break;
				}
			}
			else break;
		}
	}
}

int dir_update() {
	vector <fileData> fList;
	int return_value = 0;

	//dir listing without directories
	if (!system("dir /A:-D > flist")) {
		if (get_file_names(fList) == false) {
			rename_files(fList);
			return_value = 0;
		}
		else {
			cout << "File list read error!" << endl;
			return_value = 1;
		}

		if (system("del flist")) {
			cout << "Couldn't delete list.txt" << endl;
			return_value = -1;
		}
	}
	else {
		cout << "Command processor not found!" << endl;
		return_value = -1;
	}

	//0 ok
	//-1 command processor error
	//1	internal error
	return return_value;
}

string get_running_dir() {
	ifstream dirList("dlist", ios::in);

	string dDetails;
	if (dirList.is_open()) {
		short k = 0;
		//skipping lines till directory list
		while (getline(dirList, dDetails)) {
			if (++k == 4) {
				dDetails = dDetails.substr(14, dDetails.length() - 14);
				break;
			}
		}
		dirList.close();
	}
	return dDetails;
}

void call_recursive(const string sdir) {
	string cd_cmd = "cd " + sdir;
	cout << cd_cmd << endl;
	system(cd_cmd.c_str());
	system("dir > a.txt");
	getchar();

	//dir listing for directories
	if (!system("dir /A:D > dlist")) {
		ifstream dirList("dlist", ios::in);

		string dDetails, cd_cmd;
		if (dirList.is_open()) {
			short k = 0;
			//skipping lines till directory list
			while (getline(dirList, dDetails)) {
				if (++k == 7)
					break;
			}
			//read directory info
			while (getline(dirList, dDetails)) {
				if (lower_case(dDetails.substr(25, 3)) == "dir") {
					call_recursive(dDetails.substr(39, dDetails.length() - 39));
				}
			}
			dirList.close();
		}
		//update filenames in current directory
		cout << get_running_dir() << " " << dir_update() << endl;
		system("del dlist");
		//return back to parent directory
		system("cd ..");
	}
	else cout << "Command processor not found!" << endl;
}

int main(int argc, char* argv[]) {
	if (argc == 2 && lower_case(argv[1]) == "-r") {
		call_recursive(".");
	}
	else {
		cout << dir_update() << endl;
	}
	return 0;
}

