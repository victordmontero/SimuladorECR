#include<MainFrame.h>

#include<vector>
#include<wx/valnum.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <vector>

#define EOT 0x04
#define ACK 0x06
#define FS 0x1C
#define ENQ 0x05
#define SYN 0x16
#define EOM 0x19

static void hexDump(const char* desc, const void* addr, const int len, std::ostream& logStream);
static void parseMsg(const char* message, unsigned length, std::vector<char*>& vec);


MainFrame::MainFrame(const wxString& title, const wxPoint& point, const wxSize& size)
	:wxFrame(NULL, wxID_ANY, title, point, size)
{
	this->btnSend = NULL;
	this->txtIP = NULL;
	this->txtPort = NULL;
	this->txtFolio = NULL;
	this->txtResult = NULL;
	this->toolBar = NULL;
	this->server = NULL;

	wxMenuBar* menuBar = new wxMenuBar;
	toolBar = this->CreateToolBar(wxTB_HORIZONTAL, wxID_ANY);

	wxMenu* menuFile = new wxMenu;
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Globo ECR Simulator");

	wxIntegerValidator<short> validator;

	wxIPV4address address;
	wxString ipv4Address = address.IPAddress();

	wxPanel* panel = new wxPanel(this);

	txtIP = new wxTextCtrl(panel, ID_IPTXT, ipv4Address, wxPoint(10, 20), wxSize(100, 20));
	txtPort = new wxTextCtrl(panel, ID_PORTTXT, "2018", wxPoint(110, 20), wxSize(100, 20), 0L, validator);
	txtFolio = new wxTextCtrl(panel, ID_FOLIO_NO, "00000001", wxPoint(210, 20), wxSize(100, 20));
	txtResult = new wxTextCtrl(panel, wxID_ANY, "Results", wxPoint(10, 50), wxSize(600, 300), wxTE_MULTILINE | wxTE_READONLY | wxTE_PROCESS_TAB);

	wxBoxSizer* bSizer2 = new wxBoxSizer(wxVERTICAL);
	bSizer2->Add(txtResult, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* bSizer1 = new wxBoxSizer(wxHORIZONTAL);
	bSizer1->Add(txtIP, 1, wxALL, 5);
	bSizer1->Add(txtPort, 1, wxALL, 5);
	bSizer1->Add(txtFolio, 1, wxALL, 5);

	wxBoxSizer* cSizer = new wxBoxSizer(wxVERTICAL);
	cSizer->Add(bSizer1, 0, wxEXPAND);
	cSizer->Add(bSizer2, 1, wxEXPAND);

	panel->SetSizerAndFit(cSizer);

	wxButton* btnStartServer = new wxButton(toolBar, ID_START_SERVER, "Escuchar", wxDefaultPosition, wxSize(100, 20));
	wxButton* btnStopServer = new wxButton(toolBar, ID_STOP_SERVER, "Detener", wxDefaultPosition, wxSize(100, 20));
	wxButton* btnSendPreComp = new wxButton(toolBar, ID_SEND_PRECOMP, "Send PreComp", wxDefaultPosition, wxSize(100, 20));

	toolBar->AddControl(btnStartServer);
	toolBar->AddControl(btnStopServer);
	toolBar->AddControl(btnSendPreComp);

	Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_BUTTON, &MainFrame::OnStartServer, this, ID_START_SERVER);
	Bind(wxEVT_BUTTON, &MainFrame::OnStopServer, this, ID_STOP_SERVER);
	Bind(wxEVT_BUTTON, &MainFrame::OnSendPreComp, this, ID_SEND_PRECOMP);
	Bind(wxEVT_SOCKET, &MainFrame::OnClientConnected, this, ID_CONNECTED);
	Bind(wxEVT_SOCKET, &MainFrame::OnServerEvent, this, ID_SERVER);

	toolBar->Realize();
}

MainFrame::~MainFrame()
{
	if (server != NULL && server->IsOk())
		server->Destroy();
	server = NULL;
}

void MainFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
	event.Skip();
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox(
		"Simulador Globo",
		"About SimuladorECR",
		wxOK | wxICON_INFORMATION);

	event.Skip();
}


