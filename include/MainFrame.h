#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

enum EventsCode
{
    HELLO_ID,
    EXIT_ID,
    ABOUT_ID
};

class MainFrame : public wxFrame
{
private:
	void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    //wxDECLARE_EVENT_TABLE();

public:
	MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
};

#endif //_MAIN_H_
