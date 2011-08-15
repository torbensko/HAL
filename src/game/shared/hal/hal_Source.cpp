#include "cbase.h"
#include "igamesystem.h"
#include "hal.h"

class GameCallbacks : CAutoGameSystemPerFrame
{
public:
	GameCallbacks( char const *name = NULL ) : CAutoGameSystemPerFrame( name ) {}
	bool Init();
	void Shutdown();
	void Update(float frametime);
private:
	HALTechnique m_HAL;
};

bool GameCallbacks::Init()
{
	m_HAL.Init();
	return true;
}

void GameCallbacks::Shutdown()
{
	m_HAL.Shutdown();
}

void GameCallbacks::Update(float frametime)
{
	m_HAL.Update();
}

GameCallbacks gameCallbacks("callbacks");
