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

#include "cbase.h"

#include <windows.h>
#include <iostream>

#include "hal/faceapi.h"
#include "hal/util.h"
#include "hal/engine_dependencies.h"

using namespace std;

// makes it compatible with v3 of the faceAPI (but not preferred)
#define USE_CALLBACKS

#define THROW_ON_ERROR(x) \
{ \
    smReturnCode result = (x); \
	engine_printf("faceAPI: %d\n", result); \
}

FaceAPIData::FaceAPIData()
{
	h_headPos[FACEAPI_ROLL] = 0.0f;
	h_headPos[FACEAPI_PITCH] = 0.0f;
	h_headPos[FACEAPI_YAW] = 0.0f;
	h_headPos[FACEAPI_VERT] = 0.0f;
	h_headPos[FACEAPI_SIDEW] = 0.0f;
	h_headPos[FACEAPI_DEPTH] = 0.0f;

	h_confidence = 0.0f;
	h_frameNum = 0;
}

FaceAPI*	_faceapi;

#ifdef USE_FACEAPI_4

bool _faceapi_fetcher_running;
void __cdecl FaceAPI_dataFetcher(void *)
{
	_faceapi_fetcher_running = true;
	while(_faceapi->InternalDataFetch()) {}
	_faceapi_fetcher_running = false;
	_endthread();
}

// How we fetch the head data differs between FaceAPI 3 and 4

bool FaceAPI::InternalDataFetch()
{
	if(!engine_handle || m_shuttingDown)
		return false;

	float now = ENGINE_NOW;

	smEngineData enginedata;
	smReturnCode result = smEngineDataWaitNext(engine_handle, &enginedata, 5000);

	if(result != SM_API_OK)
	{
		engine_printf("error fetching faceAPI data\n");
	}
	else
	{
		m_data[m_nextData].h_height			= enginedata.head_pose_data->head_pos.y;
		m_data[m_nextData].h_width			= enginedata.head_pose_data->head_pos.x;
		m_data[m_nextData].h_depth			= enginedata.head_pose_data->head_pos.z;
		m_data[m_nextData].h_yaw			= enginedata.head_pose_data->head_rot.y_rads;
		m_data[m_nextData].h_pitch			= enginedata.head_pose_data->head_rot.x_rads;
		m_data[m_nextData].h_roll			= enginedata.head_pose_data->head_rot.z_rads;
		m_data[m_nextData].h_confidence		= enginedata.head_pose_data->confidence;

		m_data[m_nextData].h_frameNum		= enginedata.video_frame.frame_num;
		m_data[m_nextData].h_frameDuration	= now - m_lastUpdate;

		m_currData = m_nextData;
		m_nextData = (m_nextData + 1) % 3;
		
		m_lastUpdate = now;
	}
	
	smEngineDataDestroy(&enginedata);
	return true;
}
#else
void STDCALL receiveHeadPose(void *, smEngineHeadPoseData head_pose, smCameraVideoFrame video_frame)
{
	_faceapi->SetData(head_pose);
}
 
void FaceAPI::SetData(smEngineHeadPoseData head_pose)
{
	if(!engine_handle || m_shuttingDown)
		return;

	m_data[m_nextData].h_headPos[FACEAPI_VERT]		= METERS_TO_SOURCE(head_pose.head_pos.y);
	m_data[m_nextData].h_headPos[FACEAPI_SIDEW]		= -METERS_TO_SOURCE(head_pose.head_pos.x);
	m_data[m_nextData].h_headPos[FACEAPI_DEPTH]		= METERS_TO_SOURCE(head_pose.head_pos.z);
	m_data[m_nextData].h_headPos[FACEAPI_YAW]		= RAD_TO_DEG(head_pose.head_rot.y_rads);
	m_data[m_nextData].h_headPos[FACEAPI_PITCH]		= RAD_TO_DEG(head_pose.head_rot.x_rads);
	m_data[m_nextData].h_headPos[FACEAPI_ROLL]		= RAD_TO_DEG(head_pose.head_rot.z_rads);

	m_data[m_nextData].h_confidence		= head_pose.confidence;
	m_data[m_nextData].h_frameNum		= m_frame++;

	m_currData = m_nextData;
	m_nextData = (m_nextData + 1) % 3;
}
#endif

FaceAPI* GetFaceAPI() 
{
	if(_faceapi == NULL)
	{
		_faceapi = new FaceAPI();
		_faceapi->Init();
	}
	return _faceapi;
}

FaceAPI::FaceAPI() 
{
	// too early to initialise the actual tracking
	_faceapi = this;
	m_frame = 1;
	m_isReady = false;
}

