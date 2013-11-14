// gyazowin.cpp : ï¿½Aï¿½vï¿½ï¿½ï¿½Pï¿½[ï¿½Vï¿½ï¿½ï¿½ï¿½ï¿½ÌƒGï¿½ï¿½ï¿½gï¿½ï¿½ ï¿½|ï¿½Cï¿½ï¿½ï¿½gï¿½ï¿½ï¿½`ï¿½ï¿½ï¿½Ü‚ï¿½ï¿½B
//

#include "stdafx.h"
#include "gyazowin.h"

// ï¿½Oï¿½ï¿½ï¿½[ï¿½oï¿½ï¿½ï¿½Ïï¿½:
HINSTANCE hInst;							// ï¿½ï¿½ï¿½Ý‚ÌƒCï¿½ï¿½ï¿½^ï¿½[ï¿½tï¿½Fï¿½Cï¿½X
TCHAR *szTitle			= _T("Gyazo");		// ï¿½^ï¿½Cï¿½gï¿½ï¿½ ï¿½oï¿½[ï¿½Ìƒeï¿½Lï¿½Xï¿½g
TCHAR *szWindowClass	= _T("GYAZOWIN");	// ï¿½ï¿½ï¿½Cï¿½ï¿½ ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½E ï¿½Nï¿½ï¿½ï¿½Xï¿½ï¿½
TCHAR *szWindowClassL	= _T("GYAZOWINL");	// ï¿½ï¿½ï¿½Cï¿½ï¿½ï¿½[ ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½E ï¿½Nï¿½ï¿½ï¿½Xï¿½ï¿½
HWND hLayerWnd;

int ofX, ofY;	// ï¿½ï¿½ÊƒIï¿½tï¿½Zï¿½bï¿½g

// ï¿½vï¿½ï¿½ï¿½gï¿½^ï¿½Cï¿½vï¿½éŒ¾
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	LayerWndProc(HWND, UINT, WPARAM, LPARAM);

int					GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

BOOL				isPng(LPCTSTR fileName);
VOID				drawRubberband(HDC hdc, LPRECT newRect, BOOL erase);
VOID				execUrl(const char* str);
VOID				setClipBoardText(const char* str);
BOOL				convertPNG(LPCTSTR destFile, LPCTSTR srcFile);
BOOL				savePNG(LPCTSTR fileName, HBITMAP newBMP);
BOOL				uploadFile(HWND hwnd, LPCTSTR fileName);
std::string			getId();
BOOL				saveId(const WCHAR* str);

// ï¿½Gï¿½ï¿½ï¿½gï¿½ï¿½ï¿½[ï¿½|ï¿½Cï¿½ï¿½ï¿½g
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

	TCHAR	szThisPath[MAX_PATH];
	DWORD   sLen;

	// ï¿½ï¿½ï¿½gï¿½Ìƒfï¿½Bï¿½ï¿½ï¿½Nï¿½gï¿½ï¿½ï¿½ï¿½ï¿½æ“¾ï¿½ï¿½ï¿½ï¿½
	sLen = GetModuleFileName(NULL, szThisPath, MAX_PATH);
	for(unsigned int i = sLen; i >= 0; i--) {
		if(szThisPath[i] == _T('\\')) {
			szThisPath[i] = _T('\0');
			break;
		}
	}

	// ï¿½Jï¿½ï¿½ï¿½ï¿½ï¿½gï¿½fï¿½Bï¿½ï¿½ï¿½Nï¿½gï¿½ï¿½ï¿½ï¿½ exe ï¿½Æ“ï¿½ï¿½ï¿½ï¿½êŠï¿½ÉÝ’ï¿½
	SetCurrentDirectory(szThisPath);

	// ï¿½ï¿½ï¿½ï¿½ï¿½Éƒtï¿½@ï¿½Cï¿½ï¿½ï¿½ï¿½ï¿½wï¿½è‚³ï¿½ï¿½Ä‚ï¿½ï¿½ï¿½ï¿½ï¿½
	if ( 2 == __argc )
	{
		// ï¿½tï¿½@ï¿½Cï¿½ï¿½ï¿½ï¿½ï¿½Aï¿½bï¿½vï¿½ï¿½ï¿½[ï¿½hï¿½ï¿½ï¿½ÄIï¿½ï¿½
		if (isPng(__targv[1])) {
			// PNG ï¿½Í‚ï¿½ï¿½Ì‚Ü‚ï¿½upload
			uploadFile(NULL, __targv[1]);
		}else {
			// PNG ï¿½`ï¿½ï¿½ï¿½É•ÏŠï¿½
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);
			
			if (convertPNG(tmpFile, __targv[1])) {
				//ï¿½Aï¿½bï¿½vï¿½ï¿½ï¿½[ï¿½h
				uploadFile(NULL, tmpFile);
			} else {
				// PNGï¿½É•ÏŠï¿½ï¿½Å‚ï¿½ï¿½È‚ï¿½ï¿½ï¿½ï¿½ï¿½...
				MessageBox(NULL, _T("Cannot convert this image"), szTitle, 
					MB_OK | MB_ICONERROR);
			}
			DeleteFile(tmpFile);
		}
		return TRUE;
	}

	// ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½Eï¿½Nï¿½ï¿½ï¿½Xï¿½ï¿½oï¿½^
	MyRegisterClass(hInstance);

	// ï¿½Aï¿½vï¿½ï¿½ï¿½Pï¿½[ï¿½Vï¿½ï¿½ï¿½ï¿½ï¿½Ìï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½sï¿½ï¿½ï¿½Ü‚ï¿½:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}
	
	// ï¿½ï¿½ï¿½Cï¿½ï¿½ ï¿½ï¿½ï¿½bï¿½Zï¿½[ï¿½W ï¿½ï¿½ï¿½[ï¿½v:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

