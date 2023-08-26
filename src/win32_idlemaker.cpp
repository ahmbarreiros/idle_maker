#include <windows.h>

#define  global_variable static
#define  internal static
#define  local_variable static

global_variable BOOL Running;
global_variable void * BitmapMemory;
global_variable BITMAPINFO BitmapInfo;
global_variable HDC BitmapDeviceContext;
global_variable HBITMAP BitmapHandle;


internal void Win32ResizeDIBSection(int Width, int Height) {

  if(BitmapHandle) {
    DeleteObject(BitmapHandle);
  }
  
  if(!BitmapDeviceContext) {
    BitmapDeviceContext = CreateCompatibleDC(0); 
  }
  
  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = Height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  
  BitmapHandle = CreateDIBSection(
				  BitmapDeviceContext,
				  &BitmapInfo,
				  DIB_RGB_COLORS,
				  &BitmapMemory,
				  NULL,
				  0
				  );
}

internal void Win32StretchDIBSection(HDC DeviceContext,
			    int X,
			    int Y,
			    int Width,
			    int Height) {
    StretchDIBits(
		DeviceContext,
		X, Y, Width, Height,
		X, Y, Width, Height,
		BitmapMemory,
		&BitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
		);
}


LRESULT CALLBACK Win32Wndproc(
			      HWND Window,
			      UINT Message,
			      WPARAM wParam,
			      LPARAM lParam
			      ) {
  LRESULT Result = 0;
  switch(Message) {
  case WM_CLOSE:
    {
      DestroyWindow(Window);
      OutputDebugStringA("WM_CLOSE\n");
    } break;
  case WM_PAINT:
    {

      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(Window, &Paint);
      int X = Paint.rcPaint.right;
      int Y = Paint.rcPaint.bottom;
      int Width = Paint.rcPaint.right - Paint.rcPaint.left;
      int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;;
      PatBlt(DeviceContext, X, Y, Width, Height, BLACKNESS);
      EndPaint(Window, &Paint);
      OutputDebugStringA("WM_PAINT\n");     
    } break;
  case WM_SIZE:
    {
      RECT ClientRect;
      GetClientRect(Window, &ClientRect);
      int Width = ClientRect.right - ClientRect.left;
      int Height = ClientRect.bottom - ClientRect.top;
      
      Win32ResizeDIBSection(Width, Height);
      OutputDebugStringA("WM_SIZE\n");
    } break;
  case WM_DESTROY:
    {
      PostQuitMessage(0);
      OutputDebugStringA("WM_DESTROY\n");
    } break;
  default:
    {
      OutputDebugStringA("DEFAULT\n");
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
				   WS_OVERLAPPEDWINDOW|WS_VISIBLE,
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
      Running = true;
      while(Running) {
	MSG Message;
	BOOL bRet = GetMessage(&Message, NULL, 0, 0) != 0;
	if(bRet > 0) {
	  TranslateMessage(&Message);
	  DispatchMessage(&Message);
	} else {
	  Running = 0;
	  break;
	}
      }
    }

  }
    
  return 0;
}
