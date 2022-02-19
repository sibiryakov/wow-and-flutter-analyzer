#include "stdafx.h"
#include "DigitalControl.h"

/////////////////////////////////////////////////////////////////////////////
// CDigitalControl

//======================================================
typedef UINT (CALLBACK* LPFNDLLFUNC)(HDC,CONST PTRIVERTEX,DWORD,CONST PVOID,DWORD,DWORD);
HINSTANCE hl_msimg32=NULL;
LPFNDLLFUNC fun_GradientFill=NULL;
void initGradient()
{if (hl_msimg32!=NULL) return;
 hl_msimg32=LoadLibrary( "msimg32.dll" );
 fun_GradientFill=((LPFNDLLFUNC) GetProcAddress( hl_msimg32, "GradientFill" ));
}

void freeGradient()
{if (fun_GradientFill!=NULL) FreeLibrary(hl_msimg32);}

void fillGradient(HDC hDC,int direct,int x,int y,int w,int h,int c1,int c2)
{AFX_MANAGE_STATE(AfxGetStaticModuleState());
 initGradient();
 if (fun_GradientFill==NULL) return;
 int r1,g1,b1,r2,g2,b2;
 r1=c1&0xff; g1=(c1>>8)&0xff; b1=(c1>>16)&0xff;
 r2=c2&0xff; g2=(c2>>8)&0xff; b2=(c2>>16)&0xff;
 TRIVERTEX rcVertex[2];
 rcVertex[0].x=x;   rcVertex[0].y=y;   rcVertex[0].Red=r1<<8; rcVertex[0].Green=g1<<8; rcVertex[0].Blue=b1<<8; rcVertex[0].Alpha=0;
 rcVertex[1].x=x+w; rcVertex[1].y=y+h; rcVertex[1].Red=r2<<8; rcVertex[1].Green=g2<<8; rcVertex[1].Blue=b2<<8; rcVertex[1].Alpha=0;
 GRADIENT_RECT rect;rect.UpperLeft=0; rect.LowerRight=1;
 fun_GradientFill(hDC,rcVertex,2,&rect,1, direct);
}


CDigitalControl::CDigitalControl()
{t=0;
 n=10;
 c1=GetSysColor(COLOR_3DFACE);
 c2=RGB(255,255,255);
 c3=RGB(128,128,128);
 c4=RGB(192,192,192);
 c5=RGB(255,255,255);
 c6=RGB(192,0,0);
 c7=RGB(0,0,0);
 c8=RGB(0,0,128);
 min=0;
 max=100;
 v=0;
 amp=1;
}

CDigitalControl::~CDigitalControl()
{
 freeGradient();
}


BEGIN_MESSAGE_MAP(CDigitalControl, CStatic)
	//{{AFX_MSG_MAP(CDigitalControl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDigitalControl message handlers
void CDigitalControl::fillbar(CDC* dc,int x,int y,int w,int h,int c)
{dc->FillSolidRect(x,y,w,h,c);}

void CDigitalControl::Trangle(CDC* dc,int x1,int y1,int x2,int y2,int x3,int y3,int c)
{int n=3;
 POINT p[5]; 
 CPen pen; 
 pen.CreatePen(PS_SOLID,1,c);
 CPen* pPenOld = dc->SelectObject(&pen); 
 CBrush brush(c);
 CBrush* pBrushOld = dc->SelectObject(&brush); 
 p[0].x=x1; p[0].y=y1;
 p[1].x=x2; p[1].y=y2;
 p[2].x=x3; p[2].y=y3;
 dc->Polygon(p,n); 
 dc->SelectObject(pPenOld); 
 dc->SelectObject(pBrushOld); 
}

void CDigitalControl::line(CDC* dc,int x1,int y1,int x2,int y2,int c)
{CPen pen; 
 pen.CreatePen(PS_SOLID,1,c);
 CPen* pPenOld = dc->SelectObject(&pen); 
 dc->MoveTo(x1,y1); 
 dc->LineTo(x2,y2);  
 dc->SelectObject(pPenOld); 
}

void CDigitalControl::Gradient(CDC* dc,int direct,int x,int y,int w,int h,int c1,int c2)
{fillGradient(dc->m_hDC,direct,x,y,w,h,c1,c2);}

void CDigitalControl::Write(CDC* dc,int x,int y,int c,char *str)
{dc->SetBkMode(TRANSPARENT); 
 dc->SetTextColor(c);
 dc->TextOut(x,y,str);
}
//======================================================================
void CDigitalControl::reDraw()
{RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);}