// ï¿½wï¿½bï¿½_ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ PNG ï¿½æ‘œï¿½ï¿½ï¿½Ç‚ï¿½ï¿½ï¿½(ï¿½ê‰ž)ï¿½`ï¿½Fï¿½bï¿½N
BOOL isPng(LPCTSTR fileName)
{
	unsigned char pngHead[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	unsigned char readHead[8];
	
	FILE *fp = NULL;
	
	if (0 != _tfopen_s(&fp, fileName, _T("rb")) ||
		8 != fread(readHead, 1, 8, fp)) {
		// ï¿½tï¿½@ï¿½Cï¿½ï¿½ï¿½ï¿½ï¿½Ç‚ß‚È‚ï¿½	
		return FALSE;
	}
	fclose(fp);
	
	// compare
	for(unsigned int i=0;i<8;i++)
		if(pngHead[i] != readHead[i]) return FALSE;

	return TRUE;

}

// ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½Eï¿½Nï¿½ï¿½ï¿½Xï¿½ï¿½oï¿½^
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	// ï¿½ï¿½ï¿½Cï¿½ï¿½ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½E
	wc.style         = 0;							// WM_PAINT ï¿½ð‘—‚ï¿½È‚ï¿½
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GYAZOWIN));
	wc.hCursor       = LoadCursor(NULL, IDC_CROSS);	// + ï¿½ÌƒJï¿½[ï¿½\ï¿½ï¿½
	wc.hbrBackground = 0;							// ï¿½wï¿½iï¿½ï¿½ï¿½Ý’è‚µï¿½È‚ï¿½
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	RegisterClass(&wc);

	// ï¿½ï¿½ï¿½Cï¿½ï¿½ï¿½[ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½E
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = LayerWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GYAZOWIN));
	wc.hCursor       = LoadCursor(NULL, IDC_CROSS);	// + ï¿½ÌƒJï¿½[ï¿½\ï¿½ï¿½
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClassL;

	return RegisterClass(&wc);
}


// ï¿½Cï¿½ï¿½ï¿½Xï¿½^ï¿½ï¿½ï¿½Xï¿½Ìï¿½ï¿½ï¿½ï¿½ï¿½ï¿½iï¿½Sï¿½ï¿½Ê‚ï¿½ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½Eï¿½Å•ï¿½ï¿½ï¿½ï¿½j
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
//	HWND hLayerWnd;
	hInst = hInstance; // ï¿½Oï¿½ï¿½ï¿½[ï¿½oï¿½ï¿½ï¿½Ïï¿½ï¿½ÉƒCï¿½ï¿½ï¿½Xï¿½^ï¿½ï¿½ï¿½Xï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½iï¿½[ï¿½ï¿½ï¿½Ü‚ï¿½ï¿½B

	int x, y, w, h;

	// ï¿½ï¿½ï¿½zï¿½Xï¿½Nï¿½ï¿½ï¿½[ï¿½ï¿½ï¿½Sï¿½Ì‚ï¿½ï¿½Jï¿½oï¿½[
	x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	// x, y ï¿½ÌƒIï¿½tï¿½Zï¿½bï¿½gï¿½lï¿½ï¿½ï¿½oï¿½ï¿½ï¿½Ä‚ï¿½ï¿½ï¿½
	ofX = x; ofY = y;

	// ï¿½ï¿½ï¿½Sï¿½É“ï¿½ï¿½ß‚ï¿½ï¿½ï¿½ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½Eï¿½ï¿½ï¿½ï¿½ï¿½
	hWnd = CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if(_WIN32_WINNT >= 0x0500)
		| WS_EX_NOACTIVATE
