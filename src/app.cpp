#include<app.h>

bool App::OnInit()
{
    MainFrame* frame = new MainFrame("Simulador ECR v0.2",wxPoint(25,25),wxSize(640,480));
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(App);
