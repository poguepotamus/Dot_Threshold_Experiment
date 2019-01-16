//This is the first program in the threshold experiment. The threshold experiment will use a staircase to determine
//the dot density and dot contrast threshold. This is the dot experiment. In this version, the staircase has been 
//successfully implemented. The average is calculated for the first 4 trials, then it's 75% value is used in the 
//staircase to determine a correct or incorrect trial. All of the staircase messages are output to a console
//as well as a text file to double check. 
//The trials are currently 17 seconds long (2-practice, 15-experiment).

//Changes in this version included 
//add ceiling to threshold average6
//add if statement if reached max or min to data file


/*Configure the "Character Set" setting to "Multi-Byte" if there are errors compiling*/
#pragma comment(lib,"glu32.lib")		//Additional dependencies are listed as a safe measure to reduce compiling errors
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxerr.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"winmm.lib")
#define DIRECTINPUT_VERSION 0x0800		//Defines DirectInput version
#define WIN32_LEAN_AND_MEAN				//Removes unnecessary functions
#define SND_FILENAME 0x20000			//Define for beep sound
#define SND_LOOP 8						//Define for beep sound
#define SND_ASYNC 1						//Define for beep sound
#define MYICON 1
#include <windows.h>	//Windows header file
#include <windowsx.h>	//Windows extended header file
#include <GL/gl.h>		//OpenGL headear file
#include <GL/glu.h>		//OpenGL utilities header file
#include <dinput.h>		//DirectInput header file
#include <stdio.h>		//Standard input/output header file
#include <stdlib.h>		//Standard library header file
#include <time.h>		//Timer header file
#include <math.h>		//Math header file
#include <string.h>		//String header file
#include <errno.h>		//Error codes header file
#include <mmsystem.h>	//System timer header file
#include <iostream>		//Input output header file
#include <fstream>		//Read and write data files
#include <shellapi.h>	//Shell header file to open .exe files
#include <conio.h>		//Console input/output header file
#include <crtdbg.h>		//Console header file
#include <ctype.h>		//For toupper() function
#include <locale>		//For toupper() function
#include "resource.h"	//Input dialog box resource file
#include "guicon.h"		//Console header file
#include "TwoAFCstaircase.h" //Header file to include the TwoAFC class
#include "global.h"		//Header file to include external variables

INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);		//Message handler for the dialog box
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);		//Message handler for the window
HWND	BuildWindow(LPCTSTR, HINSTANCE, int, int);				//Registers the type of window and then creates the window. The pixel format is also described
BOOL	HeartBeat(HDC);											//Responsible for the message loop. Renders scene and swaps buffer each iteration
BOOL	CALLBACK EnumFFDevicesCallback( const DIDEVICEINSTANCE* pInst, VOID* pContext );		//Called once for each enumerated force feedback device. If we find one, create a device interface on it so we can play with it
BOOL	CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );		//Callback function for enumerating the axes on a joystick and counting each force feedback enabled axis
HRESULT InitDirectInput( HWND hwnd );							//Initialize the DirectInput variables
HRESULT SetDeviceForcesXY();									//Apply the X and Y forces to the effect we prepared
HRESULT UpdateInputState (HWND hwnd);							//Obtains the readings from the joystick
void	FreeDirectInput();										//Releases directinput objects
void	Initialize();											//Sets up the required openGL environment
void	ShutDown();												//OpenGL Cleanup
void	ReshapeScene(int, int);									//Resize and Initialize the openGL environment
void	RenderScene();											//Renders the scene
void	IllustrateScene();										//Clear the display, render the scene and swap the buffers

/* Defines, constants, and global variables.*/
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }	//Variables to release DirectInput devices
#define steerRatio 0.85					//Define the change of the viewer position per frame proportion to the turn angle of steering wheel
#define PI 3.141592657					//PI
#define MAXpoint 2500					//Maximum number of points
#define FPS (float)60.0					//60 frames/sec
#define SPEED (float)33.3				//The movement in depth each frame
#define MAXPOS  SPEED*FPS*(float)7.5	//The dots will travel for 7.5 seconds before they get to the observer. It used to be 19500 in 1.2 version.
#define	trials 100						//The number of trials we want (affects the frame count and arrays)
#define basetrials 4					//The number of trials to use as baseline measurement
#define cutoff (float).75				//The percentage of the average to use as cutoff

using namespace std;

LPDIRECTINPUT8          g_pDI = NULL;					//Instance of directinput interface
LPDIRECTINPUTDEVICE8    g_pDevice = NULL;				//Instance of directinput device
LPDIRECTINPUTEFFECT     g_pEffect = NULL;				//Instance of directinput effect
DWORD                   g_dwNumForceFeedbackAxis = 0;	//Number of force feedback axis
HWND					hwnd = NULL;					//Window handle
DIJOYSTATE2				js;								//DirectInput joystick state

