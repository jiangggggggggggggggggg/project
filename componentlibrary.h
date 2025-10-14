#ifndef COMPONENTLIBRARY_H
#define COMPONENTLIBRARY_H

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <map>
#include <vector>
#include "logicgates.h"

// 元件类型枚举
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

// 元件信息结构
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

    // 初始化元件库
    void Initialize();

    // 获取元件信息
    ComponentInfo* GetComponentInfo(ComponentType type);
    ComponentInfo* GetComponentInfo(const wxString& name);

    // 按类别获取元件
    std::vector<ComponentInfo*> GetComponentsByCategory(const wxString& category);

    // 获取所有类别
    std::vector<wxString> GetCategories();

    // 创建元件实例
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