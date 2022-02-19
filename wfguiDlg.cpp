// wfguiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wfgui.h"
#include "wfguiDlg.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>


//#include "Fourier.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define USE_MESSAGE 1
#define WM_MYMESSAGE (WM_USER + 100)

FILE *fp_wav;
FILE *fp_log;
BYTE *filebuffer;
CString messagebuff;
int loaded_bytes,offset,ticks;
double d_in_val, d_out_val;
int fft_samples;

#define FFT_LEN 4096*2
double finleft[FFT_LEN/2],fout[FFT_LEN],foutimg[FFT_LEN],fdraw[FFT_LEN/2];
double scope_val;
double peak, max_peak;

double y_30ms = exp(-1.0/(30.0*6.3)); // 30 mS at 6300 samples/sec
double y_1sec = exp(-1.0/6300.0);

double max_RMS[100];
double max_peak_10sec[100];
int index_100, index_max_RMS;

int process_sample(void);
extern "C" {
	double process_2nd_order(register double val);
	double process_flutter(register double val);
	double process_wow(register double val);
	double process_unweighted(register double val);
	double process_DIN(register double val); 
}


void CALLBACK waveInProc(HWAVEIN hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);


short int waveIn[MAX_BUFFERS][44100]; // 1 sec of input data
int running;
int first_buffer;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWfguiDlg dialog

CWfguiDlg::CWfguiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWfguiDlg::IDD, pParent)
	, m_radio3(false)
	, m_max_10_sec(_T(""))
	, m_log(false)
{
	//{{AFX_DATA_INIT(CWfguiDlg)
	m_status = _T("");
	m_freq = _T("");
	m_rms = _T("");
	m_filter_type = _T("");
	//m_peak = _T("");
	m_savefile = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CWfguiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWfguiDlg)
	DDX_Control(pDX, IDC_LEVEL, m_LevelCtrl);
	DDX_Control(pDX, IDC_FILTER_TYPE, m_filter_combo);
	//	DDX_Control(pDX, IDC_METER, m_metercontrol);
	DDX_Control(pDX, IDC_RADIO3, m_radio10);
	DDX_Control(pDX, IDC_RADIO1, m_radio01);
	DDX_Control(pDX, IDC_RADIO2, m_radio05);
	DDX_Text(pDX, IDC_STATUS, m_status);
	DDX_Text(pDX, IDC_FREQ, m_freq);
	DDX_Text(pDX, IDC_RMS, m_rms);
	DDX_Text(pDX, IDC_MAX, m_max_10_sec);
	DDX_CBString(pDX, IDC_FILTER_TYPE, m_filter_type);
	//DDX_Text(pDX, IDC_PEAK, m_peak);
	DDX_Check(pDX, IDC_FILTER, m_savefile);
	DDX_Control(pDX, IDC_NEEDLE, m_3DMeterCtrl);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_RADIO4, m_radio30);
	DDX_Control(pDX, IDCANCEL, m_3000);
	DDX_Control(pDX, IDC_FREQ, m_freq_display);
	DDX_Control(pDX, IDC_RMS, m_RMS_box);
	DDX_Control(pDX, IDC_LED, m_OK_LED);
}

BEGIN_MESSAGE_MAP(CWfguiDlg, CDialog)
	//{{AFX_MSG_MAP(CWfguiDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_MESSAGE(MM_WIM_DATA, OnWaveMessage) 
	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RADIO4, OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnBnClickedRadio5)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWfguiDlg message handlers

BOOL CWfguiDlg::OnInitDialog()
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
	CFont *myFont = new CFont();
	myFont->CreateFont( 24, 0, 0, 0, FW_HEAVY, false, false,
        0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        FIXED_PITCH|FF_MODERN, _T("Courier New") );


	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_freq_display.SetFont(myFont);
	m_RMS_box.SetFont(myFont);
	//IDC_FREQ
	CheckRadioButton(IDC_RADIO5,IDC_RADIO6,IDC_RADIO6);
	m_OK_LED.SetImage(IDB_BITMAP1 /* IDB_LEDBUTTON_GREEN*/, 15 );

