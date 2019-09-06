#include<MainFrame.h>

MainFrame::MainFrame(const wxString& title, const wxPoint& point, const wxSize& size)
    :wxFrame(NULL,wxID_ANY,title,point,size)
{
    wxMenuBar* menuBar = new wxMenuBar;

    wxMenu* menuFile = new wxMenu;
    menuFile->Append(HELLO_ID,"Hello","Greeting...");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    menuBar->Append(menuFile,"&File");
    menuBar->Append(menuHelp,"&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Globo ECR Simulator");

    Bind(wxEVT_MENU,&MainFrame::OnHello,this,HELLO_ID);
    Bind(wxEVT_MENU,&MainFrame::OnAbout,this,wxID_ABOUT);
    Bind(wxEVT_MENU,&MainFrame::OnExit,this,wxID_EXIT);
}

void MainFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello, It is me you're looking for?");
}

void MainFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
     wxMessageBox("This is a wxWidgets Hello World example",
                 "About Hello World", wxOK | wxICON_INFORMATION);
}
