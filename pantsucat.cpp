#include <windows.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <strsafe.h>
#include <shlobj.h> 
#include "shlwapi.h"
#pragma comment (lib, "Shlwapi.lib")

#include <gdiplus.h>
#pragma comment (lib, "Gdiplus.lib")
using namespace Gdiplus;

#include "wininet.h" 
#pragma comment(lib,"wininet.lib")

#include <time.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>


#define APP_NAME "pantsucat"
#define APP_HOST "pantsu.cat"
#define APP_QUERY "upload.php?output=gyazo"
#define APP_USERAGENT "Gyazowin/1.0"
#define APP_MULTIPART_BOUNDARY "----BOUNDARYBOUNDARY----"
#define APP_MULTIPART_IMG_FILENAME "image.png"
#define APP_TMP_FILENAME "tmp"
#define APP_SECURE 1
#define APP_WINDOWCLASS (UTF16IZE(APP_NAME) L"WIN")
#define APP_WINDOWCLASSL (UTF16IZE(APP_NAME) L"WINL")

#define UTF16IZE_(x) L ## x
#define UTF16IZE(x) UTF16IZE_(x)
#define nil 0


static struct {
	int virtX, virtY;
	HWND layerwnd;
} g_win;

ATOM regclasses(HINSTANCE);
int initwindow(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK LayerWndProc(HWND, UINT, WPARAM, LPARAM);
int GetEncoderClsid(const WCHAR *, CLSID *);
int ispngfile(char *);
void drawrubberband(HWND, HDC, LPRECT, BOOL);
void execurl(char *);
void setclipboardtext(char *);
int convertpng(char *, char *);
int savepng(char *, HBITMAP);
int uploadfile(char *, char *, char *);
std::string getid();
int saveid(wchar_t *);

static int
u8to16(wchar_t *dst, int dstlen, char *src) {
	int len;
	len = strlen(src) + 1;
	return MultiByteToWideChar(CP_UTF8, 0, src, len, dst, dstlen);
}

static int
u16to8(char *dst, int dstlen, wchar_t *src) {
	int len;
	len = wcslen(src) + 1;
	return WideCharToMultiByte(CP_UTF8, 0, src, len, dst, dstlen, nil, nil);
}

static void
printerror(HWND hwnd, char *fmt, ...) {
	char buf[0x1000];
	wchar_t wide[0x1000];
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	u8to16(wide, sizeof(wide), buf);
	MessageBox(hwnd, wide, UTF16IZE(APP_NAME), MB_ICONERROR | MB_OK);
	//MessageBoxA(hwnd, buf, APP_NAME, MB_ICONERROR | MB_OK);
}

static int
wineventloop(void) {
	MSG msg;

	while (GetMessage(&msg, nil, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

static void
cdself(void) {
	wchar_t path[MAX_PATH];
	int len, i;

	len = GetModuleFileName(nil, path, sizeof(path));
	for (i = len; i >= 0; i--) {
		if (path[i] != '\\') continue;
		path[i] = '\0';
		break;
	}

	SetCurrentDirectory(path);
}

static void
newtmpfile(char *buf, int bufsize) {
	wchar_t tmpdir[MAX_PATH], tmpfile[MAX_PATH];
	GetTempPath(sizeof(tmpdir), tmpdir);
	GetTempFileName(tmpdir, UTF16IZE(APP_TMP_FILENAME), 0, tmpfile);
	u16to8(buf, bufsize, tmpfile);
}

static void
deletefile(char *fn) {
	wchar_t wfn[0x1000];
	u8to16(wfn, sizeof(wfn), fn);
	DeleteFile(wfn);
}

static int
getargc(void) {
	return __argc;
}

static wchar_t *
getwarg(int i) {
	return __targv[i];
}

static int
getmime(char *buf, int buflen, char *filename) {
	wchar_t ext[0x20], mime[0x200];
	DWORD mimelen;
	HKEY key;
	int i;

	for (i = strlen(filename); i > 0; i--) {
		if (filename[i] != '.') continue;
		u8to16(ext, sizeof(ext), &filename[i]);
		break;
	}

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, ext, 0, KEY_READ, &key) != ERROR_SUCCESS) {
		return 0;
	}

	if (RegQueryValueEx(key, L"Content Type", nil, nil, (unsigned char *)mime, &mimelen) != ERROR_SUCCESS) {
		RegCloseKey(key);
		return 0;
	}

	u16to8(buf, buflen, mime);
	return (int)mimelen;
}

static char *
getext(char *filename) {
	int i;

	for (i = strlen(filename); i > 0; i--) {
		if (filename[i] != '.') continue;
		return filename + i;
	}

	return nil;
}

int APIENTRY
_tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	cdself();

	switch (getargc()) {
	default:
		regclasses(hInstance);

		if (!initwindow(hInstance, nCmdShow)) {
			printerror(nil, "cannot create window");
			return 1;
		}

		return wineventloop();

	case 2:
		char filename[0x1000];
		u16to8(filename, sizeof(filename), getwarg(1));
		char mime[0x100] = "application/octet-stream";
		char *ext = getext(filename);

		getmime(mime, sizeof(mime), filename);
		uploadfile(filename, ext, mime);
		return 0;
	}
}

int
ispng(unsigned char head[8]) {
	unsigned char valid[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
	int i;

	for (i = 0; i < 8; i++) {
		if (head[i] == valid[i]) continue;
		return 0;
	}
}

int
ispngfile(char *fn) {
	wchar_t wfn[0x1000];
	unsigned char head[8];
	FILE *fd;
	int n;

	u8to16(wfn, sizeof(wfn), fn);
	if (0 == (fd = _wfopen(wfn, L"rb"))) {
		return 0;
	}

	if (8 != (n = fread(head, 1, 8, fd))) {
		return 0;
	}

	fclose(fd);

	return ispng(head);
}

static ATOM
regclass(HINSTANCE inst, wchar_t *name, WNDPROC func, UINT style, HBRUSH bg) {
	WNDCLASS wc;

	wc.style = style;
	wc.lpfnWndProc = func;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = inst;
	wc.hIcon = nil;// LoadIcon(inst, MAKEINTRESOURCE(nil));
	wc.hCursor = LoadCursor(nil, IDC_CROSS);
	wc.hbrBackground = bg;
	wc.lpszMenuName = 0;
	wc.lpszClassName = name;

	return RegisterClass(&wc);
}

ATOM
regclasses(HINSTANCE inst) {
	regclass(
		inst, APP_WINDOWCLASS, WndProc,
		0, 0
	);
	return regclass(
		inst, APP_WINDOWCLASSL, LayerWndProc,
		CS_HREDRAW | CS_VREDRAW, (HBRUSH)GetStockObject(WHITE_BRUSH)
	);
}

int
initwindow(HINSTANCE inst, int nCmdShow) {
	HWND hWnd;
	int x, y, w, h;

	x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	g_win.virtX = x; g_win.virtY = y;

	hWnd = CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if(_WIN32_WINNT >= 0x0500)
		| WS_EX_NOACTIVATE
#endif
		,
		APP_WINDOWCLASS, nil, WS_POPUP,
		0, 0, 0, 0,
		nil, nil, inst, nil
	);

	if (!hWnd) {
		return 0;
	}

	MoveWindow(hWnd, x, y, w, h, 0);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	SetTimer(hWnd, 1, 100, nil);

	g_win.layerwnd = CreateWindowEx(
		WS_EX_TOOLWINDOW
#if(_WIN32_WINNT >= 0x0500)
		| WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
		,
		APP_WINDOWCLASSL, nil, WS_POPUP,
		100, 100, 300, 300,
		hWnd, nil, inst, nil
	);

	SetLayeredWindowAttributes(g_win.layerwnd, RGB(255, 0, 0), 100, LWA_COLORKEY | LWA_ALPHA);

	return 1;
}

int
GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
	ImageCodecInfo* info = nil;
	UINT num = 0;
	UINT size = 0;

	GetImageEncodersSize(&num, &size);
	if (size == 0) {
		return -1;
	}

	if (nil == (info = (ImageCodecInfo *)(malloc(size)))) {
		return -1;
	}

	GetImageEncoders(num, size, info);
	for (UINT j = 0; j < num; ++j) {
		if (wcscmp(info[j].MimeType, format) != 0) continue;

		*pClsid = info[j].Clsid;

		free(info);
		return j;
	}

	free(info);
	return -1;
}

