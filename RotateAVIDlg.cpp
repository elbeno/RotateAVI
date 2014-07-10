// RotateAVIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RotateAVI.h"
#include "RotateAVIDlg.h"
#include "avi_handler.h"
#include "filebuf.h"
#include "avi_exceptions.h"
#include ".\rotateavidlg.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HANDLE processThread;

class CMultiFileDialog : public CFileDialog
{
    static const size_t BUFFER_SIZE_CHARS = 32 * 1024;

    TCHAR *m_pBuffer;
    TCHAR *m_pOldBuffer;

public:
    CMultiFileDialog(TCHAR *filter)
        : CFileDialog(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT, filter)
    {
        m_pOldBuffer = GetOFN().lpstrFile;

        GetOFN().nMaxFile = BUFFER_SIZE_CHARS;
        m_pBuffer = new TCHAR[BUFFER_SIZE_CHARS];
        memset(m_pBuffer, 0, BUFFER_SIZE_CHARS * sizeof(TCHAR));
        GetOFN().lpstrFile = m_pBuffer;
    }

    ~CMultiFileDialog()
    {
        GetOFN().lpstrFile = m_pOldBuffer;
        delete[] m_pBuffer;
    }
};



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CRotateAVIDlg dialog



CRotateAVIDlg::CRotateAVIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRotateAVIDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRotateAVIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRotateAVIDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio1)
END_MESSAGE_MAP()


// CRotateAVIDlg message handlers

BOOL CRotateAVIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
#ifdef CRIPPLEWARE
	CButton *pButton = (CButton *)GetDlgItem(IDC_RADIO1);
	pButton->SetCheck(0);
	pButton->EnableWindow(0);
	pButton = (CButton *)GetDlgItem(IDC_RADIO2);
	pButton->SetCheck(1);
#else
	CButton *pButton = (CButton *)GetDlgItem(IDC_RADIO1);
	pButton->SetCheck(1);
#endif

	// set output dir to my documents by default
	IShellFolder *pDesktop = 0;
    HRESULT hr = SHGetDesktopFolder(&pDesktop);
	if (SUCCEEDED(hr))
	{
		LPITEMIDLIST pidlDocFiles = 0;
		HRESULT hr = pDesktop->ParseDisplayName(0, 
										0, 
										L"::{450d8fba-ad25-11d0-98a8-0800361b1103}", 
										0, 
										&pidlDocFiles, 
										0);
		if (SUCCEEDED(hr))
		{
			TCHAR dir_buf[MAX_PATH];
			dir_buf[0] = 0;
			SHGetPathFromIDList(pidlDocFiles, dir_buf);
			SetDlgItemText(IDC_EDIT2, dir_buf);
		}
		pDesktop->Release();
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}



void CRotateAVIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRotateAVIDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRotateAVIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

static int sListBoxWidth = 0;

void CRotateAVIDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);
	CListBox *pListBox = static_cast<CListBox *>(GetDlgItem(nIDCtl));
	CString str;
	pListBox->GetText((int)lpDrawItemStruct->itemID, str);
	int len = str.GetLength();
	TCHAR *pathname = new TCHAR[len+1];
	pathname[len] = 0;
	str.CopyChars(pathname, str.GetBuffer(), len);
	CDC dc;
	RECT rcItem = lpDrawItemStruct->rcItem;

	dc.Attach(lpDrawItemStruct->hDC);

	// adjust the text length with ellipses
	PathCompactPath(dc.m_hDC, pathname, sListBoxWidth);

	// Save these value to restore them when done drawing.
	COLORREF crOldTextColor = dc.GetTextColor();
	COLORREF crOldBkColor = dc.GetBkColor();

	// If this item is selected, set the background color 
	// and the text color to appropriate values. Also, erase
	// rect by filling it with the background color.
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
		(lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		dc.FillSolidRect(&rcItem, 
			::GetSysColor(COLOR_HIGHLIGHT));
	}
	else
		dc.FillSolidRect(&rcItem, crOldBkColor);

	// If this item has the focus, draw a red frame around the
	// item's rect.
	if ((lpDrawItemStruct->itemAction | ODA_FOCUS) &&
		(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		CBrush br(RGB(255, 0, 0));
		dc.FrameRect(&rcItem, &br);
	}

	// Draw the text.
	dc.DrawText(
		pathname,
		&rcItem,
		DT_LEFT|DT_SINGLELINE|DT_VCENTER);

	// Reset the background color and the text color back to their
	// original values.
	dc.SetTextColor(crOldTextColor);
	dc.SetBkColor(crOldBkColor);

	dc.Detach();

	delete[] pathname;
}

void CRotateAVIDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);
	CListBox *pListBox = static_cast<CListBox *>(GetDlgItem(nIDCtl));

	RECT r;
	pListBox->GetWindowRect(&r);
	sListBoxWidth = lpMeasureItemStruct->itemWidth = r.right - r.left;
}

void CRotateAVIDlg::OnTimer(UINT_PTR nID)
{
	SendDlgItemMessage(IDC_PROGRESS1, PBM_SETPOS, percent_complete, 0);
	static TCHAR titlebuf[256];
	CListBox *pListBox = static_cast<CListBox *>(GetDlgItem(IDC_LIST1));
	int files_left = pListBox->GetCount();

	CString dir;
	GetDlgItemText(IDC_EDIT2, dir);

	if (percent_complete == 100 || stop_thread)
	{
		WaitForSingleObject(processThread, INFINITE);
		KillTimer(WM_TIMER);
		SendDlgItemMessage(IDC_PROGRESS1, PBM_SETPOS, 0, 0);

		if (!stop_thread)
		{
			// kick off next one
			files_left -= 1;
			_stprintf(titlebuf, "RotateAVI: %d%%, %d remaining", percent_complete, files_left);
			pListBox->DeleteString(0);
			if (files_left > 0)
				stop_thread = !StartNextFile();
			else
				stop_thread = true;
		}

		if (stop_thread)
		{
			ProcessAVIError();

			// enable radio buttons
#ifndef CRIPPLEWARE
			GetDlgItem(IDC_RADIO1)->EnableWindow();
#endif
			GetDlgItem(IDC_RADIO2)->EnableWindow();
			GetDlgItem(IDC_RADIO3)->EnableWindow();
			GetDlgItem(IDC_RADIO4)->EnableWindow();
			GetDlgItem(IDC_RADIO5)->EnableWindow();
			// enable remove button
			GetDlgItem(IDC_BUTTON3)->EnableWindow();
			// enable output directory picking
			GetDlgItem(IDC_EDIT2)->EnableWindow();
			GetDlgItem(IDC_BUTTON2)->EnableWindow();

			// change "Stop" to "Go"
			GetDlgItem(IDOK)->SetWindowText(TEXT("Go"));

			_stprintf(titlebuf, "RotateAVI");
		}
	}
	else
	{
		_stprintf(titlebuf, "RotateAVI: %d%%, %d remaining", percent_complete, files_left);
	}

	SetWindowText(titlebuf);
}

void CRotateAVIDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	CString s;
	GetDlgItemText(IDOK, s);

	if (!s.Compare(TEXT("Go")))
	{
		eRotation rot = CW_90;
		if (IsDlgButtonChecked(IDC_RADIO2)) rot = ACW_90;
		if (IsDlgButtonChecked(IDC_RADIO3)) rot = CW_180;
		if (IsDlgButtonChecked(IDC_RADIO4)) rot = MIRROR_X;
		if (IsDlgButtonChecked(IDC_RADIO5)) rot = MIRROR_Y;

		CListBox *pListBox = static_cast<CListBox *>(GetDlgItem(IDC_LIST1));
		if (pListBox->GetCount() > 0)
		{
			if (StartNextFile())
			{
				// disable radio buttons
				GetDlgItem(IDC_RADIO1)->EnableWindow(0);
				GetDlgItem(IDC_RADIO2)->EnableWindow(0);
				GetDlgItem(IDC_RADIO3)->EnableWindow(0);
				GetDlgItem(IDC_RADIO4)->EnableWindow(0);
				GetDlgItem(IDC_RADIO5)->EnableWindow(0);
				// disable 'remove' button
				GetDlgItem(IDC_BUTTON3)->EnableWindow(0);
				// disable output directory picking
				GetDlgItem(IDC_EDIT2)->EnableWindow(0);
				GetDlgItem(IDC_BUTTON2)->EnableWindow(0);

				// change "Go" to "Stop"
				GetDlgItem(IDOK)->SetWindowText(TEXT("Stop"));
			}
		}
	}
	else
	{
		// kill the thread
		stop_thread = true;
	}
}

void CRotateAVIDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CRotateAVIDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CMultiFileDialog openDlg(TEXT("AVI Files (*.avi)|*.avi||"));
	openDlg.m_ofn.lpstrTitle = TEXT("Choose an AVI File");
	INT_PTR iRet = openDlg.DoModal();
    if(iRet == IDOK)
	{
		CListBox *pListBox = static_cast<CListBox *>(GetDlgItem(IDC_LIST1));
		POSITION pos (openDlg.GetStartPosition());
		while(pos)
		{
			CString strFileName(openDlg.GetNextPathName(pos));

			// if we haven't already got it...
			bool already_got = false;
			CString list_filename;
		    int count = pListBox->GetCount();
		    for (int i=0; i<count; ++i)
		    {
				pListBox->GetText(i, list_filename);
				if (list_filename.CompareNoCase(strFileName) == 0)
					already_got = true;
		    }
			// add it
			if (!already_got)
				pListBox->AddString(strFileName);
		}
	}
}

void CRotateAVIDlg::OnBnClickedButton2()
{
	TCHAR dir_buf[MAX_PATH];
	dir_buf[0] = 0;
	BROWSEINFO	browseinfo;
	memset(&browseinfo, 0, sizeof(BROWSEINFO));
	browseinfo.hwndOwner = m_hWnd;
	browseinfo.lpszTitle = TEXT("Choose an Output Directory");
	browseinfo.pszDisplayName = dir_buf;
	browseinfo.ulFlags = BIF_USENEWUI;
	LPITEMIDLIST pidlist = ::SHBrowseForFolder(&browseinfo);
	if (pidlist)
	{
		SHGetPathFromIDList(pidlist, dir_buf);
		SetDlgItemText(IDC_EDIT2, dir_buf);
	}
}


void CRotateAVIDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CListBox *pListBox = static_cast<CListBox *>(GetDlgItem(IDC_LIST1));
	pListBox->DeleteString(pListBox->GetCurSel());
}

struct AVIProcessInfo
{
	TCHAR source_filename[MAX_PATH];
	TCHAR dest_filename[MAX_PATH];
	eRotation rot;
};

static TCHAR error_buf[1024];

DWORD WINAPI AVIProcessThread(LPVOID param)
{
	AVIProcessInfo *pAVIInfo = (AVIProcessInfo *)param;
	bytetotal b;

	ChunkHandler *pHandler = 0;

	FileBuf fpSrc(pAVIInfo->source_filename, false, 16*1024*1024);
	FileBuf fpDest(pAVIInfo->dest_filename, true, 16*1024*1024);

    if (!fpSrc.IsValid())
	{
		avi_error = AVIE_FILE_OPEN_READ_ERROR;
		goto cleanup_raw;
	}

    if (!fpDest.IsValid())
	{
		avi_error = AVIE_FILE_OPEN_WRITE_ERROR;
		goto cleanup_raw;
	}

	rot = pAVIInfo->rot;
	pHandler = ChunkHandler::CreateRiffChunk(fpSrc, fpDest);
	if (pHandler)
	{
		b = pHandler->ProcessChunk();
		pHandler->Cleanup();
		if (b.read == -1)
			stop_thread = true;
	}
	else
		stop_thread = true;

cleanup_raw:
	if (pHandler) delete pHandler;

	delete pAVIInfo;

	return 0;
}

