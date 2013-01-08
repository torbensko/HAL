#include "cbase.h"

#include <vgui/ILocalize.h>
#include "filesystem.h"

#include "hal/settings_panel.h"
#include "hal/hal.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"




GameUI<CHTSettingsPanel> g_htSettingsPanel;
GameUI<CHL2MapPanel> g_HL2MapPanel;

IGameUI* GetHTSettingsPanel() { return &g_htSettingsPanel; }
IGameUI* GetHL2MapPanel() { return &g_HL2MapPanel; }

CON_COMMAND(ShowHeadOptions, NULL)	{ g_htSettingsPanel.GetPanel()->SetVisible(true); }
CON_COMMAND(ShowMapPanel,NULL)		{ g_HL2MapPanel.GetPanel()->SetVisible(true); }

CON_COMMAND(ResetHeadPosition, NULL)
{
	UTIL_ResetHeadPosition();
}





#define SLIDER_SCALE_FLOAT2INT 100.0f 

void HTSlider::SetValue(int value, bool bTriggerChangeMessage)
{
	vgui::Slider::SetValue(value, bTriggerChangeMessage);
	if(m_conVar)
		m_conVar->SetValue(value/SLIDER_SCALE_FLOAT2INT);
}

void HTSlider::SetRange(float min, float max)
{
	Slider::SetRange(int (min * SLIDER_SCALE_FLOAT2INT), int (max * SLIDER_SCALE_FLOAT2INT));
}

void HTSlider::GetRange(float &min, float &max)
{
	int iMin, iMax;
	Slider::GetRange(iMin, iMax);
	min = iMin / SLIDER_SCALE_FLOAT2INT;
	max = iMax / SLIDER_SCALE_FLOAT2INT;
}

void HTSlider::OnThink()
{
	Slider::OnThink();
	if(m_conVar && m_conVar->GetFloat() != GetValue()/SLIDER_SCALE_FLOAT2INT)
		vgui::Slider::SetValue(int (m_conVar->GetFloat() * SLIDER_SCALE_FLOAT2INT));
}





HTTextEntry::HTTextEntry(Panel *parent, const char *panelName, ConVar *var) : TextEntry(parent, panelName)
{ 
	m_conVar = var;
	SetAllowNumericInputOnly(true);
	m_value = -1;
}

void HTTextEntry::OnKeyTyped(wchar_t unichar)
{
	TextEntry::OnKeyTyped(unichar);
	if(m_conVar)
	{
		char buf[32];
		GetText(buf, 32);
		m_conVar->SetValue(buf);
		m_value = m_conVar->GetFloat();
	}
}


void HTTextEntry::OnThink()
{
#define TEXT_ENTRY_BUFLEN 32

	TextEntry::OnThink();
	if(m_conVar && m_conVar->GetFloat() != m_value)
	{
		char text[TEXT_ENTRY_BUFLEN];

		m_value = m_conVar->GetFloat();
		//int iValue = (int) floor(m_value);

		V_snprintf(text, TEXT_ENTRY_BUFLEN, "%3.2f", m_value );
		vgui::TextEntry::SetText(text);
	}
}


// --------------------------------------------------------
// CONVAR RADIO BUTTON
// --------------------------------------------------------
HTRadioButton::HTRadioButton(Panel *parent, const char *panelName, const char *text, ConVar *var, int option) : RadioButton(parent, panelName, text)
{
	m_conVar = var;
	m_option = option;
	OnThink();
}

void HTRadioButton::OnMousePressed(vgui::MouseCode code)
{
	RadioButton::OnMousePressed(code);
	m_conVar->SetValue(m_option);
}

void HTRadioButton::OnThink()
{
	RadioButton::OnThink();
	if(m_conVar)
		SetSelected(m_conVar->GetInt() == m_option);
}


// --------------------------------------------------------
// CONVAR CHECK BUTTON
// --------------------------------------------------------
HTCheckButton::HTCheckButton(Panel *parent, const char *panelName, const char *text, ConVar *var) : CheckButton(parent, panelName, text)
{
	m_conVar = var;
	SetSelected(m_conVar->GetBool());
}

void HTCheckButton::OnMousePressed(vgui::MouseCode code)
{
	CheckButton::OnMousePressed(code);
	m_conVar->SetValue((int)IsSelected());
}

void HTCheckButton::OnThink()
{
	CheckButton::OnThink();
	if(m_conVar->GetBool() != IsSelected())
		SetSelected(m_conVar->GetBool());
}

