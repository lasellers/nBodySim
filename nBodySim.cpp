#include "stdafx.h"
#include "resource.h"

#include "stdio.h" //for sprintf
#include "mmsystem.h" //lib: winmm.lib, for playsound
#include "math.h"

//
#include "nBodySim.h"

#include "nBody.h"
CnBody *nBody;
HDC hdctmp;
HBITMAP hbmtmp;
HANDLE hOld;
BITMAPV4HEADER bi;
LPBYTE pwBits;
bool runflag=true;
bool numeric_data_flag=false;
bool grid=true;
int TIMER_RATE;

int selected_body=0;
bool selected=false;
int selected_x=0;
int selected_y=0;

bool bSoundEffects=true;
int SoundEffectDelay=0;

//
enum
{
	TIMER_ID=1,
		SCREEN_WIDTH=640,
		SCREEN_HEIGHT=480,
};

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	//
	nBody=new CnBody(2);
	
	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_NBODYSIM, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}
	
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_NBODYSIM);
	
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	//nBody
	delete nBody;
	nBody=NULL;
	
	//
	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	
	wcex.cbSize = sizeof(WNDCLASSEX); 
	
	wcex.style			= 0; //CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_NBODYSIM);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_NBODYSIM;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
	
	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	
	hInst = hInstance; // Store instance handle in our global variable
	
	hWnd = CreateWindow(
		szWindowClass, szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		NULL, NULL, hInstance, NULL
		);
	if (!hWnd)
	{
		return FALSE;
	}
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	//
	runflag=true;
	
	//
	TIMER_RATE=33;
	SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
	
	//
	return TRUE;
}



//
struct RGBA
{
	BYTE blue;
	BYTE green;
	BYTE red;
	BYTE alpha;
} rgba;
void color(
		   BYTE r,
		   BYTE g,
		   BYTE b
		   )
{
	rgba.red=r;
	rgba.green=g;
	rgba.blue=b;
};

//
inline bool clip_yline(
					   int width,
					   int height,
					   int x,
					   int& y0,
					   int& y1
					   )
{
	int xmin=0;
	int ymin=0;
	int xmax=width-1;
	int ymax=height-1;
	
	if(y0>y1)
	{
		int tmp;
		tmp=y0;
		y0=y1;
		y1=tmp;
	}
	if(
		(x<xmin) ||
		(x>xmax) ||
		(y0<ymin && y1<ymin) ||
		(y0>ymax && y1>ymax)
		)
		return false;
	else
	{
		if(y0<ymin) y0=ymin;
		if(y1>ymax) y1=ymax;
		return true;
	}
};
//
inline bool clip_xline(
					   int width,
					   int height,
					   int& x0,
					   int& x1,
					   int y
					   )
{
	int xmin=0;
	int ymin=0;
	int xmax=width-1;
	int ymax=height-1;
	
	if(x0>x1)
	{
		int tmp;
		tmp=x0;
		x0=x1;
		x1=tmp;
	}
	if(
		(y<ymin) ||
		(y>ymax) ||
		(x0<xmin && x1<xmin) ||
		(x0>xmax && x1>xmax)
		)
		return false;
	else
	{
		if(x0<xmin) x0=xmin;
		if(x1>xmax) x1=xmax;
		return true;
	}
};
//
inline void xline(
				  BYTE *ptr,
				  int width,
				  int height,
				  int x0,
				  int x1,
				  int y
				  )
{
	if(clip_xline(width,height,x0,x1,y))
	{
		int count=x1-x0;
		RGBA *d=(RGBA *)ptr+width*y+x0;
		while(count--)
		{
			d->red=rgba.red;
			d->green=rgba.green;
			d->blue=rgba.blue;
			d++;
		}
	}
}
//
inline void yline(
				  BYTE *ptr,
				  int width,
				  int height,
				  int x,
				  int y0,
				  int y1
				  )
{
	if(clip_yline(width,height,x,y0,y1))
	{
		int count=y1-y0;
		RGBA *d=(RGBA *)ptr+x+(y0*width);
		while(count--)
		{
			d->red=rgba.red;
			d->green=rgba.green;
			d->blue=rgba.blue;
			d+=width;
		}
	}
}
//
inline void circle_filled_8points(
								  BYTE *ptr,
								  int width,
								  int height,
								  int xCenter,
								  int yCenter,
								  int x,
								  int y
								  )
{
	xline(ptr,width,height,xCenter - x, xCenter + x, yCenter + y);
	xline(ptr,width,height,xCenter - x, xCenter + x, yCenter - y);
	xline(ptr,width,height,xCenter - y, xCenter + y, yCenter + x);
	xline(ptr,width,height,xCenter - y, xCenter + y, yCenter - x);
}
//
void circle_filled(
				   BYTE *ptr,
				   int width,
				   int height,
				   int xCenter,
				   int yCenter,
				   int radius
				   )
{
	int x = 0;
	int y = radius;
	int d = 1 - radius;
	
	circle_filled_8points(ptr,width,height,xCenter, yCenter, x, y);
	
	while (x<y)
	{
		x++;
		if (d<0)
			d+=(x+1)<<1;
		else 
		{
			y--;
			d+=((x-y)+1)<<1;
		}
		circle_filled_8points(ptr,width,height,xCenter, yCenter, x, y);
	}
}

//
inline bool clip_point(
					   BYTE *ptr,
					   int width,
					   int height,
					   int x,
					   int y
					   )
{
	int xmin=0;
	int ymin=0;
	int xmax=width-1;
	int ymax=height-1;
	
	if(
		(x<xmin) ||
		(x>xmax) ||
		(y<ymin) ||
		(y>ymax)
		)
		return false;
	else
		return true;
};
//
void pixel(
		   BYTE *ptr,
		   int width,
		   int height,
		   int x,
		   int y
		   )
{
	if(clip_point(ptr,width,height,x,y))
	{
		RGBA *d=(RGBA *)ptr+x+y*width;
		d->red=rgba.red;
		d->green=rgba.green;
		d->blue=rgba.blue;
	}
}