//int		width = 1280, height = 1024;	//Window width and height [lab: w1280/h1024; experiment: w1920/h1080]
int		width = GetSystemMetrics(SM_CXSCREEN);
int		height = GetSystemMetrics(SM_CYSCREEN);
bool	fullScreen = TRUE;				//Start in fullscreen mode flag
bool	dialog = TRUE;					//Dialog result flag
bool	move = FALSE;					//Start with no movement flag
float	display_width = 103.0;			//Monitor width in cm (experimental monitor: 46" diag, 103cm width)(lab monitor, 24" diag, 52.7cm width)
float	e = 295.0;						//Viewing distance in cm, check for accuracy (experimental: 295cm)(lab: 70cm)
float	eye_height = 160;				//Cm, not so important to measure
int		plane = 325;					//Size of plane that dots will show up in
float	plane_width;					//
int		displaynum;						//
float	vis_ang;						//Visual angle based on display width and visual angle;
char	datafilename[100];				//Wheel/sine data filename string
char	RMSfilename[100];				//RMS data filename string
char	thresholdname[100];				//Threshold data filename string
char	init[10];						//Subject initials string
char	c[10];
char	num[10];						//Subject number string
int		numtrials = trials;				//Number of trials
int		trial = 0;						//Trial counter
float	rms[trials];					//RMS score array
float	wheelX = 0;						//Steering wheel position
int		start = 0;						//
int		frame = 0;						//Frame counter
bool	firstframe;						//The start of each trial
int		prev_frame = 0;					//Frame counter
int		pracFrame;						//Practice frames; changed from #define variables to global so they could be manipulated
int		expFrame;						//Experiment frames
int		NFRAMES;						//Total frames
int		points;							//Number of dots
float	avg = 0;						//Average of RMS scores for first trials
bool	ps3wheel = 1;					//Are you using the ps3 wheel? default is 1 (yes), 2 is (no)

typedef struct _starRec					//Dot position structure
{
	float x;							//X coordinate
	float y;							//Y coordinate
	float z;							//Z coordinate
} starRec;								//Return

typedef struct _sinusoid				//Sine function structure
{
	double freq;						//Frequency of function
	double amp;							//Amplitude of function
	double phase;						//Phase of function
} sinusoid;								//Return

typedef struct _stimuli					//Stimulus structure
{
	starRec point[MAXpoint];			//Array of dots
	int displaynumber;					//Which condition to display
} stimuli;								//Return

typedef struct _conditions				//Condition structure
{
	int numpoints;						//Dot density
	//float dotbright;					//Dot brightness
	//float backbright;					//Background brightness
	float rms;							//RMS score
	int answer;							//Staircase answer
} conditions;							//Return

struct data								//Data structure
{
	float wheel[2400];					//Wheel position array, we only need 8100, but providing 9000 for safe measure
	float sine[2400];					//Sine value array
};

clock_t ticks1, ticks2;					//Gets current time
clock_t initial_time;					//Start time
ofstream datafile;						//Output file for wheel/sine data
ofstream rmsdata;						//Output file for RMS scores
ofstream threshold;						//Output file for final threshold value
stimuli trialS[trials];					//24 trials
conditions conditionS[trials];			//24 conditions
sinusoid sinusoidS[3];					//3 sine patterns
starRec stars[MAXpoint];				//MAXpoint dots
data Data[trials];						//24 arrays
TwoAFC staircase;						//Staircase class instance

int NewPlane(int dispnum, GLint start, GLint numpoint, float radius, float zback)
{   
	int n;
	float x0, y0;
	
	for (n=0; n<numpoint; n++){	
		trialS[dispnum].point[start + n].x = (float)rand()/(float)(RAND_MAX+1)*radius-radius/2.;
		trialS[dispnum].point[start + n].y = -eye_height;//(float)rand()/(float)(RAND_MAX+1)*radius-radius/2.;
		trialS[dispnum].point[start + n].z = (float)rand()/(float)(RAND_MAX+1)*zback;
		
		while(abs(trialS[dispnum].point[start + n].x) > (display_width/2)/e*trialS[dispnum].point[start + n].z||
			trialS[dispnum].point[start + n].z < e){ 
			trialS[dispnum].point[start + n].x = (float)rand()/(float)(RAND_MAX+1)*radius-radius/2.;
			//trialS[dispnum].point[start + n].y = -eye_height;
			trialS[dispnum].point[start + n].z = (float)rand()/(float)(RAND_MAX+1)*zback;
		}
 
		x0 = e*trialS[dispnum].point[start + n].x /trialS[dispnum].point[start + n].z;
		y0 = e*trialS[dispnum].point[start + n].y /trialS[dispnum].point[start + n].z;
		
	}
	return start + n;
}

float sin_vel(int frame, sinusoid sinusoidS)
{
	return sinusoidS.amp*sin((float)frame/FPS*sinusoidS.freq*2*PI+sinusoidS.phase);
}

float sum_sin_vel(int frame)
{
	return sin_vel(frame, sinusoidS[1])+sin_vel(frame, sinusoidS[2])+sin_vel(frame, sinusoidS[3]);
}

void Sinusoid(void)
{
	float k=500;  //constant that determines the magnitude of the amplitudes				

	//3 nonharmonic sines have relatively prime frequencies

	sinusoidS[1].freq = (double)(5.0/60.0); //5 cycles per minute
	sinusoidS[2].freq = 9.7/60.0; //9.7 cycles per minute
	sinusoidS[3].freq = 13/60.0; //13 cycles per minute

	//the 3 sinusoids are to have equal energy, the products of their freq and amp must be the same.
	//a consequence of this is that the amplitude of one is proportionate to the product of the other two frequencies.
	//with the above frequencies and k, we get amplitudes that range from about 3 to about 10

	sinusoidS[1].amp = k*sinusoidS[2].freq*sinusoidS[3].freq;
	sinusoidS[2].amp = k*sinusoidS[1].freq*sinusoidS[3].freq;
	sinusoidS[3].amp = k*sinusoidS[1].freq*sinusoidS[2].freq;

	//the first two phases are randomly chosen, the 3rd is chosen so that the sum of sines will initially be zero.
	sinusoidS[2].phase = (float)rand()/(float)(RAND_MAX+1)*2*PI;
	//sinusoidS[2].phase = 2.47;
	sinusoidS[3].phase = (float)rand()/(float)(RAND_MAX+1)*2*PI;;
	//sinusoidS[3].phase = 0.23815;
	sinusoidS[1].phase = asin((-sin_vel(0,sinusoidS[2])- sin_vel(0,sinusoidS[3]))/sinusoidS[1].amp);
}