// --------------------------------------------------------
// CONVAR COMBO BOX
// --------------------------------------------------------
HTComboBox::HTComboBox(Panel *parent, const char *panelName, int numOfValues, std::map<int,char*> & values, ConVar *var)
		: ComboBox(parent, panelName, numOfValues, false)
{
	std::map<int,char*>::iterator it;

	// show content:
	for(it = values.begin(); it != values.end(); it++)
	{
		int index = AddItem((*it).second, NULL);
		m_valueToIndex[(*it).first] = index;
		m_indexToValue[index]		= (*it).first;
	}

	firstThink = true;
	firstCall = true;
	m_conVar = var;
}

void HTComboBox::OnMenuItemSelected()
{
	ComboBox::OnMenuItemSelected();

	if(m_conVar && !firstCall)
		m_conVar->SetValue(m_indexToValue[GetActiveItem()]);

	// hacky, but this always seems to be called when loading
	firstCall = false;
}

void HTComboBox::OnThink()
{
	ComboBox::OnThink();

	if(m_conVar && (firstThink || m_conVar->GetInt() != m_indexToValue[GetActiveItem()]))
	{
		ActivateItem(m_valueToIndex[m_conVar->GetInt()]);
		firstThink = false;
	}
}















CHTSettingsPanel::CHTSettingsPanel( vgui::VPANEL parent ) : BaseClass( NULL, "HeadSettings" )
{
 	SetParent(parent);

	// general-setting:
	HT_OPTION_CREATE(adaptSmoothAmount_p);

	// handy-cam options
	HT_OPTION_CREATE(handySmoothing_sec);

	HT_OPTION_CREATE(handyScale_f);
	HT_OPTION_CREATE(handyScalePitch_f);
	HT_OPTION_CREATE(handyScaleRoll_f);
	HT_OPTION_CREATE(handyScaleYaw_f);
	HT_OPTION_CREATE(handyScaleVert_f);
	HT_OPTION_CREATE(handyScaleSidew_f);

	HT_OPTION_CREATE(handyMaxPitch_deg);
	HT_OPTION_CREATE(handyMaxYaw_deg);
	HT_OPTION_CREATE(handyMaxRoll_deg);
	HT_OPTION_CREATE(handyMaxVert_cm);
	HT_OPTION_CREATE(handyMaxSidew_cm);

	// leaning options
	HT_OPTION_CREATE(leanOffsetMin_cm);
	HT_OPTION_CREATE(leanOffsetRange_cm);
	HT_OPTION_CREATE(leanRollMin_deg);
	HT_OPTION_CREATE(leanRollRange_deg);
	HT_OPTION_CREATE(leanSmoothing_sec);
	HT_OPTION_CREATE(leanEaseIn_p);
	HT_OPTION_CREATE(leanStabilise_p);

	m_resetToDefaults = vgui::SETUP_PANEL(new vgui::Button(this, "RestoreDefaults", ""));
	m_close = vgui::SETUP_PANEL(new vgui::Button(this, "Close", ""));
	
	// sets their position
 	LoadControlSettings("Resource/UI/HeadSettings.res");

	// general-setting:
	HT_OPTION_SETUP(adaptSmoothAmount_p);

	// handy-cam options
	HT_OPTION_SETUP(handySmoothing_sec);

	HT_OPTION_SETUP(handyScale_f);
	HT_OPTION_SETUP(handyScalePitch_f);
	HT_OPTION_SETUP(handyScaleRoll_f);
	HT_OPTION_SETUP(handyScaleYaw_f);
	HT_OPTION_SETUP(handyScaleVert_f);
	HT_OPTION_SETUP(handyScaleSidew_f);

	HT_OPTION_SETUP(handyMaxPitch_deg);
	HT_OPTION_SETUP(handyMaxYaw_deg);
	HT_OPTION_SETUP(handyMaxRoll_deg);
	HT_OPTION_SETUP(handyMaxVert_cm);
	HT_OPTION_SETUP(handyMaxSidew_cm);

	// leaning options
	HT_OPTION_SETUP(leanOffsetMin_cm);
	HT_OPTION_SETUP(leanOffsetRange_cm);
	HT_OPTION_SETUP(leanRollMin_deg);
	HT_OPTION_SETUP(leanRollRange_deg);
	HT_OPTION_SETUP(leanSmoothing_sec);
	HT_OPTION_SETUP(leanEaseIn_p);
	HT_OPTION_SETUP(leanStabilise_p);

	// Buttons:
	m_resetToDefaults->SetCommand("RestoreDefaults");
	m_close->SetCommand("Close");

	SetSizeable(false);
	SetMoveable(true);
	SetCloseButtonVisible(true);
} 