void
drawrubberband(HDC hdc, LPRECT newRect, BOOL erase) {
	static BOOL firstDraw = 1;
	static RECT lastRect = { 0 };
	static RECT clipRect = { 0 };

	if (firstDraw) {
		ShowWindow(g_win.layerwnd, SW_SHOW);
		UpdateWindow(g_win.layerwnd);

		firstDraw = 0;
	}

	if (erase) {
		ShowWindow(g_win.layerwnd, SW_HIDE);
	}

	clipRect = *newRect;
	if (clipRect.right  < clipRect.left) {
		int tmp = clipRect.left;
		clipRect.left = clipRect.right;
		clipRect.right = tmp;
	}
	if (clipRect.bottom < clipRect.top) {
		int tmp = clipRect.top;
		clipRect.top = clipRect.bottom;
		clipRect.bottom = tmp;
	}

	MoveWindow(
		g_win.layerwnd,
		clipRect.left, clipRect.top,
		clipRect.right - clipRect.left + 1, clipRect.bottom - clipRect.top + 1,
		1
	);
}

int
convertpng(char *dst, char *src) {
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	CLSID clsidEncoder;
	wchar_t wdst[MAX_PATH], wsrc[MAX_PATH];
	u8to16(wdst, sizeof(wdst), dst);
	u8to16(wsrc, sizeof(wsrc), src);

	int result = 0;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nil);
	Image *b = new Image(wsrc, 0);

	if (0 == b->GetLastStatus()
		&& GetEncoderClsid(L"image/png", &clsidEncoder)
		&& 0 == b->Save(wdst, &clsidEncoder, 0)
		) {
		result = 1;
	}

	delete b;
	GdiplusShutdown(gdiplusToken);

	return result;
}

