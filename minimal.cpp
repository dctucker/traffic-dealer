/////////////////////////////////////////////////////////////////////////////
// Name:        minimal.cpp
// Purpose:     Minimal wxWidgets sample
// Author:      D. Casey Tucker
// Created:     2007-04-31
// Copyright:   (c) D. Casey Tucker
// Licence:     Commercial. Only the author reserves the right to charge per license.
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// resources
#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "../sample.xpm"
#endif

#include <ios> 
#include "wx/splitter.h"

#include <wx/file.h>
#include <wx/process.h>
#include <wx/stream.h>
#include <wx/msgdlg.h>
#include <wx/hashmap.h>
#include <wx/list.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/fontdlg.h>
#include <wx/cmndata.h>
#include <wx/progdlg.h>


WX_DECLARE_STRING_HASH_MAP( wxString, StrHash );

#define TIMER_ID 420

//char pass[48] = "èêìÖïêãﬂèêﬂåãöëöﬂîöçèñú”ﬂõöçöâïûëëÜïﬂîûîﬂåãöîìê";
//char pass[50] =   "ØêìÖïêãﬂØ∞ﬂ¨ãöëöﬂ¥öçèñúﬂ““ﬂõöçöâïûëëÜïﬂ¥æ¥ﬂ¨ãöîìê";
char pass[50] = "ØêìÖïêãﬂØ∞ﬂ¨ãöëöﬂ¥öçèñúﬂ““ﬂõöçöâïûëëÜïﬂ¥æ¥ﬂ¨ãöîìê";

class StationData : public wxObject {
public:
	wxString ID, Freq, Name;
	long int Type;
	wxString Host, User, Pass;
	wxString Port;
	wxString Path;
//	long int Announcer;

	//int LoadString(wxString str){
	StationData(wxString str){
		wxStringTokenizer tkz(str, wxT("\x1F"));
		ID		= tkz.GetNextToken();
		Freq	= tkz.GetNextToken();
		Name	= tkz.GetNextToken();
		tkz.GetNextToken().ToLong(&Type);
		Host	= tkz.GetNextToken();
		User	= tkz.GetNextToken();
		Pass	= tkz.GetNextToken();
		Port	= tkz.GetNextToken();
		Path	= tkz.GetNextToken();
//		tkz.GetNextToken().ToLong(&Announcer);
	}
};
WX_DECLARE_STRING_HASH_MAP( StationData *, StationHash );

class LogData : public wxObject {
public:
	//StrHash *Copies;
	wxString LogID, Time, Filename;
	wxString StationID;
	StationData *Station;
	long int SponsorID;
	wxString SponsorName, Address, Phone, Website, CopyID; //, CopyText;
	long int Announcer;
	bool Sent;

	LogData(wxString str, StrHash &Copies, StationHash &Stations){
		wxString CopyText;
		//Copies = c;
		wxStringTokenizer tkz(str, wxT("\x1F"),wxTOKEN_RET_EMPTY);
		LogID		= tkz.GetNextToken();
		tkz.GetNextToken().ToLong(&Announcer);
		Sent		= tkz.GetNextToken() == "1";
		Time		= tkz.GetNextToken();
		StationID   = tkz.GetNextToken();
		Filename	= tkz.GetNextToken();
		tkz.GetNextToken().ToLong(&SponsorID);
		SponsorName	= tkz.GetNextToken();
		Address 	= tkz.GetNextToken();
		Phone   	= tkz.GetNextToken();
		Website  	= tkz.GetNextToken();
		CopyID  	= tkz.GetNextToken();
		CopyText 	= tkz.GetNextToken();

		Copies[CopyID] = CopyText;		//copies are stored alongside log items [for now]

		Station = Stations[StationID];
		//(*s)[*StationID];
//		return 0;
	}
};

// private classes
class MyApp : public wxApp {
public: // override base class virtuals
    virtual bool OnInit();
};