#endif
		,
		szWindowClass, NULL, WS_POPUP,
		0, 0, 0, 0,
		NULL, NULL, hInstance, NULL);

	// ï¿½ï¿½ï¿½È‚ï¿½ï¿½ï¿½ï¿½ï¿½...?
	if (!hWnd) return FALSE;
	
	// ï¿½Sï¿½ï¿½Ê‚ð•¢‚ï¿½
	MoveWindow(hWnd, x, y, w, h, FALSE);
	
	// nCmdShow ï¿½ð–³Žï¿½ (SW_MAXIMIZE ï¿½Æ‚ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æï¿½ï¿½ï¿½)
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// ESCï¿½Lï¿½[ï¿½ï¿½ï¿½mï¿½^ï¿½Cï¿½}ï¿½[
	SetTimer(hWnd, 1, 100, NULL);


	// ï¿½ï¿½ï¿½Cï¿½ï¿½ï¿½[ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½Eï¿½Ìì¬
	hLayerWnd = CreateWindowEx(
	 WS_EX_TOOLWINDOW
#if(_WIN32_WINNT >= 0x0500)
		| WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
		,
		szWindowClassL, NULL, WS_POPUP,
		100, 100, 300, 300,
		hWnd, NULL, hInstance, NULL);

    SetLayeredWindowAttributes(hLayerWnd, RGB(255, 0, 0), 100, LWA_COLORKEY|LWA_ALPHA);

	


	
	return TRUE;
}

// ï¿½wï¿½è‚³ï¿½ê‚½ï¿½tï¿½Hï¿½[ï¿½}ï¿½bï¿½gï¿½É‘Î‰ï¿½ï¿½ï¿½ï¿½ï¿½ Encoder ï¿½ï¿½ CLSID ï¿½ï¿½ï¿½æ“¾ï¿½ï¿½ï¿½ï¿½
// Cited from MSDN Library: Retrieving the Class Identifier for an Encoder
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

// ï¿½ï¿½ï¿½oï¿½[ï¿½oï¿½ï¿½ï¿½hï¿½ï¿½`ï¿½ï¿½.
VOID drawRubberband(HDC hdc, LPRECT newRect, BOOL erase)
{
	
	static BOOL firstDraw = TRUE;	// 1 ï¿½ï¿½Ú‚Í‘Oï¿½Ìƒoï¿½ï¿½ï¿½hï¿½Ìï¿½ï¿½ï¿½ï¿½ï¿½ï¿½sï¿½ï¿½È‚ï¿½
	static RECT lastRect  = {0};	// ï¿½ÅŒï¿½É•`ï¿½æ‚µï¿½ï¿½ï¿½oï¿½ï¿½ï¿½h
	static RECT clipRect  = {0};	// ï¿½ÅŒï¿½É•`ï¿½æ‚µï¿½ï¿½ï¿½oï¿½ï¿½ï¿½h
	
	if(firstDraw) {
		// ï¿½ï¿½ï¿½Cï¿½ï¿½ï¿½[ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½Eï¿½ï¿½\ï¿½ï¿½
		ShowWindow(hLayerWnd, SW_SHOW);
		UpdateWindow(hLayerWnd);

		firstDraw = FALSE;
	}

	if (erase) {
		// ï¿½ï¿½ï¿½Cï¿½ï¿½ï¿½[ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½Eï¿½ï¿½ï¿½Bï¿½ï¿½
		ShowWindow(hLayerWnd, SW_HIDE);
		
	}

	// ï¿½ï¿½ï¿½Wï¿½`ï¿½Fï¿½bï¿½N
	clipRect = *newRect;
	if ( clipRect.right  < clipRect.left ) {
		int tmp = clipRect.left;
		clipRect.left   = clipRect.right;
		clipRect.right  = tmp;
	}
	if ( clipRect.bottom < clipRect.top  ) {
		int tmp = clipRect.top;
		clipRect.top    = clipRect.bottom;
		clipRect.bottom = tmp;
	}
	MoveWindow(hLayerWnd,  clipRect.left, clipRect.top, 
			clipRect.right-  clipRect.left + 1, clipRect.bottom - clipRect.top + 1,true);

	
	return;

/* rakusai 2009/11/2

	// XOR ï¿½Å•`ï¿½ï¿½
	int hPreRop = SetROP2(hdc, R2_XORPEN);

	// ï¿½_ï¿½ï¿½
	HPEN hPen = CreatePen(PS_DOT , 1, 0);
	SelectObject(hdc, hPen);
	SelectObject(hdc, GetStockObject(NULL_BRUSH));

	if(!firstDraw) {
		// ï¿½Oï¿½Ì‚ï¿½ï¿½ï¿½ï¿½ï¿½
		Rectangle(hdc, lastRect.left, lastRect.top, 
			lastRect.right + 1, lastRect.bottom + 1);
	} else {
		firstDraw = FALSE;
	}
	
	// ï¿½Vï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Wï¿½ï¿½ï¿½Lï¿½ï¿½
	lastRect = *newRect;
	
	


	if (!erase) {

		// ï¿½gï¿½ï¿½`ï¿½ï¿½
		Rectangle(hdc, lastRect.left, lastRect.top, 
			lastRect.right + 1, lastRect.bottom + 1);

	}


	// ï¿½ãˆï¿½ï¿½
	SetROP2(hdc, hPreRop);
	DeleteObject(hPen);

*/

}

