#ifndef HT_SETTINGS_H
#define HT_SETTINGS_H

#ifdef _WIN32
#pragma once
#endif

#include <map>

#include <vgui_controls/panel.h>
#include <vgui_controls/label.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/RadioButton.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Frame.h>

#include "hal/data_filtering.h"
#include "cdll_util.h"




// Taken from:
// http://developer.valvesoftware.com/wiki/VGUI:_Making_GameUI_Panels
//
// IGameUI - start

#define ToggleVisibility(panel)\
	panel->SetVisible(!panel->IsVisible());

class IGameUI
{
public:
	virtual void Create( vgui::VPANEL parent ) = 0;
	virtual void Destroy( void ) = 0;
	virtual vgui::Panel *GetPanel(void) = 0;
};

template <class T>
class GameUI : public IGameUI
{
public:
	GameUI()
	{
		m_pPanel = NULL;
	}

	void Create(vgui::VPANEL parent)
	{
		m_pPanel = new T(parent);
	}

	void Destroy( void )
	{
		if(m_pPanel)
		{
			m_pPanel->SetParent( (vgui::Panel *)NULL );
			delete m_pPanel;
		}

		m_pPanel = NULL;
	}

	T* GetNativePanel()
	{
		return m_pPanel;
	}

	vgui::Panel *GetPanel(void)
	{
		return GetNativePanel();
	}

private:
	T* m_pPanel;
};

#define CenterThisPanelOnScreen() CenterPanelOnScreen(this);

inline void CenterPanelOnScreen(vgui::Panel* panel)
{
	if (!panel)
		return;

	int x,w,h;

	panel->GetBounds(x,x,w,h);
	panel->SetPos((ScreenWidth()-w)/2,(ScreenHeight()-h)/2);
}

inline int PanelTop(vgui::Panel *pPanel)	{int x,y,w,h; pPanel->GetBounds(x,y,w,h); return y;}
inline int PanelLeft(vgui::Panel *pPanel)	{int x,y,w,h; pPanel->GetBounds(x,y,w,h); return x;}
inline int PanelRight(vgui::Panel *pPanel)	{int x,y,w,h; pPanel->GetBounds(x,y,w,h); return x+w;}
inline int PanelBottom(vgui::Panel *pPanel)	{int x,y,w,h; pPanel->GetBounds(x,y,w,h); return y+h;}
inline int PanelWidth(vgui::Panel *pPanel)	{int x,y,w,h; pPanel->GetBounds(x,y,w,h); return w;}
inline int PanelHeight(vgui::Panel *pPanel)	{int x,y,w,h; pPanel->GetBounds(x,y,w,h); return h;}

// Places child at the requested position inside pParent. iAlignment is from Label::Alignment.
void AlignPanel(vgui::Panel *pChild, vgui::Panel *pParent, int alignment);

// IGameUI - end




#define DROPOUT_OPTIONS 3
#define DROPOUT_OPTION_PAUSING 0
#define DROPOUT_OPTION_NOTIFY 1
#define DROPOUT_OPTION_NOTHING 2

#define RESET_OPTIONS 2
#define RESET_OPTION_POSITION 0
#define RESET_OPTION_FULL 1

#define HT_MODES				4
#define HT_MODE_NORMAL			0
#define HT_MODE_LEANING			1
#define HT_MODE_DISABLED		2
#define HT_MODE_HANDYCAM		3

#define HT_CONF_SMOOTHING_RANGE_MIN		0.48
#define HT_CONF_SMOOTHING_RANGE			0.22


#define HT_OPTION_DEFINE(var) \
	HTSlider *m_s##var;\
	HTTextEntry *m_t##var;

#define HT_OPTION_CREATE(var) \
	m_s##var = vgui::SETUP_PANEL(new HTSlider(this, "slider_"#var, &hal_##var));\
	m_t##var = vgui::SETUP_PANEL(new HTTextEntry(this, "text_"#var, &hal_##var));

	//vgui::Label *m_l##var = vgui::SETUP_PANEL(new vgui::Label(this, "text_"#var));