void MainFrame::OnStartServer(wxCommandEvent& event)
{
	//txtResult->Clear();
	std::ostream logStream(txtResult);
	wxString posIp = txtIP->GetValue();
	unsigned long port = 0;

	txtPort->GetValue().ToULong(&port);

	wxIPV4address address;
	address.Service(port);

	if (server != NULL && server->IsOk())
	{
		server->Destroy();
		server = NULL;
	}

	server = new wxSocketServer(address);

	if (!server->IsOk())
	{
		server->Destroy();
		if (server->Error())
		{
			wxString msg;
			msg.Printf("Socket Error: %s", SocketErrorString(server->LastError()));
			SetStatusText(msg);
		}
		return;
	}

	server->SetEventHandler(*this, ID_SERVER);
	server->SetNotify(wxSOCKET_CONNECTION_FLAG);
	server->Notify(true);

	SetStatusText(wxT("Listening at port " + txtPort->GetValue()));
	event.Skip();
}

void MainFrame::OnStopServer(wxCommandEvent& event)
{
	if (server != NULL && server->IsOk())
		server->Destroy();
	server = NULL;

	SetStatusText(wxT("Server Stopped"));
}

void MainFrame::OnServerEvent(wxSocketEvent& evt)
{
	std::vector<char*> vec;
	std::vector<wxString> vec2;
	std::ostream logStream(txtResult);
	wxString dashesString(10, '-');
	wxString endTraceText = "EndTrace";
	wxIPV4address clientAddr;
	char buf[2] = { 0 };
	char messageBuf[257] = { 0 };

	wxSocketBase* sock = server->Accept(false);

	sock->GetLocal(clientAddr);
	logStream << "Connected Peer: [" << clientAddr.IPAddress() << "]" << '\n';

	sock->Read(buf, 1);
	hexDump("Received", buf, 1, logStream);

	buf[0] = EOM;
	sock->Write(buf, 1);
	logStream << "Sending EOM" << '\n';

	//Reading message
	sock->Read(messageBuf, sizeof(messageBuf));
	parseMsg(messageBuf, sizeof(messageBuf), vec);
	hexDump("Message", messageBuf, sizeof(messageBuf), logStream);
	for (std::vector<char*>::iterator it = vec.begin(); it != vec.end(); it++)
	{
		logStream << "[" << *it << "]" << '\n';
	}

	buf[0] = ACK;
	sock->Write(buf, 1);
	logStream << "Sending ACK" << '\n';

	buf[0] = 0x30;
	buf[1] = 0x30;
	sock->Write(buf, 2);
	logStream << "Sending {'0''0'} " << '\n';

	//Reading ACK
	buf[0] = 0x00;
	buf[1] = 0x00;
	sock->Read(buf, 1);
	hexDump("Received", buf, 1, logStream);

	buf[0] = EOT;
	sock->Write(buf, 1);
	logStream << "Sending EOT" << '\n';

	//Reading ACK
	buf[0] = 0x00;
	buf[1] = 0x00;
	sock->Read(buf, 1);
	hexDump("Received", buf, 1, logStream);

	sock->Destroy();
	sock = NULL;

	server->Destroy();
	server = NULL;

	wxString trxType = vec[vec.size() - 2];

	if (trxType.CompareTo(wxT("CT01")) == 0)
	{
		logStream << "Cancelling\n";
		endTraceText.Prepend(dashesString);
		endTraceText.Append(dashesString);
		logStream << '\n' << endTraceText;
		evt.Skip();
		return;
	}
	else if (trxType.CompareTo(wxT("VC01")) == 0)
	{
		if (std::strcmp(vec[3], "02") == 0)
		{
			endTraceText.Prepend(dashesString);
			endTraceText.Append(dashesString);
			logStream << '\n' << endTraceText;
			evt.Skip();
			return;
		}

		wxIPV4address address;
		address.Hostname(txtIP->GetValue());
		address.Service(7060);
		wxSocketClient* client = new wxSocketClient();

		if (!client->Connect(address, true))
		{
			wxString err = SocketErrorString(client->LastError());
			wxMessageBox("Connection Failed: " + err, "Connection failed", 5L, this);
			SetStatusText("Could nost Connect");
			client->Destroy();
			return;
		}

		buf[0] = ENQ;
		buf[1] = 0x00;
		client->Write(buf, 1);

		//Reading ACK
		buf[0] = buf[1] = 0x00;
		client->Read(buf, 1);

		client->Destroy();

		if (!client->Connect(address, true))
		{
			if (client->Error())
			{
				wxString err = SocketErrorString(client->LastError());
				wxMessageBox("Connection Failed: " + err, "Connection failed", 5L, this);
			}
			client->Destroy();
			return;
		}

		buf[0] = SYN;
		buf[1] = 0x00;
		client->Write(buf, 1);

		client->Destroy();

		unsigned long port = 0;
		txtPort->GetValue().ToULong(&port);

		wxIPV4address otherAddress;
		otherAddress.AnyAddress();
		otherAddress.Service(port);
		wxSocketServer* newServer = new wxSocketServer(otherAddress);

		if (!newServer->IsOk())
			return;

		wxSocketBase* incommingClient = newServer->Accept();

		buf[0] = 0x00;
		incommingClient->Read(buf, 1);
		logStream << "Receiving ENQ" << '\n';
		hexDump("Received", buf, 1, logStream);

		memset(messageBuf, 0x00, sizeof(messageBuf));

		vec2.push_back("CN00");
		vec2.push_back("000000050500");
		vec2.push_back("000000000500");
		vec2.push_back("000000000500");
		vec2.push_back("000056");

		int len = 0;
		for (std::vector<wxString>::iterator it = vec2.begin(); it != vec2.end(); it++)
		{
			memcpy(&messageBuf[len], (*it).c_str(), (*it).Length());
			len += (*it).Length();
			messageBuf[len] = FS;
			len++;
		}

		hexDump("Sending MessageBuf", messageBuf, len - 1, logStream);
		incommingClient->Write(messageBuf, len - 1);

		//Receiving ACK
		buf[0] = buf[1] = 0x00;
		incommingClient->Read(buf, 1);
		hexDump("Received ACK", buf, 1, logStream);

		memset(messageBuf, 0x00, sizeof(messageBuf));
		incommingClient->Read(messageBuf, sizeof(messageBuf));
		hexDump("Received POS Response", messageBuf, sizeof(messageBuf), logStream);

		buf[0] = ACK;
		buf[1] = 0x00;
		incommingClient->Write(buf, 1);

		//Receving EOT
		buf[0] = buf[1] = 0x00;
		incommingClient->Read(buf, 1);
		logStream << "Received: " << buf << '\n';
		hexDump("Received EOT", buf, 1, logStream);

		//Receiving EOM
		buf[0] = buf[1] = 0x00;
		incommingClient->Read(buf, 1);
		logStream << "Received: " << buf << '\n';
		hexDump("Received EOM", buf, 1, logStream);

		incommingClient->Destroy();

	}
	else if (trxType.CompareTo(wxT("VC02")) == 0)
	{
		wxIPV4address address;
		address.Hostname(txtIP->GetValue());
		address.Service(7060);
		wxSocketClient* client = new wxSocketClient();

		if (!client->Connect(address, true))
		{
			wxString err = SocketErrorString(client->LastError());
			wxMessageBox("Connection Failed: " + err, "Connection failed", 5L, this);
			SetStatusText("Could nost Connect");
			client->Destroy();
			return;
		}

		buf[0] = ENQ;
		buf[1] = 0x00;
		client->Write(buf, 1);

		//Reading ACK
		buf[0] = buf[1] = 0x00;
		client->Read(buf, 1);

		client->Close();
		client->Destroy();

		if (!client->Connect(address, true))
		{
			if (client->Error())
			{
				wxString err = SocketErrorString(client->LastError());
				wxMessageBox("Connection Failed: " + err, "Connection failed", 5L, this);
			}
			client->Destroy();
			return;
		}

		buf[0] = SYN;
		buf[1] = 0x00;
		client->Write(buf, 1);

		client->Close();
		client->Destroy();
		client = NULL;

		unsigned long port = 0;
		txtPort->GetValue().ToULong(&port);

		wxIPV4address otherAddress;
		otherAddress.AnyAddress();
		otherAddress.Service(port);
		wxSocketServer* newServer = new wxSocketServer(otherAddress);

		if (!newServer->IsOk())
			return;

		wxSocketBase* incommingClient = newServer->Accept();

		buf[0] = 0x00;
		incommingClient->Read(buf, 1);
		logStream << "Receiving ENQ" << '\n';
		hexDump("Received", buf, 1, logStream);

		memset(messageBuf, 0x00, sizeof(messageBuf));

		vec2.push_back("PR00");
		vec2.push_back(txtFolio->GetValue());
		vec2.push_back("000000000500");
		vec2.push_back("000000000090");
		vec2.push_back("000000000000");
		vec2.push_back("000056");

		int len = 0;
		for (std::vector<wxString>::iterator it = vec2.begin(); it != vec2.end(); it++)
		{
			memcpy(&messageBuf[len], (*it).c_str(), (*it).Length());
			len += (*it).Length();
			messageBuf[len] = FS;
			len++;
		}

		hexDump("Sending MessageBuf", messageBuf, len - 1, logStream);
		incommingClient->Write(messageBuf, len - 1);

		//Receiving ACK
		buf[0] = buf[1] = 0x00;
		incommingClient->Read(buf, 1);
		hexDump("Received ACK", buf, 1, logStream);

		memset(messageBuf, 0x00, sizeof(messageBuf));
		incommingClient->Read(messageBuf, sizeof(messageBuf));
		hexDump("Received POS Response", messageBuf, sizeof(messageBuf), logStream);

		buf[0] = ACK;
		buf[1] = 0x00;
		incommingClient->Write(buf, 1);

		//Receving EOT
		buf[0] = buf[1] = 0x00;
		incommingClient->Read(buf, 1);
		logStream << "Received: " << buf << '\n';
		hexDump("Received EOT", buf, 1, logStream);

		//Receiving EOM
		buf[0] = buf[1] = 0x00;
		incommingClient->Read(buf, 1);
		logStream << "Received: " << buf << '\n';
		hexDump("Received EOM", buf, 1, logStream);

		incommingClient->Close();
		incommingClient->Destroy();

	}
	else if (std::strcmp(vec[vec.size() - 2], "VC03") == 0)
	{
		endTraceText.Prepend(dashesString);
		endTraceText.Append(dashesString);
		logStream << '\n' << endTraceText;
		evt.Skip();
		return;
	}


	endTraceText.Prepend(dashesString);
	endTraceText.Append(dashesString);
	logStream << '\n' << endTraceText;

}

