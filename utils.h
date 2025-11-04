#pragma once
#include <wx/wx.h>
#include <vector>
#include "logicgates.h"

// 文件信息结构体
struct FileInfo {
    wxString filename;
    bool exists;
    double size; // 文件大小（字节）
    wxDateTime modifiedTime;
    wxString version;
    size_t gateCount;
    size_t wireCount;
    wxString description;
};

// 文件操作函数
bool SaveCircuitToFile(const wxString& filename, const std::vector<Gate>& gates, const std::vector<Wire>& wires = {});
bool LoadCircuitFromFile(const wxString& filename, std::vector<Gate>& gates, std::vector<Wire>& wires);

// 工具函数
wxString GetAppPath();
bool FileExists(const wxString& filename);
wxString GetFileFormatVersion(const wxString& filename);

// 文件格式验证
bool IsValidCircuitFile(const wxString& filename);
FileInfo GetFileInfo(const wxString& filename);

// 备份功能
bool CreateBackup(const wxString& filename);

// 最近文件管理
std::vector<wxString> GetRecentFiles();
void AddToRecentFiles(const wxString& filename);

// 导出功能
bool ExportToSVG(const wxString& filename, const std::vector<Gate>& gates, const std::vector<Wire>& wires);
bool ExportToPNG(const wxString& filename, const std::vector<Gate>& gates, const std::vector<Wire>& wires);
std::vector<wxString> GetSupportedExportFormats();
