#include "menu.h"
#include "utils.h"
#include <wx/toolbar.h>
#include <wx/artprov.h>
#include <fstream>

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(wxID_NEW, MyFrame::OnNew)
EVT_MENU(wxID_OPEN, MyFrame::OnOpen)
EVT_MENU(wxID_SAVE, MyFrame::OnSave)
EVT_MENU(wxID_SAVEAS, MyFrame::OnSaveAs)
EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
EVT_MENU(wxID_UNDO, MyFrame::OnUndo)
EVT_MENU(wxID_REDO, MyFrame::OnRedo)
EVT_MENU(wxID_COPY, MyFrame::OnCopy)
EVT_MENU(wxID_PASTE, MyFrame::OnPaste)
EVT_MENU(wxID_ZOOM_IN, MyFrame::OnZoomin)
EVT_MENU(wxID_ZOOM_OUT, MyFrame::OnZoomout)
EVT_MENU(ID_SHOW_STATUSBAR, MyFrame::OnToggleStatusBar)
EVT_MENU(ID_SHOW_GRID, MyFrame::OnToggleGrid)
EVT_MENU(ID_DELETE_SELECTED, MyFrame::OnDeleteSelected)
EVT_MENU(ID_DELETE_WIRE, MyFrame::OnDeleteWire)
EVT_MENU(ID_EDIT_PROPERTIES, MyFrame::OnEditProperties)
EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
EVT_COMMAND(wxID_ANY, MY_CUSTOM_EVENT, MyFrame::OnCustomEvent)
wxEND_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 650))
{
    // �˵���
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_NEW, "&New\tCtrl-N");
    menuFile->Append(wxID_OPEN, "&Open...\tCtrl-O");
    menuFile->Append(wxID_SAVE, "&Save\tCtrl-S");
    menuFile->Append(wxID_SAVEAS, "Save &As...");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, "E&xit\tAlt-F4");

    wxMenu* menuEdit = new wxMenu;
    menuEdit->Append(wxID_UNDO, "&Undo\tCtrl-Z");
    menuEdit->Append(wxID_REDO, "&Redo\tCtrl-Y");
    menuEdit->AppendSeparator();
    menuEdit->Append(wxID_COPY, "&Copy\tCtrl-C");
    menuEdit->Append(wxID_PASTE, "&Paste\tCtrl-V");
    menuEdit->AppendSeparator();
    menuEdit->Append(ID_EDIT_PROPERTIES, "�༭����\tCtrl-P");
    menuEdit->Append(ID_DELETE_SELECTED, "ɾ�����\tDel");
    menuEdit->Append(ID_DELETE_WIRE, "ɾ������\tShift-Del");

    wxMenu* menuView = new wxMenu;
    menuView->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl-+");
    menuView->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl--");
    menuView->AppendCheckItem(ID_SHOW_STATUSBAR, "Show Status Bar")->Check(true);
    menuView->AppendCheckItem(ID_SHOW_GRID, "Show &Grid\tCtrl-G")->Check(true);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "&About\tF1");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuEdit, "&Edit");
    menuBar->Append(menuView, "&View");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("׼������ - ��������: ���Ա�(Ctrl+P), ����ɾ��(�Ҽ�/Shift-Del), �������(Ctrl+G), ���ѡ��(�Ҽ�)");

    // ������
    wxToolBar* toolBar = CreateToolBar();
    toolBar->AddTool(wxID_NEW, "�½�", wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR));
    toolBar->AddTool(wxID_OPEN, "��", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR));
    toolBar->AddTool(wxID_SAVE, "����", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_UNDO, "����", wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR));
    toolBar->AddTool(wxID_REDO, "����", wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(wxID_ZOOM_IN, "�Ŵ�", wxArtProvider::GetBitmap(wxART_PLUS, wxART_TOOLBAR));
    toolBar->AddTool(wxID_ZOOM_OUT, "��С", wxArtProvider::GetBitmap(wxART_MINUS, wxART_TOOLBAR));
    toolBar->AddSeparator();
    toolBar->AddTool(ID_DELETE_SELECTED, "ɾ�����", wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR));
    toolBar->AddTool(ID_DELETE_WIRE, "ɾ������", wxArtProvider::GetBitmap(wxART_CUT, wxART_TOOLBAR));
    toolBar->Realize();

    // �ָ��
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_splitter->SetMinimumPaneSize(100);

    // ������οؼ�
    m_treeCtrl = new wxTreeCtrl(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
    wxTreeItemId root = m_treeCtrl->AddRoot("�����");
    m_treeCtrl->AppendItem(root, "AND");
    m_treeCtrl->AppendItem(root, "OR");
    m_treeCtrl->AppendItem(root, "NOT");
    m_treeCtrl->AppendItem(root, "XOR");
    m_treeCtrl->AppendItem(root, "NAND");
    m_treeCtrl->AppendItem(root, "NOR");
    m_treeCtrl->AppendItem(root, "XNOR");
    m_treeCtrl->AppendItem(root, "BUFFER");
    m_treeCtrl->AppendItem(root, "LED");
    m_treeCtrl->AppendItem(root, "����");
    m_treeCtrl->Expand(root);

    // �Ҳ��ͼ���
    m_drawPanel = new MyDrawPanel(m_splitter);

    m_splitter->SplitVertically(m_treeCtrl, m_drawPanel, 200);

    // �����οؼ��¼�
    m_treeCtrl->Bind(wxEVT_TREE_ITEM_ACTIVATED, [this](wxTreeEvent& evt) {
        wxString itemText = m_treeCtrl->GetItemText(evt.GetItem());
        if (itemText != "�����") {
            // ����״̬���ڳ���
            SaveStateForUndo();
            m_drawPanel->AddShape(itemText);
        }
        });

    UpdateTitle();
}