void CRotateAVIDlg::ProcessAVIRaw(const TCHAR *source_filename, const TCHAR *dest_filename, eRotation rot)
{
	AVIProcessInfo *avipi = new AVIProcessInfo;
	_tcscpy(avipi->source_filename, source_filename);
	_tcscpy(avipi->dest_filename, dest_filename);
	avipi->rot = rot;

	stop_thread = false;
	percent_complete = 0;
	processThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AVIProcessThread, avipi, 0, 0);

	SetTimer(WM_TIMER, 100, 0);
}

CString infilename;
CString outfilename;

bool CRotateAVIDlg::StartNextFile()
{
	CListBox *pListBox = static_cast<CListBox *>(GetDlgItem(IDC_LIST1));

	avi_error = AVIE_NOERROR;
	eRotation rot = CW_90;
	if (IsDlgButtonChecked(IDC_RADIO2)) rot = ACW_90;
	if (IsDlgButtonChecked(IDC_RADIO3)) rot = CW_180;
	if (IsDlgButtonChecked(IDC_RADIO4)) rot = MIRROR_X;
	if (IsDlgButtonChecked(IDC_RADIO5)) rot = MIRROR_Y;

	pListBox->GetText(0, infilename);
	int lastslashpos = infilename.ReverseFind('\\');
	CString infile = infilename.Mid(lastslashpos);
	GetDlgItemText(IDC_EDIT2, outfilename);
	outfilename += infile;

	if (outfilename == infilename)
	{
		_stprintf(error_buf, TEXT("Source and destination files are the same: %s"), infilename);
		MessageBox(error_buf, 0, MB_ICONERROR);
		return false;
	}

	ProcessAVIRaw(infilename, outfilename, rot);
	return true;
}

