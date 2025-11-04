#include "utils.h"
#include <fstream>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include "json.hpp"

using json = nlohmann::json;

// 辅助函数：安全地从 JSON 获取字符串
wxString GetJsonString(const json& j, const std::string& key, const wxString& defaultValue = wxEmptyString) {
    if (j.find(key) != j.end() && j[key].is_string()) {
        return wxString::FromUTF8(j[key].get<std::string>().c_str());
    }
    return defaultValue;
}

// 辅助函数：安全地从 JSON 获取整数值
int GetJsonInt(const json& j, const std::string& key, int defaultValue = 0) {
    if (j.find(key) != j.end() && j[key].is_number()) {
        return j[key].get<int>();
    }
    return defaultValue;
}

// 保存电路到文件
bool SaveCircuitToFile(const wxString& filename, const std::vector<Gate>& gates, const std::vector<Wire>& wires) {
    try {
        json j;
        j["version"] = "1.1";
        j["type"] = "circuit";

        // 修复：正确转换 wxString 到 std::string
        std::string createdTime = std::string(wxDateTime::Now().FormatISOCombined(' ').mb_str(wxConvUTF8));
        j["created"] = createdTime;
        j["description"] = "Circuit diagram created with Circuit Editor";

        // 保存门信息
        std::vector<json> gatesJson;
        for (auto& gate : gates) {
            json gateJson;

            // 修复：正确转换 wxString
            gateJson["type"] = std::string(gate.type.mb_str(wxConvUTF8));
            gateJson["x"] = gate.pos.x;
            gateJson["y"] = gate.pos.y;

            // 保存属性
            std::vector<json> propsJson;
            for (auto& prop : gate.properties) {
                json propJson;
                propJson["name"] = std::string(prop.name.mb_str(wxConvUTF8));
                propJson["value"] = std::string(prop.value.mb_str(wxConvUTF8));
                propJson["type"] = std::string(prop.type.mb_str(wxConvUTF8));
                propsJson.push_back(propJson);
            }
            gateJson["properties"] = propsJson;

            // 保存引脚信息
            std::vector<json> pinsJson;
            for (auto& pin : gate.pins) {
                json pinJson;
                pinJson["x"] = pin.position.x;
                pinJson["y"] = pin.position.y;
                pinJson["type"] = (pin.type == PinType::Input) ? "Input" : "Output";
                pinJson["name"] = std::string(pin.name.mb_str(wxConvUTF8));
                pinJson["connected"] = pin.connected;
                pinsJson.push_back(pinJson);
            }
            gateJson["pins"] = pinsJson;

            gatesJson.push_back(gateJson);
        }
        j["gates"] = gatesJson;

        // 保存连线信息
        std::vector<json> wiresJson;
        for (auto& wire : wires) {
            json wireJson;

            // 保存起点信息
            json startJson;
            startJson["gateIndex"] = wire.start.gateIndex;
            startJson["pinIndex"] = wire.start.pinIndex;
            startJson["x"] = wire.start.position.x;
            startJson["y"] = wire.start.position.y;
            wireJson["start"] = startJson;

            // 保存终点信息
            json endJson;
            endJson["gateIndex"] = wire.end.gateIndex;
            endJson["pinIndex"] = wire.end.pinIndex;
            endJson["x"] = wire.end.position.x;
            endJson["y"] = wire.end.position.y;
            wireJson["end"] = endJson;

            // 保存路径点
            std::vector<json> pathJson;
            for (auto& point : wire.path) {
                json pointJson;
                pointJson["x"] = point.x;
                pointJson["y"] = point.y;
                pathJson.push_back(pointJson);
            }
            wireJson["path"] = pathJson;

            wireJson["isSelected"] = wire.isSelected;

            wiresJson.push_back(wireJson);
        }
        j["wires"] = wiresJson;

        // 保存统计信息
        j["statistics"] = {
            {"totalGates", gates.size()},
            {"totalWires", wires.size()},
            {"fileSize", 0}
        };

        std::ofstream f(filename.ToStdString());
        if (!f.is_open()) {
            wxLogError("无法打开文件进行写入: %s", filename);
            return false;
        }

        f << j.dump(4);
        f.close();

        // 验证文件是否成功写入
        wxFile file(filename);
        if (file.Length() == 0) {
            wxLogError("文件写入失败: %s", filename);
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        wxLogError("保存文件时发生异常: %s", e.what());
        return false;
    }
    catch (...) {
        wxLogError("保存文件时发生未知异常");
        return false;
    }
}

// 从文件加载电路
bool LoadCircuitFromFile(const wxString& filename, std::vector<Gate>& gates, std::vector<Wire>& wires) {
    std::ifstream f(filename.ToStdString());
    if (!f.is_open()) {
        wxLogError("无法打开文件: %s", filename);
        return false;
    }

    try {
        json j;
        f >> j;
        f.close();

        // 验证文件格式
        if (j.find("version") == j.end() || j.find("gates") == j.end()) {
            wxLogError("无效的电路图文件格式");
            return false;
        }

        // 修复：显式转换版本字符串
        std::string versionStr = j["version"].get<std::string>();
        wxString version = wxString::FromUTF8(versionStr.c_str());

        if (version != "1.0" && version != "1.1") {
            wxLogError("不支持的文件版本: %s", version);
            return false;
        }

        gates.clear();
        wires.clear();

        // 加载门信息
        for (auto& gateJson : j["gates"]) {
            Gate gate;

            // 修复：显式转换类型字符串
            std::string typeStr = gateJson["type"].get<std::string>();
            gate.type = wxString::FromUTF8(typeStr.c_str());

            gate.pos.x = gateJson["x"].get<int>();
            gate.pos.y = gateJson["y"].get<int>();

            // 加载属性
            if (gateJson.find("properties") != gateJson.end()) {
                for (auto& propJson : gateJson["properties"]) {
                    Property prop;

                    // 修复：显式转换属性字符串
                    std::string nameStr = propJson["name"].get<std::string>();
                    std::string valueStr = propJson["value"].get<std::string>();
                    std::string typeStr = propJson["type"].get<std::string>();

                    prop.name = wxString::FromUTF8(nameStr.c_str());
                    prop.value = wxString::FromUTF8(valueStr.c_str());
                    prop.type = wxString::FromUTF8(typeStr.c_str());

                    gate.properties.push_back(prop);
                }
            }

            // 加载引脚信息（版本1.1支持）
            if (gateJson.find("pins") != gateJson.end()) {
                for (auto& pinJson : gateJson["pins"]) {
                    Pin pin;
                    pin.position.x = pinJson["x"].get<int>();
                    pin.position.y = pinJson["y"].get<int>();

                    // 修复：显式转换引脚名称
                    std::string nameStr = pinJson["name"].get<std::string>();
                    pin.name = wxString::FromUTF8(nameStr.c_str());

                    std::string typeStr = pinJson["type"].get<std::string>();
                    pin.type = (typeStr == "Input") ? PinType::Input : PinType::Output;
                    pin.connected = pinJson.value("connected", false);
                    gate.pins.push_back(pin);
                }
            }
            else {
                // 如果没有保存引脚信息，使用默认引脚定义
                auto pinIt = pinDefinitions.find(gate.type);
                if (pinIt != pinDefinitions.end()) {
                    gate.pins = pinIt->second;
                }
            }

            gates.push_back(gate);
        }

        // 加载连线信息
        if (j.find("wires") != j.end()) {
            for (auto& wireJson : j["wires"]) {
                Wire wire;

                // 加载起点
                if (wireJson.find("start") != wireJson.end()) {
                    wire.start.gateIndex = wireJson["start"]["gateIndex"].get<int>();
                    wire.start.pinIndex = wireJson["start"]["pinIndex"].get<int>();
                    wire.start.position.x = wireJson["start"]["x"].get<int>();
                    wire.start.position.y = wireJson["start"]["y"].get<int>();
                }

                // 加载终点
                if (wireJson.find("end") != wireJson.end()) {
                    wire.end.gateIndex = wireJson["end"]["gateIndex"].get<int>();
                    wire.end.pinIndex = wireJson["end"]["pinIndex"].get<int>();
                    wire.end.position.x = wireJson["end"]["x"].get<int>();
                    wire.end.position.y = wireJson["end"]["y"].get<int>();
                }

                // 加载路径
                if (wireJson.find("path") != wireJson.end()) {
                    for (auto& pointJson : wireJson["path"]) {
                        wxPoint point(pointJson["x"].get<int>(), pointJson["y"].get<int>());
                        wire.path.push_back(point);
                    }
                }
                else {
                    // 如果没有保存路径，计算路径
                    wire.path = { wire.start.position, wire.end.position };
                }

                wire.isSelected = wireJson.value("isSelected", false);

                wires.push_back(wire);
            }
        }

        wxLogMessage("成功加载电路文件: %s (版本: %s, 门: %zu, 连线: %zu)",
            filename, version, gates.size(), wires.size());
        return true;
    }
    catch (const json::exception& e) {
        wxLogError("JSON解析错误: %s", e.what());
        return false;
    }
    catch (const std::exception& e) {
        wxLogError("加载文件时发生异常: %s", e.what());
        return false;
    }
    catch (...) {
        wxLogError("加载文件时发生未知异常");
        return false;
    }
}

// 验证电路文件格式
bool IsValidCircuitFile(const wxString& filename) {
    if (!FileExists(filename)) {
        return false;
    }

    std::ifstream f(filename.ToStdString());
    if (!f.is_open()) {
        return false;
    }

    try {
        json j;
        f >> j;
        f.close();

        // 基本格式验证
        if (j.find("version") == j.end() || j.find("gates") == j.end()) {
            return false;
        }

        // 版本验证 - 修复：显式转换
        std::string versionStr = j["version"].get<std::string>();
        wxString version = wxString::FromUTF8(versionStr.c_str());

        if (version != "1.0" && version != "1.1") {
            return false;
        }

        // 基本数据结构验证
        if (!j["gates"].is_array()) {
            return false;
        }

        if (j.find("wires") != j.end() && !j["wires"].is_array()) {
            return false;
        }

        return true;
    }
    catch (...) {
        return false;
    }
}

// 获取文件格式版本
wxString GetFileFormatVersion(const wxString& filename) {
    std::ifstream f(filename.ToStdString());
    if (!f.is_open()) {
        return wxEmptyString;
    }

    try {
        json j;
        f >> j;
        f.close();

        if (j.find("version") != j.end()) {
            // 修复：显式转换版本字符串
            std::string versionStr = j["version"].get<std::string>();
            return wxString::FromUTF8(versionStr.c_str());
        }
    }
    catch (...) {
        // 忽略异常，返回空字符串
    }

    return wxEmptyString;
}

// 获取文件信息
FileInfo GetFileInfo(const wxString& filename) {
    FileInfo info;
    info.filename = filename;
    info.exists = FileExists(filename);

    if (info.exists) {
        wxFileName fn(filename);
        info.size = fn.GetSize().ToDouble();
        info.modifiedTime = fn.GetModificationTime();
        info.version = GetFileFormatVersion(filename);

        // 获取基本统计信息
        if (IsValidCircuitFile(filename)) {
            std::ifstream f(filename.ToStdString());
            if (f.is_open()) {
                try {
                    json j;
                    f >> j;
                    f.close();

                    info.gateCount = j["gates"].size();
                    info.wireCount = j.find("wires") != j.end() ? j["wires"].size() : 0;

                    // 修复：显式转换描述字符串
                    if (j.find("description") != j.end()) {
                        std::string descStr = j["description"].get<std::string>();
                        info.description = wxString::FromUTF8(descStr.c_str());
                    }
                    else {
                        info.description = "Circuit diagram";
                    }
                }
                catch (...) {
                    // 忽略解析错误
                }
            }
        }
    }

    return info;
}

// 其他工具函数
wxString GetAppPath() {
    return wxGetCwd();
}

bool FileExists(const wxString& filename) {
    return wxFileExists(filename);
}

bool CreateBackup(const wxString& filename) {
    if (!FileExists(filename)) {
        return false;
    }

    wxString backupName = filename + ".backup";
    return wxCopyFile(filename, backupName, true);
}

std::vector<wxString> GetRecentFiles() {
    std::vector<wxString> recentFiles;
    return recentFiles;
}

void AddToRecentFiles(const wxString& filename) {
    // 实现最近文件列表的持久化存储
}

bool ExportToSVG(const wxString& filename, const std::vector<Gate>& gates, const std::vector<Wire>& wires) {
    wxLogMessage("SVG导出功能待实现");
    return false;
}

bool ExportToPNG(const wxString& filename, const std::vector<Gate>& gates, const std::vector<Wire>& wires) {
    wxLogMessage("PNG导出功能待实现");
    return false;
}

std::vector<wxString> GetSupportedExportFormats() {
    return { "JSON (*.json)", "SVG (*.svg)", "PNG (*.png)" };
}