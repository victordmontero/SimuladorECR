#include<MainFrame.h>

#include<vector>
#include<wx/valnum.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include <winsock.h>
#include <vector>

#define EOT 0x04
#define ACK 0x06
#define FS 0x1C
#define ENQ 0x05
#define SYN 0x16
#define EOM 0x19

static int dieWithError(const char* message, SOCKET sock, std::ostream&);
static void hexDump(const char* desc, const void* addr, const int len, std::ostream& logStream);
static int reconnect(SOCKET* sock, const std::string&, std::ostream&);
static SOCKET doHanshake(SOCKET* sock, const std::string&, std::ostream&);
static void closeHanshake(SOCKET new_sock, std::ostream&);

SOCKET SetAsServer(SOCKET* sock, std::ostream&);

MainFrame::MainFrame(const wxString& title, const wxPoint& point, const wxSize& size)
	:wxFrame(NULL, wxID_ANY, title, point, size)
{
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

	txtIP = new wxTextCtrl(this, ID_IPTXT, "192.168.3.68", wxPoint(10, 20), wxSize(100, 20));
	txtPort = new wxTextCtrl(this, ID_PORTTXT, "2018", wxPoint(110, 20), wxSize(100, 20), 0L, validator);
	txtResult = new wxTextCtrl(this, wxID_ANY, "Results", wxPoint(10, 50), wxSize(600, 300), wxTE_MULTILINE | wxTE_READONLY);

	wxButton* btnStartServer = new wxButton(toolBar, ID_START_SERVER, "Escuchar", wxDefaultPosition, wxSize(100, 20));
	wxButton* btnStopServer = new wxButton(toolBar, ID_STOP_SERVER, "Detener", wxDefaultPosition, wxSize(100, 20));
	toolBar->AddControl(btnStartServer);
	toolBar->AddControl(btnStopServer);

	Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_BUTTON, &MainFrame::OnStartServer, this, ID_START_SERVER);
	Bind(wxEVT_BUTTON, &MainFrame::OnStopServer, this, ID_STOP_SERVER);
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
	wxMessageBox("Simulador Globo",
		"About SimuladorECR", wxOK | wxICON_INFORMATION);

	event.Skip();
}


void MainFrame::OnStartServer(wxCommandEvent& event)
{
	txtResult->Clear();
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
		return;

	server->SetEventHandler(*this, ID_SERVER);
	server->SetNotify(wxSOCKET_CONNECTION_FLAG);
	server->Notify(true);

	SetStatusText("Listening at " + txtPort->GetValue());
	event.Skip();
}

void MainFrame::OnStopServer(wxCommandEvent& event)
{
	if (server != NULL && server->IsOk())
		server->Destroy();
	server = NULL;
}

void MainFrame::OnServerEvent(wxSocketEvent& evt)
{
	std::ostream logStream(txtResult);
	wxIPV4address clientAddr;
	char buf[2] = { 0 };
	char messageBuf[256] = { 0 };
	wxSocketBase* sock = server->Accept(false);

	/*sock->SetEventHandler(*this, ID_CONNECTED);
	sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
	sock->Notify(true);*/

	sock->GetLocal(clientAddr);
	logStream << "Connected Peer: " << clientAddr.IPAddress() << '\n';

	sock->Read(buf, 1);
	logStream << "Received " << wxString(buf).Printf("%x", buf[0]) << '\n';

	buf[0] = EOM;
	sock->Write(buf, 1);
	logStream << "Sending EOM" << '\n';

	sock->Read(messageBuf, sizeof(messageBuf));
	hexDump("Message", messageBuf, sizeof(messageBuf), logStream);

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
	logStream << "Received " << wxString(buf).Printf("%X", buf[0]) << '\n';

	buf[0] = EOT;
	sock->Write(buf, 1);
	logStream << "Sending EOT" << '\n';

	//Reading ACK
	buf[0] = 0x00;
	buf[1] = 0x00;
	sock->Read(buf, 1);
	logStream << "Received " << wxString(buf).Printf("%X", buf[0]) << '\n';
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
		clientSock->Destroy();
		SetStatusText("Stopped Listening");
		break;
	case wxSOCKET_LOST:
		SetStatusText("Stopped Listening");
		clientSock->Destroy();
		break;
	default:
		break;
	}

}

SOCKET doHanshake(SOCKET* sock, const std::string& posIp, std::ostream& logout)
{
	int rc = reconnect((SOCKET*)sock, posIp, logout);

	if (rc < 0)
		return rc;

	char request = 0;
	int responseSize = 0;
	char response[512] = { 0 };

	logout << "Sending ENQ" << std::endl;

	request = ENQ;
	responseSize = send(*sock, &request, 1, 0);

	if (responseSize <= 0)
		return dieWithError("Error Sending ENQ", *sock, logout);


	logout << "Receiving ACK" << std::endl;

	responseSize = recv(*sock, (char*)response, sizeof(response), 0);

	if (responseSize <= 0)
		return dieWithError("Error sending ACK", *sock, logout);

	//hexDump("Received ACK", response, responseSize, logStream);

	rc = reconnect(sock, posIp, logout);

	if (rc < 0)
		return rc;

	logout << "Sending SYN" << std::endl;

	request = SYN;
	responseSize = send(*sock, &request, 1, 0);

	if (responseSize <= 0)
		return dieWithError("Error Sending SYN", *sock, logout);

	int new_sock = SetAsServer(sock, logout);

	logout << "Receiving ENQ" << std::endl;

	responseSize = recv(new_sock, (char*)response, sizeof(response), 0);

	if (responseSize <= 0)
		return dieWithError("Error sending ENQ", new_sock, logout);

	//hexDump("Received ENQ", response, responseSize, logStream);

	return new_sock;
}