void WriteResponse(void)
{
	//For wheel/sine data file
	for (int x = pracFrame; x < NFRAMES; x++) //Excluding the practice frames, from the beginning of trial to the end
	{
		for (int y = 0; y < trials; y++) //For the N trials
		{
			datafile << Data[y].sine[x]<<"\t"<<Data[y].wheel[x]<<"\t";	//Outputs the sine value, tab, wheel value, tab
		}
		datafile << "\n";												//Line return
	}

	//For threshold data file
	if (staircase.numbReversals < staircase.maxReversals)
	{

	//if (staircase.currentValue <= staircase.minValue || staircase.currentValue >= staircase.maxValue)
	//{
		threshold << ceil(staircase.currentValue);	//Output the final threshold value (current value or max or min)
	}else
	{
		threshold << ceil(staircase.average6);	//Output the final threshold value (average of reversals)
		}
}

void CalcRMS(void)
{
	//This code can be consolidated. It was broken up for debugging purposes
	float diff = 0;				//Difference between Sine and Wheel
	float square = 0;			//Difference squared
	long double RMSaccum = 0;	//Difference running total
	float rms = 0;				//Final RMS score

	for (int x = pracFrame; x < NFRAMES; x ++)
	{
		diff = Data[trial].sine[x] - Data[trial].wheel[x]; //Get difference between sine and wheel
		square = diff * diff;	//Square the difference
		RMSaccum += square;		//Running total
	}
		rms = sqrt(RMSaccum/expFrame);		//RMS = SquareRoot of average
		conditionS[trial].rms = rms;
		
		cout<< rms << "\n";
}

void CalcAvg(void)
{
	float run = 0;		//Running total
	for (int a = 0; a < basetrials; a++)
	{
		run += conditionS[a].rms;	//Running total
	}
	avg = run/(basetrials);			//Calculate average
	cout << "average: " << avg << "\n";	//Output average to console

	rmsdata << "Average:" << "\t" << avg << "\t" << "Cutoff:" <<"\t" << avg/cutoff << "\n";	//Output average and cutoff to RMS data file
	rmsdata << "Trial:" << "\t" << "Dots:" << "\t" << "RMS:" << "\t" << "Ans:" << "\n";		//Output headers to RMS data file
}

void EndofTrial(void)
{
	int answer = 0;
	CalcRMS();									//Calculate the RMS score for the previous trial
	if (trial < (basetrials-1))					//Just output the scores if still base trials
	{
		rmsdata <<  trial << "\t" << conditionS[trial].numpoints << "\t" << conditionS[trial].rms << "\t" << conditionS[trial].answer << "\n";
	}
	else if (trial == (basetrials-1))			//If it's the last base trial, get an average of the base trials
	{
		rmsdata <<  trial << "\t" << conditionS[trial].numpoints << "\t" << conditionS[trial].rms << "\t" << conditionS[trial].answer << "\n";
		CalcAvg();
	}
	else if (trial >= basetrials)				//If the current trial is after the base trials
	{
		if (conditionS[trial].rms <= (avg/cutoff))	//If previous RMS score is less than the average
		{
			staircase.correct = 1;				//Mark as 'correct'
			conditionS[trial].answer = 1;							//Set answer to correct/1
		}
		else
		{		
			staircase.correct = 0;				//Else, mark as 'incorrect'
			conditionS[trial].answer = 0;							//Set answer to incorrect/0
		}

		staircase.countReversals();				//
		staircase.storeReversal();				//
		staircase.evalCorrect();				//
		staircase.calcResponse();				//
		rmsdata <<  trial << "\t" << conditionS[trial].numpoints << "\t" << conditionS[trial].rms << "\t" << conditionS[trial].answer << "\n";
	}

	trial++;								//Increment the trial
	Sinusoid();								//Create new sine function
	wheelX = 0;								//Reset
	frame=0;								//Reset
	prev_frame = 0;							//Reset
	firstframe = 1;							//Reset
}
void MoveStars(void)
{
	GLint n;

	pracFrame = 2*FPS;					//Practice frames; 5 seconds, which there is no lateral movement.
	expFrame = 15*FPS;					//Experimental frames; 15 seconds
	NFRAMES = expFrame + pracFrame;		//Total frames
	
	if(firstframe)			//At the beginning of the trial
	{
		ticks1 = clock();	//Start timer
		firstframe = 0;		//Set flag off
		wheelX = 0;			//Reset wheelX from last trial
	}

	if (frame < pracFrame)	//For the practice frames
	{		
		frame = (int)(FPS*(clock()-ticks1)/1000);		  
		wheelX = 0;			//If the user moves the steering wheel, the position will not be added when the trial begins
		if(frame>prev_frame)
		{
			for (n = 0; n < plane; n++)
			{
				trialS[trial].point[n].z -= SPEED;		//Move forward
			}
			prev_frame++;
		}
		
		if(frame==pracFrame)	//At start of trial
		{
			//Beep(800,200);		//System beep
			PlaySound(TEXT("beep.wav"),NULL,SND_FILENAME|SND_ASYNC);
		}
	}
	
	else if (frame < NFRAMES)	//For the remainder of the frames
	{	
		wheelX+=(float)(js.lX -32767)/32767* (float)steerRatio;	//Move the display according to wheel movement
		frame = (int)(FPS*(clock()-ticks1)/1000);				//Actual frame count	  
		int frame2 = frame - pracFrame;							//Start at frame 1 for sine function after practice

		if(frame>prev_frame)
		{
			for (n = 0; n < plane; n++)
			{
				trialS[trial].point[n].x -= wheelX;							//Move with wheel position
				trialS[trial].point[n].x += sum_sin_vel(frame2);			//Move with sine function
				trialS[trial].point[n].z -= SPEED;							//Move forward
				Data[trial].sine[prev_frame] = sum_sin_vel(frame2);			//Write data to array
				Data[trial].wheel[prev_frame] = wheelX;						//Write data to array
			}
			prev_frame++;
		}
	}
	else					//At the end of the trial
	{
		ticks2 = clock();	//
		prev_frame = 0;		//Reset
		firstframe = 1;		//Reset
		
		if(staircase.numbReversals == staircase.maxReversals || 
			staircase.currentValue <= staircase.minValue || 
			staircase.currentValue >= staircase.maxValue ||
			trial > trials)		//Shutdown after the maxReversals has been met
		{
			Beep(500,400);	//System beep
			ShutDown();		//Exit the program
		}
	}
}