void MainFrame::OnClientConnected(wxSocketEvent& evt)
{
	char buf[2] = { 0 };
	wxIPV4address clientAddr;
	wxSocketBase* clientSock = evt.GetSocket();

	clientSock->GetLocal(clientAddr);
	txtResult->WriteText(clientAddr.IPAddress());

	switch (evt.GetSocketEvent())
	{
	case wxSOCKET_INPUT:
		clientSock->Read(buf, 1);
		txtResult->WriteText(wxString(buf));
		clientSock->Close();
		clientSock->Destroy();
		SetStatusText("Stopped Listening");
		break;
	case wxSOCKET_LOST:
		SetStatusText("Stopped Listening");
		clientSock->Close();
		clientSock->Destroy();
		break;
	default:
		break;
	}

}

wxString MainFrame::SocketErrorString(wxSocketError err)
{
	switch (err)
	{
	case wxSOCKET_INVOP:
		return wxT("Invalid operation.");
	case wxSOCKET_IOERR:
		return wxT("Input/Output error.");
	case wxSOCKET_INVADDR:
		return wxT("Invalid address passed to wxSocket");
	case wxSOCKET_INVSOCK:
		return wxT("Invalid socket (uninitialized).");
	case wxSOCKET_NOHOST:
		return wxT("No corresponding host.");
	case wxSOCKET_INVPORT:
		return wxT("Invalid port.");
	case wxSOCKET_WOULDBLOCK:
		return wxT("The socket is non-blocking and the operation would block.");
	case wxSOCKET_TIMEDOUT:
		return wxT("The timeout for this operation expired.");
	case wxSOCKET_MEMERR:
		return wxT("Memory exhausted.");
	case wxSOCKET_NOERROR:
		return wxT("No error happened.");
	default:
		return wxT("Error not listed");
	}
}