// PNG ï¿½`ï¿½ï¿½ï¿½É•ÏŠï¿½
BOOL convertPNG(LPCTSTR destFile, LPCTSTR srcFile)
{
	BOOL				res = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;

	// GDI+ ï¿½Ìï¿½ï¿½ï¿½ï¿½ï¿½
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Image *b = new Image(srcFile, 0);

	if (0 == b->GetLastStatus()) {
		if (GetEncoderClsid(L"image/png", &clsidEncoder)) {
			// save!
			if (0 == b->Save(destFile, &clsidEncoder, 0) ) {
					// ï¿½Û‘ï¿½ï¿½Å‚ï¿½ï¿½ï¿½
					res = TRUE;
			}
		}
	}

	// ï¿½ï¿½nï¿½ï¿½
	delete b;
	GdiplusShutdown(gdiplusToken);

	return res;
}

// PNG ï¿½`ï¿½ï¿½ï¿½Å•Û‘ï¿½ (GDI+ ï¿½gï¿½p)
BOOL savePNG(LPCTSTR fileName, HBITMAP newBMP)
{
	BOOL				res = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;

	// GDI+ ï¿½Ìï¿½ï¿½ï¿½ï¿½ï¿½
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	
	// HBITMAP ï¿½ï¿½ï¿½ï¿½ Bitmap ï¿½ï¿½ï¿½ì¬
	Bitmap *b = new Bitmap(newBMP, NULL);
	
	if (GetEncoderClsid(L"image/png", &clsidEncoder)) {
		// save!
		if (0 ==
			b->Save(fileName, &clsidEncoder, 0) ) {
				// ï¿½Û‘ï¿½ï¿½Å‚ï¿½ï¿½ï¿½
				res = TRUE;
		}
	}
	
	// ï¿½ï¿½nï¿½ï¿½
	delete b;
	GdiplusShutdown(gdiplusToken);

	return res;
}

// ï¿½ï¿½ï¿½Cï¿½ï¿½ï¿½[ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½Eï¿½vï¿½ï¿½ï¿½Vï¿½[ï¿½Wï¿½ï¿½
LRESULT CALLBACK LayerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	RECT clipRect	= {0, 0, 500, 500};
	HBRUSH hBrush;
	HPEN hPen;
	HFONT hFont;


	switch (message)
	{
	case WM_ERASEBKGND:
		 GetClientRect(hWnd, &clipRect);
		
		hdc = GetDC(hWnd);
        hBrush = CreateSolidBrush(RGB(100,100,100));
        SelectObject(hdc, hBrush);
		hPen = CreatePen(PS_DASH,1,RGB(255,255,255));
		SelectObject(hdc, hPen);
		Rectangle(hdc,0,0,clipRect.right,clipRect.bottom);

		//ï¿½ï¿½`ï¿½ÌƒTï¿½Cï¿½Yï¿½ï¿½ï¿½oï¿½ï¿½
		int fHeight;
		fHeight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		hFont = CreateFont(fHeight,    //ï¿½tï¿½Hï¿½ï¿½ï¿½gï¿½ï¿½ï¿½ï¿½
			0,                    //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			0,                    //ï¿½eï¿½Lï¿½Xï¿½gï¿½ÌŠpï¿½x
			0,                    //ï¿½xï¿½[ï¿½Xï¿½ï¿½ï¿½Cï¿½ï¿½ï¿½Æ‚ï¿½ï¿½ï¿½ï¿½Æ‚ÌŠpï¿½x
			FW_REGULAR,            //ï¿½tï¿½Hï¿½ï¿½ï¿½gï¿½Ìdï¿½ï¿½ï¿½iï¿½ï¿½ï¿½ï¿½ï¿½j
			FALSE,                //ï¿½Cï¿½^ï¿½ï¿½ï¿½bï¿½Nï¿½ï¿½
			FALSE,                //ï¿½Aï¿½ï¿½ï¿½_ï¿½[ï¿½ï¿½ï¿½Cï¿½ï¿½
			FALSE,                //ï¿½Å‚ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			ANSI_CHARSET,    //ï¿½ï¿½ï¿½ï¿½ï¿½Zï¿½bï¿½g
			OUT_DEFAULT_PRECIS,    //ï¿½oï¿½Íï¿½ï¿½x
			CLIP_DEFAULT_PRECIS,//ï¿½Nï¿½ï¿½ï¿½bï¿½sï¿½ï¿½ï¿½Oï¿½ï¿½ï¿½x
			PROOF_QUALITY,        //ï¿½oï¿½Í•iï¿½ï¿½
			FIXED_PITCH | FF_MODERN,//ï¿½sï¿½bï¿½`ï¿½Æƒtï¿½@ï¿½~ï¿½ï¿½ï¿½[
			L"Tahoma");    //ï¿½ï¿½ï¿½Ì–ï¿½

		SelectObject(hdc, hFont);
		// show size
		int iWidth, iHeight;
		iWidth  = clipRect.right  - clipRect.left;
		iHeight = clipRect.bottom - clipRect.top;

		wchar_t sWidth[200], sHeight[200];
		swprintf_s(sWidth, L"%d", iWidth);
		swprintf_s(sHeight, L"%d", iHeight);

		int w,h,h2;
		w = -fHeight * 2.5 + 8;
		h = -fHeight * 2 + 8;
		h2 = h + fHeight;

		SetBkMode(hdc,TRANSPARENT);
		SetTextColor(hdc,RGB(0,0,0));
		TextOut(hdc, clipRect.right-w+1,clipRect.bottom-h+1,(LPCWSTR)sWidth,wcslen(sWidth));
		TextOut(hdc, clipRect.right-w+1,clipRect.bottom-h2+1,(LPCWSTR)sHeight,wcslen(sHeight));
		SetTextColor(hdc,RGB(255,255,255));
		TextOut(hdc, clipRect.right-w,clipRect.bottom-h,(LPCWSTR)sWidth,wcslen(sWidth));
		TextOut(hdc, clipRect.right-w,clipRect.bottom-h2,(LPCWSTR)sHeight,wcslen(sHeight));

		DeleteObject(hPen);
		DeleteObject(hBrush);
		DeleteObject(hFont);
		ReleaseDC(hWnd, hdc);

		return TRUE;

        break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}

// ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½Eï¿½vï¿½ï¿½ï¿½Vï¿½[ï¿½Wï¿½ï¿½
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	
	static BOOL onClip		= FALSE;
	static BOOL firstDraw	= TRUE;
	static RECT clipRect	= {0, 0, 0, 0};
	
	switch (message)
	{
	case WM_RBUTTONDOWN:
		// ï¿½Lï¿½ï¿½ï¿½ï¿½ï¿½Zï¿½ï¿½
		DestroyWindow(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	case WM_TIMER:
		// ESCï¿½Lï¿½[ï¿½ï¿½ï¿½ï¿½ï¿½ÌŒï¿½ï¿½m
		if (GetKeyState(VK_ESCAPE) & 0x8000){
			DestroyWindow(hWnd);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_MOUSEMOVE:
		if (onClip) {
			// ï¿½Vï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Wï¿½ï¿½ï¿½Zï¿½bï¿½g
			clipRect.right  = LOWORD(lParam) + ofX;
			clipRect.bottom = HIWORD(lParam) + ofY;
			
			hdc = GetDC(NULL);
			drawRubberband(hdc, &clipRect, FALSE);

			ReleaseDC(NULL, hdc);
		}
		break;
	

	case WM_LBUTTONDOWN:
		{
			// ï¿½Nï¿½ï¿½ï¿½bï¿½vï¿½Jï¿½n
			onClip = TRUE;
			
			// ï¿½ï¿½ï¿½ï¿½ï¿½Ê’uï¿½ï¿½ï¿½Zï¿½bï¿½g
			clipRect.left = LOWORD(lParam) + ofX;
			clipRect.top  = HIWORD(lParam) + ofY;
			


			// ï¿½}ï¿½Eï¿½Xï¿½ï¿½ï¿½Lï¿½ï¿½ï¿½vï¿½`ï¿½ï¿½
			SetCapture(hWnd);
		}
		break;

	case WM_LBUTTONUP:
		{
			// ï¿½Nï¿½ï¿½ï¿½bï¿½vï¿½Iï¿½ï¿½
			onClip = FALSE;
			
			// ï¿½}ï¿½Eï¿½Xï¿½ÌƒLï¿½ï¿½ï¿½vï¿½`ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			ReleaseCapture();
		
			// ï¿½Vï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Wï¿½ï¿½ï¿½Zï¿½bï¿½g
			clipRect.right  = LOWORD(lParam) + ofX;
			clipRect.bottom = HIWORD(lParam) + ofY;

			// ï¿½ï¿½Ê‚É’ï¿½ï¿½Ú•`ï¿½ï¿½Cï¿½ï¿½ï¿½ÄŒ`
			HDC hdc = GetDC(NULL);

			// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			drawRubberband(hdc, &clipRect, TRUE);

			// ï¿½ï¿½ï¿½Wï¿½`ï¿½Fï¿½bï¿½N
			if ( clipRect.right  < clipRect.left ) {
				int tmp = clipRect.left;
				clipRect.left   = clipRect.right;
				clipRect.right  = tmp;
			}
			if ( clipRect.bottom < clipRect.top  ) {
				int tmp = clipRect.top;
				clipRect.top    = clipRect.bottom;
				clipRect.bottom = tmp;
			}
			
			// ï¿½æ‘œï¿½ÌƒLï¿½ï¿½ï¿½vï¿½`ï¿½ï¿½
			int iWidth, iHeight;
			iWidth  = clipRect.right  - clipRect.left + 1;
			iHeight = clipRect.bottom - clipRect.top  + 1;

			if(iWidth == 0 || iHeight == 0) {
				// ï¿½æ‘œï¿½É‚È‚ï¿½ï¿½Ä‚È‚ï¿½, ï¿½È‚É‚ï¿½ï¿½ï¿½ï¿½È‚ï¿½
				ReleaseDC(NULL, hdc);
				DestroyWindow(hWnd);
				break;
			}

			// ï¿½rï¿½bï¿½gï¿½}ï¿½bï¿½vï¿½oï¿½bï¿½tï¿½@ï¿½ï¿½ï¿½ì¬
			HBITMAP newBMP = CreateCompatibleBitmap(hdc, iWidth, iHeight);
			HDC	    newDC  = CreateCompatibleDC(hdc);
			
			// ï¿½Ö˜Aï¿½Ã‚ï¿½
			SelectObject(newDC, newBMP);

			// ï¿½æ‘œï¿½ï¿½ï¿½æ“¾
			BitBlt(newDC, 0, 0, iWidth, iHeight, 
				hdc, clipRect.left, clipRect.top, SRCCOPY);
			
			// ï¿½Eï¿½Bï¿½ï¿½ï¿½hï¿½Eï¿½ï¿½ï¿½Bï¿½ï¿½!
			ShowWindow(hWnd, SW_HIDE);
			/*
			// ï¿½æ‘œï¿½ï¿½ï¿½Nï¿½ï¿½ï¿½bï¿½vï¿½{ï¿½[ï¿½hï¿½ÉƒRï¿½sï¿½[
			if ( OpenClipboard(hWnd) ) {
				// ï¿½ï¿½ï¿½ï¿½
				EmptyClipboard();
				// ï¿½Zï¿½bï¿½g
				SetClipboardData(CF_BITMAP, newBMP);
				// ï¿½Â‚ï¿½ï¿½ï¿½
				CloseClipboard();
			}
			*/
			
			// ï¿½eï¿½ï¿½ï¿½|ï¿½ï¿½ï¿½ï¿½ï¿½tï¿½@ï¿½Cï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);
			
			if (savePNG(tmpFile, newBMP)) {

				// ï¿½ï¿½ï¿½ï¿½
				if (!uploadFile(hWnd, tmpFile)) {
					// ï¿½Aï¿½bï¿½vï¿½ï¿½ï¿½[ï¿½hï¿½ÉŽï¿½ï¿½s...
					// ï¿½Gï¿½ï¿½ï¿½[ï¿½ï¿½ï¿½bï¿½Zï¿½[ï¿½Wï¿½ÍŠï¿½ï¿½É•\ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä‚ï¿½ï¿½ï¿½

					/*
					TCHAR sysDir[MAX_PATH];
					if (SUCCEEDED(StringCchCopy(sysDir, MAX_PATH, tmpFile)) &&
						SUCCEEDED(StringCchCat(sysDir, MAX_PATH, _T(".png")))) {
						
						MoveFile(tmpFile, sysDir);
						SHELLEXECUTEINFO lsw = {0};
						
						lsw.hwnd	= hWnd;
						lsw.cbSize	= sizeof(SHELLEXECUTEINFO);
						lsw.lpVerb	= _T("open");
						lsw.lpFile	= sysDir;

						ShellExecuteEx(&lsw);
					}
					*/
				}
			} else {
				// PNGï¿½Û‘ï¿½ï¿½ï¿½ï¿½s...
				MessageBox(hWnd, _T("Cannot save png image"), szTitle, 
					MB_OK | MB_ICONERROR);
			}

			// ï¿½ï¿½nï¿½ï¿½
			DeleteFile(tmpFile);
			
			DeleteDC(newDC);
			DeleteObject(newBMP);

			ReleaseDC(NULL, hdc);
			DestroyWindow(hWnd);
			PostQuitMessage(0);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ï¿½Nï¿½ï¿½ï¿½bï¿½vï¿½{ï¿½[ï¿½hï¿½É•ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Rï¿½sï¿½[
VOID setClipBoardText(const char* str)
{

	HGLOBAL hText;
	char    *pText;
	size_t  slen;

	slen  = strlen(str) + 1; // NULL

	hText = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, slen * sizeof(TCHAR));

	pText = (char *)GlobalLock(hText);
	strncpy_s(pText, slen, str, slen);
	GlobalUnlock(hText);
	
	// ï¿½Nï¿½ï¿½ï¿½bï¿½vï¿½{ï¿½[ï¿½hï¿½ï¿½ï¿½Jï¿½ï¿½
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hText);
	CloseClipboard();

	// ï¿½ï¿½ï¿½
	GlobalFree(hText);
}

// ï¿½wï¿½è‚³ï¿½ê‚½ URL (char*) ï¿½ï¿½ï¿½uï¿½ï¿½ï¿½Eï¿½Uï¿½ÅŠJï¿½ï¿½
VOID execUrl(const char* str)
{
	size_t  slen;
	size_t  dcount;
	slen  = strlen(str) + 1; // NULL

	TCHAR *wcUrl = (TCHAR *)malloc(slen * sizeof(TCHAR));
	
	// ï¿½ï¿½ï¿½Cï¿½hï¿½ï¿½ï¿½ï¿½ï¿½É•ÏŠï¿½
	mbstowcs_s(&dcount, wcUrl, slen, str, slen);
	
	// open ï¿½Rï¿½}ï¿½ï¿½ï¿½hï¿½ï¿½ï¿½ï¿½ï¿½s
	SHELLEXECUTEINFO lsw = {0};
	lsw.cbSize = sizeof(SHELLEXECUTEINFO);
	lsw.lpVerb = _T("open");
	lsw.lpFile = wcUrl;

	ShellExecuteEx(&lsw);

	free(wcUrl);
}

// ID ï¿½ð¶ï¿½ï¿½Eï¿½ï¿½ï¿½[ï¿½hï¿½ï¿½ï¿½ï¿½
std::string getId()
{

    TCHAR idFile[_MAX_PATH];
	TCHAR idDir[_MAX_PATH];

    SHGetSpecialFolderPath( NULL, idFile, CSIDL_APPDATA, FALSE );

	 _tcscat_s( idFile, _T("\\Gyazo"));
	 _tcscpy_s( idDir, idFile);
	 _tcscat_s( idFile, _T("\\id.txt"));

	const TCHAR*	 idOldFile			= _T("id.txt");
	BOOL oldFileExist = FALSE;

	std::string idStr;

	// ï¿½Ü‚ï¿½ï¿½Íƒtï¿½@ï¿½Cï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ID ï¿½ï¿½ï¿½ï¿½ï¿½[ï¿½h
	std::ifstream ifs;

	ifs.open(idFile);
	if (! ifs.fail()) {
		// ID ï¿½ï¿½Ç‚Ýï¿½ï¿½ï¿½
		ifs >> idStr;
		ifs.close();
	} else{		
		std::ifstream ifsold;
		ifsold.open(idOldFile);
		if (! ifsold.fail()) {
			// ï¿½ï¿½ï¿½ï¿½fï¿½Bï¿½ï¿½ï¿½Nï¿½gï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ID ï¿½ï¿½Ç‚Ýï¿½ï¿½ï¿½(ï¿½ï¿½ï¿½oï¿½[ï¿½Wï¿½ï¿½ï¿½ï¿½ï¿½Æ‚ÌŒÝŠï¿½ï¿½ï¿½)
			ifsold >> idStr;
			ifsold.close();
		}
	}

	return idStr;
}

// Save ID
BOOL saveId(const WCHAR* str)
{

    TCHAR idFile[_MAX_PATH];
	TCHAR idDir[_MAX_PATH];

    SHGetSpecialFolderPath( NULL, idFile, CSIDL_APPDATA, FALSE );

	 _tcscat_s( idFile, _T("\\Gyazo"));
	 _tcscpy_s( idDir, idFile);
	 _tcscat_s( idFile, _T("\\id.txt"));

	const TCHAR*	 idOldFile			= _T("id.txt");

	size_t  slen;
	size_t  dcount;
	slen  = _tcslen(str) + 1; // NULL

	char *idStr = (char *)malloc(slen * sizeof(char));
	// ï¿½oï¿½Cï¿½gï¿½ï¿½ï¿½ï¿½ï¿½É•ÏŠï¿½
	wcstombs_s(&dcount, idStr, slen, str, slen);

	// ID ï¿½ï¿½Û‘ï¿½ï¿½ï¿½ï¿½ï¿½
	CreateDirectory(idDir,NULL);
	std::ofstream ofs;
	ofs.open(idFile);
	if (! ofs.fail()) {
		ofs << idStr;
		ofs.close();

		// ï¿½ï¿½ï¿½Ý’ï¿½tï¿½@ï¿½Cï¿½ï¿½ï¿½Ìíœ
		if (PathFileExists(idOldFile)){
			DeleteFile(idOldFile);
		}
	}else{
		free(idStr);
		return FALSE;
	}

	free(idStr);
	return TRUE;
}

// PNG ï¿½tï¿½@ï¿½Cï¿½ï¿½ï¿½ï¿½ï¿½Aï¿½bï¿½vï¿½ï¿½ï¿½[ï¿½hï¿½ï¿½ï¿½ï¿½.
BOOL uploadFile(HWND hwnd, LPCTSTR fileName)
{
	const TCHAR* UPLOAD_SERVER	= _T("pomf.se");
	const TCHAR* UPLOAD_PATH = _T("blackniggers/kittens.php");

	const char*  sBoundary = "----BOUNDARYBOUNDARY----";		// boundary
	const char   sCrLf[]   = { 0xd, 0xa, 0x0 };					// ï¿½ï¿½ï¿½s(CR+LF)g
	const TCHAR* szHeader  = 
		_T("Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----");

	std::ostringstream	buf;	// ï¿½ï¿½ï¿½Mï¿½ï¿½ï¿½bï¿½Zï¿½[ï¿½W
	std::string			idStr;	// ID
	
	// ID ï¿½ï¿½ï¿½æ“¾
	idStr = getId();

	// ï¿½ï¿½ï¿½bï¿½Zï¿½[ï¿½Wï¿½Ì\ï¿½ï¿½
	// -- "id" part
	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"id\"";
	buf << sCrLf;
	buf << sCrLf;
	buf << idStr;
	buf << sCrLf;

	// -- "imagedata" part
	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"imagedata\"; filename=\"gyazo.com\"";
	buf << sCrLf;
	//buf << "Content-type: image/png";	// ï¿½ê‰ž
	//buf << sCrLf;
	buf << sCrLf;

	// ï¿½{ï¿½ï¿½: PNG ï¿½tï¿½@ï¿½Cï¿½ï¿½ï¿½ï¿½Ç‚Ýï¿½ï¿½ï¿½
	std::ifstream png;
	png.open(fileName, std::ios::binary);
	if (png.fail()) {
		MessageBox(hwnd, _T("PNG open failed"), szTitle, MB_ICONERROR | MB_OK);
		png.close();
		return FALSE;
	}
	buf << png.rdbuf();		// read all & append to buffer
	png.close();

	// ï¿½ÅŒï¿½
	buf << sCrLf;
	buf << "--";
	buf << sBoundary;
	buf << "--";
	buf << sCrLf;

	// ï¿½ï¿½ï¿½bï¿½Zï¿½[ï¿½Wï¿½ï¿½ï¿½ï¿½
	std::string oMsg(buf.str());

	// WinInet ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ (proxy ï¿½ï¿½ ï¿½Kï¿½ï¿½ÌÝ’ï¿½ð—˜—p)
	HINTERNET hSession    = InternetOpen(szTitle, 
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(NULL == hSession) {
		MessageBox(hwnd, _T("Cannot configure wininet"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}
	
	// ï¿½Ú‘ï¿½ï¿½ï¿½
	HINTERNET hConnection = InternetConnect(hSession, 
		UPLOAD_SERVER, INTERNET_DEFAULT_HTTP_PORT,
		NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	if(NULL == hSession) {
		MessageBox(hwnd, _T("Cannot initiate connection"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// ï¿½vï¿½ï¿½ï¿½ï¿½ÌÝ’ï¿½
	HINTERNET hRequest    = HttpOpenRequest(hConnection,
		_T("POST"), UPLOAD_PATH, NULL,
		NULL, NULL, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, NULL);
	if(NULL == hSession) {
		MessageBox(hwnd, _T("Cannot compose post request"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// User-Agentï¿½ï¿½ï¿½wï¿½ï¿½
	const TCHAR* ua = _T("User-Agent: Gyazowin/1.0\r\n");
	BOOL bResult = HttpAddRequestHeaders(
		hRequest, ua, _tcslen(ua), 
		HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	if (FALSE == bResult) {
		MessageBox(hwnd, _T("Cannot set user agent"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}
	
	// ï¿½vï¿½ï¿½ï¿½ð‘—M
	if (HttpSendRequest(hRequest,
                    szHeader,
					lstrlen(szHeader),
                    (LPVOID)oMsg.c_str(),
					(DWORD) oMsg.length()))
	{
		// ï¿½vï¿½ï¿½ï¿½Íï¿½ï¿½ï¿½
		
		DWORD resLen = 8;
		TCHAR resCode[8];

		// status code ï¿½ï¿½ï¿½æ“¾
		HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE, resCode, &resLen, 0);
		if( _ttoi(resCode) != 200 ) {
			// upload ï¿½ï¿½ï¿½s (status error)
			MessageBox(hwnd, _T("Failed to upload (unexpected result code, under maintainance?)"),
				szTitle, MB_ICONERROR | MB_OK);
		} else {
			// upload succeeded

			// get new id
			DWORD idLen = 100;
			TCHAR newid[100];
			
			memset(newid, 0, idLen*sizeof(TCHAR));	
			_tcscpy_s(newid, _T("X-Gyazo-Id"));

			HttpQueryInfo(hRequest, HTTP_QUERY_CUSTOM, newid, &idLen, 0);
			if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND && idLen != 0) {
				//save new id
				saveId(newid);
			}

			// ï¿½ï¿½ï¿½ï¿½ (URL) ï¿½ï¿½ÇŽï¿½ï¿½
			DWORD len;
			char  resbuf[1024];
			std::string result;
			
			// ï¿½ï¿½ï¿½ï¿½È‚É’ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æ‚Í‚È‚ï¿½ï¿½ï¿½ï¿½Ç‚Ü‚ï¿½ï¿½ê‰ž
			while(InternetReadFile(hRequest, (LPVOID) resbuf, 1024, &len) 
				&& len != 0)
			{
				result.append(resbuf, len);
			}

			// ï¿½æ“¾ï¿½ï¿½ï¿½Ê‚ï¿½ NULL terminate ï¿½ï¿½ï¿½ï¿½Ä‚ï¿½ï¿½È‚ï¿½ï¿½Ì‚ï¿½
			result += '\0';

			// ï¿½Nï¿½ï¿½ï¿½bï¿½vï¿½{ï¿½[ï¿½hï¿½ï¿½ URL ï¿½ï¿½ï¿½Rï¿½sï¿½[
			setClipBoardText(result.c_str());
			
			// URL ï¿½ï¿½ï¿½Nï¿½ï¿½
			execUrl(result.c_str()); 

			return TRUE;
		}
	} else {
		// ï¿½Aï¿½bï¿½vï¿½ï¿½ï¿½[ï¿½hï¿½ï¿½ï¿½s...
		MessageBox(hwnd, _T("Failed to upload"), szTitle, MB_ICONERROR | MB_OK);
	}

	return FALSE;

}