//	CAboutDlg Dlg;
//	Dlg.DoModal();

	if(FillDevices()<=0)
	{
		AfxMessageBox("NO Input Devices Found..",MB_ICONERROR);
		CDialog::OnOK();
	}

	


  CRect rect;
  GetDlgItem(IDC_OSCOPE)->GetWindowRect(rect) ;
  ScreenToClient(rect) ;

  // create the control
  m_OScopeCtrl.Create(WS_VISIBLE | WS_CHILD, rect, this) ; 

  // customize the control

  m_OScopeCtrl.SetRange(-0.4, 0.4, 2) ;
  m_OScopeCtrl.SetYUnits("Deviation") ;
  m_OScopeCtrl.SetXUnits("Samples") ;
  m_OScopeCtrl.SetBackgroundColor(RGB(0, 0, 64)) ;
  m_OScopeCtrl.SetGridColor(RGB(192, 192, 255)) ;
  m_OScopeCtrl.SetPlotColor(RGB(0, 255, 255)) ;

  m_3DMeterCtrl.SetRange(0.0, 0.4);

  m_radio05.SetCheck(1);

#if 0
	CRect rct(15,400,560,600);
	m_graph.Create("STATIC","",WS_VISIBLE|WS_CHILD,rct,this,1001);

//	m_graph.SetGraphType(FG_SPECTRUM);
	m_data.pData = &m_graph;
#endif
//	m_metercontrol.SetRange(0,5000);

	m_LevelCtrl.SetRange(0,0x8000);

	m_filter_combo.AddString("Unweighted");
	m_filter_combo.AddString("DIN");
	m_filter_combo.AddString("Wow");
	m_filter_combo.AddString("Flutter");
	m_filter_combo.SetCurSel(1);
	//m_filter_combo.SelectString(-1,"DIN");
	//m_filter_type = "DIN";
	m_filter_combo.GetLBText(1, m_filter_type);
	
  UpdateData (FALSE);	


	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWfguiDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWfguiDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWfguiDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



short error_3150;


double freq,err1, err2;
double nanosec_per_sample;
double sum_of_squares, sum_of_squares1;
int good_samples;
int last_val, this_val;
int nread;
double interval,_remainder,delta;

int samples;
double summ, average;
int max, min,started;

short sample;
FILE *outf;
//FILE *logfile;

double RMS_sums[10];
int current_buffer; // 0 to 9


LRESULT CWfguiDlg::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(wParam);
   UNREFERENCED_PARAMETER(lParam);

   UpdateData (FALSE);

   return 0;
}





double proper_interval = 0.5 * 10e8/3150; // half a period of 3150 sampled

int process_sample(void){

      last_val = this_val;

	  return error_3150;
}

short input_wav[4410]; // 0.1 sec worth of the wave
int scope_samples;

int quasi_peak_val;
int ticks2;


void CWfguiDlg::OnTimer(UINT nIDEvent) 
{
	int i,j;
	int integrated;
	int zero_cross;
	double err, filtered,v;



	if(nIDEvent == 2){
		ticks2++;
		if(quasi_peak_val == 1000){
			printf("%d\n", quasi_peak_val); 
			return;
		}
		quasi_peak_val += 1;
		if((ticks2 % 20) == 0){ 
			printf("%d\n", quasi_peak_val); 
			//m_metercontrol.SetPos(quasi_peak_val);
		}
		return;

	}



	if(ticks == 0){
		sum_of_squares = 0;
		sum_of_squares1 = 0;
		good_samples = 0;
	}

	UpdateData (FALSE);	

	CDialog::OnTimer(nIDEvent);
}

void CWfguiDlg::OnRadio1() 
{
	m_radio10.SetCheck(0);
	m_radio05.SetCheck(0);
	m_radio30.SetCheck(0);
 m_OScopeCtrl.SetRange(-0.1, 0.1, 2) ;
 m_3DMeterCtrl.SetRange(0.0, 0.1); 
 m_3DMeterCtrl.SetScaleDecimals(1);
 //m_metercontrol.SetRange(0,1000);
	
}

