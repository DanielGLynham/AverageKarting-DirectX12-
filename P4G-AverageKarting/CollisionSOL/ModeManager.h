#ifndef MODEMANAGER_H
#define MODEMANAGER_H

using namespace std;

class ModeManager
{
public:
	ModeManager();

	const int GetMode() const;
	void SetMode(const int mode);

private:
	int mode;
};
#endif