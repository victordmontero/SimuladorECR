#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/socket.h>

enum EventsIds
{
	ID_START_SERVER,
	ID_STOP_SERVER,
	ID_IPTXT,
	ID_PORTTXT,
	ID_EVENT_COMPLETED,
	ID_SERVER,
	ID_CONNECTED
};

class MainFrame : public wxFrame
{
private:
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnStartServer(wxCommandEvent& event);
	void OnStopServer(wxCommandEvent& event);

	void OnServerEvent(wxSocketEvent& evt);
	void OnClientConnected(wxSocketEvent& evt);

	wxString SocketErrorString(wxSocketError err);

	static MainFrame* ref;

public:
	MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	~MainFrame();

	wxButton* btnSend = NULL;
	wxTextEntry* txtIP = NULL;
	wxTextEntry* txtPort = NULL;
	wxTextCtrl* txtResult = NULL;
	wxToolBar* toolBar = NULL;

	wxSocketServer* server = NULL;
};

#endif //_MAIN_H_