int
savepng(char *filename, HBITMAP bmp) {
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	CLSID clsidEncoder;
	wchar_t wfilename[MAX_PATH];
	u8to16(wfilename, sizeof(wfilename), filename);

	int result = 0;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nil);
	Bitmap *b = new Bitmap(bmp, nil);

	if (GetEncoderClsid(L"image/png", &clsidEncoder)
		&& 0 == b->Save(wfilename, &clsidEncoder, 0)
		) {
		result = 1;
	}

	delete b;
	GdiplusShutdown(gdiplusToken);

	return result;
}

LRESULT CALLBACK
LayerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	RECT clipRect = { 0, 0, 500, 500 };
	HBRUSH hBrush;
	HPEN hPen;
	HFONT hFont;

	switch (message) {
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_ERASEBKGND:
		GetClientRect(hWnd, &clipRect);

		hdc = GetDC(hWnd);
		hBrush = CreateSolidBrush(RGB(100, 100, 100));
		SelectObject(hdc, hBrush);
		hPen = CreatePen(PS_DASH, 1, RGB(255, 255, 255));
		SelectObject(hdc, hPen);
		Rectangle(hdc, 0, 0, clipRect.right, clipRect.bottom);

		int fHeight;
		fHeight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		hFont = CreateFontA(
			fHeight,
			0,
			0,
			0,
			FW_REGULAR,
			0,
			0,
			0,
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			PROOF_QUALITY,
			FIXED_PITCH | FF_MODERN,
			"Tahoma"
		);

		SelectObject(hdc, hFont);

		int iWidth, iHeight;
		iWidth = clipRect.right - clipRect.left;
		iHeight = clipRect.bottom - clipRect.top;

		char strwidth[200], strheight[200];
		sprintf(strwidth, "%d", iWidth);
		sprintf(strheight, "%d", iHeight);

		int w, h, h2;
		w = (int)(-fHeight * 2.5 + 8);
		h = (int)(-fHeight * 2 + 8);
		h2 = h + fHeight;

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(0, 0, 0));
		TextOutA(hdc, clipRect.right - w + 1, clipRect.bottom - h + 1, strwidth, strlen(strwidth));
		TextOutA(hdc, clipRect.right - w + 1, clipRect.bottom - h2 + 1, strheight, strlen(strheight));
		SetTextColor(hdc, RGB(255, 255, 255));
		TextOutA(hdc, clipRect.right - w, clipRect.bottom - h, strwidth, strlen(strwidth));
		TextOutA(hdc, clipRect.right - w, clipRect.bottom - h2, strwidth, strlen(strwidth));

		DeleteObject(hPen);
		DeleteObject(hBrush);
		DeleteObject(hFont);
		ReleaseDC(hWnd, hdc);

		return 1;
	}

	return 0;
}

LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	HDC hdc;

	static BOOL onClip = 0;
	static BOOL firstDraw = 1;
	static RECT clipRect = { 0, 0, 0, 0 };

	switch (message) {
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_RBUTTONDOWN:
		DestroyWindow(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_TIMER:
		if (GetKeyState(VK_ESCAPE) & 0x8000) {
			DestroyWindow(hWnd);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_MOUSEMOVE:
		if (!onClip) {
			break;
		}
		clipRect.right = LOWORD(lParam) + g_win.virtX;
		clipRect.bottom = HIWORD(lParam) + g_win.virtY;

		hdc = GetDC(nil);
		drawrubberband(hdc, &clipRect, 0);
		ReleaseDC(nil, hdc);
		break;

	case WM_LBUTTONDOWN:
		onClip = 1;

		clipRect.left = LOWORD(lParam) + g_win.virtX;
		clipRect.top = HIWORD(lParam) + g_win.virtY;

		SetCapture(hWnd);
		break;

	case WM_LBUTTONUP:
		onClip = 0;

		ReleaseCapture();

		clipRect.right = LOWORD(lParam) + g_win.virtX;
		clipRect.bottom = HIWORD(lParam) + g_win.virtY;

		HDC hdc = GetDC(nil);
		drawrubberband(hdc, &clipRect, 1);

		if (clipRect.right  < clipRect.left) {
			int tmp = clipRect.left;
			clipRect.left = clipRect.right;
			clipRect.right = tmp;
		}
		if (clipRect.bottom < clipRect.top) {
			int tmp = clipRect.top;
			clipRect.top = clipRect.bottom;
			clipRect.bottom = tmp;
		}

		int iWidth, iHeight;
		iWidth = clipRect.right - clipRect.left + 1;
		iHeight = clipRect.bottom - clipRect.top + 1;

		if (iWidth == 0 || iHeight == 0) {

			ReleaseDC(nil, hdc);
			DestroyWindow(hWnd);
			break;
		}

		HBITMAP bmp = CreateCompatibleBitmap(hdc, iWidth, iHeight);
		HDC newDC = CreateCompatibleDC(hdc);

		SelectObject(newDC, bmp);
		BitBlt(newDC, 0, 0, iWidth, iHeight, hdc, clipRect.left, clipRect.top, SRCCOPY);
		ShowWindow(hWnd, SW_HIDE);

		char tmpfile[0x600];
		newtmpfile(tmpfile, sizeof(tmpfile));

		if (savepng(tmpfile, bmp)) {
			uploadfile(tmpfile, APP_MULTIPART_IMG_FILENAME, "image/png");
		} else {
			printerror(hWnd, "cannot save png image");
		}

		deletefile(tmpfile);

		DeleteDC(newDC);
		DeleteObject(bmp);

		ReleaseDC(nil, hdc);
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	}

	return 0;
}

static void
setclipboardbmp(HBITMAP bmp) {
	OpenClipboard(nil);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, bmp);
	CloseClipboard();
}

