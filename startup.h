#include <iostream>
#include <vector>
#include <string>
#include <tchar.h>
#include <io.h>

using namespace std;

vector<string> getFiles(string path) {
    vector<string> files;
    _finddata_t file;
    long lf;
    if ((lf=_findfirst(path.c_str(), &file)) == -1) {
        cout<<path<<" not found!!!"<<endl;
    }
    else {
        while(_findnext(lf, &file) == 0) {
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
                continue;
            if (strcmp(file.name, "desktop.ini")!=0)
                files.push_back(file.name);
        }
    }
    _findclose(lf);
    return files;
}