void MainFrame::OnSendPreComp(wxCommandEvent& event)
{
	std::vector<char*> vec;
	std::vector<wxString> vec2;
	std::ostream logStream(txtResult);
	wxString dashesString(10, '-');
	wxString endTraceText = "EndTrace";
	wxIPV4address clientAddr;
	char buf[2] = { 0 };
	char messageBuf[257] = { 0 };

	wxIPV4address address;
	address.Hostname(txtIP->GetValue());
	address.Service(7060);
	wxSocketClient* client = new wxSocketClient();

	txtResult->Clear();
	logStream << "Connecting to [" << txtIP->GetValue().ToStdString() << ":7060]......\n";
	if (!client->Connect(address, true))
	{
		wxString err = SocketErrorString(client->LastError());
		wxMessageBox("Connection Failed: " + err, "Connection failed", 5L, this);
		SetStatusText("Could nost Connect");
		client->Close();
		client->Destroy();
		return;
	}
	logStream << "Connected\n";

	logStream << "Sending ENQ\n";
	buf[0] = ENQ;
	buf[1] = 0x00;
	client->Write(buf, 1);

	logStream << "Reading ACK\n";
	//Reading ACK
	buf[0] = buf[1] = 0x00;
	client->Read(buf, 1);

	client->Close();
	client->Destroy();

	logStream << "Connecting to [" << txtIP->GetValue().ToStdString() << ":7060]......\n";
	if (!client->Connect(address, true))
	{
		if (client->Error())
		{
			wxString err = SocketErrorString(client->LastError());
			wxMessageBox("Connection Failed: " + err, "Connection failed", 5L, this);
		}
		client->Close();
		client->Destroy();
		return;
	}
	logStream << "Connected\n";

	logStream << "Sending SYN\n";
	buf[0] = SYN;
	buf[1] = 0x00;
	client->Write(buf, 1);

	client->Close();
	client->Destroy();
	client = NULL;

	unsigned long port = 0;
	txtPort->GetValue().ToULong(&port);

	wxIPV4address otherAddress;
	otherAddress.AnyAddress();
	otherAddress.Service(port);
	wxSocketServer* newServer = new wxSocketServer(otherAddress);

	if (!newServer->IsOk())
		return;
	logStream << "Converting to Server......\n";
	wxSocketBase* incommingClient = newServer->Accept();

	buf[0] = 0x00;
	incommingClient->Read(buf, 1);
	logStream << "Receiving ENQ" << '\n';
	hexDump("Received", buf, 1, logStream);

	memset(messageBuf, 0x00, sizeof(messageBuf));

	vec2.clear();
	vec2.push_back("CN00");
	//vec2.push_back(txtFolio->GetValue());
	vec2.push_back("000000000110");
	vec2.push_back("000000000000");
	vec2.push_back("000000000000");
	vec2.push_back("123456");

	int len = 0;
	for (std::vector<wxString>::iterator it = vec2.begin(); it != vec2.end(); it++)
	{
		memcpy(&messageBuf[len], (*it).c_str(), (*it).Length());
		len += (*it).Length();
		messageBuf[len] = FS;
		len++;
	}

	hexDump("Sending MessageBuf", messageBuf, len - 1, logStream);
	incommingClient->Write(messageBuf, len - 1);

	//Receiving ACK
	buf[0] = buf[1] = 0x00;
	incommingClient->Read(buf, 1);
	hexDump("Received ACK", buf, 1, logStream);

	memset(messageBuf, 0x00, sizeof(messageBuf));
	incommingClient->Read(messageBuf, sizeof(messageBuf));
	hexDump("Received POS Response", messageBuf, sizeof(messageBuf), logStream);

	buf[0] = ACK;
	buf[1] = 0x00;
	incommingClient->Write(buf, 1);

	//Receving EOT
	buf[0] = buf[1] = 0x00;
	incommingClient->Read(buf, 1);
	logStream << "Received: " << buf << '\n';
	hexDump("Received EOT", buf, 1, logStream);

	//Receiving EOM
	buf[0] = buf[1] = 0x00;
	incommingClient->Read(buf, 1);
	logStream << "Received: " << buf << '\n';
	hexDump("Received EOM", buf, 1, logStream);

	incommingClient->Close();
	incommingClient->Destroy();

	newServer->Close();
	newServer->Destroy();
}

