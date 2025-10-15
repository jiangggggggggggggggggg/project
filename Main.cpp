#include <wx/wx.h>
#include <wx/splitter.h> // ���� wxSplitterWindow ��
#include <wx/treectrl.h> // ���� wxTreeCtrl��wxTreeItemId �༰ wxTR_HAS_BUTTONS ��
#include "menu.h"
#include "canvas.h"
#include "utils.h"

// �Զ���Ӧ���ࣨ�̳�wxApp��
class MyApp : public wxApp {
public:
    virtual bool OnInit() override;
};

// ʵ��Ӧ�ó����ʼ��
bool MyApp::OnInit() {
    // ����������
    wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "Logisim Clone",
        wxDefaultPosition, wxSize(800, 600));

    // �����˵���
    MenuHandler* menuHandler = new MenuHandler(frame);
    frame->SetMenuBar(menuHandler->CreateMenuBar());

    // �󶨲˵��¼�
    menuHandler->BindEvents();

    // �����ָ��
    wxSplitterWindow* splitter = new wxSplitterWindow(frame, wxID_ANY);
    splitter->SetSashGravity(0.3);
    splitter->SetMinimumPaneSize(100);

    // �������οؼ�
    wxTreeCtrl* tree = new wxTreeCtrl(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS);
    tree->SetForegroundColour(*wxBLACK);
    tree->SetBackgroundColour(*wxWHITE);

    // ������νڵ�
    wxTreeItemId root = tree->AddRoot("Components");
    wxTreeItemId main = tree->AppendItem(root, "main");
    wxTreeItemId wiring = tree->AppendItem(root, "Wiring");
    wxTreeItemId gates = tree->AppendItem(root, "Gates");
    wxTreeItemId plexers = tree->AppendItem(root, "Plexers");
    wxTreeItemId arithmetic = tree->AppendItem(root, "Arithmetic");
    wxTreeItemId memory = tree->AppendItem(root, "Memory");
    wxTreeItemId input_output = tree->AppendItem(root, "Input/Output");
    wxTreeItemId base = tree->AppendItem(root, "Base");

    // ��� Wiring ���ӽڵ�
    tree->AppendItem(wiring, "Splitter");
    tree->AppendItem(wiring, "Pin");
    tree->AppendItem(wiring, "Probe");
    tree->AppendItem(wiring, "Tunnel");
    tree->AppendItem(wiring, "Pull Resistor");
    tree->AppendItem(wiring, "Clock");
    tree->AppendItem(wiring, "Constant");

    // ��� Gates ���ӽڵ�
    tree->AppendItem(gates, "NOT Gate ����");
    tree->AppendItem(gates, "Buffer ������");
    tree->AppendItem(gates, "AND Gate ����");
    tree->AppendItem(gates, "OR Gate ����");
    tree->AppendItem(gates, "NAND Gate �����");
    tree->AppendItem(gates, "NOR Gate �����");
    tree->AppendItem(gates, "XOR Gate �����");
    tree->AppendItem(gates, "XNOR Gate ������");

    // ���Arithmetic�ڵ���ӽڵ�
    tree->AppendItem(arithmetic, "Adder");
    tree->AppendItem(arithmetic, "Subtractor");
    tree->AppendItem(arithmetic, "Multiplier");
    tree->AppendItem(arithmetic, "Divider");

    // ���Memory�ڵ���ӽڵ�
    tree->AppendItem(memory, "D Flip-Flop D������");
    tree->AppendItem(memory, "SR Flip-Flop SR������");
    tree->AppendItem(memory, "JK Flip-Flop JK������");
    tree->AppendItem(memory, "T Flip-Flop T������");
    tree->AppendItem(memory, "Register �Ĵ���");

    // ���Input/Output�ڵ���ӽڵ�
    tree->AppendItem(input_output, "LED");
    tree->AppendItem(input_output, "Button");

    // չ�����нڵ�
    tree->Expand(root);

    // ��������
    Canvas* canvas = new Canvas(splitter);

    // �ָ��
    splitter->SplitVertically(tree, canvas);

    // ʹ��sizer����
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(splitter, 1, wxEXPAND | wxALL, 0);
    frame->SetSizer(sizer);

    // ��ʾ����
    frame->Show(true);
    return true;
}

// ����Ӧ�����
wxIMPLEMENT_APP(MyApp);
