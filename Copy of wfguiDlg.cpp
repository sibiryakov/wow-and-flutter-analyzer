// wfguiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wfgui.h"
#include "wfguiDlg.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


FILE *fp_wav;
BYTE *filebuffer;
CString messagebuff;
int loaded_bytes,offset;
double d_in_val, d_out_val;

int process_sample(void);
double process_2nd_order(register double val);


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
{
	//{{AFX_DATA_INIT(CWfguiDlg)
	m_status = _T("");
	m_freq = _T("");
	m_rms = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWfguiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWfguiDlg)
	DDX_Text(pDX, IDC_STATUS, m_status);
	DDX_Text(pDX, IDC_FREQ, m_freq);
	DDX_Text(pDX, IDC_RMS, m_rms);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWfguiDlg, CDialog)
	//{{AFX_MSG_MAP(CWfguiDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_FILE, OnFile)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	CRect rect;
  GetDlgItem(IDC_OSCOPE)->GetWindowRect(rect) ;
  ScreenToClient(rect) ;

  // create the control
  m_OScopeCtrl.Create(WS_VISIBLE | WS_CHILD, rect, this) ; 

  // customize the control
  m_OScopeCtrl.SetRange(-4.0, 4.0, 1) ;
  m_OScopeCtrl.SetYUnits("Error") ;
  m_OScopeCtrl.SetXUnits("Samples") ;
  m_OScopeCtrl.SetBackgroundColor(RGB(0, 0, 64)) ;
  m_OScopeCtrl.SetGridColor(RGB(192, 192, 255)) ;
  m_OScopeCtrl.SetPlotColor(RGB(0, 255, 255)) ;


	
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


struct _WavHeader
{
  /// Contains the letters "RIFF" in ASCII form.
  unsigned int  chunkID;
  /// Size of the rest of the chunk following this number.
  unsigned int  chunkSize;
  /// Contains the letters "WAVE".
  unsigned int format;
  /// Contains the letters "fmt ".
  unsigned int subchunk1ID;
  /// 16 for PCM.  This is the size of the rest of the Subchunk
  /// which follows this number.
  unsigned int  subchunk1Size;
  /// PCM = 1 (i.e. Linear quantization). Values other than 1 indicate some
  /// form of compression.
  unsigned short audioFormat;
  /// Mono = 1, Stereo = 2, etc.
  unsigned short numChannels;
  /// 8000, 44100, etc.
  unsigned int   sampleRate;
  /// SampleRate * NumChannels * BitsPerSample/8
  unsigned int   byteRate;
  /// NumChannels * BitsPerSample/8
  unsigned short blockAlign; 
  /// 8 bits = 8, 16 bits = 16, etc.
  unsigned short bitsPerSample;
  /// Contains the letters "data".
  unsigned int subchunk2ID;
  /// Number of bytes in the data.
  unsigned int subchunk2Size;

} WavHeader;


short error_3150;


double freq,err1, err2;
double nanosec_per_sample;
double sum_of_squares;
int good_samples;
int last_val, this_val;
int nread;
double interval,remainder,delta;

int samples;
double summ, average;
int max, min;

short sample;
FILE *outf;
FILE *logfile;