void closeHanshake(SOCKET new_sock, std::ostream& logout)
{
	char response[257] = { 0 };
	char request = 0;
	int responseSize = 0;

	logout << "Sending ACK" << std::endl;

	request = ACK;
	responseSize = send(new_sock, &request, 1, 0);

	if (responseSize < 0)
		dieWithError("Error sending ACK", new_sock, logout);

	logout << "Receiving EOT" << std::endl;

	responseSize = recv(new_sock, (char*)response, 1, 0);

	if (responseSize <= 0)
		dieWithError("Error sending EOT", new_sock, logout);


	logout << "Receiving EOM" << std::endl;

	responseSize = recv(new_sock, (char*)response, 1, 0);

	if (responseSize <= 0)
		dieWithError("Error sending EOM", new_sock, logout);

	//hexDump("Received EOM", response, responseSize);
}

int reconnect(SOCKET* sock, const std::string& posIp, std::ostream& logout)
{

	if (*sock >= 0)
	{
		closesocket(*sock);
		*sock = -1;
	}

	struct sockaddr_in clientSockAddr;
	const int posPort = 7060;
	int responseSize = 0;
	char request = 0;

	*sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (*sock < 0)
		return dieWithError("Error creating Socket", *sock, logout);

	memset(&clientSockAddr, 0x00, sizeof(clientSockAddr));
	clientSockAddr.sin_family = AF_INET;
	clientSockAddr.sin_addr.s_addr = inet_addr(posIp.c_str());
	clientSockAddr.sin_port = htons(posPort);

	logout << "Connecting to "
		<< inet_ntoa(clientSockAddr.sin_addr) << ":"
		<< ntohs(clientSockAddr.sin_port) << std::endl;

	responseSize = connect(*sock, (struct sockaddr*) & clientSockAddr, sizeof(clientSockAddr));

	if (responseSize < 0)
	{
		return dieWithError("Error connecting", *sock, logout);
	}
}

SOCKET SetAsServer(SOCKET* sock, std::ostream& logout)
{
	sockaddr_in sockAddr, clientSockAddr;
	const int bindPort = 2018;
	int addresslen = sizeof(sockaddr);

	memset(&sockAddr, 0x00, sizeof(sockAddr));
	memset(&clientSockAddr, 0x00, sizeof(clientSockAddr));

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(bindPort);
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	SOCKET new_sock;
	if (sock >= 0)
	{
		logout << "Closing...." << std::endl;
		closesocket(*sock);
		*sock = -1;
	}

	std::cout << "Setting as Server" << std::endl;
	*sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (*sock < 0)
	{
		logout << "Error al Crear el Socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	if (bind(*sock, (struct sockaddr*) & sockAddr, sizeof(sockAddr)) < 0)
	{
		logout << "Error al conectar: " << WSAGetLastError() << std::endl;
		return 1;
	}

	if (listen(*sock, 3) < 0)
	{
		logout << "Error al escuchar " << WSAGetLastError() << std::endl;
		return 1;
	}

	std::cout << "Acepting...." << std::endl;
	if ((new_sock = accept(*sock, (sockaddr*)& clientSockAddr, &addresslen)) < 0)
	{
		logout << "Error al aceptar " << WSAGetLastError() << std::endl;
		if (WSAGetLastError() != WSAEWOULDBLOCK)
			return 1;
	}

	return new_sock;
}

int dieWithError(const char* message, SOCKET sock, std::ostream& logout)
{
	logout << message << ":" << WSAGetLastError() << std::endl;
	WSACleanup();
	closesocket(sock);
	sock = -1;
	return sock;
}

void hexDump(const char* desc, const void* addr, const int len, std::ostream& logStream)
{
	int i;
	unsigned char buff[17];
	const unsigned char* pc = (const unsigned char*)addr;

	char temp[4] = { 0 };

	// Output description if given.
	if (desc != NULL)
		//printf("%s:\n", desc);
		logStream << desc << ":\n";

	if (len == 0)
	{
		printf("  ZERO LENGTH\n");
		logStream << "  ZERO LENGTH\n";
		return;
	}
	if (len < 0)
	{
		printf("  NEGATIVE LENGTH: %i\n", len);
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
			if (i != 0)
				//printf("  %s\n", buff);
				logStream << "  " << buff << '\n';

			// Output the offset.
			//printf("  %04x ", i);
			logStream << "  ";
			logStream
				<< std::setbase(std::ios_base::hex)
				<< std::setfill('0')
				<< std::setw(4)
				<< i;
			logStream << " ";
		}

		// Now the hex code for the specific character.
		//printf(" %02x", pc[i]);
		sprintf_s(temp, " %02x", pc[i]);
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
		printf("   ");
		logStream << "   ";
		i++;
	}

	// And print the final ASCII bit.
	printf("  %s\n", buff);
	logStream << "  " << buff << '\n';
}