void
setclipboardtext(const char *str) {
	HGLOBAL block;
	char *p;
	size_t slen;

	slen = strlen(str) + 1;

	block = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, slen * sizeof(TCHAR));

	p = (char *)GlobalLock(block);
	strncpy_s(p, slen, str, slen);
	GlobalUnlock(block);

	OpenClipboard(nil);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, block);
	CloseClipboard();

	GlobalFree(block);
}

void
execUrl(const char* str) {
	size_t slen;
	size_t dcount;
	slen = strlen(str) + 1;

	TCHAR *wcUrl = (TCHAR *)malloc(slen * sizeof(TCHAR));

	mbstowcs_s(&dcount, wcUrl, slen, str, slen);

	SHELLEXECUTEINFO lsw = { 0 };
	lsw.cbSize = sizeof(SHELLEXECUTEINFO);
	lsw.lpVerb = _T("open");
	lsw.lpFile = wcUrl;

	ShellExecuteEx(&lsw);

	free(wcUrl);
}

std::string
getId() {
	wchar_t idfile[MAX_PATH], iddir[MAX_PATH];

	SHGetSpecialFolderPath(nil, idfile, CSIDL_APPDATA, 0);

	wcscat(idfile, L"\\" UTF16IZE(APP_NAME));
	wcscpy(iddir, idfile);
	wcscat(idfile, L"\\id.txt");

	wchar_t *oldidfile = L"id.txt";

	std::string idstr;
	std::ifstream ifs;

	ifs.open(idfile);
	if (!ifs.fail()) {
		ifs >> idstr;
		ifs.close();
		return idstr;
	}

	std::ifstream ifsold;
	ifsold.open(oldidfile);
	if (ifsold.fail()) {
		return idstr;
	}

	ifsold >> idstr;
	ifsold.close();

	return idstr;
}

int
saveId(wchar_t *str) {
	wchar_t idfile[MAX_PATH], iddir[MAX_PATH];

	SHGetSpecialFolderPath(nil, idfile, CSIDL_APPDATA, 0);

	wcscat(idfile, L"\\" UTF16IZE(APP_NAME));
	wcscpy(iddir, idfile);
	wcscat(idfile, L"\\id.txt");

	wchar_t *oldidfile = L"id.txt";

	size_t  slen;
	size_t  dcount;
	slen = _tcslen(str) + 1;

	char *idstr = (char *)malloc(slen * sizeof(char));

	wcstombs_s(&dcount, idstr, slen, str, slen);

	CreateDirectory(iddir, nil);
	std::ofstream ofs;
	ofs.open(idfile);
	if (ofs.fail()) {
		free(idstr);
		return 0;
	}

	ofs << idstr;
	ofs.close();
	if (PathFileExists(oldidfile)) {
		DeleteFile(oldidfile);
	}

	free(idstr);
	return 1;
}

static void
fillmultipartlong(std::ostringstream& buf, char *idstr, char *name, char *mime, std::ifstream& data) {
	char *disposition = "Content-Disposition: form-data; ";
	char *target = "files[]";
	char *crlf = "\r\n";

	buf << "--" << APP_MULTIPART_BOUNDARY << crlf;
	buf << disposition << "name=\"id\"" << crlf;
	buf << crlf;
	buf << idstr;
	buf << crlf;

	buf << "--" << APP_MULTIPART_BOUNDARY << crlf;
	buf << disposition << "name=\"" << target << "\"; filename=\"" << name << "\"" << crlf;
	buf << "Content-type: " << mime << crlf;
	buf << crlf;
	buf << data.rdbuf();
	buf << crlf;

	buf << "--" << APP_MULTIPART_BOUNDARY << "--" << crlf;
}