void CWfguiDlg::OnRadio2() 
{
	m_radio10.SetCheck(0);
	m_radio01.SetCheck(0);
	m_radio30.SetCheck(0);

	m_OScopeCtrl.SetRange(-0.4, 0.4, 2) ;
 //m_metercontrol.SetRange(0,5000);
	m_3DMeterCtrl.SetRange(0.0, 0.4); 
	m_3DMeterCtrl.SetScaleDecimals(1);
}

void CWfguiDlg::OnRadio3() 
{
	m_radio01.SetCheck(0);
	m_radio05.SetCheck(0);
	m_radio30.SetCheck(0);

	m_OScopeCtrl.SetRange(-1.0, 1.0, 2) ;
	//m_metercontrol.SetRange(0,10000);
	m_3DMeterCtrl.SetRange(0.0, 1.0); 
	m_3DMeterCtrl.SetScaleDecimals(1);
}

void CWfguiDlg::OnBnClickedRadio4()
{
	m_radio01.SetCheck(0);
	m_radio05.SetCheck(0);
	m_radio10.SetCheck(0);

	m_OScopeCtrl.SetRange(-4.0, 4.0, 2) ;
	m_3DMeterCtrl.SetRange(0.0, 4.0); 
	m_3DMeterCtrl.SetScaleDecimals(0);
}





void CWfguiDlg::OnButton1() //STOP
{
	running = 0;
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	Sleep(200);

	GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
	if(outf > 0){
		fclose(outf);
		outf = NULL;
	}

	if(fp_log > 0){
		fclose(fp_log);
		fp_log = NULL;
	}

	
	
	// TODO: Add your control notification handler code here
//	waveInStop(m_hWaveIn);
//	waveInClose(m_hWaveIn);
//	KillTimer(1);
	
	//UnPrepareBuffers();
	//waveInClose(m_hWaveIn);
	
}

void CWfguiDlg::OnButton2() // START
{
	MMRESULT mRes;
	GetDlgItem(IDC_FILTER_TYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	int filter_sel = m_filter_combo.GetCurSel();
	m_filter_combo.GetLBText(filter_sel, m_filter_type);

	UpdateData(TRUE);

//	if(fp_wav)
	//SetTimer(1,50,NULL);
	running = 1;
	first_buffer = 1;
	current_buffer = 0;

	summ = 0.0;
	average = 0.0;
	ticks = 0;
	for(int i=0; i<100; i++){
		max_RMS[i] = 0.0;
		max_peak_10sec[i] = 0.0;
	}
	index_100 = 0;
	index_max_RMS = 0;

//	SetTimer(1,50,NULL);
	started = 0; // for discarding a few first points
	peak = 0.0;
	max_peak = 0.0;
	nanosec_per_sample = 10e8 / 44100;


	if(m_savefile)
			outf = fopen("WF_out.dat","wb");

	if(m_log)
			fp_log = fopen("log.txt","w");

	OpenDevice();	
	PrepareBuffers();
	mRes=waveInStart(m_hWaveIn);
}

int CWfguiDlg::FillDevices(void)
{
	CComboBox *pBox=(CComboBox*)GetDlgItem(IDC_DEVICES);
	UINT nDevices,nC1;
	WAVEINCAPS stWIC={0};
	MMRESULT mRes;

	pBox->ResetContent();
	nDevices=waveInGetNumDevs();

	for(nC1=0;nC1<nDevices;++nC1)
	{
		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
		mRes=waveInGetDevCaps(nC1,&stWIC,sizeof(WAVEINCAPS));
		if(mRes==0)
			pBox->AddString(stWIC.szPname);
//		else
//			StoreError(mRes,TRUE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
	}
	if(pBox->GetCount())
	{
		pBox->SetCurSel(0);
		OnCbnSelchangeDevices();
	}
	return nDevices;
}

void CWfguiDlg::OnCbnSelchangeDevices()
{

	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_DEVICES);
	WAVEINCAPS stWIC={0};
	MMRESULT mRes;
	int nSel, res;

	nSel=pDevices->GetCurSel();
	if(nSel!=-1)
	{
		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
		mRes=waveInGetDevCaps(nSel,&stWIC,sizeof(WAVEINCAPS));
		res = (WAVE_FORMAT_4M16==(stWIC.dwFormats&WAVE_FORMAT_4M16));
		if(res == 0)
			AfxMessageBox("Format not supported by device");
	}

}

