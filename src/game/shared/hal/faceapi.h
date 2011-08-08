/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
In a gist, you are free to use however you see fit, just please remember
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

class FaceAPIData
{
public:
	FaceAPIData();

	float			h_pitch;
	float			h_yaw;
	float			h_roll;
	float			h_depth;
	float			h_width;
	float			h_height;
	float			h_confidence;
	float			h_frameDuration;
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
	float			m_lastUpdate;

	bool			m_shuttingDown;

	int				m_versionMajor;
	int				m_versionMinor;
	int				m_versionMaintenance;

	int				m_frame;
};

#endif FACEAPI_H