void CRotateAVIDlg::ProcessAVIError()
{
	switch (avi_error)
	{
		case AVIE_NOERROR:
			return;

		case AVIE_FILE_OPEN_READ_ERROR:
			_stprintf(error_buf, TEXT("Couldn't open file %s for reading"), infilename);
			break;
		case AVIE_FILE_OPEN_WRITE_ERROR:
			_stprintf(error_buf, TEXT("Couldn't open file %s for writing"), outfilename);
			break;
		case AVIE_FILE_READ_ERROR:
			_stprintf(error_buf, TEXT("Error reading file %s"), infilename);
			break;
		case AVIE_FILE_WRITE_ERROR:
			_stprintf(error_buf, TEXT("Error writing file %s"), outfilename);
			break;

		case AVIE_BAD_RIFF_CODE:
			_stprintf(error_buf, TEXT("%s\nis not a RIFF file (code is '%c%c%c%c')"), infilename,
						error_data&0xff, (error_data>>8)&0xff, (error_data>>16)&0xff, (error_data>>24)&0xff);
			break;
		case AVIE_BAD_AVI_CODE:
			_stprintf(error_buf, TEXT("%s\nis not an AVI file (FCC code is '%c%c%c%c')"), infilename,
					error_data&0xff, (error_data>>8)&0xff, (error_data>>16)&0xff, (error_data>>24)&0xff);
			break;

		case AVIE_COPYRIGHTED:
			_stprintf(error_buf, TEXT("%s\nis copyrighted: not processing it"), infilename);
			break;

		case AVIE_STREAM_HEADER_NOT_MJPG:
			_stprintf(error_buf, TEXT("Video stream in\n%s\nis not a motion JPEG(MJPG) stream (header FCC code is %c%c%c%c)"), infilename,
					error_data&0xff, (error_data>>8)&0xff, (error_data>>16)&0xff, (error_data>>24)&0xff);
			break;

		case AVIE_STREAM_FORMAT_NOT_MJPG:
			_stprintf(error_buf, TEXT("Video stream in\n%s\nis not a motion JPEG(MJPG) stream (format FCC code is %c%c%c%c)"), infilename,
					error_data&0xff, (error_data>>8)&0xff, (error_data>>16)&0xff, (error_data>>24)&0xff);
			break;

		case AVIE_STREAM_FORMAT_NOT_24BIT:
			_stprintf(error_buf, TEXT("Video stream in\n%s\nis not a 24-bit stream (header indicates %d bits)"), infilename, error_data);
			break;

		case AVIE_STRH_OUTSIDE_STRL:
			_stprintf(error_buf, TEXT("%s\nis malformed: stream header chunk 'strh' appears outside of stream list 'strl'"), infilename);
			break;
		case AVIE_STRF_OUTSIDE_STRL:
			_stprintf(error_buf, TEXT("%s\nis malformed: stream format chunk 'strf' appears outside of stream list 'strl'"), infilename);
			break;
		case AVIE_STRD_OUTSIDE_STRL:
			_stprintf(error_buf, TEXT("%s\nis malformed: stream chunk 'strd' appears outside of stream list 'strl'"), infilename);
			break;
		case AVIE_STRN_OUTSIDE_STRL:
			_stprintf(error_buf, TEXT("%s\nis malformed: stream chunk 'strn' appears outside of stream list 'strl'"), infilename);
			break;
		case AVIE_STRF_BEFORE_STRH:
			_stprintf(error_buf, TEXT("%s\nis malformed: stream format chunk 'strf' appears before stream header chunk 'strh'"), infilename);
			break;

		case AVIE_MOVI_BEFORE_STRL:
			_stprintf(error_buf, TEXT("%s\nis malformed: stream chunk list 'movi' appears before stream header list 'strl'"), infilename);
			break;
		case AVIE_REC_OUTSIDE_MOVI:
			_stprintf(error_buf, TEXT("%s\nis malformed: stream chunk list 'rec ' appears outside stream chunk list 'movie'"), infilename);
			break;
		case AVIE_IDX1_BEFORE_MOVI:
			_stprintf(error_buf, TEXT("%s\nis malformed: index chunk 'idx1' appears before stream chunk list 'movi'"), infilename);
			break;

		case AVIE_STREAMCHUNK_OUTSIDE_MOVI_OR_REC:
			_stprintf(error_buf, TEXT("%s\nis malformed: stream chunk '%c%c%c%c' appears outside of list 'movi' or 'rec '"), infilename,
				error_data&0xff, (error_data>>8)&0xff, (error_data>>16)&0xff, (error_data>>24)&0xff);
			break;
	}

	MessageBox(error_buf, 0, MB_ICONERROR);
}

void CRotateAVIDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: Add your message handler code here and/or call default
	UINT  uNumFiles;
	TCHAR szNextFile [MAX_PATH];

	// Get the # of files being dropped.
	uNumFiles = DragQueryFile ( hDropInfo, -1, NULL, 0 );

	CListBox *pListBox = static_cast<CListBox *>(GetDlgItem(IDC_LIST1));

	for ( UINT uFile = 0; uFile < uNumFiles; uFile++ )
	{
		// Get the next filename from the HDROP info.
		if ( DragQueryFile ( hDropInfo, uFile, szNextFile, MAX_PATH ) > 0 )
		{
			size_t len = _tcslen(szNextFile);
			// if it's an avi file...
			if (!_tcsicmp(&szNextFile[len-4], ".avi"))
			{
				// and we haven't already got it...
				bool already_got = false;
				CString list_filename;
		        int count = pListBox->GetCount();
		        for (int i=0; i<count; ++i)
		        {
					pListBox->GetText(i, list_filename);
					if (list_filename.CompareNoCase(szNextFile) == 0)
						already_got = true;
		        }
				// add it
				if (!already_got)
					pListBox->AddString(szNextFile);
			}
		}
	}

	// Free up memory.
	DragFinish ( hDropInfo );

	//CDialog::OnDropFiles(hDropInfo);
}

void CRotateAVIDlg::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
#ifdef CRIPPLEWARE
	MessageBox(TEXT("This feature is disabled in the demo version."));
#endif
}