static void parseMsg(const char* message, unsigned length, std::vector<char*>& vec)
{
	std::string temp(message);

	char delim[2] = { FS,'\0' };

	char* tok = std::strtok((char*)message, delim);
	while (tok != NULL)
	{
		vec.push_back(tok);
		tok = std::strtok(NULL, delim);
	}
}

void hexDump(const char* desc, const void* addr, const int len, std::ostream& logStream)
{
	int i;
	unsigned char buff[17] = { 0 };
	const unsigned char* pc = static_cast<const unsigned char*>(addr);

	char temp[256] = { 0 };

	// Output description if given.
	if (desc != NULL)
		logStream << desc << ":\n";

	if (len == 0)
	{
		logStream << "  ZERO LENGTH\n";
		return;
	}
	if (len < 0)
	{
		logStream << "  NEGATIVE LENGTH: " << len << '\n';
		return;
	}

	// Process every byte in the data.
	for (i = 0; i < len; i++)
	{
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0)
		{
			// Just don't print ASCII for the zeroth line.
			if (i != 0) {
				wxSprintf(temp, "  %s\n", buff);
				logStream << temp;
			}

			// Output the offset.

			wxSprintf(temp, "  %04x ", i);
			logStream << temp;
		}

		// Now the hex code for the specific character.
		wxSprintf(temp," %02x", pc[i]);
		logStream << temp;

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0)
	{
		logStream << "   ";
		i++;
	}

	// And print the final ASCII bit.
	logStream << "  " << buff << '\n';
}