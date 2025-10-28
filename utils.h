#pragma once
#include <wx/wx.h>
#include <vector>
#include "logicgates.h"

// 文件操作工具函数
bool SaveCircuitToFile(const wxString& filename, const std::vector<Gate>& gates);
bool LoadCircuitFromFile(const wxString& filename, std::vector<Gate>& gates);

// 其他工具函数
wxString GetAppPath();
bool FileExists(const wxString& filename);
