#include "utils.h"
#include <fstream>

bool SaveCircuitToFile(const wxString& filename, const std::vector<Gate>& gates) {
    // 实现文件保存逻辑
    return true;
}

bool LoadCircuitFromFile(const wxString& filename, std::vector<Gate>& gates) {
    // 实现文件加载逻辑
    return true;
}

wxString GetAppPath() {
    return wxGetCwd();
}

bool FileExists(const wxString& filename) {
    return wxFileExists(filename);
}