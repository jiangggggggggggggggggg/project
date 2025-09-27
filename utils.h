#ifndef UTILS_H
#define UTILS_H

#include <wx/wx.h>
#include <string>

// �ļ�������
class FileUtils {
public:
    // ���ص�·�ļ�
    static bool LoadCircuit(const wxString& filePath);

    // �����·�ļ�
    static bool SaveCircuit(const wxString& filePath);

    // ��ȡ�ļ���չ��
    static wxString GetFileExtension(const wxString& filePath);

    // ��ʾ�ļ�������Ϣ
    static void ShowFileError(const wxString& message);
};

// ��·���湤����
class SimulationUtils {
public:
    // ������·����
    static void StartSimulation();

    // ֹͣ��·����
    static void StopSimulation();

    // ����ִ�з���
    static void StepSimulation();

    // ����·�Ƿ��д���
    static bool CheckCircuitForErrors(wxString& errorMessage);
};

// ���깤����
class CoordinateUtils {
public:
    // ���������ת��Ϊ��������
    static wxPoint RelativeToAbsolute(wxPoint relativePos, wxPoint origin);

    // ����������ת��Ϊ�������
    static wxPoint AbsoluteToRelative(wxPoint absolutePos, wxPoint origin);

    // �������굽����
    static wxPoint SnapToGrid(wxPoint pos, int gridSize = 10);
};

#endif // UTILS_H
#pragma once
