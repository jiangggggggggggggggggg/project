#include "componentlibrary.h"
#include <wx/mstream.h>
#include <memory>  // ��� memory ͷ�ļ�

// �Ƴ�������Ŀ����鶨��
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
    // ע�� Wiring ���Ԫ��
    RegisterComponent(ComponentType::WIRING_SPLITTER, "Splitter", "Wiring", "�źŷ�����", 1, 8);
    RegisterComponent(ComponentType::WIRING_PIN, "Pin", "Wiring", "����", 1, 1);
    RegisterComponent(ComponentType::WIRING_PROBE, "Probe", "Wiring", "̽��", 1, 0);
    RegisterComponent(ComponentType::WIRING_TUNNEL, "Tunnel", "Wiring", "���", 1, 1);
    RegisterComponent(ComponentType::WIRING_PULL_RESISTOR, "Pull Resistor", "Wiring", "��������", 1, 1);
    RegisterComponent(ComponentType::WIRING_CLOCK, "Clock", "Wiring", "ʱ���ź�", 0, 1);
    RegisterComponent(ComponentType::WIRING_CONSTANT, "Constant", "Wiring", "����", 0, 1);

    // ע�� Gates ���Ԫ��
    RegisterComponent(ComponentType::GATE_NOT, "NOT Gate", "Gates", "����", 1, 1);
    RegisterComponent(ComponentType::GATE_BUFFER, "Buffer", "Gates", "������", 1, 1);
    RegisterComponent(ComponentType::GATE_AND, "AND Gate", "Gates", "����", 2, 1);
    RegisterComponent(ComponentType::GATE_OR, "OR Gate", "Gates", "����", 2, 1);
    RegisterComponent(ComponentType::GATE_NAND, "NAND Gate", "Gates", "�����", 2, 1);
    RegisterComponent(ComponentType::GATE_NOR, "NOR Gate", "Gates", "�����", 2, 1);
    RegisterComponent(ComponentType::GATE_XOR, "XOR Gate", "Gates", "�����", 2, 1);
    RegisterComponent(ComponentType::GATE_XNOR, "XNOR Gate", "Gates", "������", 2, 1);

    // ע�� Arithmetic ���Ԫ��
    RegisterComponent(ComponentType::ARITHMETIC_ADDER, "Adder", "Arithmetic", "�ӷ���", 3, 2);
    RegisterComponent(ComponentType::ARITHMETIC_SUBTRACTOR, "Subtractor", "Arithmetic", "������", 3, 2);
    RegisterComponent(ComponentType::ARITHMETIC_MULTIPLIER, "Multiplier", "Arithmetic", "�˷���", 2, 2);
    RegisterComponent(ComponentType::ARITHMETIC_DIVIDER, "Divider", "Arithmetic", "������", 2, 2);

    // ע�� Memory ���Ԫ��
    RegisterComponent(ComponentType::MEMORY_D_FLIPFLOP, "D Flip-Flop", "Memory", "D������", 2, 2);
    RegisterComponent(ComponentType::MEMORY_SR_FLIPFLOP, "SR Flip-Flop", "Memory", "SR������", 2, 2);
    RegisterComponent(ComponentType::MEMORY_JK_FLIPFLOP, "JK Flip-Flop", "Memory", "JK������", 2, 2);
    RegisterComponent(ComponentType::MEMORY_T_FLIPFLOP, "T Flip-Flop", "Memory", "T������", 1, 2);
    RegisterComponent(ComponentType::MEMORY_REGISTER, "Register", "Memory", "�Ĵ���", 4, 4);

    // ע�� Input/Output ���Ԫ��
    RegisterComponent(ComponentType::IO_LED, "LED", "Input/Output", "LED��ʾ", 1, 0);
    RegisterComponent(ComponentType::IO_BUTTON, "Button", "Input/Output", "��ť", 0, 1);
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

    // ������ͼ�꣨16x16 �ļ�λͼ��
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

    // ����Ԫ�����ʹ�����Ӧ���߼���ʵ��
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
        // �����������͵�Ԫ�������� nullptr ���ߴ�����Ӧ�ľ�����
        // ���ܴ��������� LogicGate ��ʵ��
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