#define HT_OPTION_SETUP(var) \
	float min_##var, max_##var; \
	hal_##var.GetMin(min_##var);\
	hal_##var.GetMax(max_##var);\
	m_s##var->SetRange(min_##var, max_##var);

#define HT_OPTION_REVERT(var) \
	hal_##var.Revert();





class HTSlider : public vgui::Slider
{
public:
	HTSlider(Panel *parent, const char *panelName, TunableVar *var) : Slider(parent, panelName) { m_conVar = var; }

	void	SetValue(int value, bool bTriggerChangeMessage = true);
    void	SetRange(float min, float max);	 // set to max and min range of rows to display
	void	GetRange(float &min, float &max);
	
	void OnThink();
private:
	TunableVar *m_conVar;
};

class HTTextEntry : public vgui::TextEntry
{
public:
	HTTextEntry(Panel *parent, const char *panelName, TunableVar *var);
	virtual void OnKeyTyped(wchar_t unichar);
	void OnThink();
private:
	TunableVar *m_conVar;
	float	m_value;
};

class HTRadioButton : public vgui::RadioButton
{
	//DECLARE_CLASS_SIMPLE( HTRadioButton, vgui::RadioButton );
public:
	HTRadioButton(Panel *parent, const char *panelName, const char *text, TunableVar *var, int option);
	void OnThink();
	void OnMousePressed(vgui::MouseCode code);
private:
	TunableVar *m_conVar;
	int m_option;
};


class HTCheckButton : public vgui::CheckButton
{
public:
	HTCheckButton(Panel *parent, const char *panelName, const char *text, TunableVar *var);
	void OnMousePressed(vgui::MouseCode code);
	void OnThink();
private:
	TunableVar *m_conVar;
};


class HTComboBox : public vgui::ComboBox
{
public:
	HTComboBox(Panel *parent, const char *panelName, int numOfValues, std::map<int,char*> & values, TunableVar *var);
	void OnThink();
protected:
	void OnMenuItemSelected();
private:
	std::map<int,int> m_indexToValue;
	std::map<int,int> m_valueToIndex;
	TunableVar *m_conVar;
	bool firstThink;
	bool firstCall;
};






class CHTSettingsPanel : public vgui::Frame
{
 	DECLARE_CLASS_SIMPLE(CHTSettingsPanel, vgui::Frame);
 public:
 	CHTSettingsPanel( vgui::VPANEL parent );

	void OnCommand(const char* command);
	void SetVisible(bool state);
	//void OnThink();

private:
	vgui::Button *m_close;
	vgui::Button *m_resetToDefaults;

	//HTComboBox    *m_dropoutOpt;
	//HTComboBox    *m_resetOpt;
	//HTCheckButton *m_showData;
	
	HT_OPTION_DEFINE(adaptSmoothAmount_p);

	HT_OPTION_DEFINE(handySmoothing_sec);

	HT_OPTION_DEFINE(handyScale_f);
	HT_OPTION_DEFINE(handyScalePitch_f);
	HT_OPTION_DEFINE(handyScaleRoll_f);
	HT_OPTION_DEFINE(handyScaleYaw_f);
	HT_OPTION_DEFINE(handyScaleVert_f);
	HT_OPTION_DEFINE(handyScaleSidew_f);

	HT_OPTION_DEFINE(handyMaxPitch_deg);
	HT_OPTION_DEFINE(handyMaxYaw_deg);
	HT_OPTION_DEFINE(handyMaxRoll_deg);
	HT_OPTION_DEFINE(handyMaxVert_cm);
	HT_OPTION_DEFINE(handyMaxSidew_cm);

	HT_OPTION_DEFINE(leanOffsetMin_cm);
	HT_OPTION_DEFINE(leanOffsetRange_cm);
	HT_OPTION_DEFINE(leanRollMin_deg);
	HT_OPTION_DEFINE(leanRollRange_deg);
	HT_OPTION_DEFINE(leanSmoothing_sec);
	HT_OPTION_DEFINE(leanEaseIn_p);
	HT_OPTION_DEFINE(leanStabilise_p);
};

IGameUI* GetHTSettingsPanel();

#endif HT_SETTINGS_H