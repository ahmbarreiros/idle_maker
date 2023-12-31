#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <math.h>


#define  global_variable static
#define  internal static
#define  local_variable static
#define Pi32 3.14159265359f

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;


typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

global_variable bool Running;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)

typedef X_INPUT_GET_STATE(x_input_get_state);
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
  return 0;
};

X_INPUT_SET_STATE(XInputSetStateStub) {
  return 0;
};

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define  XInputGetState XInputGetState_
#define  XInputSetState XInputSetState_

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
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
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

internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize) {
  HMODULE DSoundLibrary = LoadLibrary("dsound.dll");
  if(DSoundLibrary) {
    direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
    LPDIRECTSOUND DirectSound;
    if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {
      WAVEFORMATEX WaveFormat = {};

      WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
      WaveFormat.nChannels = 2;
      WaveFormat.nSamplesPerSec = SamplesPerSecond;
      WaveFormat.wBitsPerSample = 16;
      WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
      WaveFormat.nAvgBytesPerSec = (WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign);
      
      if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {
	LPDIRECTSOUNDBUFFER PrimaryBuffer;
	DSBUFFERDESC BufferDescription = {};
	
	BufferDescription.dwSize = sizeof(BufferDescription);
	BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

	if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))) {
	  if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))) {
	    OutputDebugStringA("Primary Buffer");
	  } else {
	    // TODO: PRIMARY BUFFER FORMAT NOT SET
	  }
	} else {
	  // TODO: PRIMARY BUFFER NOT CREATED
	}
      } else {
	// TODO: DSOUND COOPERATIVE LEVEL NOT SET
      }
      DSBUFFERDESC BufferDescription = {};
	
      BufferDescription.dwSize = sizeof(BufferDescription);
      BufferDescription.dwFlags = 0;
      BufferDescription.dwBufferBytes = BufferSize;
      BufferDescription.lpwfxFormat = &WaveFormat;

      if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0))) {
	OutputDebugStringA("Secondary Buffer");
      } else {
	// TODO: SECONDARY BUFFER NOT CREATED
      }
    } else {
      // TODO: DSOUND NOT CREATED
    }
  } else {
    // TODO: DSOUNDLIB NOT SET
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

  case WM_KEYUP:
    {
      WPARAM VKCode = wParam;
      if(VKCode == 'W') {
	OutputDebugStringA("W\n");
      }
    } break;
  case WM_KEYDOWN:
    {} break;
  case WM_SYSKEYUP:
    {} break;
  case WM_SYSKEYDOWN:
    {} break;
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

struct win32_sound_output {
  int SamplesPerSecond;
  int ToneHz;
  int16 ToneVolume;
  uint32 RunningSampleIndex;
  int WavePeriod;
  int BytesPerSample;
  int SecondaryBufferSize;
  real32 tSine;
  int LatencySampleCount;
};

internal void Win32FillSoundBuffer(win32_sound_output* SoundOutput,
				   DWORD ByteToLock,
				   DWORD BytesToWrite) {
  VOID *Region1;
  DWORD Region1Size;
  VOID *Region2;
  DWORD Region2Size;

  if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock,
					   BytesToWrite,
					   &Region1,
					   &Region1Size,
					   &Region2,
					   &Region2Size,
					   0))) {
    int16 *SampleOut = (int16 *)Region1;
    DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
    for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; SampleIndex++) {
      real32 SineValue = sinf(SoundOutput->tSine);
      int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;

      SoundOutput->tSine += (2.0f * Pi32 * 1.0f) / (real32)SoundOutput->WavePeriod;
      ++SoundOutput->RunningSampleIndex;
    }
    DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
    SampleOut = (int16 *)Region2;
    for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; SampleIndex++) {
      real32 SineValue = sinf(SoundOutput->tSine);
      int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;

      SoundOutput->tSine += (2.0f * Pi32 * 1.0f) / (real32)SoundOutput->WavePeriod;
      ++SoundOutput->RunningSampleIndex;
    }

    GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
  }
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

      win32_sound_output SoundOutput = {};
      SoundOutput.SamplesPerSecond = 40000;
      SoundOutput.ToneHz = 256;
      SoundOutput.RunningSampleIndex = 0;
      SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
      SoundOutput.BytesPerSample = sizeof(int16) * 2;
      SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
      SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
      Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
      Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.SecondaryBufferSize);
      GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
      
      while(Running) {
	HDC DeviceContext = GetDC(Window);
	MSG Message;
	
	while(PeekMessageA(&Message, Window, 0, 0, PM_REMOVE)) {
	  if(Message.message == WM_QUIT) {
	    Running = false;
	  }

	  for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++) { 
	    XINPUT_STATE ControllerState;
	    ZeroMemory(&ControllerState, sizeof(XINPUT_STATE));

	    if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
	      // NOTE: Controller available
	      XINPUT_GAMEPAD Pad = ControllerState.Gamepad;
	      
	      bool Up = (Pad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
	      bool Right = (Pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
	      bool Down = (Pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
	      bool Left = (Pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
	      bool Start = (Pad.wButtons & XINPUT_GAMEPAD_START);
	      bool Back = (Pad.wButtons & XINPUT_GAMEPAD_BACK);
	      bool LeftShoulder = (Pad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
	      bool RightShoulder = (Pad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
	      bool AButton = (Pad.wButtons & XINPUT_GAMEPAD_A);
	      bool BButton = (Pad.wButtons & XINPUT_GAMEPAD_B);
	      bool XButton = (Pad.wButtons & XINPUT_GAMEPAD_X);
	      bool YButton = (Pad.wButtons & XINPUT_GAMEPAD_Y);

	      int16 ThumbX = Pad.sThumbLX;
	      int16 ThumbY = Pad.sThumbLY;
	      
	      YOffset += ThumbY / 4096;
	      XOffset += ThumbX / 4096;;

	      SoundOutput.ToneHz = 512 + (int)(256.0f*((real32)ThumbY / 30000.0f));
	      SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
	      
	    } else {
	      // TODO: Controller not available
	    }
	  }
	  
	  TranslateMessage(&Message);
	  DispatchMessage(&Message);

	  DWORD PlayCursor;
	  DWORD WriteCursor;
	  if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor,
								 &WriteCursor))) {
	    DWORD ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
	    DWORD TargetCursor = ((PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize);
	    DWORD BytesToWrite;
	    if(ByteToLock > TargetCursor) {
	      BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
	      BytesToWrite += TargetCursor;
	    } else {
	      BytesToWrite = TargetCursor - ByteToLock;
	    }
	    Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite);
	  }
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