class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);

    // event handlers (these functions should _not_ be virtual)
	void OnClose(wxCloseEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
	void OnFont(wxCommandEvent& event);
	void OnAudio(wxCommandEvent& event);
	void OnUpload(wxCommandEvent& event);
	void OnWave(wxCommandEvent& event);
	void OnAnnouncer(wxCommandEvent& event);
	void OnClockMouse(wxMouseEvent& event);
//	void OnMouse(wxMouseEvent& event);
//	void OnKey(wxKeyEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnProcessTerm(wxProcessEvent& event);

	void GetLog();
	void DoGet();
	void PrevEntry();
	void NextEntry();
	void UpdateEntry();
	void UpdateType(long);
	void LoadLog();
	void DropLog();
	void Decrypt(char *, int);
	void SetBorder(wxColor);
	void InitProcess(wxString);
	void ClockBorder();
	void WriteSentLog();

	StrHash Copies;
	StrHash Announcers;
	StationHash Stations;

private:
	int timediv;
	long int Announcer;
	wxTimer *timer;
	wxList *Log;
	wxNode *Current, *Sending, *tempnode;

	wxChoice		*cAnnouncer, *cDate,				*cCopy;				
	wxTextCtrl												*tFile;
	wxBitmapButton					*bLoad, *bSave, 			*bWave, *bUpload;
	wxStaticText												*tFileTime,	*tClock,

			*tStationID, *tStationFreq, *tStationName,	*tSponsorName,		*tTime;

	wxTextCtrl				*tLeft,								*tRight;

	wxStaticText										*tPhone, *tWebsite,
															*tAddress;
    wxProcess *m_process;
	wxInputStream  *m_in, *m_err;
	wxOutputStream *m_out;
	wxString procout, UploadPercent, strLog;
	wxProgressDialog *pd;


    DECLARE_EVENT_TABLE() // any class wishing to process wxWidgets events must use this macro
};

// constants
enum { // menu items
    Minimal_Quit = wxID_EXIT,
	Minimal_Font = wxID_PROPERTIES,
	Minimal_Audio= wxID_OPEN,
    Minimal_About = wxID_ABOUT,
	B_Upload = wxID_REPLACE,
	B_Wave = wxID_PREVIEW,
	C_Announcer = wxID_PAGE_SETUP,
	T_Clock = wxID_FORWARD
};

// Event tables for MyFrame

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_CLOSE(MyFrame::OnClose)
    EVT_MENU(Minimal_Quit,	MyFrame::OnQuit)
    EVT_MENU(Minimal_About,	MyFrame::OnAbout)
	EVT_MENU(Minimal_Font,  MyFrame::OnFont)
	EVT_MENU(Minimal_Audio, MyFrame::OnAudio)
	EVT_BUTTON(B_Upload,	MyFrame::OnUpload)
	EVT_BUTTON(B_Wave,      MyFrame::OnWave)
	EVT_CHOICE(C_Announcer, MyFrame::OnAnnouncer)
//	EVT_MOUSEWHEEL(MyFrame::OnMouse)
//	EVT_CHAR(MyFrame::OnKey)
	EVT_TIMER(TIMER_ID, MyFrame::OnTimer)
    EVT_END_PROCESS(wxID_ANY, MyFrame::OnProcessTerm)
END_EVENT_TABLE()

class MyTextCtrl : public wxTextCtrl {
private:
	MyFrame *pFrame;
	long sel1, sel2;

public:
	MyTextCtrl(MyFrame *f, wxWindow* parent, wxWindowID id, const wxString& value = "", const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, 
		const wxString& name = wxTextCtrlNameStr) 
		: wxTextCtrl (parent, id, value, pos, size, style, validator, name) {
		pFrame = f;
	}
	void OnMouse(wxMouseEvent& event) {
		int r = event.GetWheelRotation();
		if(r < 0){
			pFrame->NextEntry();
		} else if(r > 0){
			pFrame->PrevEntry();
		}
	}
	void OnKey(wxKeyEvent& event) {
		switch(event.GetKeyCode()){
			case WXK_PAGEUP:
				pFrame->PrevEntry();
				break;
			case WXK_PAGEDOWN:
				pFrame->NextEntry();
				break;
			default:
				event.Skip();
		}
	}
	void OnFocus(wxFocusEvent& WXUNUSED(event)) {
		SetSelection(sel1, sel2);
	}
	void OnUnFocus(wxFocusEvent& WXUNUSED(event)) {
		GetSelection(&sel1, &sel2);
	}
	DECLARE_EVENT_TABLE()
};


//events for MyTextCtrl
BEGIN_EVENT_TABLE(MyTextCtrl, wxTextCtrl)
	EVT_CHAR(MyTextCtrl::OnKey)
	EVT_MOUSEWHEEL(MyTextCtrl::OnMouse)
	EVT_SET_FOCUS(MyTextCtrl::OnFocus)
	EVT_KILL_FOCUS(MyTextCtrl::OnUnFocus)
END_EVENT_TABLE()

IMPLEMENT_APP(MyApp)
bool MyApp::OnInit() { // 'Main program' equivalent: the program execution "starts" here

	MyFrame *frame = new MyFrame(_T("Traffic Dealer client"));
    frame->Show(true);
    
    return true; // success: wxApp::OnRun() will be called. If return false, exit immediately.
}