void CHTSettingsPanel::OnCommand(const char* command)
{
	if(!stricmp(command, "RestoreDefaults")) 
	{	
		HT_OPTION_REVERT(leanOffsetMin_cm);
		HT_OPTION_REVERT(leanOffsetRange_cm);
		HT_OPTION_REVERT(leanRollMin_deg);
		HT_OPTION_REVERT(leanRollRange_deg);
		HT_OPTION_REVERT(leanSmoothing_sec);
		HT_OPTION_REVERT(leanEaseIn_p);
		HT_OPTION_REVERT(leanStabilise_p);
		
		HT_OPTION_REVERT(handySmoothing_sec);

		HT_OPTION_REVERT(handyScale_f);
		HT_OPTION_REVERT(handyScalePitch_f);
		HT_OPTION_REVERT(handyScaleRoll_f);
		HT_OPTION_REVERT(handyScaleYaw_f);
		HT_OPTION_REVERT(handyScaleVert_f);
		HT_OPTION_REVERT(handyScaleSidew_f);

		HT_OPTION_REVERT(handyMaxPitch_deg);
		HT_OPTION_REVERT(handyMaxYaw_deg);
		HT_OPTION_REVERT(handyMaxRoll_deg);
		HT_OPTION_REVERT(handyMaxVert_cm);
		HT_OPTION_REVERT(handyMaxSidew_cm);

		HT_OPTION_REVERT(adaptSmoothAmount_p);
	}
	else if(!stricmp(command, "ResetNeutral"))
	{
		engine->ClientCmd("ResetHeadPosition");
	}
	else if(!stricmp(command, "Close"))
	{
		SetVisible(false);
	}
}

// positioned under the lean settings, on the left
void CHTSettingsPanel::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if(state)
	{
		CenterPanelOnScreen(this);
		MoveToFront();
	}
	
}






CHL2MapPanel::CHL2MapPanel( vgui::VPANEL parent ) : BaseClass( NULL, "MapPanel" )
{
 	SetParent(parent);

	m_maps		= vgui::SETUP_PANEL(new vgui::ComboBox(this, "MapList", 10, false));
	m_loadMap	= vgui::SETUP_PANEL(new vgui::Button(this, "LoadMap", ""));
	m_cancel	= vgui::SETUP_PANEL(new vgui::Button(this, "Cancel", ""));
	m_intro		= vgui::SETUP_PANEL(new vgui::Label(this, "Intro", ""));
	
 	LoadControlSettings("Resource/UI/MapPicker.res");

	m_loadMap->SetCommand("loadmap");
	m_cancel->SetCommand("close");

	FileFindHandle_t maps;
	char buf[128];

	const char *mapname = filesystem->FindFirst("maps/*.bsp", &maps);
	while(mapname != NULL) {
		engine_sprintf(buf, 128, "%s", mapname);
		char *end = strstr(buf, ".bsp");
		if(end) *end = '\0';
		if(strncmp(buf, "background", 10))
			if(strncmp(buf, "fo_", 3))
				m_maps->AddItem(buf, NULL);
		mapname = filesystem->FindNext(maps);
	}
	if(maps != NULL)
		FindClose((HANDLE) maps);
	
	if(m_maps->GetItemCount() == 0) 
	{
		engine_sprintf(buf, 128, "No compatible maps found");
		m_maps->AddItem(buf, NULL);
	}
	// we know that we have at least item in the combo
	m_maps->ActivateItem(0);

 	CenterThisPanelOnScreen();
 	SetVisible(false);
	SetSizeable(false);
}

void CHL2MapPanel::OnCommand(const char* command) 
{
	//DevMsg(command);
	if(!stricmp(command, "loadmap")) 
	{
		char buf[64];
		m_maps->GetItemText(m_maps->GetActiveItem(), buf, 64);
		if(strlen(buf) > 0) 
		{
			char cmd[68];
			engine_sprintf(cmd, 68, "map %s", buf);
			engine->ClientCmd(cmd);
		}
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void CHL2MapPanel::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if(state)
		MoveToFront();
}