void CWfguiDlg::OnFile() 
{
	        OPENFILENAME OpenFileName;   /* buffers for the file names. */
			char szFile[64],szFileTitle[128];
			HWND hwnd = NULL;
//			HANDLE hInst; 
			char szFilterInits[10][40];
			char filter[80];
			TCHAR *p; 
			struct _stat statbuf;
			int result;



			p=filter;
			strcpy(p,"wav");
			p += strlen ("wav") +1; 
			strcpy(p,"*.wav");
			p += strlen ("*.wav") +1;
			*p++=0;
			*p++=0;

		//	lstrcpy(&szFilterInits[0][0], TEXT("Srec Files (*.s)")) ;
		//	lstrcpy(&szFilterInits[1][0], TEXT("*.s")) ; 
			//lstrcpy(&szFilterInits[2][0], TEXT("Fat Files (*.fat)")) ;
			//lstrcpy(&szFilterInits[3][0], TEXT("*.fat")) ;
			szFilterInits[2][0] = (TCHAR) 0 ; 

			
			memset(szFile,0,64);
			memset(szFileTitle,0,128);  

			OpenFileName.lStructSize       = sizeof(OPENFILENAME); 
            OpenFileName.hwndOwner         = hwnd; 
        //    OpenFileName.hInstance         = (HANDLE) hInst; 
            OpenFileName.lpstrFilter       = (char *) filter;
				szFilterInits; // built in WM_CREATE 
            OpenFileName.lpstrCustomFilter = NULL;
			OpenFileName.nMaxCustFilter    = 0L; 
            OpenFileName.nFilterIndex      = 1L;
			OpenFileName.lpstrFile         = szFile;
			OpenFileName.nMaxFile          = MAX_PATH; 
            OpenFileName.lpstrFileTitle    = szFileTitle; 
            OpenFileName.nMaxFileTitle     = MAX_PATH; 
            OpenFileName.lpstrInitialDir   = NULL;  
			OpenFileName.lpstrTitle        = "WAV file";
			OpenFileName.nFileOffset       = 0;
			OpenFileName.nFileExtension    = 0; 
            OpenFileName.lpstrDefExt       = NULL; 
			OpenFileName.lCustData         = 0; 
            OpenFileName.lpfnHook          = NULL; 
            OpenFileName.lpTemplateName    = NULL; 
			OpenFileName.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

			if (!GetOpenFileName(&OpenFileName))
				return;
			

			result = _stat( szFile, &statbuf );
			if(result){
				MessageBox("Can't stat file!");
				return;
			}


			fp_wav = fopen(szFile,"rb");

			fread(&WavHeader,sizeof(WavHeader),1,fp_wav);

			if(WavHeader.numChannels != 1){
				MessageBox("Wav file not mono!");
				return;
			}

			if(WavHeader.sampleRate != 44100){
				MessageBox("Rate not 44.1!");
				return;
			}

			outf = fopen("intervals.wav","wb");
			logfile = fopen("log.txt","w");
		


			nanosec_per_sample = 10e8 / WavHeader.sampleRate;
			sum_of_squares = 0;
			good_samples = 0;

			last_val = 0;
			WavHeader.sampleRate = 6300; // to write back the error wave
			fwrite(&WavHeader,sizeof(WavHeader),1,outf);


			while((nread = fread(&sample,2,1,fp_wav)) > 0){

				// apply a 2nd order band pass filter
				d_in_val = double(sample);
				d_out_val= process_2nd_order(d_in_val);
			//	d_out_val= d_in_val;
				this_val = (int)(d_out_val);
				

			     if((this_val > 0) && (last_val < 0)){
					delta = -last_val * nanosec_per_sample / (this_val - last_val);
					interval += delta;
					remainder = nanosec_per_sample - delta;
					//break;
					result = process_sample();
					if(samples <100)
						m_OScopeCtrl.AppendPoint((double)(result/1000));
					continue;
				 } 
				
				if((this_val < 0) && (last_val > 0)){
						delta = last_val * nanosec_per_sample / (last_val - this_val) ;
						interval += delta;
						remainder = nanosec_per_sample - delta;
						//break;
						result = process_sample();
						if(samples <100)
							m_OScopeCtrl.AppendPoint((double)(result/1000));
						continue;
				}
				

				interval += nanosec_per_sample; // nS between samples

				if(this_val == 0){
					remainder = 0;
						//break;
					result = process_sample();
					if(samples <100)
						m_OScopeCtrl.AppendPoint((double)(result/1000));

				} else
					last_val = this_val;

			}

			//filebuffer = (BYTE *)malloc(500*1024); // 500K more than enough

			average=summ/(double) samples;
			freq = 1000000000 / average /2;

			m_freq.Format("%.1f", freq);
			m_rms.Format("%.4f", sqrt(sum_of_squares/good_samples) * 100);

			if(loaded_bytes){
				m_status.Format("Loaded %d bytes", loaded_bytes);
//				GetDlgItem(IDC_UPLOAD)->EnableWindow();
				//loaded_bytes = (loaded_bytes + 1023)%1024;
				//loaded_bytes = 1024;
			} else {
				m_status.Format("Error!");
			}
//			m_offset.Format("%08x", offset);
			UpdateData (FALSE);	
//			SendBrec(szFile);
			fclose(outf);
			fclose(logfile);
			fclose(fp_wav);
			samples = 0;


			fp_wav = fopen("intervals.wav","rb");

			fread(&WavHeader,sizeof(WavHeader),1,fp_wav);
			SetTimer(1,50,NULL);

	
}

double proper_interval = 0.5 * 10e8/3150; // half a period of 3150 sampled

int process_sample(void){

      last_val = this_val;

      error_3150 = (short)(10 * (proper_interval - interval));
		// for 1% will be 315

	  fprintf(logfile,"%d\n", error_3150);
	  if(samples > 100) // skip a few (filter transition, etc.)
		fwrite(&error_3150,2,1,outf);

	  if((error_3150 < 3000) && (error_3150 > -3000)){

		  sum_of_squares += (proper_interval - interval)*(proper_interval - interval)/(proper_interval *proper_interval);
		  good_samples++;

		 // fwrite(&error_3150,2,1,outf);
		  // append the new value to the plot

	  }
      summ += (double) interval;
      //ints[j++] = interval;
      interval = remainder;
      samples++;
	  return error_3150;
}

double
process_2nd_order(register double val) {
   static double buf[4];
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 3*sizeof(double));
   // use 0.00120740519032883 below for unity gain at 100% level
   iir= val * 0.001207405190260069;
   iir -= 0.9483625336008361*tmp; fir= tmp;
   iir -= -1.73410899821474*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= 0.9533938855978508*tmp; fir= tmp;
   iir -= -1.781298800713404*buf[2]; fir += buf[2]+buf[2];
   fir += iir;
   buf[3]= iir; val= fir;
   return val;
}




//DEL void CAboutDlg::OnTimer(UINT nIDEvent) 
//DEL {
//DEL 	// TODO: Add your message handler code here and/or call default
//DEL 	nread = fread(&sample,2,1,fp_wav);
//DEL 	if(nread < 1)
//DEL 		KillTimer(1) ;
//DEL 
//DEL 	m_OScopeCtrl.AppendPoint((double)(sample/1000));
//DEL 
//DEL 	
//DEL 	CDialog::OnTimer(nIDEvent);
//DEL }

void CWfguiDlg::OnTimer(UINT nIDEvent) 
{
	int i,j;
	int integrated;
	// TODO: Add your message handler code here and/or call default
	for(i=0; i<10; i++){
		integrated = 0;
		for(j=0; j<6; j++){
			nread = fread(&sample,2,1,fp_wav);
 			if(nread < 1)
 				KillTimer(1) ;
			integrated+=sample;
 			
		}
		m_OScopeCtrl.AppendPoint((double)(integrated/10000));
		if(samples++ == 10000)
				KillTimer(1);
	}
	
	CDialog::OnTimer(nIDEvent);
}
