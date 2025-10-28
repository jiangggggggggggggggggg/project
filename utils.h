#pragma once
#include <wx/wx.h>
#include <vector>
#include "logicgates.h"

// �ļ��������ߺ���
bool SaveCircuitToFile(const wxString& filename, const std::vector<Gate>& gates);
bool LoadCircuitFromFile(const wxString& filename, std::vector<Gate>& gates);

// �������ߺ���
wxString GetAppPath();
bool FileExists(const wxString& filename);