void MyFrame::UpdateType(long t) {
	wxColour c, f;

	switch(t){
		case 1: // TV
			c = wxColour(192, 192, 0);
			f = wxColour(64, 64, 16);
			break;
		case 2: // Commercial
			c = wxColour(0, 96, 0);
			f = wxColour(64, 255, 64);
			break;
		default: // Other / non-profit
			c = wxColour(16, 16, 96);
			f = wxColour(64, 224, 255);
			break;
	}
	tStationID   -> SetBackgroundColour(c);
	tStationName -> SetBackgroundColour(c);
	tStationFreq -> SetBackgroundColour(c);
	cCopy        -> SetBackgroundColour(c);

	tStationID   -> SetForegroundColour(f);
	tStationName -> SetForegroundColour(f);
	tStationFreq -> SetForegroundColour(f);

}


void MyFrame::ClockBorder(){
	wxDateTime tc = wxDateTime::Now();
	wxDateTime tt; tt.ParseTime(tTime->GetLabel());
	wxDateTime t5; t5.ParseTime(tTime->GetLabel());
	t5.Subtract(wxTimeSpan(0, 5, 0, 0));

	LogData *ld = (LogData *)Current->GetData();

	//if(!m_process) {
	if(ld->Sent){
		SetBorder(wxColour(0, 16, 127)); // has been sent
	} else {
		if(tc.IsLaterThan(tt)){
			SetBorder(wxColour(242, 32, 0)); // hasn't been sent, red
		} else if(tc.IsLaterThan(t5)){
			SetBorder(wxColour(242, 242, 0)); // five minute yellow 
		} else {
			SetBorder(wxColour(32, 242, 0)); // green light
		}
	}
}

void MyFrame::UpdateEntry() {
	LogData *ld = (LogData *)Current->GetData();

	UpdateType( Stations[ ld->StationID ]->Type );


	tRight		->ChangeValue( Copies[ld->CopyID] );
	tSponsorName->SetLabel(	ld->SponsorName );
	tTime		->SetLabel(	ld->Time );
	tAddress	->SetLabel(	ld->Address );
	tPhone		->SetLabel(	ld->Phone );
	tWebsite	->SetLabel(	ld->Website );
	tFile		->SetLabel(	ld->Station->Path + _T("/") + ld->Filename );

	tStationID	->SetLabel(	Stations[ ld->StationID ]->ID );
	tStationName->SetLabel( Stations[ ld->StationID ]->Name );
	tStationFreq->SetLabel( Stations[ ld->StationID ]->Freq );
	ClockBorder();
}

void MyFrame::PrevEntry() {
	LogData *ld;
	tempnode = Current;
	while(tempnode->GetPrevious()) {
		tempnode = tempnode->GetPrevious();
		ld = (LogData *)tempnode->GetData();
		if(ld->Announcer == Announcer) {
			if(Current != tempnode){
				Current = tempnode;
				UpdateEntry();
				return;
			}
		}
	}
}

void MyFrame::NextEntry() {
	LogData *ld;
	tempnode = Current;
	while(tempnode->GetNext()) {
		tempnode = tempnode->GetNext();
		ld = (LogData *)tempnode->GetData();
		if(ld->Announcer == Announcer) {
			if(Current != tempnode){
				Current = tempnode;
				UpdateEntry();
				return;
			}
		}
	}
}


