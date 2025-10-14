#ifndef COMPONENTLIBRARY_H
#define COMPONENTLIBRARY_H

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <map>
#include <vector>
#include "logicgates.h"

// Ԫ������ö��
enum class ComponentType {
    WIRING_SPLITTER,
    WIRING_PIN,
    WIRING_PROBE,
    WIRING_TUNNEL,
    WIRING_PULL_RESISTOR,
    WIRING_CLOCK,
    WIRING_CONSTANT,

    GATE_NOT,
    GATE_BUFFER,
    GATE_AND,
    GATE_OR,
    GATE_NAND,
    GATE_NOR,
    GATE_XOR,
    GATE_XNOR,

    ARITHMETIC_ADDER,
    ARITHMETIC_SUBTRACTOR,
    ARITHMETIC_MULTIPLIER,
    ARITHMETIC_DIVIDER,

    MEMORY_D_FLIPFLOP,
    MEMORY_SR_FLIPFLOP,
    MEMORY_JK_FLIPFLOP,
    MEMORY_T_FLIPFLOP,
    MEMORY_REGISTER,

    IO_LED,
    IO_BUTTON
};

// Ԫ����Ϣ�ṹ
struct ComponentInfo {
    ComponentType type;
    wxString name;
    wxString category;
    wxString description;
    wxBitmap icon;
    int inputCount;
    int outputCount;
};

class ComponentLibrary {
public:
    ComponentLibrary();
    ~ComponentLibrary();

    // ��ʼ��Ԫ����
    void Initialize();

    // ��ȡԪ����Ϣ
    ComponentInfo* GetComponentInfo(ComponentType type);
    ComponentInfo* GetComponentInfo(const wxString& name);

    // ������ȡԪ��
    std::vector<ComponentInfo*> GetComponentsByCategory(const wxString& category);

    // ��ȡ�������
    std::vector<wxString> GetCategories();

    // ����Ԫ��ʵ��
    LogicGate* CreateComponent(ComponentType type);
    LogicGate* CreateComponent(const wxString& name);

private:
    void RegisterComponent(ComponentType type, const wxString& name,
        const wxString& category, const wxString& description,
        int inputCount, int outputCount);

    std::map<ComponentType, ComponentInfo*> componentMap;
    std::map<wxString, ComponentType> nameToTypeMap;
    std::map<wxString, std::vector<ComponentType>> categoryMap;
};

#endif // COMPONENTLIBRARY_H