//
inline void circle_8points(
						   BYTE *ptr,
						   int width,
						   int height,
						   int xCenter,
						   int yCenter,
						   int x,
						   int y
						   )
{
	pixel(ptr,width,height,xCenter + x, yCenter + y);
	pixel(ptr,width,height,xCenter - x, yCenter + y);
	pixel(ptr,width,height,xCenter + x, yCenter - y);
	pixel(ptr,width,height,xCenter - x, yCenter - y);
	pixel(ptr,width,height,xCenter + y, yCenter + x);
	pixel(ptr,width,height,xCenter - y, yCenter + x);
	pixel(ptr,width,height,xCenter + y, yCenter - x);
	pixel(ptr,width,height,xCenter - y, yCenter - x);
}
void circle(
			BYTE *ptr,
			int width,
			int height,
			int xCenter,
			int yCenter,
			int radius
			)
{
	int x = 0;
	int y = radius;
	int d = 1 - radius;
	
	circle_8points(ptr,width,height,xCenter, yCenter, x, y);
	
	while (x<y)
	{
		x++;
		if (d<0)
			d+=(x+1)<<1;
		else 
		{
			y--;
			d+=((x-y)+1)<<1;
		}
		circle_8points(ptr,width,height,xCenter, yCenter, x, y);
	}
}
//
void line(
		  BYTE *ptr,
		  int width,
		  int height,
		  int x1,
		  int y1,
		  int x2,
		  int y2
		  )
{
	int x, y;
	int dx, dy;
	int incx, incy;
	int balance;
	
	if (x2 >= x1)
	{
		dx = x2 - x1;
		incx = 1;
	}
	else
	{
		dx = x1 - x2;
		incx = -1;
	}
	
	if (y2 >= y1)
	{
		dy = y2 - y1;
		incy = 1;
	}
	else
	{
		dy = y1 - y2;
		incy = -1;
	}
	
	x = x1;
	y = y1;
	
	if (dx >= dy)
	{
		dy <<= 1;
		balance = dy - dx;
		dx <<= 1;
		
		while (x != x2)
		{
			pixel(ptr,width,height,x,y);
			if (balance >= 0)
			{
				y += incy;
				balance -= dx;
			}
			balance += dy;
			x += incx;
		} 
		pixel(ptr,width,height,x,y);
	}
	else
	{
		dx <<= 1;
		balance = dx - dy;
		dy <<= 1;
		
		while (y != y2)
		{
			pixel(ptr,width,height,x,y);
			if (balance >= 0)
			{
				x += incx;
				balance -= dy;
			}
			balance += dx;
			y += incy;
		}
		pixel(ptr,width,height,x,y);
	}
}



//
double lr=0.0;
double tb=0.0;
double m=0.0;
double w=0.0;
double z=1.0;
void viewport(const int width, const int height)
{
	lr=nBody->right-nBody->left;
	tb=nBody->bottom-nBody->top;
	m=(lr>tb)?lr:tb;
	w=((width<height)?width:height);
	z=w/m;
}
//
inline void fxline(
				   BYTE *ptr,
				   int width,
				   int height,
				   double x0,
				   double x1,
				   double y
				   )
{
	viewport(width,height);
	xline(ptr,width,height,(int)((x0-nBody->centerx)*z)+(width>>1),(int)((x1-nBody->centerx)*z)+(width>>1), (int)((y-nBody->centery)*z)+(height>>1) );
}
//
inline void fyline(
				   BYTE *ptr,
				   const int width,
				   const int height,
				   const double x,
				   const double y0,
				   const double y1
				   )
{
	viewport(width,height);
	yline(ptr,width,height,(int)((x-nBody->centerx)*z)+(width>>1), (int)((y0-nBody->centery)*z)+(height>>1), (int)((y1-nBody->centery)*z)+(height>>1) );
}
//
inline void fxy(
				const int width,
				const int height,
				const double xCenter,
				const double yCenter,
				int& x,
				int& y
				)
{
	viewport(width,height);
	x=(int)((xCenter-nBody->centerx)*z)+(width>>1);
	y=(int)((yCenter-nBody->centery)*z)+(height>>1);
}
//
inline void fxyr(
				 const int width,
				 const int height,
				 const double xCenter,
				 const double yCenter,
				 const double radius,
				 int& x,
				 int& y,
				 int& r
				 )
{
	viewport(width,height);
	x=(int)((xCenter-nBody->centerx)*z)+(width>>1);
	y=(int)((yCenter-nBody->centery)*z)+(height>>1);
	r=(int)(radius*z);
}
//
void fcircle_filled(
					BYTE *ptr,
					const int width,
					const int height,
					const double xCenter,
					const double yCenter,
					const double radius
					)
{
	viewport(width,height);
	circle_filled(ptr,width,height,(int)((xCenter-nBody->centerx)*z)+(width>>1), (int)((yCenter-nBody->centery)*z)+(height>>1), (int)(radius*z) );
}
//
void fcircle(
			 BYTE *ptr,
			 const int width,
			 const int height,
			 const double xCenter,
			 const double yCenter,
			 const double radius
			 )
{
	viewport(width,height);
	circle(ptr,width,height,(int)((xCenter-nBody->centerx)*z)+(width>>1), (int)((yCenter-nBody->centery)*z)+(height>>1), (int)(radius*z) );
}

