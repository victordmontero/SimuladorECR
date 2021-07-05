#ifndef DEFS_H_
#define DEFS_H_

#include<map>
#include<wx/wxchar.h>

enum FuncIds {
	FUNCID_SEND_ENQ,
	FUNCID_SEND_SYN,
	FUNCID_SEND_ACK,
	FUNCID_RECV_ACK,
	FUNCID_RECV_EOM,
	FUNCID_RECV_ENQ,
	FUNCID_RECV_EOT,
	FUNCID_SEND_DATA,
	FUNCID_RECV_DATA
};

typedef int (*FuncPtr)(std::map<wxString,void*>&);

#endif // !DEFS_H_
