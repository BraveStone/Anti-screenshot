#ifndef __ANTI_SCREENSHOTLIB_H__
#define __ANTI_SCREENSHOTLIB_H__


#ifndef ANTISCREENSHOTLIBAPI 
#define ANTISCREENSHOTLIBAPI extern "C" __declspec(dllimport)
#endif


///////////////////////////////////////////////////////////////////////////////


ANTISCREENSHOTLIBAPI BOOL WINAPI AntiScreenshotLib_HookAllApps(BOOL bInstall,
   DWORD dwThreadId);


#endif // !__ANTI_SCREENSHOTLIB_H__

//////////////////////////////// End of File //////////////////////////////////