void CALLBACK waveInProc(HWAVEIN hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	WAVEHDR *pHdr=NULL;
	switch(uMsg)
	{
		case WIM_CLOSE:
			break;

		case WIM_DATA:
			{
				CWfguiDlg *pDlg=(CWfguiDlg*)dwInstance;
				pDlg->ProcessHeader((WAVEHDR *)dwParam1);
			}
			break;

		case WIM_OPEN:
			break;

		default:
			break;
	}
}

void CWfguiDlg::OpenDevice()
{
	int nT1=0;
	CString csT1;
	MMRESULT mRes=0;
	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_DEVICES);

	
	m_stWFEX.nSamplesPerSec=44100;
	m_stWFEX.wBitsPerSample = 16;
	m_stWFEX.nChannels=1;
	m_stWFEX.wFormatTag=WAVE_FORMAT_PCM;
	m_stWFEX.nBlockAlign=m_stWFEX.nChannels*m_stWFEX.wBitsPerSample/8;
	m_stWFEX.nAvgBytesPerSec=m_stWFEX.nSamplesPerSec*m_stWFEX.nBlockAlign;
	m_stWFEX.cbSize=sizeof(WAVEFORMATEX);


	mRes=waveInOpen(&m_hWaveIn,
		pDevices->GetCurSel(),
		&m_stWFEX,
		/*(DWORD_PTR)*/
#if USE_MESSAGE
		(LONG)m_hWnd, NULL, CALLBACK_WINDOW
#else
		(DWORD )waveInProc,		(DWORD ) this, CALLBACK_FUNCTION
#endif		 
		);
	if(mRes!=MMSYSERR_NOERROR)
	{
		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
		throw m_csErrorText;
	}
}

CString CWfguiDlg::StoreError(MMRESULT mRes, BOOL bDisplay, LPCTSTR lpszFormat, ... )
{
	MMRESULT mRes1=0;
	char szErrorText[1024]={0};
	char szT1[2*MAX_PATH]={0};
	
	va_list args;
	va_start(args, lpszFormat);
	_vsntprintf(szT1, MAX_PATH, lpszFormat, args);
	va_end(args);

	m_csErrorText.Empty();
	if(m_bRun)
	{
		mRes1=waveInGetErrorText(mRes,szErrorText,1024);
		if(mRes1!=0)
			wsprintf(szErrorText,"Error %d in querying the error string for error code %d",mRes1,mRes);
		m_csErrorText.Format("%s: %s",szT1,szErrorText);
		if(bDisplay)
			AfxMessageBox(m_csErrorText);
	}
	return m_csErrorText;
}

