#include "utils.h"
#include <fstream>

bool SaveCircuitToFile(const wxString& filename, const std::vector<Gate>& gates) {
    // ʵ���ļ������߼�
    return true;
}

bool LoadCircuitFromFile(const wxString& filename, std::vector<Gate>& gates) {
    // ʵ���ļ������߼�
    return true;
}

wxString GetAppPath() {
    return wxGetCwd();
}

bool FileExists(const wxString& filename) {
    return wxFileExists(filename);
}