void MyFrame::SaveStateForUndo() {
    // ���浱ǰ״̬������ջ
    undoStack.push(m_drawPanel->GetState());
    // �������ջ
    while (!redoStack.empty()) redoStack.pop();

    // ���²˵�״̬
    wxMenuBar* mb = GetMenuBar();
    if (mb) {
        mb->Enable(wxID_UNDO, !undoStack.empty());
        mb->Enable(wxID_REDO, !redoStack.empty());
    }
}

void MyFrame::OnCustomEvent(wxCommandEvent& event) {
    // �����Զ����¼�������״̬���ڳ���
    SaveStateForUndo();
}

void MyFrame::OnNew(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->ClearShapes();
    m_currentFile.clear();
    UpdateTitle();
}

void MyFrame::OnOpen(wxCommandEvent& event) {
    SaveStateForUndo();

    wxFileDialog openFileDialog(this, "�򿪵�·ͼ", "", "",
        "Circuit files (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = openFileDialog.GetPath();
    std::ifstream f(filename.ToStdString());
    if (!f.is_open()) {
        wxLogError("�޷����ļ�: %s", filename);
        return;
    }

    try {
        json j;
        f >> j;

        std::vector<Gate> gates;
        for (auto& gateJson : j["gates"]) {
            Gate gate;
            gate.type = gateJson["type"].get<std::string>();
            gate.pos.x = gateJson["x"];
            gate.pos.y = gateJson["y"];

            // ��������
            if (gateJson.find("properties") != gateJson.end()) {
                for (auto& propJson : gateJson["properties"]) {
                    Property prop;
                    prop.name = propJson["name"].get<std::string>();
                    prop.value = propJson["value"].get<std::string>();
                    prop.type = propJson["type"].get<std::string>();
                    gate.properties.push_back(prop);
                }
            }

            gates.push_back(gate);
        }

        m_drawPanel->SetGates(gates);
        m_currentFile = filename;
        UpdateTitle();
    }
    catch (const std::exception& e) {
        wxLogError("�����ļ�ʧ��: %s", e.what());
    }
}

void MyFrame::OnSave(wxCommandEvent& event) {
    if (m_currentFile.empty()) {
        OnSaveAs(event);
    }
    else {
        DoSave(m_currentFile);
    }
}

void MyFrame::OnSaveAs(wxCommandEvent& event) {
    wxFileDialog saveFileDialog(this, "�����·ͼ", "", "",
        "Circuit files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

    wxString filename = saveFileDialog.GetPath();
    if (!filename.Lower().EndsWith(".json")) {
        filename += ".json";
    }

    DoSave(filename);
}

void MyFrame::DoSave(const wxString& filename) {
    json j;
    j["version"] = "1.0";
    j["type"] = "circuit";

    std::vector<json> gatesJson;
    for (auto& gate : m_drawPanel->GetGates()) {
        json gateJson;
        gateJson["type"] = gate.type.ToStdString();
        gateJson["x"] = gate.pos.x;
        gateJson["y"] = gate.pos.y;

        // ��������
        std::vector<json> propsJson;
        for (auto& prop : gate.properties) {
            json propJson;
            propJson["name"] = prop.name.ToStdString();
            propJson["value"] = prop.value.ToStdString();
            propJson["type"] = prop.type.ToStdString();
            propsJson.push_back(propJson);
        }
        gateJson["properties"] = propsJson;

        gatesJson.push_back(gateJson);
    }
    j["gates"] = gatesJson;

    std::ofstream f(filename.ToStdString());
    if (!f.is_open()) {
        wxLogError("�޷������ļ�: %s", filename);
        return;
    }

    f << j.dump(4);
    m_currentFile = filename;
    UpdateTitle();
}

void MyFrame::OnQuit(wxCommandEvent& event) {
    Close(true);
}

void MyFrame::OnUndo(wxCommandEvent& event) {
    if (!undoStack.empty()) {
        // ���浱ǰ״̬������ջ
        redoStack.push(m_drawPanel->GetState());
        // �ָ���һ��״̬
        m_drawPanel->SetState(undoStack.top());
        undoStack.pop();

        // ���²˵�״̬
        wxMenuBar* mb = GetMenuBar();
        if (mb) {
            mb->Enable(wxID_UNDO, !undoStack.empty());
            mb->Enable(wxID_REDO, !redoStack.empty());
        }

        SetStatusText("�����ɹ�");
    }
    else {
        SetStatusText("û�пɳ����Ĳ���");
    }
}

void MyFrame::OnRedo(wxCommandEvent& event) {
    if (!redoStack.empty()) {
        // ���浱ǰ״̬������ջ
        undoStack.push(m_drawPanel->GetState());
        // �ָ�����״̬
        m_drawPanel->SetState(redoStack.top());
        redoStack.pop();

        // ���²˵�״̬
        wxMenuBar* mb = GetMenuBar();
        if (mb) {
            mb->Enable(wxID_UNDO, !undoStack.empty());
            mb->Enable(wxID_REDO, !redoStack.empty());
        }

        SetStatusText("�����ɹ�");
    }
    else {
        SetStatusText("û�п������Ĳ���");
    }
}

void MyFrame::OnCopy(wxCommandEvent& event) {
    // ��ʵ��
    SetStatusText("���ƹ��ܴ�ʵ��");
}

void MyFrame::OnPaste(wxCommandEvent& event) {
    // ��ʵ��
    SetStatusText("ճ�����ܴ�ʵ��");
}

void MyFrame::OnZoomin(wxCommandEvent& event) {
    m_drawPanel->ZoomIn();
    SetStatusText("�Ŵ���ͼ");
}

void MyFrame::OnZoomout(wxCommandEvent& event) {
    m_drawPanel->ZoomOut();
    SetStatusText("��С��ͼ");
}

void MyFrame::OnToggleStatusBar(wxCommandEvent& event) {
    wxMenuBar* mb = GetMenuBar();
    wxMenuItem* item = mb->FindItem(ID_SHOW_STATUSBAR);
    bool show = item->IsChecked();
    GetStatusBar()->Show(show);
    Layout();

    if (show) {
        SetStatusText("״̬������ʾ");
    }
    else {
        SetStatusText("״̬��������");
    }
}

void MyFrame::OnToggleGrid(wxCommandEvent& event) {
    wxMenuBar* mb = GetMenuBar();
    wxMenuItem* item = mb->FindItem(ID_SHOW_GRID);
    bool show = item->IsChecked();
    m_drawPanel->ToggleGrid();
    item->Check(m_drawPanel->IsGridVisible());

    if (m_drawPanel->IsGridVisible()) {
        SetStatusText("��������ʾ");
    }
    else {
        SetStatusText("����������");
    }
}

void MyFrame::OnDeleteSelected(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->DeleteSelected();
    SetStatusText("ɾ��ѡ�����");
}

void MyFrame::OnDeleteWire(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->DeleteSelectedWire();
    SetStatusText("ɾ��ѡ������");
}

void MyFrame::OnEditProperties(wxCommandEvent& event) {
    SaveStateForUndo();
    m_drawPanel->EditSelectedProperties();
    SetStatusText("�༭�������");
}

void MyFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("��·ͼ�༭��\n֧��������ơ����ߡ����Ա༭��������롢����ɾ���ȹ���\n\n"
        "ʹ��˵��:\n"
        "- ˫����������ӵ�����\n"
        "- �����ק�ƶ����\n"
        "- ����������ק����\n"
        "- �Ҽ���������ʾ�˵�\n"
        "- �Ҽ��������ɾ������\n"
        "- Ctrl+Z ����\n"
        "- Ctrl+Y ����\n"
        "- Ctrl+P �༭����\n"
        "- Del ɾ��ѡ�����\n"
        "- Shift+Del ɾ��ѡ������\n"
        "- Ctrl+G ��ʾ/��������", "����", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::UpdateTitle() {
    wxString title = "��·ͼ�༭��";
    if (!m_currentFile.empty()) {
        title += " - " + m_currentFile;
    }
    SetTitle(title);
}