//
void PaintAWorld(HDC hdc,HWND hWnd, PAINTSTRUCT ps)
{
	if(pwBits)
	{
		//
		RECT rt;
		GetClientRect(hWnd, &rt);
		int clientwidth = (rt.right-rt.left);
		int clientheight = (rt.bottom-rt.top);
		
		//
		HBRUSH hbrBkGnd = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		FillRect(hdctmp, &rt, hbrBkGnd);
		DeleteObject(hbrBkGnd);
		
		//
		TCHAR szStatusLine[1024];
		int n;
		
		//
		viewport(clientwidth,clientheight);

		// grid
		if(grid)
		{
			color(242,246,242);

			for(int y=-10000;y<10000;y+=100)
			{
				fxline(pwBits,clientwidth,clientheight,-10000.0,10000.0,y);
			}
			for(int x=-10000;x<10000;x+=100)
			{
				fyline(pwBits,clientwidth,clientheight,x,-10000.0,10000.0);
			}
		}

		
		// +-100 static grid
		color(0,0,0);
		fxline(pwBits,clientwidth,clientheight,-100.0,100.0,-100.0);
		fxline(pwBits,clientwidth,clientheight,-100.0,100.0,100.0);
		
		fyline(pwBits,clientwidth,clientheight,-100.0,-100.0,100.0);
		fyline(pwBits,clientwidth,clientheight,100.0,-100.0,100.0);
		
		// viewport distance grid
		if(nBody->viewport_distance)
		{
			color(230,230,230);
			fxline(pwBits,clientwidth,clientheight,-nBody->viewport_distance,nBody->viewport_distance,-nBody->viewport_distance);
			fxline(pwBits,clientwidth,clientheight,-nBody->viewport_distance,nBody->viewport_distance,nBody->viewport_distance);
			fyline(pwBits,clientwidth,clientheight,-nBody->viewport_distance,-nBody->viewport_distance,nBody->viewport_distance);
			fyline(pwBits,clientwidth,clientheight,nBody->viewport_distance,-nBody->viewport_distance,nBody->viewport_distance);
			
			color(255,230,230);
			fxline(pwBits,clientwidth,clientheight,-nBody->viewport_distance+nBody->centerx,nBody->viewport_distance+nBody->centerx,+nBody->centery);
			fyline(pwBits,clientwidth,clientheight,+nBody->centerx,-nBody->viewport_distance+nBody->centery,nBody->viewport_distance+nBody->centerx);
		}
		
		//
		if(numeric_data_flag)
		{
			rt.top=16;
			sprintf(szStatusLine,
				"centerx=%f centery=%f left=%f top=%f right=%f bottom=%f\n",
				nBody->centerx,nBody->centery,nBody->left,nBody->top,nBody->right,nBody->bottom);
			DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_RIGHT);
			rt.top+=16;
			sprintf(szStatusLine,"z=%f w=%f m=%f lr=%f tb=%f\n",z,w,m,lr,tb);
			DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_RIGHT);
			
			for(n=0;n<nBody->nBodies;n++)
			{
				rt.top=(n+3)*16;
				sprintf(szStatusLine,"%c %3d] %c%c%c %em %er [%10.1fx %10.1fy %10.1fz] %10.4fxv %10.4fyv %10.4fzv\n",
					(n==selected_body)?'*':' ',
					n,
					nBody->body[n].collision?'C':' ',
					nBody->body[n].ignore?'I':' ',
					nBody->body[n].merge?'M':' ',
					nBody->body[n].mass,
					nBody->body[n].radius,
					nBody->body[n].x.position, nBody->body[n].y.position, nBody->body[n].z.position,
					nBody->body[n].x.velocity, nBody->body[n].y.velocity, nBody->body[n].z.velocity
					);
				DrawText(hdctmp, szStatusLine, strlen(szStatusLine), &rt, DT_LEFT);
			}
		}
		
		//
		if(nBody->nViewable==0)
		{
			char sztmp[132+1];
			sprintf(sztmp,"No bodies in viewport.");
			DrawText(hdctmp, sztmp, strlen(sztmp), &rt, DT_CENTER);
		}
		else
		{
			//
			for(n=0;n<nBody->nBodies;n++)
			{
				//
				if(n==selected_body)
				{
					color(0,255,0);
					fcircle(pwBits,clientwidth,clientheight,nBody->body[n].x.position,nBody->body[n].y.position,nBody->body[n].radius*1.2);
					
					if(selected)
					{
						color(0,255,0);
						int y,x;
						fxy(clientwidth,clientheight, nBody->body[n].x.position, nBody->body[n].y.position, x,y);
						line(pwBits,clientwidth,clientheight,x,y,selected_x,selected_y);
					}
				}
				
				
				//
				if(nBody->body[n].collision)
					color(255,0,0);
				else
				{
					if(nBody->body_type==CnBody::RIGID_BODY && nBody->dimension_type==CnBody::THREE_DIMENSIONS)
					{
						if(nBody->body[n].z.position>=0)
						{
							int const c=(int)((nBody->body[n].z.position/nBody->distant)*256.0);
							color(c,c>>1,c>>1);
						}
						else
						{
							int const c=(int)((nBody->body[n].z.position/nBody->close)*256.0);
							color(0,0,c);
						}
					}
					else if(nBody->body_type==CnBody::RIGID_BODY && nBody->dimension_type==CnBody::TWO_DIMENSIONS)
					{
						int const c=(int)(((float)n/(float)nBody->nBodies)*240.0f);
						color(c,c,c);
					}
				}
				
				//
				if(nBody->body[n].ignore)
					fcircle(pwBits,clientwidth,clientheight,nBody->body[n].x.position,nBody->body[n].y.position,nBody->body[n].radius);
				else
					fcircle_filled(pwBits,clientwidth,clientheight,nBody->body[n].x.position,nBody->body[n].y.position,nBody->body[n].radius);
				if(nBody->body[n].mass<=0)
				{
					fcircle(pwBits,clientwidth,clientheight,nBody->body[n].x.position,nBody->body[n].y.position,nBody->body[n].radius*1.3);
					fcircle(pwBits,clientwidth,clientheight,nBody->body[n].x.position,nBody->body[n].y.position,nBody->body[n].radius*1.4);
				}
			}
			
		}
		
	}
}