void CWfguiDlg::ProcessHeader(WAVEHDR *pHdr)
{
	MMRESULT mRes=0;
	int i, max_val;
	short sample;
	int zero_cross;
	double err, v;
	sum_of_squares = 0.0;
	static short prev_sample;
	int min_crossings, max_crossings;
	int center_freq;

	if(IsDlgButtonChecked(IDC_RADIO5))
		center_freq = 3000;
	else
		center_freq = 3150;
	
	proper_interval = 0.5 * 10e8/center_freq; // half a period of 3150 sampled

	min_crossings = center_freq / 5 * 0.95;
	max_crossings = center_freq / 5 * 1.05;

	if(running == 0){
		waveInStop(m_hWaveIn);
		//UnPrepareBuffers();
		waveInClose(m_hWaveIn);
		m_OK_LED.Depress(false);
		return;
	}


	//TRACE("%d",pHdr->dwUser);
	if(WHDR_DONE==(WHDR_DONE &pHdr->dwFlags))
	{
		max_val = 0;
		zero_cross = 0;
		for(i=0; i<4410; i++){
			sample = *(short *)(pHdr->lpData + i*2);
			if(sample > max_val)
				max_val = sample;

			if(((sample >= 0) && (prev_sample < 0)) || ((sample < 0) && (prev_sample >= 0)))
				zero_cross++;
			prev_sample = sample;
		}

		if(max_val < 50){
			m_status = "Signal too low";
			UpdateData(false);
			mRes=waveInAddBuffer(m_hWaveIn,pHdr,sizeof(WAVEHDR));
			m_OK_LED.Depress(false);
			return;
		}

		if((zero_cross < min_crossings) || (zero_cross > max_crossings)){
			mRes=waveInAddBuffer(m_hWaveIn,pHdr,sizeof(WAVEHDR));
			//m_status.Format("%d Hz", zero_cross * 5);
			m_freq.Format("%d Hz", zero_cross * 5);
			UpdateData(false);
			m_OK_LED.Depress(false);
			return;
		}
		m_status = "";

		m_OK_LED.Depress(true);

		for(i=0; i<4410; i++){
			sample = *(short *)(pHdr->lpData + i*2);

			d_in_val = double(sample);
			d_out_val= process_2nd_order(d_in_val);
			this_val = (int)(d_out_val);
		//		this_val = sample;
			zero_cross = 0;
					

			if(((this_val > 0) && (last_val < 0)) || ((this_val < 0) && (last_val > 0))) { // sign changed
				delta = -last_val * nanosec_per_sample / (this_val - last_val);
				interval += delta;
				_remainder = nanosec_per_sample - delta;
				zero_cross = 1;
			} else {
				interval += nanosec_per_sample; // nS between samples
			}

			if(this_val == 0){
				_remainder = 0;
				zero_cross = 1;
			}
			
			last_val = this_val;

			if(zero_cross){
				  error_3150 = (short)(10 * (proper_interval - interval));
				// for 1% will be 315

		
				if(first_buffer){
					good_samples = 0;
					first_buffer = 0;
					continue;
				}
  
				  
				if(m_savefile)
					if(outf > 0)
						fwrite(&error_3150,2,1,outf);
		
		
				err = (proper_interval - interval)   / proper_interval; // in %


				if(m_filter_type == "DIN")
					err = process_DIN(err);
				else {
					if(m_filter_type == "Wow") {
						err = process_wow(err);
					} else 
						if(m_filter_type == "Flutter") {
							err = process_flutter(err);
						} else
							err = process_unweighted(err);
				}



				scope_val += err  * 100;

#if 1
				if(++scope_samples == 7){
					  m_OScopeCtrl.AppendPoint(scope_val/7);	
					  scope_val = 0;
					  scope_samples = 0;
				}
#endif	
	
			//	if(err > 0.03)
			//		continue;

#define OLD_DYNAMICS 0
#if OLD_DYNAMICS
				v =	fabs(err) * 10000 / 93; // emperical calibration
				if(v > peak)
					//	peak += (v - peak)/630;
						peak = peak * y_30ms + v * (1 - y_30ms);
				else
						peak = peak * y_1sec;

				if(peak > max_peak)
						max_peak = peak;
#else

				v =	fabs(err) * 10000 / 85; // emperical calibration
				if(v > peak)
					peak += (v - peak)/500;
				else
					peak += (v - peak)/6000;

				max_peak = peak;

#endif

#if 0
				if(samples%1000 == 0){
//						m_metercontrol.SetPos((int)(peak * 10000));
						m_peak.Format("%.4f", max_peak);
						
						//UpdateData (FALSE);	
				}
#endif

				sum_of_squares += err * err;
			    sum_of_squares1 += err * err;
				good_samples++;

				summ += (double) interval;
				  //ints[j++] = interval;
				interval = _remainder;
				samples++;


				average=summ/(double) good_samples;// samples;
				freq = 1000000000 / average /2;

				//UpdateData (FALSE);	


			} // if zerocross

		} // for i

		
		m_3DMeterCtrl.UpdateNeedle(max_peak) ;
#if OLD_DYNAMICS
		max_peak = 0.0;
#endif
	
#if 0
		if(++ticks == 10){
			m_rms.Format("%.4f", sqrt(sum_of_squares1/good_samples) * 100);
			ticks = 0;
		}
#endif
	//	UpdateData (FALSE);	

		RMS_sums[current_buffer] = sum_of_squares;

	
		max_peak_10sec[index_100] = max_peak;

	
		if(++index_100 == 100)
			index_100 = 0;


		if(++current_buffer == 10){
			int i;
			double max_peak_10 = 0.0;
			double max_RMS_10 = 0.0;

			sum_of_squares1 = 0.0;
			for(i=0; i<10; i++){
				sum_of_squares1 += RMS_sums[i];
			}
			max_RMS[index_100] = sqrt(sum_of_squares1/good_samples) * 100;
			m_rms.Format("%.4f", max_RMS[index_100]);

			// now the moving max in 10 sec

			
			for(i=0; i<100; i++){
				if(max_RMS[i] > max_RMS_10)
					max_RMS_10 = max_RMS[i];
				if(max_peak_10sec[i] > max_peak_10)
					max_peak_10 = max_peak_10sec[i];
			}
			m_max_10_sec.Format("Max in 10 sec %.4f RMS %.4f quasi-peak", max_RMS_10, max_peak_10);


			good_samples = 0;
			current_buffer = 0;
			summ = 0.0;
			samples = 0;
			m_freq.Format("%.1f", freq );


			if(m_log && (fp_log > 0))
				fprintf(fp_log, "%.1f %.4f %.4f\n", freq, max_RMS[index_100], max_peak_10);
		}

		m_LevelCtrl.SetPos(max_val);

		AfxGetMainWnd()->SendMessage(WM_MYMESSAGE) ; 
		

//		mmioWrite(m_hOPFile,pHdr->lpData,pHdr->dwBytesRecorded);
		mRes=waveInAddBuffer(m_hWaveIn,pHdr,sizeof(WAVEHDR));
		if(mRes!=0)
			StoreError(mRes,TRUE,"File: %s ,Line Number:%d",__FILE__,__LINE__);

	}
}

