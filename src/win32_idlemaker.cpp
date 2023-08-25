#include <windows.h>


LRESULT CALLBACK Win32Wndproc(
			      HWND Window,
			      UINT Message,
			      WPARAM wParam,
			      LPARAM lParam
			      ) {
  LRESULT Result = 0;
  switch(Message) {
  case WM_CREATE:
    {
      OutputDebugStringA("WM_SIZE\n");
    } break;
  case WM_PAINT:
    {
      OutputDebugStringA("WM_SIZE\n");     
    } break;
  case WM_SIZE:
    {
      OutputDebugStringA("WM_SIZE\n");
    } break;
  case WM_DESTROY:
    {
      OutputDebugStringA("WM_SIZE\n");
    } break;
  default:
    {
      OutputDebugStringA("WM_SIZE\n");
      Result = DefWindowProc(Window, Message, wParam, lParam);
    } break;
  }
  return Result;
}


int WINAPI WinMain(
		   HINSTANCE Instance,
		   HINSTANCE PrevInstance,
		   LPSTR     CmdLine,
		   int       ShowCmd
		   ) {
  WNDCLASSA WindowClass = {};
  WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
  WindowClass.lpfnWndProc = Win32Wndproc;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "IdleMakerWindowClass";

  if(RegisterClassA(&WindowClass)) {
    HWND WindowHandle;
    WindowHandle = CreateWindowExA(
				   0,
				   WindowClass.lpszClassName,
				   "IdleMaker",
				   WS_OVERLAPPED|WS_VISIBLE,
				   CW_USEDEFAULT,
				   CW_USEDEFAULT,
				   CW_USEDEFAULT,
				   CW_USEDEFAULT,
				   0,
				   0,
				   Instance,
				   0
				   );
    if(WindowHandle) {
      for(;;) {
	MSG Message;
	BOOL bRet = GetMessage(&Message, NULL, 0, 0) != 0;
	if(bRet != -1) {
	  TranslateMessage(&Message);
	  DispatchMessage(&Message);
	} else {
	  break;
	}
      }
    }

  }
    
  return 0;
}
