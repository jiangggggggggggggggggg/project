#ifndef UTILS_H
#define UTILS_H

#include <wx/wx.h>
#include <string>

// 文件工具类
class FileUtils {
public:
    // 加载电路文件
    static bool LoadCircuit(const wxString& filePath);

    // 保存电路文件
    static bool SaveCircuit(const wxString& filePath);

    // 获取文件扩展名
    static wxString GetFileExtension(const wxString& filePath);

    // 显示文件错误消息
    static void ShowFileError(const wxString& message);
};

// 电路仿真工具类
class SimulationUtils {
public:
    // 启动电路仿真
    static void StartSimulation();

    // 停止电路仿真
    static void StopSimulation();

    // 单步执行仿真
    static void StepSimulation();

    // 检查电路是否有错误
    static bool CheckCircuitForErrors(wxString& errorMessage);
};

// 坐标工具类
class CoordinateUtils {
public:
    // 将相对坐标转换为绝对坐标
    static wxPoint RelativeToAbsolute(wxPoint relativePos, wxPoint origin);

    // 将绝对坐标转换为相对坐标
    static wxPoint AbsoluteToRelative(wxPoint absolutePos, wxPoint origin);

    // 对齐坐标到网格
    static wxPoint SnapToGrid(wxPoint pos, int gridSize = 10);
};

#endif // UTILS_H
#pragma once
