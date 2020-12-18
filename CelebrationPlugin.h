#pragma once
#pragma comment(lib, "BakkesMod.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"

class CelebrationPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	bool enabled = false;
	Vector FOCUS;
	Rotator ROTATION, SWIVEL;
	float DISTANCE, FOV;
	bool overrideValue[8];
	bool isInBallCam = false;
	bool isInRearCam = false;

public:
	void onLoad() override;
	void onUnload() override;

	void CreateValues();
	void CelebrationReset();
	void ResetButton();

	void Initialize();
	bool CanCreateValues();
	bool IsCVarNull(std::string cvarName);
	void Enable();
	void Disable();
	void HandleValues();
	Rotator GetSwivel();
};
