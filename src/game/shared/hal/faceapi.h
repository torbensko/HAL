/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
As such you are free to use the code for any purpose as long as you remember 
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND, 
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE.

This code in this file was originally taken from one of Seeing Machines'
sample programs (file: TestAppConsole.cpp).

*/

#ifndef FACEAPI_H
#define FACEAPI_H

#include <string>
#include <sstream>

#include "sm_api.h"
typedef struct smEngineHandle__* smEngineHandle;

// Once the faceAPI 4 has been publicly released, uncomment this line to use it
//#define USE_FACEAPI_4

#define FACEAPI_ROLL	0
#define FACEAPI_YAW		1
#define FACEAPI_PITCH	2
#define FACEAPI_VERT	3
#define FACEAPI_SIDEW	4
#define FACEAPI_DEPTH	5

class FaceAPIData
{
public:
	FaceAPIData();

	float			h_headPos[6];
	float			h_confidence;
	unsigned int	h_frameNum;
};

class FaceAPI
{
public:
	FaceAPI();
	void			Init();
	void			Shutdown();
	void			GetVersion(int &major, int &minor, int &maintenance);
	void			GetCameraDetails(char *modelBuf, int bufLen, int &framerate, int &resWidth, int &resHeight);
	void			RestartTracking();

	FaceAPIData		GetHeadData();		// not a halting function
	float			GetTrackingConf();

	bool			IsReady() { return m_isReady; }
	
#	ifdef USE_FACEAPI_4
	bool			InternalDataFetch();
#	else
	void			SetData(smEngineHeadPoseData head_pose);
#	endif

protected:
	smEngineHandle	engine_handle;

	FaceAPIData		m_data[3];
	int				m_currData;
	int				m_nextData;

	bool			m_shuttingDown;

	int				m_versionMajor;
	int				m_versionMinor;
	int				m_versionMaintenance;

	int				m_frame;
	bool			m_isReady;
};

FaceAPI* GetFaceAPI();

#endif FACEAPI_H
