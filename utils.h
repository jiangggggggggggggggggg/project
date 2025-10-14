#ifndef UTILS_H
#define UTILS_H

#include <wx/wx.h>
#include <vector>

class FileUtils {
public:
    static bool SaveToFile(const wxString& filename, const wxString& content);
    static wxString LoadFromFile(const wxString& filename);
    static bool FileExists(const wxString& filename);
};

class DrawingUtils {
public:
    static void DrawGateShape(wxDC& dc, const wxPoint& position, const wxSize& size, const wxString& label);
    static void DrawWire(wxDC& dc, const wxPoint& start, const wxPoint& end);
    static void DrawPort(wxDC& dc, const wxPoint& position, bool isInput);
};

class CircuitUtils {
public:
    static bool ValidateCircuit();
    static void SimulateCircuit();
    static void OptimizeCircuit();
};

#endif