void MyFrame::SetBorder(wxColor c) {
	if(GetBackgroundColour() != c) {
		tClock->SetForegroundColour(c);
		tTime->SetForegroundColour(c);
		SetBackgroundColour(c);
		Refresh();
	}
}

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title) {

	//menus
	wxMenuBar *menuBar = new wxMenuBar(); {
		wxMenu *fileMenu = new wxMenu; //file menu
			fileMenu->Append(Minimal_Font, _T("&Fonts"), _T("Set the program's default font"));
			fileMenu->Append(Minimal_Audio,_T("&Audio editor..."), _T("Choose your audio editor"));
			fileMenu->Append(Minimal_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));
		menuBar->Append(fileMenu, _T("&File"));

		wxMenu *helpMenu = new wxMenu; // help menu
			helpMenu->Append(Minimal_About, _T("&About...\tF1"), _T("Show about dialog"));
		menuBar->Append(helpMenu, _T("&Help"));
	}

	//top panels
	wxPanel		*pTop  = new wxPanel(this); {

		//top left side
		wxPanel		*pTop1 = new wxPanel(pTop); {
			cAnnouncer = new wxChoice(pTop1,C_Announcer);
				cAnnouncer->SetBackgroundColour(wxColor(0,128,128));
				cAnnouncer->SetForegroundColour(wxColor(255,255,255));
				//cAnnouncer->SetFont(wxFont( 14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL ));

			bLoad = new wxBitmapButton(pTop1,1,wxBitmap("Load"));
			bSave = new wxBitmapButton(pTop1,2,wxBitmap("Save"));
			cDate = new wxChoice(pTop1,3);
				cDate ->SetBackgroundColour(wxColor(0,128,128));

			wxBoxSizer 	*sTop1 = new wxBoxSizer(wxHORIZONTAL);
				sTop1->Add(cAnnouncer,1,wxEXPAND);
				sTop1->Add(bLoad,0,wxEXPAND);
				sTop1->Add(bSave,0,wxEXPAND);
				sTop1->Add(cDate,0,wxEXPAND);
			pTop1->SetSizer(sTop1);
		}

		//top right side
		wxPanel		*pTop2 = new wxPanel(pTop); {
			const wxString a[2] = {"1","2"};
			cCopy		= new wxChoice(pTop2, 4, wxDefaultPosition, wxSize(100,22), 2, a, wxRAISED_BORDER);
				cCopy ->SetBackgroundColour(wxColor(32,32,128));
				cCopy ->SetForegroundColour(wxColor(255,255,255));
				//	cCopy->SetWindowStyle(wxRAISED_BORDER);

			tFile		= new wxTextCtrl(		pTop2,5,"",wxDefaultPosition,wxDefaultSize,wxRAISED_BORDER);
				tFile ->SetBackgroundColour(wxColor(120,127,130));
				tFile->SetForegroundColour(wxColor(242,242,242));

			tFileTime	= new wxStaticText(		pTop2,6,"",wxDefaultPosition,wxSize(50,20),wxRAISED_BORDER | wxALIGN_CENTRE | wxST_NO_AUTORESIZE );
				tFileTime ->SetBackgroundColour(wxColor(120,127,130));
				tFileTime->SetForegroundColour(wxColor(242,242,242));

			bWave		= new wxBitmapButton(	pTop2, B_Wave, wxBitmap("Wave"));
			bUpload		= new wxBitmapButton(	pTop2, B_Upload, wxBitmap("Up"));
			tClock		= new wxStaticText(		pTop2, 9, "00:00:00", wxDefaultPosition, wxSize(100,20), wxALIGN_CENTRE | wxST_NO_AUTORESIZE );
				tClock->SetBackgroundColour(wxColor(63,63,63));
				//tClock->SetForegroundColour(wxColor(255,0,0));
				tClock->SetFont(wxFont(14,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD));

			wxBoxSizer	*sTop2 = new wxBoxSizer( wxHORIZONTAL ); {
				sTop2->Add( cCopy,		0, wxEXPAND );
				sTop2->Add( tFile,		1, wxEXPAND );
				sTop2->Add( tFileTime,	0, wxEXPAND );
				sTop2->Add( bWave,		0, wxEXPAND );
				sTop2->Add( bUpload,	0, wxEXPAND );
				sTop2->Add( tClock, 	0, wxEXPAND );
			} pTop2->SetSizer(sTop2);
		}

		wxGridSizer	*sTop  = new wxGridSizer(2); {
			sTop->Add(pTop1,1,wxEXPAND);
			sTop->Add(pTop2,1,wxEXPAND);
		} pTop->SetSizer(sTop);
	}

	//setup splitter
	wxSplitterWindow* pSplit = new wxSplitterWindow( this, 0, wxDefaultPosition, wxDefaultSize, wxSP_NOBORDER | wxSP_NO_XP_THEME);
		pSplit->SetFont(wxFont( 14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL ));

	pSplit->SetMinimumPaneSize(100);

		//left split
		wxPanel		*pLeft = new wxPanel(pSplit,0,0,-1,-1);

			//upper left
			wxPanel *pLeft1= new wxPanel(pLeft,0,0,-1,-1);
			pLeft1->SetBackgroundColour(wxColour(0,0,63));
				tStationName = new wxStaticText(pLeft1, 98, "", wxDefaultPosition, wxDefaultSize,  wxRAISED_BORDER | wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
					tStationName->SetForegroundColour(wxColor(242,242,242));
				tStationID   = new wxStaticText(pLeft1, 99, "", wxDefaultPosition, wxSize(100,24), wxRAISED_BORDER | wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
					tStationID->SetForegroundColour(wxColor(242,242,242));
				tStationFreq = new wxStaticText(pLeft1, 97, "", wxDefaultPosition, wxSize(100,24), wxRAISED_BORDER | wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
					tStationFreq->SetForegroundColour(wxColor(242,242,242));
			wxBoxSizer  *sLeft1= new wxBoxSizer(wxHORIZONTAL);
				sLeft1->Add( tStationName, 4, wxEXPAND);
				sLeft1->Add( tStationFreq, 2, wxEXPAND);
				sLeft1->Add( tStationID,   3, wxEXPAND);
			pLeft1->SetSizer(sLeft1);

			//large traffic display
			tLeft = new MyTextCtrl(this,pLeft, 10,"",wxDefaultPosition,wxDefaultSize, wxTE_MULTILINE | wxTE_CENTRE);
				tLeft->SetBackgroundColour(wxColor(15,15,15));
				tLeft->SetForegroundColour(wxColor(255,127,31));
				tLeft->SetFont(wxFont(16,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL));
				//tLeft->SetTextAlignment(wxLEFT);

			//tLeft->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(MyFrame::OnKey));

		wxBoxSizer	*sLeft = new wxBoxSizer(wxVERTICAL);
			sLeft->Add(pLeft1, 0, wxEXPAND | wxRIGHT, 5);
			sLeft->Add(tLeft,  1, wxEXPAND | wxRIGHT, 5);
		pLeft->SetSizer(sLeft);


		//right split
		wxPanel		*pRight  = new wxPanel(pSplit,0,0,-1,-1); {

			//upper right
			wxPanel	*pRight1 = new wxPanel(pRight,0,0,-1,-1);
				tTime	 = new wxStaticText(pRight1,9,"00:00:00",wxDefaultPosition,wxSize(100,24),wxALIGN_CENTRE | wxST_NO_AUTORESIZE );
					tTime->SetBackgroundColour(wxColor(63,63,63));
					//tTime->SetForegroundColour(wxColor(255,0,0));
					tTime->SetFont(wxFont(14,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD));

				tSponsorName = new wxStaticText(pRight1, 95,"",wxDefaultPosition,wxDefaultSize,wxRAISED_BORDER | wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
					tSponsorName->SetBackgroundColour(wxColour(255,255,255));
			wxBoxSizer  *sRight1 = new wxBoxSizer(wxHORIZONTAL);
				sRight1->Add(tSponsorName,1,wxEXPAND);
				sRight1->Add(tTime,0,wxEXPAND);
			pRight1->SetSizer(sRight1);


			//large copy display
			tRight	 = new MyTextCtrl(this, pRight, 10, "",wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE | wxSIMPLE_BORDER | wxTE_CENTRE);
				tRight->SetBackgroundColour(wxColor(15,15,15));
				tRight->SetForegroundColour(wxColor(224,224,224));
				tRight->SetFont(wxFont(16,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL));


			wxPanel *pRight2 = new wxPanel(pRight,0,0,-1,-1); {
				pRight2->SetBackgroundColour(wxColor(15,15,15));
				
				wxPanel *pRight3 = new wxPanel(pRight2,0,0,-1,-1);{
					tPhone		= new wxStaticText( pRight3, 193,"Phone",   wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER | wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
						tPhone->SetBackgroundColour(wxColor(255,242,128));
					tWebsite	= new wxStaticText( pRight3, 193,"Website", wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER | wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
						tWebsite->SetBackgroundColour(wxColor(255,242,128));

					wxBoxSizer *sRight3 = new wxBoxSizer(wxHORIZONTAL);
						sRight3->Add(tPhone,   2, wxEXPAND | wxLEFT | wxRIGHT, 10);
						sRight3->Add(tWebsite, 3, wxEXPAND | wxLEFT | wxRIGHT, 10);
					pRight3->SetSizer(sRight3);
				}

				tAddress = new wxStaticText(pRight2,192,"Address",wxDefaultPosition,wxDefaultSize, wxRAISED_BORDER | wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
					tAddress->SetBackgroundColour(wxColor(255,242,128));

				wxBoxSizer *sRight2 = new wxBoxSizer(wxVERTICAL);
					sRight2->Add(pRight3,  0, wxEXPAND| wxBOTTOM | wxLEFT | wxRIGHT, 10);
					sRight2->Add(tAddress, 0, wxEXPAND| wxALL, 6);
				pRight2->SetSizer(sRight2);
			}

			wxBoxSizer	*sRight  = new wxBoxSizer(wxVERTICAL);
				//sRight->Add(tAddress,1,wxEXPAND);
				sRight->Add(pRight1, 0, wxEXPAND | wxLEFT, 5);
				sRight->Add(tRight,  1, wxEXPAND | wxLEFT, 5);
				sRight->Add(pRight2, 0, wxEXPAND | wxLEFT, 5);
			pRight->SetSizer(sRight);
		}

	pSplit->SplitVertically(pLeft,pRight);

	//main window

	wxBoxSizer* sMain = new wxBoxSizer(wxVERTICAL);
		sMain->Add(pTop,0,wxEXPAND | wxALL & ~wxBOTTOM,10);
		sMain->Add(pSplit,1,wxEXPAND | (wxALL & ~wxTOP),10);
	SetSizer(sMain);

	SetIcon(wxICON(sample));
	SetMenuBar(menuBar);     // attach this menu bar to the frame
	//SetBackgroundColour(wxColor(255,0,0));
	SetBorder(wxColor(0,192,0));
	SetSize(-1, -1, 800, 480);
	tLeft->SetFocus();

#if wxUSE_STATUSBAR
    CreateStatusBar(5);
    SetStatusText(_T("Ready."));
#endif // wxUSE_STATUSBAR


	timer = new wxTimer(this, TIMER_ID);
	timer->Start(50);
	m_process = (wxProcess *)NULL;

	//Current = NULL;
	Sending = NULL;


/*	wxBitmap bitmap;

	wxSplashScreen* splash = new wxSplashScreen(bitmap,
          wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
          6000, NULL, -1, wxDefaultPosition, wxDefaultSize,
          wxSIMPLE_BORDER|wxSTAY_ON_TOP);
	wxYield();*/
	GetLog();
//	pd->Close();
	LoadLog();
	UpdateEntry();
	//tLeft->PushEventHandler(this);
	//tRight->PushEventHandler(this);

	Show();

	tClock->Connect(wxID_ANY, wxEVT_LEFT_DOWN, wxMouseEventHandler(MyFrame::OnClockMouse), NULL, this);
}


void MyFrame::WriteSentLog() {
	wxString outfile;
	LogData *ld;
	Current = Log->GetFirst();
	while (Current) {
		ld = (LogData *)Current->GetData();
		if(ld->Sent) {
			outfile += ld->LogID + _T("\n");
		}
		Current = Current->GetNext();
	}
	wxDateTime dt = wxDateTime::Now();
	wxString fn = _T("sent/") + dt.Format(_T("%Y-%m-%d_%M%S%l")) + _T(".txt");
	wxFile f(fn, wxFile::write_append);
	f.Write(outfile);
	f.Close();
	::wxExecute(_T("pscp -pw 1n40rc3 ") + fn + _T(" wilmington@srv.inforcemedia.net:") + fn, wxEXEC_SYNC);

}

// event handlers
void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event)) {
	//Write the sendlog
    Close(true); // true is to force the frame to close
}

void MyFrame::OnClose(wxCloseEvent& WXUNUSED(event)) {
	WriteSentLog();

	if(m_process) {
		wxProcess::Kill(m_process->GetPid(), wxSIGKILL, wxKILL_CHILDREN);
	}

	timer->Stop();
	delete timer;
	DropLog();
	this->Destroy();

}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
    wxString msg;
    msg.Printf( _T("Copyright 2007-2008 D. Casey Tucker.\n")
                _T("Welcome to %s"), wxVERSION_STRING);

    wxMessageBox(msg, _T("About Traffic Dealer"), wxOK | wxICON_INFORMATION, this);
}
void MyFrame::OnFont(wxCommandEvent& WXUNUSED(event)){
	wxFont lf = tLeft->GetFont();

	wxFontData cf;

	cf.SetInitialFont(lf);
	cf.EnableEffects(false);
	cf.SetAllowSymbols(false);

	wxFontDialog fd(this, cf);

	if(fd.ShowModal() == wxID_OK) {
		tLeft ->SetFont(fd.GetFontData().GetChosenFont());
		tRight->SetFont(fd.GetFontData().GetChosenFont());
	}
}

void MyFrame::OnAudio(wxCommandEvent& WXUNUSED(event)){
	wxFileDialog fd(this, wxT("Select your audio editor"), "", "", "Application executables|*.exe", wxFD_FILE_MUST_EXIST );
	fd.ShowModal();
}

void MyFrame::OnWave(wxCommandEvent &WXUNUSED(event)){
	wxString fn;
	fn = tFile->GetValue();
#ifdef __WINDOWS__
	fn.Replace("/", "\\");
	::wxExecute(_T("C:\\Program Files\\CoolPro2\\coolpro2.exe \"C:\\Program Files\\Affiliates") + fn + _T("\""));
#else
	::wxExecute(_T("audacity \"~/affiliates/") + fn + _T("\""));
#endif
}

void MyFrame::OnAnnouncer(wxCommandEvent& WXUNUSED(event)) {
	wxString a = ((wxStringClientData *)cAnnouncer->GetClientObject(cAnnouncer->GetSelection()))->GetData();
	a.ToLong(&Announcer);
}
void MyFrame::OnUpload(wxCommandEvent& WXUNUSED(event)){

	if(!m_process) {
		wxString fn;
		LogData *ld = (LogData *)Current->GetData();
		fn = tFile->GetValue();

#if defined __WINDOWS__ 
		fn = _T("C:/Program Files/Affiliates") + fn;
		fn.Replace("/", "\\");
#else
		fn = _T("~/affiliates") + fn;
#endif
		wxString cmd = _T("pscp -P ") + ld->Station->Port + _T(" -pw ") + ld->Station->Pass +
			_T(" \"") + fn + _T("\" ") + ld->Station->User + _T("@") + ld->Station->Host + _T(":");

		//cmd = _T("pscp -P 22822 -pw john316 d:/emailsig.jpg gjtraffic@familyradio.serveftp.net:"); // debug

		UploadPercent = _T("");
//		SetStatusText("Uploading...");
		m_process = wxProcess::Open(cmd);
		m_in  = m_process->GetInputStream();
		m_err = m_process->GetErrorStream();
		m_out = m_process->GetOutputStream();
		m_process->SetNextHandler(this);
		DoGet();
		SetBorder(wxColour(242,200,0));
		Sending = Current;

	} else { 

		wxProcess::Kill(m_process->GetPid(), wxSIGKILL, wxKILL_CHILDREN);
		GetStatusBar()->SetFieldsCount();
//		SetStatusText("Upload cancelled.");
		SetBorder(wxColour(255,0,0));

	}
	timediv=0;
}

void MyFrame::OnProcessTerm(wxProcessEvent& WXUNUSED(event)) {
    DoGet();
	if(UploadPercent.Find("100%") != wxNOT_FOUND){
		if(Current == Sending) SetBorder(wxColour(0,16,127));
		if(Sending != NULL) {
			((LogData *)Sending->GetData())->Sent = true;
		}
	}

    delete m_process;
    m_process = NULL;
	Sending = NULL;
	if(pd != NULL) pd->Close();
	delete pd;
	pd = NULL;
	procout = wxT("");
	m_in = NULL;
	m_err = NULL;
}

void MyFrame::DoGet() {
	timer->Stop();

    while ( m_in->CanRead() ) {
        wxChar buffer[4096];
        buffer[m_in->Read(buffer, WXSIZEOF(buffer) - 1).LastRead()] = _T('\0');

        procout += buffer;
    }
    while ( m_err->CanRead() ) {
        wxChar buffer[4096];
        buffer[m_err->Read(buffer, WXSIZEOF(buffer) - 1).LastRead()] = _T('\0');

        procout += buffer;
    }
	if(procout.Find(_T("? (y/n")) != wxNOT_FOUND) {
		int reply = wxMessageDialog(this, procout, _T("Press yes to continue uploading"), wxYES_NO | wxCANCEL).ShowModal();
		if(reply == wxID_YES){
			m_out->Write("y\n",2);
		} else if(reply == wxID_NO) {
			m_out->Write("n\n",2);
		} else {
			wxProcess::Kill(m_process->GetPid(), wxSIGKILL, wxKILL_CHILDREN);
		}
	} else {
//		if(pd != NULL) { 
//			strLog += procout;
//		} else
		{

			GetStatusBar()->SetFieldsCount(5);
			wxStringTokenizer tkz(procout, _T("\r"), wxTOKEN_RET_EMPTY);
			while(tkz.HasMoreTokens()) {
				wxString s = tkz.GetNextToken();
				wxStringTokenizer tkz2(s, _T("|"), wxTOKEN_RET_EMPTY);
				for(int i=0; i < 5 && tkz2.HasMoreTokens(); i++) {
					wxString t = tkz2.GetNextToken();
					GetStatusBar()->SetStatusText(t, i);
					if(i==4) {
						UploadPercent = t;
					}
					if(pd != NULL){
						//long int up;
						//UploadPercent.ToLong(&up);
						if(i==4) {
							pd->Update(-1, t);
						}
					}
				}
				GetStatusBar()->Refresh();
				GetStatusBar()->Update();
				Refresh();
				Update();
			}
		}
	}
	procout = _T("");

	timer->Start();
}

void MyFrame::OnTimer(wxTimerEvent& WXUNUSED(event)) {
    // do whatever you want to do every second here
	ClockBorder();
	if(m_process != NULL) {
		DoGet();
	}
	wxDateTime now = wxDateTime::Now();
	wxString time = now.Format(wxT("%H:%M:%S"));
	if( tClock->GetLabel().CompareTo(time) != 0 ){
		tClock->SetLabel(time);
	}
	if(m_process) { // if we're sending, flash yellow
		if(timediv % 20 == 0) {
			SetBorder(wxColour(32, 16, 0));
		} else if(timediv % 10 == 4) {
			SetBorder(wxColour(242, 200, 0));
		}
		/*double c = (timediv > 10) ? (20 - timediv) : (timediv);
		SetBorder(wxColour(242 * (c*0.1), 200 * (c*0.1), 0));
		timediv++;*/
		++timediv %= 20;
	}
}

void MyFrame::DropLog(){
	for(StrHash::iterator it = Copies.begin(); it != Copies.end(); ++it) {
		it->second="";
	}
	Copies.clear();

	Log->DeleteContents(true);
	delete Log;

	for(StationHash::iterator it = Stations.begin(); it != Stations.end(); ++it ){
		delete it->second;
	}
	Stations.clear();

}

void MyFrame::GetLog(){
	strLog=wxT("");
	pd = new wxProgressDialog( wxT("Loading log"), wxT("Please wait while the log is loaded"), 0, this, wxPD_APP_MODAL);

	//InitProcess("plink -ssh -pw wendy wilmington@srv.inforcemedia.net \"./getlog 2007-10-03\"");
	wxDateTime dt = wxDateTime::Today();
	wxString fn = _T("logs/") + dt.Format(_T("%Y-%m-%d")) + _T(".tdl");
	InitProcess( _T("pscp -pw 1n40rc3 wilmington@srv.inforcemedia.net:") + fn + _T(" ./logs/") );
	while(m_process != NULL){
		wxYield();
	}
}

void MyFrame::Decrypt(char *buf, int len){
	for(int i=0; i < len; i++){
		buf[i] ^= pass[ (i % 42) + 5 ];
	}
}

void MyFrame::LoadLog(){
	char buf[102400];
	int len;
	wxDateTime dt = wxDateTime::Today();
	wxString fn = _T("logs/") + dt.Format(_T("%Y-%m-%d")) + _T(".tdl");

	if(!wxFileExists(fn)) {
		wxMessageBox(_T("Could not load today's log. Exiting now."), _T("Traffic Dealer"), wxOK | wxICON_ERROR, this);
		exit(1);
	}
	wxFile f(fn);
	len = f.Read(&buf, 102400);
	//wxString &buf =  strLog;
	//len = strLog.Len();
	Decrypt( buf, len );

	wxString part; // what we're dealing with here is a lack of announcer data in our file.
	f.Close();

	cAnnouncer->Clear();

	wxStringTokenizer tkzOuter(buf, wxT("\x1D"));

//	Stations = new StationHash();	

		part = tkzOuter.GetNextToken(); // load announcer data
		wxStringTokenizer tkzA(part, wxT("\x1E"), wxTOKEN_RET_EMPTY);
		while(tkzA.HasMoreTokens()){
			//wxString key, val;
			wxString announcer = tkzA.GetNextToken();
			wxStringTokenizer tkz2(announcer, wxT("\x1F"), wxTOKEN_RET_EMPTY);
			wxString id = tkz2.GetNextToken();
			Announcers[id] = tkz2.GetNextToken();
			cAnnouncer->Append(Announcers[id], new wxStringClientData(id));
		}

		part = tkzOuter.GetNextToken(); // load station data
		wxStringTokenizer tkz2(part, wxT("\x1E"), wxTOKEN_RET_EMPTY);
		while(tkz2.HasMoreTokens()){
			StationData *st = new StationData( tkz2.GetNextToken() ) ;
			Stations[st->ID] = st;
		}

		part = tkzOuter.GetNextToken(); // load log data
		wxStringTokenizer tkz(part, wxT("\x1E"), wxTOKEN_RET_EMPTY);
		Log = new wxList();
		while(tkz.HasMoreTokens()){
			Log->Append(new LogData(tkz.GetNextToken(), Copies, Stations));
		}

		wxArrayString as;
		StrHash::iterator it; // load copies from log
		for( it = Copies.begin(); it != Copies.end(); it++ ){
			wxString key = it->first, value = it->second;
			as.Add(key);
		}
		as.Sort();
		for( unsigned int i=0; i < as.GetCount(); i++){
			(*cCopy).Append( as[i] );
		}

	Current = Log->GetFirst();
}

void MyFrame::InitProcess(wxString cmdline) {
	m_process = wxProcess::Open(cmdline);
	m_in  = m_process->GetInputStream();
	m_err = m_process->GetErrorStream();
	m_out = m_process->GetOutputStream();
	m_process->SetNextHandler(this);
	DoGet();
	SetBorder(wxColour(242,200,0));
//	Sending = Current;
}

void MyFrame::OnClockMouse(wxMouseEvent&  WXUNUSED( event)) {
	LogData *ld;
	tempnode = Log->GetFirst();

	wxDateTime tc = wxDateTime::Now();
	wxDateTime tt;
	//tt.ParseTime(tTime->GetLabel());
	do {
		ld = (LogData *)tempnode->GetData();
		if(ld->Announcer == Announcer) {
			//if(Current != tempnode){
				Current = tempnode;
				UpdateEntry();
				
				tt.ParseTime(tTime->GetLabel());

				if(tt.IsLaterThan(tc)) return;
			//}
		}
		tempnode = tempnode->GetNext();
	} while(tempnode->GetNext());

}