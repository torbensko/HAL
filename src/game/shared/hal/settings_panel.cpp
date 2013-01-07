#include "cbase.h"

#include "hal/settings_panel.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"



#define PANELS_FLUSH_BUFFER	5


GameUI<CHTSettingsPanel> g_htSettingsPanel;

CON_COMMAND(ShowHeadOptions, NULL)
{
	GetHTSettingsPanel()->GetPanel()->SetVisible(true);
}

CON_COMMAND(ResetHeadPosition, NULL)
{
	//GetHTGame()->MakeCurrentHeadPositionNeutral();
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

 	//vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
 	//SetScheme( scheme );

	// general-setting:
	HT_OPTION_CREATE(adaptSmoothAmount_p);

	// handy-cam options	
	HT_OPTION_CREATE(handyScale_f);
	HT_OPTION_CREATE(handyScalePitch_f);
	HT_OPTION_CREATE(handyScaleRoll_f);
	HT_OPTION_CREATE(handyScaleYaw_f);
	HT_OPTION_CREATE(handyScaleOffsets_f);
	HT_OPTION_CREATE(adaptSmoothConfSample_sec);
	HT_OPTION_CREATE(handyMaxPitch_deg);
	HT_OPTION_CREATE(handyMaxYaw_deg);

	// leaning options
	HT_OPTION_CREATE(leanOffsetMin_cm);
	HT_OPTION_CREATE(leanOffsetRange_cm);
	HT_OPTION_CREATE(leanRollMin_deg);
	HT_OPTION_CREATE(leanRollRange_deg);
	HT_OPTION_CREATE(leanSmoothing_sec);
	HT_OPTION_CREATE(leanScale_f);
	HT_OPTION_CREATE(leanEaseIn_p);
	HT_OPTION_CREATE(leanStabilise_p);
	//HT_OPTION_CREATE(leanButtonAmount_p);


	//std::map<int,char*> values;
	//values[DROPOUT_OPTION_NOTHING] = "Nothing";
	//values[DROPOUT_OPTION_NOTIFY] = "Notify";
	//values[DROPOUT_OPTION_PAUSING] = "Pause";
	//m_dropoutOpt = vgui::SETUP_PANEL(new HTComboBox(this, "DropoutOpt", values.size(), values, &fo_dropoutOpt));

	//std::map<int,char*> resetValues;
	//resetValues[RESET_OPTION_FULL]		= "Hard reset";
	//resetValues[RESET_OPTION_POSITION]	= "Soft reset";
	//m_resetOpt = vgui::SETUP_PANEL(new HTComboBox(this, "ResetOpt", resetValues.size(), resetValues, &fo_resetOpt));

	m_resetToDefaults = vgui::SETUP_PANEL(new vgui::Button(this, "RestoreDefaults", ""));
	//m_resetNeutral = vgui::SETUP_PANEL(new vgui::Button(this, "ResetNeutral", ""));
	m_close = vgui::SETUP_PANEL(new vgui::Button(this, "Close", ""));
	
	// sets their position
 	LoadControlSettings("Resource/UI/HeadSettings.res");

	// general-setting:
	HT_OPTION_SETUP(adaptSmoothAmount_p);

	// handy-cam options
	HT_OPTION_SETUP(handyScale_f);
	HT_OPTION_SETUP(handyScalePitch_f);
	HT_OPTION_SETUP(handyScaleRoll_f);
	HT_OPTION_SETUP(handyScaleYaw_f);
	HT_OPTION_SETUP(handyScaleOffsets_f);
	HT_OPTION_SETUP(adaptSmoothConfSample_sec);
	HT_OPTION_SETUP(handyMaxPitch_deg);
	HT_OPTION_SETUP(handyMaxYaw_deg);

	// leaning options
	HT_OPTION_SETUP(leanOffsetMin_cm);
	HT_OPTION_SETUP(leanOffsetRange_cm);
	HT_OPTION_SETUP(leanRollMin_deg);
	HT_OPTION_SETUP(leanRollRange_deg);
	HT_OPTION_SETUP(leanSmoothing_sec);
	HT_OPTION_SETUP(leanScale_f);
	HT_OPTION_SETUP(leanEaseIn_p);
	//HT_OPTION_SETUP(leanButtonAmount_p);
	HT_OPTION_SETUP(leanStabilise_p);

	// Buttons:
	//m_resetNeutral->SetCommand("ResetNeutral");
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
		//HT_OPTION_REVERT(dropoutOpt);
		//HT_OPTION_REVERT(resetOpt);
		//fo_showData.Revert();
		
		HT_OPTION_REVERT(leanOffsetMin_cm);
		HT_OPTION_REVERT(leanOffsetRange_cm);
		HT_OPTION_REVERT(leanRollMin_deg);
		HT_OPTION_REVERT(leanRollRange_deg);
		HT_OPTION_REVERT(leanSmoothing_sec);
		HT_OPTION_REVERT(leanScale_f);
		HT_OPTION_REVERT(leanEaseIn_p);
		//HT_OPTION_REVERT(leanButtonAmount_p);
		HT_OPTION_REVERT(leanStabilise_p);
		
		HT_OPTION_REVERT(handyScale_f);
		HT_OPTION_REVERT(handyScalePitch_f);
		HT_OPTION_REVERT(handyScaleRoll_f);
		HT_OPTION_REVERT(handyScaleYaw_f);
		HT_OPTION_REVERT(handyScaleOffsets_f);
		HT_OPTION_REVERT(adaptSmoothConfSample_sec);
		HT_OPTION_REVERT(handyMaxPitch_deg);
		HT_OPTION_REVERT(handyMaxYaw_deg);
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

IGameUI* GetHTSettingsPanel() { return &g_htSettingsPanel; }