//
void update_menu_settings(HWND hWnd)
{
	if(nBody)
	{
		char tmp[512];
		if(nBody->viewport_distance)
		_snprintf(tmp,512,"%s - viewport distance %d %s - bodies %d (%d viewable) - frame %d",
			szTitle,
			nBody->viewport_distance,
			nBody->lock_viewport?"[LOCKED]":"",
			nBody->nBodies,
			nBody->nViewable,
			nBody->frame
			);
		else
		_snprintf(tmp,512,"%s - viewport distance INFINITE %s - bodies %d (%d viewable) - frame %d",
			szTitle,
			nBody->lock_viewport?"[LOCKED]":"",
			nBody->nBodies,
			nBody->nViewable,
			nBody->frame
			);
		SetWindowText(hWnd,tmp);
	}
	
	//
	HMENU hMenu=GetMenu(hWnd);
	
	CheckMenuItem(hMenu,IDM_TIMEFACTOR_1,		(nBody->time_factor==1.0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_TIMEFACTOR_10,		(nBody->time_factor==10.0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_TIMEFACTOR_100,		(nBody->time_factor==100.0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_TIMEFACTOR_1000,	(nBody->time_factor==1000.0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_TIMEFACTOR_10000,	(nBody->time_factor==10000.0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_TIMEFACTOR_100000,	(nBody->time_factor==100000.0)?MF_CHECKED:MF_UNCHECKED);
	
	CheckMenuItem(hMenu,IDM_RESTITUTION_00,  (nBody->e==0.0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_RESTITUTION_05,  (nBody->e==0.5)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_RESTITUTION_10,  (nBody->e==1.0)?MF_CHECKED:MF_UNCHECKED);
	
	CheckMenuItem(hMenu,IDM_FPS_1,	(TIMER_RATE==1000)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_FPS_30,	(TIMER_RATE==33)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_FPS_60,	(TIMER_RATE==16)?MF_CHECKED:MF_UNCHECKED);
	
	CheckMenuItem(hMenu,IDM_LOCK_VIEWPORT,	nBody->lock_viewport?MF_CHECKED:MF_UNCHECKED);
	
	CheckMenuItem(hMenu,IDM_NUMERIC_DATA,	numeric_data_flag?MF_CHECKED:MF_UNCHECKED);
	
	CheckMenuItem(hMenu,IDM_IGNORE,	nBody->body[selected_body].ignore?MF_CHECKED:MF_UNCHECKED);
	
	CheckMenuItem(hMenu,IDM_BODY_RIGID, nBody->body_type==CnBody::RIGID_BODY?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_BODY_FLUID, nBody->body_type==CnBody::FLUID_BODY?MF_CHECKED:MF_UNCHECKED);
	
	CheckMenuItem(hMenu,IDM_STATIC_COLLISION, nBody->collision_type==CnBody::STATIC_COLLISION?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_DYNAMIC_COLLISION, nBody->collision_type==CnBody::DYNAMIC_COLLISION?MF_CHECKED:MF_UNCHECKED);
	
	CheckMenuItem(hMenu,IDM_DIMENSIONS_TWO, nBody->dimension_type==CnBody::TWO_DIMENSIONS?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_DIMENSIONS_THREE, nBody->dimension_type==CnBody::THREE_DIMENSIONS?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu,IDM_GRID,	grid?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu,IDM_INITIAL_VELOCITIES_NONE,	nBody->random_initial_velocities==false?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_INITIAL_VELOCITIES_RANDOM,	nBody->random_initial_velocities==true?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu,IDM_SOUNDEFFECTS,				bSoundEffects?MF_CHECKED:MF_UNCHECKED);
}


//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	
	//
	switch (message) 
	{
	case WM_LBUTTONDOWN: //selects a body
		{
			WORD fwKeys = wParam;        // key flags 
			WORD xPos = LOWORD(lParam);  // horizontal position of cursor 
			WORD yPos = HIWORD(lParam);  // vertical position of cursor 
			
			RECT rt;
			GetClientRect(hWnd, &rt);
			const int clientwidth = (rt.right-rt.left);
			const int clientheight = (rt.bottom-rt.top);
			
			for(int n=0;n<nBody->nBodies;n++)
			{
				int x=0,y=0,r=0;
				fxyr(clientwidth,clientheight,
					nBody->body[n].x.position,
					nBody->body[n].y.position,
					nBody->body[n].radius,
					x,y,r
					);
				
				const int dx=x-xPos;
				const int dy=y-yPos;
				double dst=sqrt( (double)(dx*dx + dy*dy));
				
				if(dst<=r)
				{
					selected_body=n;
				}
			}
		}
		break;
		
	case WM_MOUSEMOVE: // selects velocities
		{
			WORD fwKeys = wParam;        // key flags 
			WORD xPos = LOWORD(lParam);  // horizontal position of cursor 
			WORD yPos = HIWORD(lParam);  // vertical position of cursor 
			
			if(selected)
			{
				selected_x=xPos;
				selected_y=yPos;
			}
		}
		break;
		
	case WM_RBUTTONDOWN: // selects velocities
		{
			WORD fwKeys = wParam;        // key flags 
			WORD xPos = LOWORD(lParam);  // horizontal position of cursor 
			WORD yPos = HIWORD(lParam);  // vertical position of cursor 
			
			selected_x=xPos;
			selected_y=yPos;
			selected=true; //persist the line for 10 frames
		}
		break;
		
	case WM_RBUTTONUP: // selects velocities
		{
			WORD fwKeys = wParam;        // key flags 
			WORD xPos = LOWORD(lParam);  // horizontal position of cursor 
			WORD yPos = HIWORD(lParam);  // vertical position of cursor 
			
			selected=false; //persist the line for 10 frames
			
			RECT rt;
			GetClientRect(hWnd, &rt);
			const int clientwidth = (rt.right-rt.left);
			const int clientheight = (rt.bottom-rt.top);
			
			int x=0,y=0,r=0;
			fxyr(clientwidth,clientheight,
				nBody->body[selected_body].x.position,
				nBody->body[selected_body].y.position,
				nBody->body[selected_body].radius,
				x,y,r
				);
			
			const int dx=x-xPos;
			const int dy=y-yPos;
			const double dst=sqrt( (double)(dx*dx + dy*dy));
			
			const double mag=0.005 * ( (double)SCREEN_WIDTH/(nBody->right - nBody->left) + (double)SCREEN_HEIGHT/(nBody->bottom - nBody->top) );
			nBody->body[selected_body].x.velocity=-mag*(dst*(dx/dst));
			nBody->body[selected_body].y.velocity=-mag*(dst*(dy/dst));
		}
		break;
		
#ifndef WM_MOUSEWHEEL
#error No WM_MOUSEWHEEL. Include latest Platform SDK if using VC++ 6.
#else
	case WM_MOUSEWHEEL:
		{
			int fwKeys = LOWORD(wParam);    // key flags
			short zDelta = (short) HIWORD(wParam);    // wheel rotation
			short xpos= (short) LOWORD(lParam);    // horizontal position of pointer
			short yPos = (short) HIWORD(lParam);    // vertical position of pointer
			
			const float delta=(float)(zDelta/WHEEL_DELTA);
			nBody->viewport_distance+=(int)(delta*100.0f);
			if(nBody->viewport_distance<0)
				nBody->viewport_distance=0; //infinite
			else if(nBody->viewport_distance>10000)
				nBody->viewport_distance=10000;
			nBody->unignore_all();
			nBody->recompute_viewport();
		}
		break;
#endif
		
	case WM_COMMAND:
		
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
			
		case IDM_CONTENTS:
			WinHelp(hWnd,"nBodySim.hlp",HELP_CONTENTS,0);
			break;
		case IDM_INDEX:
			WinHelp(hWnd,"nBodySim.hlp",HELP_INDEX,0);
			break;
		case IDM_FIND:
			WinHelp(hWnd,"nBodySim.hlp",HELP_FINDER,0);
			break;

		case IDM_PAUSERESUME:
			runflag=!runflag;
			break;
			
		case IDM_GRID:
			grid=!grid;
			break;
			
		case IDM_RESTITUTION_00:
			nBody->e=0.0;
			break;
		case IDM_RESTITUTION_05:
			nBody->e=0.5;
			break;
		case IDM_RESTITUTION_10:
			nBody->e=1.0;
			break;
			
		case IDM_Z_PLUS:
			nBody->body[selected_body].z.position+=((nBody->distant - nBody->close)/30.0);
			break;
		case IDM_Z_MINUS:
			nBody->body[selected_body].z.position-=((nBody->distant - nBody->close)/30.0);
			break;
			
			//
		case IDM_SCENARIO_SOLSYSTEM:
			numeric_data_flag=true;
			nBody->Reset(5);
			nBody->viewport_distance=0;
			
			// SOL
			nBody->body[0].mass=1.989e30;
			nBody->body[0].radius=log(nBody->body[0].mass);
			nBody->body[0].x.position=0;
			nBody->body[0].y.position=0;
			nBody->body[0].x.velocity=0;
			nBody->body[0].y.velocity=0;
			
			// mercury
			nBody->body[1].mass=3.302e23;
			nBody->body[1].radius=4878;
			nBody->body[1].x.position=5.790e10;
			nBody->body[1].y.position=0;
			nBody->body[1].x.velocity=0;
			nBody->body[1].y.velocity=0;
			
			// venus
			nBody->body[2].mass=4.869e24;
			nBody->body[2].radius=12104;
			nBody->body[2].x.position=1.082e11;
			nBody->body[2].y.position=0;
			nBody->body[2].x.velocity=0;
			nBody->body[2].y.velocity=0;
			
			// earth
			nBody->body[3].mass=5.974e24;
			nBody->body[3].radius=12756;
			nBody->body[3].x.position=1.496e11;
			nBody->body[3].y.position=0;
			nBody->body[3].x.velocity=0;
			nBody->body[3].y.velocity=0;
			
			// mars
			nBody->body[4].mass=6.419e23;
			nBody->body[4].radius=6794;
			nBody->body[4].x.position=2.279e11;
			nBody->body[4].y.position=0;
			nBody->body[4].x.velocity=0;
			nBody->body[4].y.velocity=0;
			
			runflag=true;
			InvalidateRect ( hWnd,NULL, FALSE );
			break;
			
		case IDM_SCENARIO_2BODY_COLLISION_A:
			numeric_data_flag=true;
			nBody->Reset(2);
			
			nBody->body[0].mass=1e3;
			nBody->body[0].radius=log(nBody->body[0].mass);
			nBody->body[0].x.position=0;
			nBody->body[0].y.position=0;
			nBody->body[0].x.velocity=0;
			nBody->body[0].y.velocity=0;
			
			nBody->body[1].mass=1e4;
			nBody->body[1].radius=log(nBody->body[1].mass);
			nBody->body[1].x.position=100;
			nBody->body[1].y.position=0;
			nBody->body[1].x.velocity=-1;
			nBody->body[1].y.velocity=0;
			
			nBody->left=-200;
			nBody->right=200;
			nBody->top=-200;
			nBody->bottom=200;
			
			nBody->centerx=0;
			nBody->centery=0;
			nBody->lock_viewport=true;
			
			runflag=true;
			InvalidateRect ( hWnd,NULL, FALSE );
			break;
			
		case IDM_SCENARIO_2BODY_COLLISION_D:
			numeric_data_flag=true;
			nBody->Reset(2);
			
			nBody->body[0].mass=1e4;
			nBody->body[0].radius=log(nBody->body[0].mass);
			nBody->body[0].x.position=0;
			nBody->body[0].y.position=0;
			nBody->body[0].x.velocity=0;
			nBody->body[0].y.velocity=0;
			
			nBody->body[1].mass=1e4;
			nBody->body[1].radius=log(nBody->body[1].mass);
			nBody->body[1].x.position=100;
			nBody->body[1].y.position=100;
			nBody->body[1].x.velocity=-1;
			nBody->body[1].y.velocity=-.8;
			
			nBody->left=-200;
			nBody->right=200;
			nBody->top=-200;
			nBody->bottom=200;
			
			nBody->centerx=0;
			nBody->centery=0;
			nBody->lock_viewport=true;
			
			runflag=true;
			InvalidateRect ( hWnd,NULL, FALSE );
			break;
			
		case IDM_SCENARIO_2BODYCOLLISIONEQUAL:
			numeric_data_flag=true;
			nBody->Reset(2);
			
			nBody->body[0].mass=1e3;
			nBody->body[0].radius=log(nBody->body[0].mass);
			nBody->body[0].x.position=0;
			nBody->body[0].y.position=0;
			nBody->body[0].x.velocity=0;
			nBody->body[0].y.velocity=0;
			
			nBody->body[1].mass=1e3;
			nBody->body[1].radius=log(nBody->body[1].mass);
			nBody->body[1].x.position=100;
			nBody->body[1].y.position=0;
			nBody->body[1].x.velocity=-1;
			nBody->body[1].y.velocity=0;
			
			nBody->left=-200;
			nBody->right=200;
			nBody->top=-200;
			nBody->bottom=200;
			
			nBody->centerx=0;
			nBody->centery=0;
			nBody->lock_viewport=true;
			
			runflag=true;
			InvalidateRect ( hWnd,NULL, FALSE );
			break;
			
		case IDM_SCENARIO_2COLLIDE:
			numeric_data_flag=true;
			nBody->Reset(2);
			
			nBody->body[0].mass=1e3;
			nBody->body[0].radius=log(nBody->body[0].mass);
			nBody->body[0].x.position=-100;
			nBody->body[0].y.position=0;
			nBody->body[0].x.velocity=1;
			nBody->body[0].y.velocity=0;
			
			nBody->body[1].mass=1e3;
			nBody->body[1].radius=log(nBody->body[1].mass);
			nBody->body[1].x.position=100;
			nBody->body[1].y.position=0;
			nBody->body[1].x.velocity=-1;
			nBody->body[1].y.velocity=0;
			
			nBody->left=-200;
			nBody->right=200;
			nBody->top=-200;
			nBody->bottom=200;
			
			nBody->centerx=0;
			nBody->centery=0;
			nBody->lock_viewport=true;
			
			runflag=true;
			InvalidateRect ( hWnd,NULL, FALSE );
			break;
			
		case IDM_SCENARIO_2BODY_ORBIT:
			numeric_data_flag=true;
			nBody->Reset(2);
			
			nBody->body[0].mass=1e6;
			nBody->body[0].radius=log(nBody->body[0].mass);
			nBody->body[0].x.position=-30;
			nBody->body[0].y.position=0;
			nBody->body[0].x.velocity=0;
			nBody->body[0].y.velocity=-.025;
			
			nBody->body[1].mass=1e6;
			nBody->body[1].radius=log(nBody->body[1].mass);
			nBody->body[1].x.position=30;
			nBody->body[1].y.position=0;
			nBody->body[1].x.velocity=0;
			nBody->body[1].y.velocity=.025;
			
			nBody->left=-200;
			nBody->right=200;
			nBody->top=-200;
			nBody->bottom=200;
			
			nBody->centerx=0;
			nBody->centery=0;
			nBody->lock_viewport=true;
			
			runflag=true;
			InvalidateRect ( hWnd,NULL, FALSE );
			break;
			
		case IDM_SCENARIO_3BODYCOLLISION:
			numeric_data_flag=true;
			nBody->Reset(3);
			
			nBody->body[0].mass=1e2;
			nBody->body[0].radius=log(nBody->body[0].mass);
			nBody->body[0].x.position=-100;
			nBody->body[0].y.position=0;
			nBody->body[0].x.velocity=0;
			nBody->body[0].y.velocity=0;
			
			nBody->body[1].mass=1e5;
			nBody->body[1].radius=log(nBody->body[1].mass);
			nBody->body[1].x.position=0;
			nBody->body[1].y.position=0;
			nBody->body[1].x.velocity=0;
			nBody->body[1].y.velocity=0;
			
			nBody->body[2].mass=1e3;
			nBody->body[2].radius=log(nBody->body[2].mass);
			nBody->body[2].x.position=100;
			nBody->body[2].y.position=0;
			nBody->body[2].x.velocity=-1;
			nBody->body[2].y.velocity=0;
			
			nBody->left=-200;
			nBody->right=200;
			nBody->top=-200;
			nBody->bottom=200;
			
			nBody->centerx=0;
			nBody->centery=0;
			nBody->lock_viewport=true;
			
			runflag=true;
			InvalidateRect ( hWnd,NULL, FALSE );
			break;
			
		case IDM_SCENARIO_3BODYCOLLISIONREVERSE:
			numeric_data_flag=true;
			nBody->Reset(3);
			
			nBody->body[0].mass=1e2;
			nBody->body[0].radius=log(nBody->body[0].mass);
			nBody->body[0].x.position=-100;
			nBody->body[0].y.position=0;
			nBody->body[0].x.velocity=1;
			nBody->body[0].y.velocity=0;
			
			nBody->body[1].mass=1e4;
			nBody->body[0].radius=log(nBody->body[1].mass);
			nBody->body[1].x.position=0;
			nBody->body[1].y.position=0;
			nBody->body[1].x.velocity=0;
			nBody->body[1].y.velocity=0;
			
			nBody->body[2].mass=1e3;
			nBody->body[2].radius=log(nBody->body[2].mass);
			nBody->body[2].x.position=100;
			nBody->body[2].y.position=0;
			nBody->body[2].x.velocity=0;
			nBody->body[2].y.velocity=0;
			
			nBody->left=-200;
			nBody->right=200;
			nBody->top=-200;
			nBody->bottom=200;
			
			nBody->centerx=0;
			nBody->centery=0;
			nBody->lock_viewport=true;
			
			runflag=true;
			InvalidateRect ( hWnd,NULL, FALSE );
			break;
			
			//
		case IDM_KILL_VELOCITIES:
			nBody->kill_velocities();
			break;
		case IDM_KILL_VELOCITY:
			nBody->kill_velocity(selected_body);
			break;
			
		case IDM_NUMERIC_DATA:
			numeric_data_flag=!numeric_data_flag;
			break;
			
			//
		case IDM_INCREASE_MASS:
			nBody->body[selected_body].mass*=10.0;
			nBody->body[selected_body].radius=log(fabs(nBody->body[selected_body].mass));
			break;
		case IDM_DECREASE_MASS:
			nBody->body[selected_body].mass*=0.1;
			nBody->body[selected_body].radius=log(fabs(nBody->body[selected_body].mass));
			break;
		case IDM_NEGATIVE_MASS:
			nBody->body[selected_body].mass=-nBody->body[selected_body].mass;
			break;
			
			//
		case IDM_BODY_RIGID:
			nBody->body_type=CnBody::RIGID_BODY;
			break;
		case IDM_BODY_FLUID:
			nBody->body_type=CnBody::FLUID_BODY;
			break;

		case IDM_STATIC_COLLISION:
			nBody->collision_type=CnBody::STATIC_COLLISION;
			break;
		case IDM_DYNAMIC_COLLISION:
			nBody->collision_type=CnBody::DYNAMIC_COLLISION;
			break;

		case IDM_DIMENSIONS_TWO:
			nBody->dimension_type=CnBody::TWO_DIMENSIONS;
			break;
		case IDM_DIMENSIONS_THREE:
			nBody->dimension_type=CnBody::THREE_DIMENSIONS;
			break;

			//
		case IDM_SELECT_NEXT:
			if(++selected_body>=nBody->nBodies)
				selected_body=0;
			break;
		case IDM_SELECT_PREVIOUS:
			if(--selected_body<0)
				selected_body=nBody->nBodies-1;
			break;

			//
		case IDM_VIEWPORT_100:
			nBody->unignore_all();
			nBody->lock_viewport=true;
			nBody->viewport_distance=100;
			nBody->recompute_viewport();
			break;
		case IDM_VIEWPORT_200:
			nBody->unignore_all();
			nBody->lock_viewport=true;
			nBody->viewport_distance=200;
			nBody->recompute_viewport();
			break;
		case IDM_VIEWPORT_500:
			nBody->unignore_all();
			nBody->lock_viewport=true;
			nBody->viewport_distance=500;
			nBody->recompute_viewport();
			break;
		case IDM_VIEWPORT_1000:
			nBody->unignore_all();
			nBody->lock_viewport=true;
			nBody->viewport_distance=1000;
			nBody->recompute_viewport();
			break;
		case IDM_VIEWPORT_5000:
			nBody->unignore_all();
			nBody->lock_viewport=true;
			nBody->viewport_distance=5000;
			nBody->recompute_viewport();
			break;
		case IDM_VIEWPORT_10000:
			nBody->unignore_all();
			nBody->lock_viewport=true;
			nBody->viewport_distance=10000;
			nBody->recompute_viewport();
			break;
			
			//
		case IDM_AUTOZOOM:
			nBody->lock_viewport=false;
			nBody->viewport_distance=0;
			nBody->unignore_all();
			break;
		case IDM_AUTOZOOM_100:
			nBody->lock_viewport=false;
			nBody->viewport_distance=200;
			nBody->unignore_all();
			break;
		case IDM_AUTOZOOM_200:
			nBody->lock_viewport=false;
			nBody->viewport_distance=200;
			nBody->unignore_all();
			break;
		case IDM_AUTOZOOM_1000:
			nBody->lock_viewport=false;
			nBody->viewport_distance=1000;
			nBody->unignore_all();
			break;
		case IDM_AUTOZOOM_5000:
			nBody->lock_viewport=false;
			nBody->viewport_distance=5000;
			nBody->unignore_all();
			break;
		case IDM_AUTOZOOM_10000:
			nBody->lock_viewport=false;
			nBody->viewport_distance=10000;
			nBody->unignore_all();
			break;
			
			//
		case IDM_UNIGNORE_ALL:
			nBody->unignore_all();
			break;
		case IDM_IGNORE_ALL:
			nBody->ignore_all();
			break;
			
			//
		case IDM_IGNORE:
			nBody->body[selected_body].ignore=!nBody->body[selected_body].ignore;
			break;
			
			//
		case IDM_TIMEFACTOR_1:
			nBody->TimeFactor(1);
			break;
		case IDM_TIMEFACTOR_10:
			nBody->TimeFactor(10);
			break;
		case IDM_TIMEFACTOR_100:
			nBody->TimeFactor(100);
			break;
		case IDM_TIMEFACTOR_1000:
			nBody->TimeFactor(1000);
			break;
		case IDM_TIMEFACTOR_10000:
			nBody->TimeFactor(10000);
			break;
		case IDM_TIMEFACTOR_100000:
			nBody->TimeFactor(100000);
			break;
			
		case IDM_FPS_1:
			TIMER_RATE=1000;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			break;
		case IDM_FPS_30:
			TIMER_RATE=33;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			break;
		case IDM_FPS_60:
			TIMER_RATE=16;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			break;
			
		case IDM_LOCK_VIEWPORT:
			nBody->lock_viewport=!nBody->lock_viewport;
			break;
			
		case IDM_INITIAL_VELOCITIES_NONE:
			nBody->random_initial_velocities=false;
			break;
		case IDM_INITIAL_VELOCITIES_RANDOM:
			nBody->random_initial_velocities=true;
			break;
			
		case IDM_BODIES_1:
			nBody->Reset(1);
			break;
		case IDM_BODIES_2:
			nBody->Reset(2);
			break;
		case IDM_BODIES_3:
			nBody->Reset(3);
			break;
		case IDM_BODIES_4:
			nBody->Reset(4);
			break;
		case IDM_BODIES_5:
			nBody->Reset(5);
			break;
		case IDM_BODIES_10:
			nBody->Reset(10);
			break;
		case IDM_BODIES_20:
			nBody->Reset(20);
			break;
		case IDM_BODIES_100:
			nBody->Reset(100);
			break;
		case IDM_BODIES_1000:
			nBody->Reset(1000);
			break;
		case IDM_BODIES_10000:
			nBody->Reset(10000);
			break;
			
			//
		case IDM_SOUNDEFFECTS:
			bSoundEffects=!bSoundEffects;
			break;

			//
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		
		case WM_ERASEBKGND:
			return 1;
			
		case WM_PAINT:
			{
				update_menu_settings(hWnd);
				
				// begin to repaint the screen....
				hdc = BeginPaint(hWnd, &ps);
				RECT rt;
				GetClientRect(hWnd, &rt);
				
				// first, since we don't want the screen to flicker or use slow-as-hell GDI
				//primatives to try to do the graphics rendering, we crate a temporary memory buffer
				// to do all our rendering to....
				hdctmp=CreateCompatibleDC(hdc);
				
				// now prepare to give it a directly accessable RGBA memory buffer
				//that we can draw to directly as a block of memory.
				bi.bV4BitCount = 32;
				bi.bV4ClrImportant = 0;
				bi.bV4ClrUsed = 0;
				bi.bV4V4Compression = BI_RGB;
				bi.bV4Planes = 1;
				bi.bV4Size = sizeof(BITMAPV4HEADER);
				bi.bV4SizeImage = 0; //(rt.right-rt.left)*(rt.bottom-rt.top);
				bi.bV4Width = (rt.right-rt.left);
				bi.bV4Height = -(rt.bottom-rt.top);
				bi.bV4XPelsPerMeter = 0;
				bi.bV4YPelsPerMeter = 0;
				bi.bV4AlphaMask = 0;
				bi.bV4CSType = 0;
				
				// get the DIB....
				hbmtmp = CreateDIBSection(hdc, (BITMAPINFO *) &bi, DIB_RGB_COLORS, (LPVOID *)&pwBits, NULL, 0);
				hOld = SelectObject(hdctmp, hbmtmp); //and select it to draw on.
				
				PaintAWorld(hdc,hWnd,ps); //do it, if all's ok.
				
				// Now we copy what we rendered from our buffer to the screen as fast
				//as possible to avoid flickering.
				BitBlt(hdc,rt.left,rt.top,rt.right-rt.left,rt.bottom-rt.top,hdctmp,0,0,SRCCOPY);
				
				// un allocate the memory we grabbed.
				SelectObject(hdctmp,hOld);
				DeleteObject(hbmtmp);
				DeleteDC(hdctmp);
				
				// we're done
				EndPaint(hWnd, &ps);
			}
			break;

			//
		case WM_TIMER:
			if(runflag)
			{
				if(nBody)
				{
					//note: only play collision sound once a ever 500ms (half a minute)
					if(nBody->ComputeForModel())
					{
						if(bSoundEffects==true)
						{
							if(SoundEffectDelay<=0)
							{
								PlaySound(MAKEINTRESOURCE(IDR_WAVE_COLLISION), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
								SoundEffectDelay=500;
							}
						}
					}
					SoundEffectDelay-=TIMER_RATE;

					//
					InvalidateRect(hWnd,NULL,FALSE);
				}
			}
			break;
			
		case WM_DESTROY:
			PostQuitMessage(0);
			KillTimer(hWnd, TIMER_ID);
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		// Header: Declared in mmsystem.h.
  // Import Library: Use winmm.lib.
		PlaySound(MAKEINTRESOURCE(IDR_WAVE_ABOUT), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
		return TRUE;
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
    return FALSE;
}
