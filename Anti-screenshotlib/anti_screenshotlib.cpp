
#include "..\CommonFiles\CmnHdr.h"
#include <WindowsX.h>
#include <tchar.h>
#include <stdio.h>
#include "APIHook.h"

#define ANTISCREENSHOTLIBAPI extern "C" __declspec(dllexport)
#include "anti_screenshotlib.h"
#include <StrSafe.h>


///////////////////////////////////////////////////////////////////////////////


// Prototypes for the hooked functions
typedef BOOL(WINAPI *PFNBITBLT)(_In_ HDC hdc, _In_ int x, _In_ int y, _In_ int cx,
	_In_ int cy, _In_opt_ HDC hdcSrc, _In_ int x1, _In_ int y1, _In_ DWORD rop);

// We need to reference these variables before we create them.
extern CAPIHook g_BitBlt;


///////////////////////////////////////////////////////////////////////////////


// This function sends the MessageBox info to our main dialog box
void SendLastMsgBoxInfo(BOOL bUnicode, 
   PVOID pvCaption, PVOID pvText, int nResult) {

   // Get the pathname of the process displaying the message box
   wchar_t szProcessPathname[MAX_PATH];
   GetModuleFileNameW(NULL, szProcessPathname, MAX_PATH);

   // Convert the return value into a human-readable string
   PCWSTR pszResult = L"(Unknown)";
   switch (nResult) {
      case IDOK:       pszResult = L"Ok";        break;
      case IDCANCEL:   pszResult = L"Cancel";    break;
      case IDABORT:    pszResult = L"Abort";     break;
      case IDRETRY:    pszResult = L"Retry";     break;
      case IDIGNORE:   pszResult = L"Ignore";    break;
      case IDYES:      pszResult = L"Yes";       break;
      case IDNO:       pszResult = L"No";        break;
      case IDCLOSE:    pszResult = L"Close";     break;
      case IDHELP:     pszResult = L"Help";      break;
      case IDTRYAGAIN: pszResult = L"Try Again"; break;
      case IDCONTINUE: pszResult = L"Continue";  break;
   }

   // Construct the string to send to the main dialog box
   wchar_t sz[2048];
   StringCchPrintfW(sz, _countof(sz), bUnicode 
      ? L"Process: (%d) %s\r\nCaption: %s\r\nMessage: %s\r\nResult: %s"
      : L"Process: (%d) %s\r\nCaption: %S\r\nMessage: %S\r\nResult: %s",
      GetCurrentProcessId(), szProcessPathname,
      pvCaption, pvText, pszResult);

   // Send the string to the main dialog box
   COPYDATASTRUCT cds = { 0, ((DWORD)wcslen(sz) + 1) * sizeof(wchar_t), sz };
   FORWARD_WM_COPYDATA(FindWindow(NULL, TEXT("Anti screenshot info")), 
      NULL, &cds, SendMessage);
}


///////////////////////////////////////////////////////////////////////////////

BOOL WINAPI Hook_BitBlt(_In_ HDC hdc, _In_ int x, _In_ int y, _In_ int cx, _In_ int cy, _In_opt_ HDC hdcSrc, _In_ int x1, _In_ int y1, _In_ DWORD rop)
{
	HWND hWnd = WindowFromDC(hdcSrc);
	HWND hDeskWnd = GetDesktopWindow();
	HDC clipedDC = NULL;//用来暂存要被传给剪贴板的截图
	if (hWnd == hDeskWnd)
	{
		clipedDC = CreateCompatibleDC(NULL);
		if (clipedDC) {
			hdcSrc = clipedDC;
			SendLastMsgBoxInfo(FALSE, (PVOID)("BitBlt"), (PVOID)("----------------"), 55);
		}
	}
	BOOL bRet = ((PFNBITBLT)(PROC)g_BitBlt)
		(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
	if (clipedDC)
		DeleteDC(clipedDC);
	return bRet;
}


///////////////////////////////////////////////////////////////////////////////

CAPIHook g_BitBlt("gdi32.dll", "BitBlt",
(PROC)Hook_BitBlt);


HHOOK g_hhook = NULL;


///////////////////////////////////////////////////////////////////////////////


static LRESULT WINAPI GetMsgProc(int code, WPARAM wParam, LPARAM lParam) {
   return(CallNextHookEx(g_hhook, code, wParam, lParam));
}


///////////////////////////////////////////////////////////////////////////////


// Returns the HMODULE that contains the specified memory address
static HMODULE ModuleFromAddress(PVOID pv) {

   MEMORY_BASIC_INFORMATION mbi;
   return((VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) 
      ? (HMODULE) mbi.AllocationBase : NULL);
}


///////////////////////////////////////////////////////////////////////////////


BOOL WINAPI AntiScreenshotLib_HookAllApps(BOOL bInstall, DWORD dwThreadId) {

   BOOL bOk;

   if (bInstall) {

      chASSERT(g_hhook == NULL); // Illegal to install twice in a row

      // Install the Windows' hook
      g_hhook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, 
         ModuleFromAddress(AntiScreenshotLib_HookAllApps), dwThreadId);

      bOk = (g_hhook != NULL);
   } else {

      chASSERT(g_hhook != NULL); // Can't uninstall if not installed
      bOk = UnhookWindowsHookEx(g_hhook);
      g_hhook = NULL;
   }

   return(bOk);
}


//////////////////////////////// End of File //////////////////////////////////
