#include "componentlibrary.h"
#include <wx/mstream.h>
#include <memory>  // 添加 memory 头文件

// 移除有问题的空数组定义
// static const unsigned char gate_icon_data[] = {};

ComponentLibrary::ComponentLibrary() {
    Initialize();
}

ComponentLibrary::~ComponentLibrary() {
    for (auto& pair : componentMap) {
        delete pair.second;
    }
}

void ComponentLibrary::Initialize() {
    // 注册 Wiring 类别元件
    RegisterComponent(ComponentType::WIRING_SPLITTER, "Splitter", "Wiring", "信号分离器", 1, 8);
    RegisterComponent(ComponentType::WIRING_PIN, "Pin", "Wiring", "引脚", 1, 1);
    RegisterComponent(ComponentType::WIRING_PROBE, "Probe", "Wiring", "探针", 1, 0);
    RegisterComponent(ComponentType::WIRING_TUNNEL, "Tunnel", "Wiring", "隧道", 1, 1);
    RegisterComponent(ComponentType::WIRING_PULL_RESISTOR, "Pull Resistor", "Wiring", "上拉电阻", 1, 1);
    RegisterComponent(ComponentType::WIRING_CLOCK, "Clock", "Wiring", "时钟信号", 0, 1);
    RegisterComponent(ComponentType::WIRING_CONSTANT, "Constant", "Wiring", "常量", 0, 1);

    // 注册 Gates 类别元件
    RegisterComponent(ComponentType::GATE_NOT, "NOT Gate", "Gates", "非门", 1, 1);
    RegisterComponent(ComponentType::GATE_BUFFER, "Buffer", "Gates", "缓冲器", 1, 1);
    RegisterComponent(ComponentType::GATE_AND, "AND Gate", "Gates", "与门", 2, 1);
    RegisterComponent(ComponentType::GATE_OR, "OR Gate", "Gates", "或门", 2, 1);
    RegisterComponent(ComponentType::GATE_NAND, "NAND Gate", "Gates", "与非门", 2, 1);
    RegisterComponent(ComponentType::GATE_NOR, "NOR Gate", "Gates", "或非门", 2, 1);
    RegisterComponent(ComponentType::GATE_XOR, "XOR Gate", "Gates", "异或门", 2, 1);
    RegisterComponent(ComponentType::GATE_XNOR, "XNOR Gate", "Gates", "异或非门", 2, 1);

    // 注册 Arithmetic 类别元件
    RegisterComponent(ComponentType::ARITHMETIC_ADDER, "Adder", "Arithmetic", "加法器", 3, 2);
    RegisterComponent(ComponentType::ARITHMETIC_SUBTRACTOR, "Subtractor", "Arithmetic", "减法器", 3, 2);
    RegisterComponent(ComponentType::ARITHMETIC_MULTIPLIER, "Multiplier", "Arithmetic", "乘法器", 2, 2);
    RegisterComponent(ComponentType::ARITHMETIC_DIVIDER, "Divider", "Arithmetic", "除法器", 2, 2);

    // 注册 Memory 类别元件
    RegisterComponent(ComponentType::MEMORY_D_FLIPFLOP, "D Flip-Flop", "Memory", "D触发器", 2, 2);
    RegisterComponent(ComponentType::MEMORY_SR_FLIPFLOP, "SR Flip-Flop", "Memory", "SR触发器", 2, 2);
    RegisterComponent(ComponentType::MEMORY_JK_FLIPFLOP, "JK Flip-Flop", "Memory", "JK触发器", 2, 2);
    RegisterComponent(ComponentType::MEMORY_T_FLIPFLOP, "T Flip-Flop", "Memory", "T触发器", 1, 2);
    RegisterComponent(ComponentType::MEMORY_REGISTER, "Register", "Memory", "寄存器", 4, 4);

    // 注册 Input/Output 类别元件
    RegisterComponent(ComponentType::IO_LED, "LED", "Input/Output", "LED显示", 1, 0);
    RegisterComponent(ComponentType::IO_BUTTON, "Button", "Input/Output", "按钮", 0, 1);
}

void ComponentLibrary::RegisterComponent(ComponentType type, const wxString& name,
    const wxString& category, const wxString& description,
    int inputCount, int outputCount) {
    ComponentInfo* info = new ComponentInfo();
    info->type = type;
    info->name = name;
    info->category = category;
    info->description = description;
    info->inputCount = inputCount;
    info->outputCount = outputCount;

    // 创建简单图标（16x16 的简单位图）
    info->icon = wxBitmap(16, 16);

    componentMap[type] = info;
    nameToTypeMap[name] = type;
    categoryMap[category].push_back(type);
}

ComponentInfo* ComponentLibrary::GetComponentInfo(ComponentType type) {
    auto it = componentMap.find(type);
    if (it != componentMap.end()) {
        return it->second;
    }
    return nullptr;
}

ComponentInfo* ComponentLibrary::GetComponentInfo(const wxString& name) {
    auto it = nameToTypeMap.find(name);
    if (it != nameToTypeMap.end()) {
        return GetComponentInfo(it->second);
    }
    return nullptr;
}

std::vector<ComponentInfo*> ComponentLibrary::GetComponentsByCategory(const wxString& category) {
    std::vector<ComponentInfo*> components;
    auto it = categoryMap.find(category);
    if (it != categoryMap.end()) {
        for (ComponentType type : it->second) {
            components.push_back(GetComponentInfo(type));
        }
    }
    return components;
}

std::vector<wxString> ComponentLibrary::GetCategories() {
    std::vector<wxString> categories;
    for (const auto& pair : categoryMap) {
        categories.push_back(pair.first);
    }
    return categories;
}

LogicGate* ComponentLibrary::CreateComponent(ComponentType type) {
    ComponentInfo* info = GetComponentInfo(type);
    if (!info) return nullptr;

    // 根据元件类型创建对应的逻辑门实例
    switch (type) {
    case ComponentType::GATE_AND:
        return new AndGate();
    case ComponentType::GATE_OR:
        return new OrGate();
    case ComponentType::GATE_NOT:
        return new NotGate();
    case ComponentType::GATE_XOR:
        return new XorGate();
    case ComponentType::GATE_NAND:
        return new NandGate();
    case ComponentType::GATE_NOR:
        return new NorGate();
    case ComponentType::GATE_XNOR:
        return new XnorGate();
    case ComponentType::GATE_BUFFER:
        return new BufferGate();
    default:
        // 对于其他类型的元件，返回 nullptr 或者创建对应的具体类
        // 不能创建抽象类 LogicGate 的实例
        return nullptr;
    }
}

LogicGate* ComponentLibrary::CreateComponent(const wxString& name) {
    auto it = nameToTypeMap.find(name);
    if (it != nameToTypeMap.end()) {
        return CreateComponent(it->second);
    }
    return nullptr;
}