static HINTERNET
serverpost(char *host, char *query, int secure, void *data, int len) {
	wchar_t *header = L"Content-Type: multipart/form-data; boundary=" UTF16IZE(APP_MULTIPART_BOUNDARY);
	INTERNET_PORT port = secure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
	DWORD secflag = secure ? INTERNET_FLAG_SECURE : 0;
	wchar_t whost[0x100], wquery[0x100];
	u8to16(whost, sizeof(whost), host);
	u8to16(wquery, sizeof(wquery), query);

	HINTERNET session = InternetOpen(whost, INTERNET_OPEN_TYPE_PRECONFIG, nil, nil, 0);
	if (!session) {
		printerror(nil, "cannot configure wininet");
		return nil;
	}

	HINTERNET conn = InternetConnect(session, whost, port, nil, nil, INTERNET_SERVICE_HTTP, 0, nil);
	if (!conn) {
		printerror(nil, "cannot initiate connection");
		return nil;
	}

	HINTERNET req = HttpOpenRequest(
		conn, L"POST", wquery, nil, nil, nil,
		INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD | secflag, nil
	);

	if (!req) {
		printerror(nil, "cannot compose post request");
		return nil;
	}

	wchar_t *ua = L"User-Agent: "  UTF16IZE(APP_USERAGENT) L"\r\n";
	if (!HttpAddRequestHeaders(
		req, ua, wcslen(ua),
		HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE)
		) {
		printerror(nil, "cannot set user-agent");
		return nil;
	}

	if (!HttpSendRequest(
		req,
		header,
		wcslen(header),
		(LPVOID)data,
		(DWORD)len)
		) {
		printerror(nil, "failed to upload");
		return nil;
	}

	return req;
}

static int
ifstreamopen(std::ifstream& stream, char *filename) {
	wchar_t wfilename[0x400];
	u8to16(wfilename, sizeof(wfilename), filename);
	stream.open(wfilename, std::ios::binary);
	if (stream.fail()) {
		stream.close();
		return 0;
	}
	return 1;
}

int
uploadfile(char *filename, char *name, char *mime) {
	std::ostringstream buf;
	std::string idstr;

	idstr = getId();

	std::ifstream img;
	if (!ifstreamopen(img, filename)) {
		img.close();
		printerror(nil, "cannot open image");
		return 0;
	}
	fillmultipartlong(buf, (char *)idstr.c_str(), name, mime, img);
	img.close();

	std::string body(buf.str());

	HINTERNET req = serverpost(APP_HOST, APP_QUERY, APP_SECURE, (void *)body.c_str(), body.length());
	if (!req) {
		return 0;
	}

	DWORD codelen = 8;
	wchar_t code[8];

	HttpQueryInfo(req, HTTP_QUERY_STATUS_CODE, code, &codelen, 0);
	if (_wtoi(code) != 200) {
		printerror(nil, "server returned error %d", _wtoi(code));
		return 0;
	}

	DWORD newidlen = 100;
	wchar_t newid[100];

	memset(newid, 0, newidlen*sizeof(TCHAR));
	wcscpy(newid, L"X-Gyazo-Id");

	HttpQueryInfo(req, HTTP_QUERY_CUSTOM, newid, &newidlen, 0);
	if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND && newid != 0) {
		saveId(newid);
	}

	DWORD len;
	char respbuf[1024];
	std::string result;

	while (InternetReadFile(req, (LPVOID)respbuf, 1024, &len) && len != 0) {
		result.append(respbuf, len);
	}

	result += '\0';
	setclipboardtext(result.c_str());
	execUrl(result.c_str());

	return 1;
}
