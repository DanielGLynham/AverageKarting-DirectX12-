#include "ModeManager.h"

ModeManager::ModeManager()
{
	mode = 0;
}

const int ModeManager::GetMode() const
{
	return mode;
}

void ModeManager::SetMode(const int mode)
{
	this->mode = mode;
}