void CWfguiDlg::PrepareBuffers()
{
	MMRESULT mRes=0;
	int nT1=0;

	for(nT1=0;nT1<MAX_BUFFERS;++nT1)
	{
		m_stWHDR[nT1].lpData= (LPSTR)waveIn[nT1];
			//(LPSTR)HeapAlloc(GetProcessHeap(),8,m_stWFEX.nAvgBytesPerSec);
		m_stWHDR[nT1].dwBufferLength=m_stWFEX.nAvgBytesPerSec/10; // I want 0.1 sec
		m_stWHDR[nT1].dwUser=nT1;
		m_stWHDR[nT1].dwFlags = 0L;
		m_stWHDR[nT1].dwLoops = 0L;
#if 1
		mRes=waveInPrepareHeader(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
		if(mRes!=0)
		{
			StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
			throw m_csErrorText;
		}
#endif
		mRes=waveInAddBuffer(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
		if(mRes!=0)
		{
			StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
			throw m_csErrorText;
		}
	}

}

afx_msg LONG CWfguiDlg::OnWaveMessage(UINT wParam, LONG lParam)
{
 ProcessHeader((WAVEHDR *)lParam);
 return 0;
}

void CWfguiDlg::UnPrepareBuffers(void)
{
		MMRESULT mRes=0;
	int nT1=0;

	if(m_hWaveIn)
	{
		mRes=waveInStop(m_hWaveIn);
		for(nT1=0;nT1<3;++nT1)
		{
			if(m_stWHDR[nT1].lpData)
			{
				mRes=waveInUnprepareHeader(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
				HeapFree(GetProcessHeap(),0,m_stWHDR[nT1].lpData);
				ZeroMemory(&m_stWHDR[nT1],sizeof(WAVEHDR));
			}
		}
	}
}


void CWfguiDlg::OnBnClickedRadio5()
{
	// TODO: Add your control notification handler code here
}
