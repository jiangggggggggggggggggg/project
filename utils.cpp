#include <wx/file.h>  // ����wxFile������ͷ�ļ�
#include "utils.h"
#include "canvas.h"
#include "logicgates.h"

// FileUtils ʵ��
bool FileUtils::LoadCircuit(const wxString& filePath) {
    wxFile file;  // �ȹ���յ�wxFile����
    if (!file.Open(filePath, wxFile::read)) {  // ����Openʱ�������ļ�·���Ͷ�ȡģʽ
        ShowFileError("Could not open file for reading: " + filePath);
        return false;
    }

    // ��ȡ�ļ����ݲ�����...

    file.Close();
    return true;
}
bool FileUtils::SaveCircuit(const wxString& filePath) {
    // ʵ�ֱ����·�ļ����߼�

    wxFile file; // �ȹ���յ�wxFile����
    if (!file.Open(filePath, wxFile::write)) { // ����Openʱ��ͬʱ���ļ�·����ģʽ
        ShowFileError("Could not open file for writing: " + filePath);
        return false;
    }

    // д���·����...

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

// SimulationUtils ʵ��
void SimulationUtils::StartSimulation() {
    // ���������߼�
}

void SimulationUtils::StopSimulation() {
    // ֹͣ�����߼�
}

void SimulationUtils::StepSimulation() {
    // ���������߼�
}

bool SimulationUtils::CheckCircuitForErrors(wxString& errorMessage) {
    // ����·����
    // ʵ��ʵ����Ҫ��������Ԫ�������߼��

    // ʾ�������Ƿ���û�д���
    errorMessage = "";
    return true;
}

// CoordinateUtils ʵ��
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