void CDigitalControl::setLimit(double v1,double v2)
{min=v1;
 max=v2;
}

void CDigitalControl::setValue(double v1)
{v=v1;
 reDraw();
}

double CDigitalControl::getValue()
{return v;}

void CDigitalControl::setType(int t1) 
{t=t1;}

void CDigitalControl::setBackgroundColor(int c11,int c21,int c31)
{c1=c11;
 c2=c21;
 c3=c31;
} 

void CDigitalControl::setGradientColor(int c11,int c21)
{c4=c11;
 c5=c21;
} 

void CDigitalControl::setScaleColor(int c11,int c21)
{c6=c11;
 c7=c21;
} 

void CDigitalControl::setTextColor(int c11)
{c8=c11;} 

//=======================================================================
void CDigitalControl::OnPaint() 
{CPaintDC dc(this);  
 RECT rect; 
 GetWindowRect(&rect); 
 w=rect.right-rect.left; 
 h=rect.bottom-rect.top;
 h=h/2; h=h*2;
 w=w/2; w=w*2;
 fillbar((CDC *)&dc,0,0,w,h,c1);
 line((CDC *)&dc,0,0,w,0,c2);
 line((CDC *)&dc,0,1,w,1,c2);
 line((CDC *)&dc,0,0,0,h,c2);
 line((CDC *)&dc,1,0,1,h,c2);  
 line((CDC *)&dc,0,h,w,h,c3);
 line((CDC *)&dc,0,h-1,w,h-1,c3);
 line((CDC *)&dc,w,0,w,h,c3);
 line((CDC *)&dc,w-1,0,w-1,h,c3);  

 char str[128];
 double dy,dv;
 int i,h1,y1,y2;
 double value=v/amp;
 if (value>max) value=max;
 if (value<min) value=min;
 double vv=2*(max-min)/n;

 if (t==0)
 {Gradient((CDC *)&dc,0,10,10,w/2-9,h-20,c4,c5);
  Gradient((CDC *)&dc,0,w/2,10,w/2-10,h-20,c5,c4);
  Trangle((CDC *)&dc,w/2-5,0+2,w/2+5,0+2,w/2,10,c6);
  Trangle((CDC *)&dc,w/2-5,h-2,w/2+5,h-2,w/2,h-10,c6);
  dy=(double)((w-20.0)/n/2.0);
  h1=(w-20)/2-(int)((w-20)*(value-min)/(max-min));
  for (i=-2*n;i<=2*n+2*n;i++) 
  {y1=(int)(10+i*dy+h1);
   if ((y1>10)&(y1<w-10)) 
	   line((CDC *)&dc,y1,10,y1,15,c7);
  }
  for (i=-n/2;i<=n/2+n/2;i++) 
  {y1=(int)(10+2*2*i*dy+h1);
   if ((y1>10)&(y1<w-10)) 
	   line((CDC *)&dc,y1,10,y1,20,c7);
   dv=i*vv-min;
   sprintf(str,"%d",(int)dv);
   y2=(int)(10+(2*2*i*dy)+h1-strlen(str)*3);
   if ((y2>12)&(y2<w-20)) 
	   Write((CDC *)&dc,y2,h/2-5,c8,str); 
  }
 }
 
 if (t==1)
 {Gradient((CDC *)&dc,1,10,10,w-20,h/2-9,c4,c5);
  Gradient((CDC *)&dc,1,10,h/2,w-20,h/2-10,c5,c4);
  Trangle((CDC *)&dc,2,h/2-5,2,h/2+5,10,h/2,c6);
  Trangle((CDC *)&dc,w-2,h/2-5,w-2,h/2+5,w-10,h/2,c6);
  dy=(double)((h-20.0)/n/2.0);
  h1=(h-20)/2-(int)((h-20)*(value-min)/(max-min));
  for (i=-2*n;i<=2*n+2*n;i++) 
   {y1=(int)(10+i*dy-h1);
    if ((y1>10)&(y1<h-10)) 
		line((CDC *)&dc,10,y1,15,y1,c7);
   }
  for (i=-n/2;i<=n/2+n/2;i++) 
   {y1=(int)(10+2*2*i*dy-h1);
    if ((y1>10)&(y1<h-10)) 
		line((CDC *)&dc,10,y1,20,y1,c7);
    y2=(int)(h-10-(2*2*i*dy+7)-h1);
	dv=i*vv-min;
    sprintf(str,"%d",(int)dv);
    if ((y2>12)&(y2<h-20)) 
		Write((CDC *)&dc,w/2-5,y2,c8,str); 
   }
 }
}

