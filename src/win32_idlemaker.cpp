#include <windows.h>
#include <stdint.h>

#define  global_variable static
#define  internal static
#define  local_variable static

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;


typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global_variable bool Running;

struct win32_backbuffer {
  void * Memory;
  BITMAPINFO Info;
  int Width;
  int Height;
  int BytesPerPixel;
  int Pitch;
};

struct win32_window_dimension {
  int Width;
  int Height;
};

global_variable win32_backbuffer GlobalBackbuffer;

internal void RenderGradient(win32_backbuffer Buffer, int BlueOffset, int GreenOffset) {
  uint8 *Row = (uint8*) Buffer.Memory;
  for(int Y = 0; Y < Buffer.Height; Y++) {
    uint32 *Pixel = (uint32 *) Row;
    for(int X = 0; X < Buffer.Width; X++) {
      uint8 Blue = X + BlueOffset;
      uint8 Green = Y + GreenOffset;
      uint8 Red = 255;
      *Pixel++ = ((Red << 16)|(Green << 8)|Blue);
    }
    Row += Buffer.Pitch;
  }
}



internal void Win32ResizeDIBSection(win32_backbuffer *Buffer, int Width, int Height) {

  if(Buffer->Memory) {
    VirtualFree(
		Buffer->Memory,
		0,
		MEM_RELEASE
		);
  }

  Buffer->Width = Width;
  Buffer->Height = Height;
  
  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;

  Buffer->BytesPerPixel = 4;
  int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
  Buffer->Memory = VirtualAlloc(
				0,
				BitmapMemorySize,
				MEM_COMMIT|MEM_RESERVE,
				PAGE_READWRITE
	       );
  Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;
  RenderGradient(*Buffer, 0, 0);
}

internal void Win32UpdateWindow(win32_backbuffer Buffer,
				HDC DeviceContext,
				int WindowWidth,
				int WindowHeight
				) {
  StretchDIBits(
		DeviceContext,
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer.Width, Buffer.Height,
		Buffer.Memory,
		&Buffer.Info,
		DIB_RGB_COLORS,
		SRCCOPY
		);
}

internal win32_window_dimension Win32GetWindowDimension(HWND Window) {
  win32_window_dimension Result;
  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;
  return Result;
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
      Running = false;
      OutputDebugStringA("WM_CLOSE\n");
    } break;
  case WM_PAINT:
    {

      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(Window, &Paint);
      win32_window_dimension Dimension = Win32GetWindowDimension(Window);
      Win32UpdateWindow(GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height);
      EndPaint(Window, &Paint);
      OutputDebugStringA("WM_PAINT\n");     
    } break;
  case WM_SIZE:
    {
      OutputDebugStringA("WM_SIZE\n");
    } break;
  case WM_DESTROY:
    {
      PostQuitMessage(0);
      Running = false;
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

  Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);
  
  WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
  WindowClass.lpfnWndProc = Win32Wndproc;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "IdleMakerWindowClass";

  if(RegisterClassA(&WindowClass)) {
    HWND Window;
    Window = CreateWindowExA(
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
    if(Window) {
      Running = true;
      int XOffset = 0;
      int YOffset = 0;
      while(Running) {
	HDC DeviceContext = GetDC(Window);
	MSG Message;
	
	while(PeekMessageA(&Message, Window, 0, 0, PM_REMOVE)) {
	  if(Message.message == WM_QUIT) {
	    Running = false;
	  }
	  TranslateMessage(&Message);
	  DispatchMessage(&Message);
	}
	win32_window_dimension Dimension = Win32GetWindowDimension(Window);
	Win32UpdateWindow(GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height);
	RenderGradient(GlobalBackbuffer, XOffset, YOffset);
	++XOffset;
	++YOffset;
      }
    }
  }
  
  return 0;
}