void ShowStars(void)
{
	int n;			//Index
	float x0, y0;	//X and y coordinates
	
	if (trial < basetrials )							//For the first 4 trials
	{
		glColor3f(1.0,1.0,1.0 );			//For testing purposes, makes the dots red for the first trials
		points = 325;						//Make the number of points the max
		conditionS[trial].numpoints = points;	//Set trial dot number into array
		//points = staircase.maxValue;		//To get this to work, make public
	}
	else
	{
		glColor3f(1.0,1.0,1.0 );			//For testing purposes, makes the remaining trials white dots
		points = staircase.currentValue;	//Make the number of points to staircase value
		conditionS[trial].numpoints = points;	//Set trial dot number into array
	}

	glPointSize(4.0);				//Set dot size
	glBegin(GL_POINTS);				//Begins list of vertices
	
	for (n = 0; n < points; n++)
	{
		if(trialS[trial].point[n].z < 0)
		{
			trialS[trial].point[n].x = (float)rand()/(float)(RAND_MAX+1)*plane_width-plane_width/2.;
			trialS[trial].point[n].z += MAXPOS;
		}
		
		if(abs(trialS[trial].point[n].x) > (display_width/2)/e*trialS[trial].point[n].z)
		{
			trialS[trial].point[n].x = -trialS[trial].point[n].x;
		}

		x0 = e*trialS[trial].point[n].x /(trialS[trial].point[n].z);
		y0 = e*trialS[trial].point[n].y /(trialS[trial].point[n].z);
	
		glVertex2f(x0,y0);				//Draws the dots	
	}

	glEnd();							//End the list of vertices
}

void CreateDisplays(void)
{
	int n = 0;

	for (displaynum = 0; displaynum < trials ; displaynum++)
	{
		n = NewPlane(displaynum, 0, plane, plane_width, MAXPOS);
		trialS[displaynum].displaynumber = displaynum;
		n = 0;
	}
}

void UpdateStars(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//Clears the buffer to draw again
}

void GetFileName(void)
{
	bool filenameok = 0;

	CreateDirectory("data",NULL);				//Creates a folder for the data files
	strcpy_s(datafilename, "data\\TE_");		//Change this to get the data in the right place
	strcat_s(datafilename, num);				//Append initials
	strcat_s(datafilename, "_");				//Append underscore
	strcat_s(datafilename, init);				//Append subject number
	strcat_s(datafilename, "_Dot_DATA_3.8");		//Append underscore
	strcat_s(datafilename, ".txt");				//Append file extension
	
	fstream fs(datafilename, ios_base::in);		//Attempt open for read
	while(!filenameok)
	{
		if (!fs)	//File doesn't exist; create a new one
		{
			fs.open(datafilename, ios_base::out); 
			filenameok = 1;
		}
		else //Ok, file exists; close
		{
			fs.close();
			//Pop Up A Message Box Letting User Know The Program Is Closing.
			MessageBox(NULL,"File Already Exists.","ERROR",MB_OK|MB_ICONSTOP);
			exit(0);	
		}
		datafile.open(datafilename); 
		if (!datafile)//data directory not found
		{   
			//Pop Up A Message Box Letting User Know The Program Is Closing.
			MessageBox(NULL,"Directory Not Found.","ERROR",MB_OK|MB_ICONSTOP);
			exit(0);
		}
	}
}

void CreateRMSfile(void)
{
	bool filenameok = 0;
	
	strcpy_s(RMSfilename, "data\\TE_");		//Change this to get the data in the right place
	strcat_s(RMSfilename, num);				//Append initials
	strcat_s(RMSfilename, "_");				//Append underscore
	strcat_s(RMSfilename, init);			//Append subject number
	strcat_s(RMSfilename, "_Dot_RMS_3.8");		//Append underscore
	strcat_s(RMSfilename, ".txt");			//Append file extension
	
	fstream fs(RMSfilename, ios_base::in);		//Attempt open for read
	while(!filenameok)
	{
		if (!fs)	//File doesn't exist; create a new one
		{
			fs.open(RMSfilename, ios_base::out); 
			filenameok = 1;
		}
		else //Ok, file exists; close
		{
			fs.close();
			//Pop Up A Message Box Letting User Know The Program Is Closing.
			MessageBox(NULL,"File Already Exists.","ERROR",MB_OK|MB_ICONSTOP);
			exit(0);	
		}
		rmsdata.open(RMSfilename); 
		if (!rmsdata)//data directory not found
		{   
			//Pop Up A Message Box Letting User Know The Program Is Closing.
			MessageBox(NULL,"Directory Not Found.","ERROR",MB_OK|MB_ICONSTOP);
			exit(0);
		}
	}
}

