#ifndef _APP_H_
#define _APP_H_

#include<wx/wxprec.h>

#ifndef WX_PRECOMP
#include<wx/wx.h>
#endif

#include <MainFrame.h>

class App : public wxApp
{
public:
	virtual bool OnInit();
};


#endif // _APP_H_
