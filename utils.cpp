#include <wx/file.h>  // 加入wxFile的声明头文件
#include "utils.h"
#include "canvas.h"
#include "logicgates.h"

// FileUtils 实现
bool FileUtils::LoadCircuit(const wxString& filePath) {
    wxFile file;  // 先构造空的wxFile对象
    if (!file.Open(filePath, wxFile::read)) {  // 调用Open时，传入文件路径和读取模式
        ShowFileError("Could not open file for reading: " + filePath);
        return false;
    }

    // 读取文件内容并解析...

    file.Close();
    return true;
}
bool FileUtils::SaveCircuit(const wxString& filePath) {
    // 实现保存电路文件的逻辑

    wxFile file; // 先构造空的wxFile对象
    if (!file.Open(filePath, wxFile::write)) { // 调用Open时，同时传文件路径和模式
        ShowFileError("Could not open file for writing: " + filePath);
        return false;
    }

    // 写入电路数据...

    file.Close();
    return true;
}

wxString FileUtils::GetFileExtension(const wxString& filePath) {
    int dotPos = filePath.Find('.', true);
    if (dotPos != wxNOT_FOUND && dotPos < filePath.Length() - 1) {
        return filePath.Right(filePath.Length() - dotPos - 1).Lower();
    }
    return "";
}

void FileUtils::ShowFileError(const wxString& message) {
    wxMessageBox(message, "File Error", wxOK | wxICON_ERROR);
}

// SimulationUtils 实现
void SimulationUtils::StartSimulation() {
    // 启动仿真逻辑
}

void SimulationUtils::StopSimulation() {
    // 停止仿真逻辑
}

void SimulationUtils::StepSimulation() {
    // 单步仿真逻辑
}

bool SimulationUtils::CheckCircuitForErrors(wxString& errorMessage) {
    // 检查电路错误
    // 实际实现需要遍历所有元件和连线检查

    // 示例：总是返回没有错误
    errorMessage = "";
    return true;
}

// CoordinateUtils 实现
wxPoint CoordinateUtils::RelativeToAbsolute(wxPoint relativePos, wxPoint origin) {
    return wxPoint(origin.x + relativePos.x, origin.y + relativePos.y);
}

wxPoint CoordinateUtils::AbsoluteToRelative(wxPoint absolutePos, wxPoint origin) {
    return wxPoint(absolutePos.x - origin.x, absolutePos.y - origin.y);
}

wxPoint CoordinateUtils::SnapToGrid(wxPoint pos, int gridSize) {
    if (gridSize <= 0) return pos;

    return wxPoint(
        (pos.x / gridSize) * gridSize,
        (pos.y / gridSize) * gridSize
    );
}