void CreateThresholdFile(void)
{
	bool filenameok = 0;
	
	strcpy_s(thresholdname, "data\\TE_");		//Change this to get the data in the right place
	strcat_s(thresholdname, num);				//Append initials
	strcat_s(thresholdname, "_");				//Append underscore
	strcat_s(thresholdname, init);				//Append subject number
	strcat_s(thresholdname, "_Dot_THRESHOLD_3.8");		//Append underscore
	strcat_s(thresholdname, ".txt");			//Append file extension
	
	fstream fs(thresholdname, ios_base::in);		//Attempt open for read
	while(!filenameok)
	{
		if (!fs)	//File doesn't exist; create a new one
		{
			fs.open(thresholdname, ios_base::out); 
			filenameok = 1;
		}
		else //Ok, file exists; close
		{
			fs.close();
			//Pop Up A Message Box Letting User Know The Program Is Closing.
			MessageBox(NULL,"File Already Exists.","ERROR",MB_OK|MB_ICONSTOP);
			exit(0);	
		}
		threshold.open(thresholdname); 
		if (!threshold)//data directory not found
		{   
			//Pop Up A Message Box Letting User Know The Program Is Closing.
			MessageBox(NULL,"Directory Not Found.","ERROR",MB_OK|MB_ICONSTOP);
			exit(0);
		}
	}
}

void Initialize()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);					//Clears the color bit
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	//Really Nice Perspective Calculations
	glEnable(GL_DEPTH_TEST);

	firstframe = 1;							//Set the flag to true
	vis_ang = 2*atan((display_width/2)/e);	//Calculates the visual angle
	plane_width = display_width/e*MAXPOS;	//Calculates the plane width

	staircase.initStaircase();						//Initializes the staircase
	srand((unsigned int) time(NULL));				//Random seed
	initial_time = ((int)(clock()/CLOCKS_PER_SEC));	//
	Sinusoid();										//Creates the sinusoidal function
	CreateDisplays();								//Creates list of display numbers
	ticks1 = clock();								//

	return;
}

void ShutDown(void)
{
	WriteResponse();	//Write data file
	FreeDirectInput();	//Releases all DirectInput devices
	exit(0);			//Closes the window
	return;
}

void ReshapeScene(GLsizei width, GLsizei height)
{
	if (height == 0)
	{
	  height = 1;		//Prevent from dividing by zero
	}

	glViewport(0, 0,width, height);		//Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-display_width/2, display_width/2, -display_width/2*height/width, display_width/2*height/width);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return;
}

void RenderScene()
{
	glLoadIdentity();	//Clears all states to start drawing
		
	if (move == TRUE)	//If move has been set on, move stars
	{
		MoveStars();	//Gets new coordinates
		UpdateStars();	//Clears screen
		ShowStars();	//Draws the dots
	}
	else				//If move not set, don't move dots
	{
		UpdateStars();	//Clears screen
		ShowStars();	//Draws the dots
	}
		glFlush();
}

void IllustrateScene()
{
	glClearColor(0,0,0,0);	//Sets background color	
	glClear(GL_COLOR_BUFFER_BIT);	//Clears the color buffer bit
	RenderScene();					//Draws the scene

	return;
}

HRESULT InitDirectInput( HWND hwnd )
{
    HRESULT hr;

    //Register with the DirectInput subsystem and get a pointer
    //to a IDirectInput interface we can use.
    if( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION,
                                         IID_IDirectInput8, ( VOID** )&g_pDI, NULL ) ) )
    {
        return hr;
    }

    //Look for a force feedback device we can use
    if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL,
                                         EnumFFDevicesCallback, g_pDI,
                                         DIEDFL_ATTACHEDONLY | DIEDFL_FORCEFEEDBACK ) ) )
    {
        return hr;
    }

	//Make sure we got a joytick
    if( NULL == g_pDevice )
    {
		MessageBox( NULL,  "Force feedback device not found. ",
                           "Threshold Experiment" ,
						   MB_ICONERROR | MB_OK );
        EndDialog( hwnd, 0 );
        return S_OK;
    }

	//Set the data format to "simple joystick" - a predefined data format. A
    //data format specifies which controls on a device we are interested in,
    //and how they should be reported. This tells DirectInput that we will be 
	//passing a DIJOYSTATE structure to IDirectInputDevice8::GetDeviceState(). 
    if( FAILED( hr = g_pDevice->SetDataFormat( &c_dfDIJoystick2 ) ) )
        return hr;

    //Set the cooperative level to let DInput know how this device should
    //interact with the system and with other DInput applications.
    //Exclusive access is required in order to perform force feedback.
	if( FAILED( hr = g_pDevice->SetCooperativeLevel( hwnd,
                                                     DISCL_EXCLUSIVE |
                                                     DISCL_FOREGROUND ) ) )
    {
		return hr;
    }

    //Since we will be playing force feedback effects, we should disable the
    //auto-centering spring.
	DIPROPDWORD dipdw;		//data structure for joystick
    dipdw.diph.dwSize = sizeof( DIPROPDWORD );
    dipdw.diph.dwHeaderSize = sizeof( DIPROPHEADER );
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
    dipdw.dwData = DIPROPAUTOCENTER_ON;
	
	//Set the property values of the joystick
	if( FAILED( hr = g_pDevice->SetProperty( DIPROP_AUTOCENTER, &dipdw.diph ) ) )
       return hr;

    //Enumerate and count the axes of the joystick 
    if( FAILED( hr = g_pDevice->EnumObjects( EnumAxesCallback,
											( VOID* )&g_dwNumForceFeedbackAxis, DIDFT_AXIS ) ) )
        return hr;

    //This simple sample only supports one or two axis joysticks
    if( g_dwNumForceFeedbackAxis > 2 )
        g_dwNumForceFeedbackAxis = 2;

    //This application needs only one effect: Applying raw forces.
    DWORD rgdwAxes[2] = { DIJOFS_X, DIJOFS_Y };
    LONG rglDirection[2] = { 0, 0 };
    DICONSTANTFORCE cf = { 0 };
   
	DIEFFECT eff;		//Data structure for force feedback effect
    ZeroMemory( &eff, sizeof( eff ) );
    eff.dwSize = sizeof( DIEFFECT );
    eff.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    eff.dwDuration = INFINITE;
    eff.dwSamplePeriod = 0;
    eff.dwGain = DI_FFNOMINALMAX;
    eff.dwTriggerButton = DIEB_NOTRIGGER;
    eff.dwTriggerRepeatInterval = INFINITE;
    eff.cAxes = g_dwNumForceFeedbackAxis;
    eff.rgdwAxes = rgdwAxes;
    eff.rglDirection = rglDirection;
    eff.lpEnvelope = NULL;
    eff.cbTypeSpecificParams = sizeof( DICONSTANTFORCE );
    eff.lpvTypeSpecificParams = &cf;
    eff.dwStartDelay = 0;
	
    //Create the prepared effect
    if( FAILED( hr = g_pDevice->CreateEffect( GUID_ConstantForce,
                                              &eff, &g_pEffect, NULL ) ) )
    {
        return hr;
    }

    if( NULL == g_pEffect )
        return E_FAIL;

	return S_OK;
}

BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext )
{
    DWORD* pdwNumForceFeedbackAxis = ( DWORD* )pContext;

    if( ( pdidoi->dwFlags & DIDOI_FFACTUATOR ) != 0 )
		( *pdwNumForceFeedbackAxis )++;
		
	return DIENUM_CONTINUE;

}

BOOL CALLBACK EnumFFDevicesCallback( const DIDEVICEINSTANCE* pInst, VOID* pContext )
{
    LPDIRECTINPUTDEVICE8 pDevice;	//DirectInput device
    HRESULT hr;

    //Obtain an interface to the enumerated force feedback device.
    hr = g_pDI->CreateDevice( pInst->guidInstance, &pDevice, NULL );

    //If it failed, then we can't use this device for some
    //bizarre reason.  (Maybe the user unplugged it while we
    //were in the middle of enumerating it.)  So continue enumerating
    if( FAILED( hr ) )
        return DIENUM_CONTINUE;

    //We successfully created an IDirectInputDevice8.  So stop looking 
    //for another one.
    g_pDevice = pDevice;	//set g_pDevice (steering wheel) to the device that we found

	return DIENUM_STOP;

}

VOID FreeDirectInput()
{
    //Unacquire the device one last time just in case 
    //the app tried to exit while the device is still acquired.
    if( g_pDevice )
        g_pDevice->Unacquire();

    //Release any DirectInput objects.
    SAFE_RELEASE( g_pEffect );
    SAFE_RELEASE( g_pDevice );
    SAFE_RELEASE( g_pDI );
}