// The main function: setup a tracking engine and show a video window, then loop on the keyboard.
void FaceAPI::Init()
{
    // Log API debugging information to a file (good for tech support)
    THROW_ON_ERROR(smLoggingSetFileOutputEnable(SM_API_TRUE));

#   ifdef _DEBUG
    // Hook up log message callback
    THROW_ON_ERROR(smLoggingRegisterCallback(0,receiveLogMessage));
#   endif

    // Get the version
    //int major, minor, maint;
    THROW_ON_ERROR(smAPIVersion(&m_versionMajor, &m_versionMinor, &m_versionMaintenance));
	engine_printf("faceAPI version: %d.%d.%d\n", m_versionMajor, m_versionMinor, m_versionMaintenance);
    
	// Print detailed license info
    char *buff;
    int size;
    THROW_ON_ERROR(smAPILicenseInfoString(0,&size,SM_API_TRUE));
    buff = new char[size];
    THROW_ON_ERROR(smAPILicenseInfoString(buff,&size,SM_API_TRUE));
	engine_printf("faceAPI license: %s\n", buff);
	delete [] buff;

	// avoid loading any QT stuff, which currently has a bug associted to it
	THROW_ON_ERROR(smAPIInternalQtGuiDisable());
    
	// Initialize the API
    THROW_ON_ERROR(smAPIInit());

    // Register the WDM category of cameras
    THROW_ON_ERROR(smCameraRegisterType(SM_API_CAMERA_TYPE_WDM));

    // Create a new Head-Tracker engine that uses the camera
    THROW_ON_ERROR(smEngineCreate(SM_API_ENGINE_LATEST_HEAD_TRACKER,&engine_handle));

    // Check license for particular engine version (always ok for non-commercial license)
    const bool engine_licensed = smEngineIsLicensed(engine_handle) == SM_API_OK;

#	ifdef SHOW_FACEAPI_WINDOW
	// Create and show a video-display window
    smVideoDisplayHandle video_display_handle = 0;
    THROW_ON_ERROR(smVideoDisplayCreate(engine_handle,&video_display_handle,0,TRUE));

    // Setup the VideoDisplay
	unsigned short g_overlay_flags(SM_API_VIDEO_DISPLAY_HEAD_MESH);
    THROW_ON_ERROR(smVideoDisplaySetFlags(video_display_handle,g_overlay_flags));

    smWindowHandle win_handle = 0;
    THROW_ON_ERROR(smVideoDisplayGetWindowHandle(video_display_handle,&win_handle));    
    SetWindowText(win_handle, _T(""));
	SetWindowPos(win_handle, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
	//SetWindowLong(win_handle, GWL_STYLE, GetWindowLong(win_handle, GWL_STYLE) & ~(WS_BORDER | WS_DLGFRAME | WS_THICKFRAME));
    //SetWindowLong(win_handle, GWL_EXSTYLE, GetWindowLong(win_handle, GWL_EXSTYLE) & ~WS_EX_DLGMODALFRAME);
#	endif

	//smHTV2SetHeadPoseFilterLevel(engine_handle, 2);

#	ifndef USE_FACEAPI_4
	THROW_ON_ERROR(smHTRegisterHeadPoseCallback(engine_handle, 0, receiveHeadPose));
#	endif

    // Start tracking
    THROW_ON_ERROR(smEngineStart(engine_handle));

#	ifdef USE_FACEAPI_4
	// start up the fetcher
	m_shuttingDown = false;
	m_currData = 0;
	m_nextData = 1;
	(HANDLE)_beginthread(FaceAPI_dataFetcher, 0, (void *) 0);
#	endif

	m_isReady = true;
}

void FaceAPI::Shutdown() 
{
	m_isReady = false;

#	ifdef USE_FACEAPI_4
	// wait for the fetcher thread to die
	m_shuttingDown = true;
	while(_faceapi_fetcher_running)
		Sleep(100);
#	endif

	// Destroy engine
    THROW_ON_ERROR(smEngineDestroy(&engine_handle));
	THROW_ON_ERROR(smAPIQuit());
}

void FaceAPI::GetVersion(int &major, int &minor, int &maintenance)
{
	major		= m_versionMajor;
	minor		= m_versionMinor;
	maintenance = m_versionMaintenance;
}

void FaceAPI::RestartTracking()
{
	THROW_ON_ERROR(smEngineStart(engine_handle));
}

FaceAPIData FaceAPI::GetHeadData()
{
	int curr = m_currData;
	return m_data[curr];
}

float FaceAPI::GetTrackingConf()
{
	return m_data[m_currData].h_confidence;
}

void FaceAPI::GetCameraDetails(char *modelBuf, int bufLen, int &framerate, int &resWidth, int &resHeight)
{
	smCameraVideoFormat video_format;
	smStringHandle model_name;
	smCameraHandle camera_handle;
	int length;
	wchar_t *camera_model;
	smStringCreate(&model_name);
	smEngineGetCamera(engine_handle, &camera_handle);
	smCameraGetCurrentFormat(camera_handle, &video_format);
	smCameraGetModelName(camera_handle, model_name);
	smStringGetBufferW(model_name, &camera_model, &length);
	engine_sprintf(modelBuf, bufLen, "%ls", camera_model);
	if(strlen(modelBuf) > 0)
	{
		framerate = (int)video_format.framerate;
		resWidth = video_format.res.w;
		resHeight = video_format.res.h;
	}
	else
	{
		framerate = 0;
		resWidth = 0;
		resHeight = 0;
	}
	smStringDestroy(&model_name);
}