/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
As such you are free to use the code for any purpose as long as you remember 
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND, 
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE.

*/

#include "cbase.h"

#include "hal/hal.h"
#include "hal/util.h"

#define FILTER_ROLL		0
#define FILTER_PITCH	1
#define FILTER_YAW		2
#define FILTER_VERT		3
#define FILTER_SIDEW	4
#define FILTER_LEAN		5


HALTechnique* __hal;

HALTechnique::HALTechnique() {
	__hal = this;
}

// We initialise it here, to ensure the other parts of the system have been
// initialised themselves - such as the TunableVars

void HALTechnique::Init()
{
	m_faceAPI.Init();

	// Setup the filtering of the head data:
	m_handySmoothing_auto = new TunableVar("hal_handySmoothing_auto", "-1", 0); // for increasing the smoothing during low confidence periods
	m_leanSmoothing_auto = new TunableVar("hal_leanSmoothing_auto", "-1", 0);
	m_smoothedConf = new SmoothFilter(&hal_adaptSmoothConfSample_sec);
	m_handyScaleAuto = new TunableVar("hal_handyScale_auto", "-1", 0); // for suppressing the handycam while leaning

	// These are used by both the handy-cam and leaning, hence why we create them first
	WeightedMeanOffsetFilter *meanRoll = new WeightedMeanOffsetFilter(FACEAPI_ROLL, &hal_leanRollMin_deg);
	MeanOffsetFilter *meanYaw = new MeanOffsetFilter(FACEAPI_YAW);
	MeanOffsetFilter *meanPitch = new MeanOffsetFilter(FACEAPI_PITCH);
	MeanOffsetFilter *meanVert = new MeanOffsetFilter(FACEAPI_VERT);
	MeanOffsetFilter *meanSidew = new MeanOffsetFilter(FACEAPI_SIDEW);

	// change this to alter how each aspect of the head data is filtered
	m_filteredHeadData[FILTER_ROLL] =
			new FadeFilter(&hal_fadingDuration_s,
				new LimitFilter(&hal_handyMaxRoll_deg, 
					new ScaleFilter(&hal_handyScaleRoll_f, 
						new ScaleFilter(&hal_handyScale_f,
							new ScaleFilter(m_handyScaleAuto,
								new SmoothFilter(m_handySmoothing_auto, meanRoll) )))));

	m_filteredHeadData[FILTER_PITCH] =
			new FadeFilter(&hal_fadingDuration_s, 
				new LimitFilter(&hal_handyMaxPitch_deg,
					new ScaleFilter(&hal_handyScalePitch_f, 
						new ScaleFilter(&hal_handyScale_f,
							new ScaleFilter(m_handyScaleAuto,
								new SmoothFilter(m_handySmoothing_auto, meanPitch) )))));
	
	m_filteredHeadData[FILTER_YAW] =
			new FadeFilter(&hal_fadingDuration_s, 
				new LimitFilter(&hal_handyMaxYaw_deg,
					new ScaleFilter(&hal_handyScaleYaw_f, 
						new ScaleFilter(&hal_handyScale_f,
							new ScaleFilter(m_handyScaleAuto,
								new SmoothFilter(m_handySmoothing_auto, meanYaw) )))));
	
	m_filteredHeadData[FILTER_VERT] =
			new FadeFilter(&hal_fadingDuration_s, 
				new LimitFilter(&hal_handyMaxVert_cm,
					new ScaleFilter(&hal_handyScaleOffsets_f, 
						new ScaleFilter(&hal_handyScale_f,
							new ScaleFilter(m_handyScaleAuto,
								new SmoothFilter(m_handySmoothing_auto, meanVert) )))));
	
	m_filteredHeadData[FILTER_SIDEW] = 
			new FadeFilter(&hal_fadingDuration_s, 
				new LimitFilter(&hal_handyMaxSidew_cm, 
					new ScaleFilter(&hal_handyScaleOffsets_f, 
						new ScaleFilter(&hal_handyScale_f,
							new ScaleFilter(m_handyScaleAuto,
								new SmoothFilter(m_handySmoothing_auto, meanSidew) )))));

	m_filteredHeadData[FILTER_LEAN] =
			new FadeFilter(&hal_fadingDuration_s,
				new EaseInFilter(&hal_leanEaseIn_p, 
					new ClampFilter(-1, 1, 
						new ScaleFilter(&hal_leanScale_f, 
							new SumFilter(
								new NormaliseFilter(&hal_leanRollMin_deg, &hal_leanRollRange_deg, 
									new SmoothFilter(m_leanSmoothing_auto, meanRoll)
								),
								new NormaliseFilter(&hal_leanOffsetMin_cm, &hal_leanOffsetRange_cm,
									new SmoothFilter(m_leanSmoothing_auto, meanSidew)
								)
							)
						)
					)
				)
			);
}

void HALTechnique::Shutdown()
{
	m_faceAPI.Shutdown();
}

void HALTechnique::Update()
{
	if(!m_faceAPI.IsReady())
		return;

	FaceAPIData	data = m_faceAPI.GetHeadData();

	if(data.h_confidence > 0.0f)
	{
		// Update our adaptive smoothing value
		float adapt = 1 - (data.h_confidence - hal_adaptSmoothMinConf_f.GetFloat()) / 
				(hal_adaptSmoothMaxConf_f.GetFloat() - hal_adaptSmoothMinConf_f.GetFloat());
		adapt = 1 + clamp(adapt, 0, 1) * hal_adaptSmoothAmount_p.GetFloat() / 100.0f;
		adapt = m_smoothedConf->Update(adapt);

		m_handySmoothing_auto->SetValue(hal_handySmoothing_sec.GetFloat() * adapt);
		m_leanSmoothing_auto->SetValue(hal_leanSmoothing_sec.GetFloat() * adapt);

		// We suppress the yaw and pitch when rolling to ensure they don't interfere with the leaning technique
		m_handyScaleAuto->SetValue(
				1 - min(1, hal_leanStabilise_p.GetFloat()/100.0f * fabs(m_filteredHeadData[FILTER_LEAN]->GetValue())) );
		
		DevMsg("adapt: %6.2f, handy: %6.2f\n", adapt, m_handyScaleAuto->GetFloat());

		//m_filteredHeadData[FILTER_LEAN]->Update(data);
		for(int i = 0; i < sizeof(m_filteredHeadData)/sizeof(Filter*); i++)
			m_filteredHeadData[i]->Update(data);
	}
	else
	{
		for(int i = 0; i < sizeof(m_filteredHeadData)/sizeof(Filter*); i++)
			m_filteredHeadData[i]->Update();
	}
}

CameraOffsets HALTechnique::GetCameraShake()
{
	CameraOffsets offset;
	offset.pitch	= m_filteredHeadData[FILTER_PITCH]->GetValue();
	offset.roll		= m_filteredHeadData[FILTER_ROLL]->GetValue();
	offset.yaw		= m_filteredHeadData[FILTER_YAW]->GetValue();
	offset.vertOff	= m_filteredHeadData[FILTER_VERT]->GetValue();
	offset.horOff	= m_filteredHeadData[FILTER_SIDEW]->GetValue();
	return offset;
}

float HALTechnique::GetLeanAmount()
{
	return m_filteredHeadData[FILTER_LEAN]->GetValue();
}

float UTIL_GetLeanAmount()
{
	return (__hal) ? __hal->GetLeanAmount() : 0;
}

CameraOffsets UTIL_GetHandycamShake()
{
	return (__hal) ? __hal->GetCameraShake() : CameraOffsets();
}