HRESULT UpdateInputState(HWND hwnd)
{
	HRESULT hr;
	
	if (NULL == g_pDevice)		//
		return S_OK;
	
	hr = g_pDevice->Poll();		//Poll the device to read the current state
	
	if( FAILED( hr ) )
    {
        //DInput is telling us that the input stream has been
        //interrupted. We aren't tracking any state between polls, so
        //we don't have any special reset that needs to be done. We
        //just re-acquire and try again.
        hr = g_pDevice->Acquire();
		while( hr == DIERR_INPUTLOST )
            hr = g_pDevice->Acquire();
	
        //hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        //may occur when the app is minimized or in the process of 
        //switching, so just try again later 
        return S_OK;
    }

    //Get the input's device state
	if( FAILED( hr = g_pDevice->GetDeviceState( sizeof( DIJOYSTATE2 ), &js ) ) )
	{
		return hr; //The device should have been acquired during the Poll()
	}

	//Increment trials when a joystick button is pressed

	if (ps3wheel)	//Use these codes for the shift lever and horn
	{
		if (js.rgbButtons[12] & 0x80 && frame > (expFrame-1))	//If shift lever is pressed toward after a 10 second start 12/13
		{
			EndofTrial();
		}

		if (js.rgbButtons[13] & 0x80 && frame > (expFrame-1))	//If shift lever is pressed away after a 10 second start
		{
			EndofTrial();
		}

		if (js.rgbButtons[19] & 0x80)	//When the horn is pressed
		{
			move = TRUE;									//Start dot movement
		}
	}
	else	//Use this codes for paddles and shift levers
	{
		if (js.rgbButtons[8] & 0x80 && frame > (expFrame-1))	//If shift lever is pressed toward after a 10 second start 12/13
		{
			EndofTrial();
		}

		if (js.rgbButtons[9] & 0x80 && frame > (expFrame-1))	//If shift lever is pressed away after a 10 second start
		{
			EndofTrial();
		}

		if (js.rgbButtons[0] || js.rgbButtons[1] & 0x80)	//When either paddle is pressed
		{
			move = TRUE;									//Start dot movement
		}
	}
	

	return S_OK;

}
HRESULT SetDeviceForcesXY()
{
    //Modifying an effect is basically the same as creating a new one, except
    //you need only specify the parameters you are modifying
    long rglDirection[2] = { 0, 0 };
	DICONSTANTFORCE cf;

	//If only one force feedback axis, then apply only one direction
    if( g_dwNumForceFeedbackAxis == 1 )
    {
		if (frame < pracFrame)		//No wind, so no force feedback
		{
			cf.lMagnitude = 0;
			rglDirection[0] = 0;
		}
		else if (frame < NFRAMES)	//Set the force feeback to value of sin function to replicate the wind
		{
			cf.lMagnitude = sum_sin_vel(frame)*285;	//285 derived from range of FF magnitude / range of sin wave
			rglDirection[0] = 0;
		}
		else if (frame >= NFRAMES)	//Trial has ended, so no force feedback
		{
			cf.lMagnitude = 0;
			rglDirection[0] = 0;
		}
    }
    else
    {
        //If two force feedback axis, then apply magnitude from both directions 
        rglDirection[0] = 100;
        rglDirection[1] = 100;
        cf.lMagnitude = ( DWORD )sqrt( ( double )100 * ( double )100 +
                                       ( double )100 * ( double )100 );
    }

    DIEFFECT eff;	//Effect data structure
    ZeroMemory( &eff, sizeof( eff ) );
    eff.dwSize = sizeof( DIEFFECT );
    eff.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff.dwDuration = NULL;
	eff.dwTriggerButton = NULL;
    eff.cAxes = g_dwNumForceFeedbackAxis;
    eff.rglDirection = rglDirection;
    eff.lpEnvelope = NULL;
    eff.cbTypeSpecificParams = sizeof( DICONSTANTFORCE );
	eff.lpvTypeSpecificParams = &cf;
    eff.dwStartDelay = 0;

    //Now set the new parameters and start the effect immediately.
    return g_pEffect->SetParameters( &eff, DIEP_DIRECTION |
                                     DIEP_TYPESPECIFICPARAMS |
                                     DIEP_START );
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SIZE:										//If resized
			ReshapeScene(LOWORD(lParam), HIWORD(lParam));	//Apply ReShape settings
			return 0;
			break;

		case WM_ACTIVATE:									//If activated
			if( WA_INACTIVE != wParam && g_pDevice )
			{		
				//g_pDevice->Acquire();						//Make sure the device is acquired, if we are gaining focus.

				//if( g_pEffect )
				//g_pEffect->Start( 1, 0 );					//Start the force feedback effect

				//g_pDevice->Poll();							//Poll the steering wheel
			}
			break;

		case WM_KEYDOWN:									//If a key is pressed down
			if (wParam == VK_SPACE)							//Change to next condition with spacebar
			{
				trial++;									//Increment the trial
				if(trial>numtrials-1)
				{
					ShutDown();								//Shutdown once we reached end of trials
				}
				Sinusoid();									//Create new sine function
				wheelX = 0;									//Reset
				frame=0;									//Reset
				prev_frame = 0;								//Reset
				firstframe = 1;								//Reset				
			}
			else if (wParam == VK_ESCAPE)					//Exit the program if Esc key is pressed
			{
				ShutDown();
			}
			break;

		case WM_DESTROY:									//If the window is closed
			ShutDown();										//Shutdown
			return 0;
			break;

		case WM_CLOSE:
			PostQuitMessage(0);								//Send quit message
			return 0;
    }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}
INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_COMMAND:
			switch (LOWORD(wparam))
		{
			case IDOK:										//After pressing 'OK'
				GetDlgItemText(hwnd, initials, init, 10);	//Gets the subject's initials
				GetDlgItemText(hwnd, number, num, 10);		//Gets the subject's number
				EndDialog(hwnd, IDOK);						//Close the dialog box

				//This converts any lowercase initials to uppercase
				locale loc;
				use_facet < ctype<char> > (loc).toupper ( init, init+sizeof(init));
				//cout << init;

				return true;
		}
	}
	return false;
}
HWND BuildWindow(LPCTSTR szTitle, HINSTANCE hInstance, int width, int height)
{
	int			pf = 0;					//Holds The Results After Searching For A Match
	HDC			hdc   = NULL;			//
	HWND		hwnd = NULL;			//Window handle
	DWORD		dwExStyle;				//Window Extended Style
	DWORD		dwStyle;				//Window Style
	RECT		WindowRect;				//Grabs Rectangle Upper Left /Lower Right Values
	DEVMODE		devModeScreen;			//Fullscreen data structure
	WindowRect.left=(long)0;			//Set Left Value To 0
	WindowRect.right=(long)width;		//Set Right Value To Requested Width
	WindowRect.top=(long)0;				//Set Top Value To 0
	WindowRect.bottom=(long)height;		//Set Bottom Value To Requested Height
	
	WNDCLASS wndClass = {	CS_HREDRAW | CS_VREDRAW | CS_OWNDC,		//Redraw On Size, And Own DC For Window.
							WindowProc,								//WndProc Handles Messages
							0,										//No Extra Window Data
							0,										//No Extra Window Data
							hInstance,								//Set The Instance
							LoadIcon(NULL, IDI_WINLOGO),			//Load The Default Icon
							LoadCursor(NULL, IDC_ARROW),			//Load The Arrow Pointer
							NULL,									//No Background Required For GL
							NULL,									//We Don't Want A Menu
							"OpenGL" };								//Set The Class Name

	if (!RegisterClass(&wndClass))									//Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;												//Return FALSE
	}

	if (fullScreen)
	{
		memset(&devModeScreen,0, sizeof(devModeScreen));	//Clear the DEVMODE structure
		devModeScreen.dmSize = sizeof(devModeScreen);		//Size of the structure
		devModeScreen.dmPelsWidth = width;					//Set the width
		devModeScreen.dmPelsHeight = height;				//Set the height
		devModeScreen.dmBitsPerPel = 16;					//Set the the bits per pixel
		devModeScreen.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

		//Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&devModeScreen,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			//If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullScreen=FALSE;		//Windowed Mode Selected. Fullscreen = FALSE
			}
			else
			{
				//Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									//Return FALSE
			}
		}
	}

	//Ask The User Which Screen Mode They Prefer
	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullScreen=FALSE;										//Windowed Mode
	}

	if (fullScreen)												//Are We Still In Fullscreen Mode?
	{
		dwExStyle =WS_EX_APPWINDOW;								//Window Extended Style
		dwStyle=WS_POPUP ;										//Windows Style
		ShowCursor(FALSE);										//Hide Mouse Pointer
	}
	else														//If we aren't in fullscreen mode
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			//Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							//Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);	//Adjust Window To True Requested Size
   
	hwnd = CreateWindowEx(	dwExStyle,							//Extended Style For The Window
							"OpenGL",							//Class Name
							szTitle,							//Window Title
							dwStyle |							//Defined Window Style
							WS_CLIPSIBLINGS |					//Required Window Style
							WS_CLIPCHILDREN,					//Required Window Style
							0,0,								//Window Position
							WindowRect.right - WindowRect.left,	//Calculate Window Width
							WindowRect.bottom - WindowRect.top,	//Calculate Window Height
							NULL,								//No Parent Window
							NULL,								//No Menu
							hInstance,							//Instance
							NULL);								//Dont Pass Anything To WM_CREATE

	hdc = GetDC(hwnd);							//Return a handle to a display device context (DC) that has the font set onto it
	PIXELFORMATDESCRIPTOR pfd = {0};			//Pfd Tells Windows How We Want Things To Be

	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize      = sizeof(pfd);				//Size Of This Pixel Format Descriptor
	pfd.nVersion   = 1;							//Version Number
	pfd.dwFlags    = PFD_DRAW_TO_WINDOW |		//Format Must Support Window
					 PFD_SUPPORT_OPENGL |		//Format Must Support OpenGL
					 PFD_DOUBLEBUFFER;			//Must Support Double Buffering
	pfd.iPixelType = PFD_TYPE_RGBA;				//Request An RGBA Format
	pfd.cDepthBits = 32;						//Select Our Color Depth
	pfd.cColorBits = 16;						//Select our Color Bits
	pfd.cStencilBits = 8;						//Select our Stencil Bits

	pf = ChoosePixelFormat(hdc, &pfd);			//
	SetPixelFormat(hdc, pf, &pfd);				//Requests that the encoder use the specified pixel format
	ReleaseDC(hwnd, hdc);						//Release a device context handle

	return hwnd;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	#ifdef _DEBUG			//Allows the write to a console
	RedirectIOToConsole();
	#endif

	//Creates a modal dialog box from a dialog box template resource
	int dialogResult = DialogBoxParam(NULL,							//Handle to the module whose executable file contains the dialog box template
									MAKEINTRESOURCE(IDD_DIALOG1),	//Specifies the dialog box template
									NULL,							//Handle to the window that owns the dialog box
									DialogProc,						//Pointer to the dialog box procedure
									(LPARAM)0);						//Specifies the value to pass to the dialog box in the lParam parameter of the WM_INITDIALOG message
	
	GetFileName();													//Creates wheel/sine data output file
	CreateRMSfile();												//Creates RMS data output file 
	CreateThresholdFile();											//Creates Threshold data output file
	staircase.StairOutput();										//Creates Staircase data output file
	
	if ((hwnd = BuildWindow("Threshold Experiment - Dots", hInstance, width, height)) != NULL)
	{
		HDC   hdc = GetDC(hwnd);			//Device context of device that OpenGL calls are to be drawn on
		HGLRC hrc = wglCreateContext(hdc);	//Handle to the OpenGL rendering context to delete
		wglMakeCurrent(hdc, hrc);			//Makes a specified OpenGL rendering context the calling thread's current rendering context
		Initialize();						//Initialize OpenGL environment
		InitDirectInput(hwnd);				//Initialize DirectInput environment
		ShowWindow(hwnd, nCmdShow);			//
		SetForegroundWindow(hwnd);			//Slightly Higher Priority
		SetFocus(hwnd);						//Sets Keyboard Focus To The Window
		ReshapeScene(width, height);		//Set Up Our Perspective GL Screen
		HeartBeat(hdc);						//Makes the program come to life
		wglMakeCurrent(hdc, NULL);			//Makes a specified OpenGL rendering context the calling thread's current rendering context
		wglDeleteContext(hrc);				//Deletes a specified OpenGL rendering context
		ReleaseDC(hwnd, hdc);				//Release a device context handle
	}

return 0;
}

BOOL HeartBeat(HDC hdc)
{
	MSG		msg   = {0};	//Message
	BOOL	bActive = TRUE;	//Variable to say we are done
	BOOL	bMsg = FALSE;	//Second message

	while (msg.message != WM_QUIT)
	{
		if (bActive != FALSE)
		{
			bMsg = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		}
		else
		{
			bMsg = GetMessage(&msg, NULL, 0, 0);
		}
      
		if (bMsg != FALSE)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	
		IllustrateScene();			//Illustrates scene
		UpdateInputState(hwnd);		//Gets the joystick state
		//SetDeviceForcesXY();		//Apply force feedback effects [THIS IS DISABLED FOR EXPERIMENT 1]
		SwapBuffers(hdc);			//Swaps the drawing buffer for motion
	}
   return TRUE;
}
