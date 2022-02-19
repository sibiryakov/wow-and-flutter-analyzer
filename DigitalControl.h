// Designed by HE LINGSONG
// School of Mechanical Engineering
// Huazhong University, China
// http://heliso.tripod.com  (English)
// http://drhe.blog.sohu.com (Chinese)

class CDigitalControl : public CStatic
{
// Construction
public:
	CDigitalControl();
	void reDraw();
    void setType(int t); 
	void setLimit(double v1,double v2);
	void setValue(double v);
	double getValue();
	void setBackgroundColor(int c1,int c2,int c3); 
	void setGradientColor(int c1,int c2);
	void setScaleColor(int c1,int c2);
	void setTextColor(int c1);

protected:
	void fillbar(CDC* dc,int x,int y,int w,int h,int c);
	void line(CDC* dc,int x1,int y1,int x2,int y2,int c);
    void Gradient(CDC* dc,int direct,int x,int y,int w,int h,int c1,int c2);
    void Trangle(CDC* dc,int x1,int y1,int x2,int y2,int x3,int y3,int c);
    void Write(CDC* dc,int x,int y,int c,char *str);
// Attributes
public:
	HWND hl;
    int w,h;
	int n;
	int c1,c2,c3,c4,c5,c6,c7,c8;
	int t;
	double min,max,v,amp;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDigitalControl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDigitalControl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDigitalControl)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
