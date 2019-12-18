#include "CelebrationPlugin.h"
#include "bakkesmod\wrappers\includes.h"
#include <sstream>
#include <filesystem>

using namespace std;

BAKKESMOD_PLUGIN(CelebrationPlugin, "Template for creating camera control plugins", "1.0", PLUGINTYPE_FREEPLAY)

/*
TO-DO
	- Handle player pressing the reset button on their controller to reset their position


*/


void CelebrationPlugin::onLoad()
{
	Initialize();
	cvarManager->registerNotifier("CelebrationEnable", [this](std::vector<string> params){Enable();}, "Enables celebration plugin", PERMISSION_ALL);
	cvarManager->registerNotifier("CelebrationDisable", [this](std::vector<string> params){Disable();}, "Disables celebration plugin", PERMISSION_ALL);
	cvarManager->registerNotifier("CelebrationReset", [this](std::vector<string> params){CelebrationReset();}, "Reset celebration to beginning state", PERMISSION_ALL);

	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.OnPlayerRestarted", bind(&CelebrationPlugin::ResetButton, this));
}


void CelebrationPlugin::CreateValues()
{
	if(gameWrapper->IsInFreeplay())
	{
		FOCUS = Vector{450,40,20};
		ROTATION = Rotator{-1600,-31675,0};
		DISTANCE = 400;
		FOV = 70;
	}
}

void CelebrationPlugin::ResetButton()
{
	//Delay resetting the car and ball after the player has pressed their reset shot binding
	gameWrapper->SetTimeout(bind(&CelebrationPlugin::CelebrationReset, this), 0.05f);
}

void CelebrationPlugin::CelebrationReset()
{
	if(enabled)
	{
		CarWrapper car = gameWrapper->GetLocalCar();
		ServerWrapper server = gameWrapper->GetGameEventAsServer();
		if(!car.IsNull() && gameWrapper->IsInFreeplay()) //move car/ball position and enable podium mode
		{
			car.SetVelocity({0,0,0});
			car.SetAngularVelocity({0,0,0}, false);
			car.SetLocation({450,40,40});
			car.SetRotation({-100,6248,0});
			car.SetbPodiumMode(1);

			if(!server.IsNull())
			{
				BallWrapper ball = server.GetBall();
				if(!ball.IsNull())
				{
					ball.SetLocation({-1000,-1000,90});
					ball.SetRotation({0,0,0});
					ball.SetVelocity({0,0,0});
					ball.SetAngularVelocity({0,0,0}, false);
				}
			}
		}
	}
}









//LEAVE THESE UNCHANGED


void CelebrationPlugin::onUnload(){}
void CelebrationPlugin::Initialize()
{
	//Install parent plugin if it isn't already installed. Ensure parent plugin is loaded.
	if(!experimental::filesystem::exists(".\\bakkesmod\\plugins\\CameraControl.dll"))
		cvarManager->executeCommand("bpm_install 71");
	cvarManager->executeCommand("plugin load CameraControl", false);

	//Hook events
	gameWrapper->HookEvent("Function ProjectX.Camera_X.ClampPOV", std::bind(&CelebrationPlugin::HandleValues, this));
	gameWrapper->HookEvent("Function TAGame.PlayerController_TA.PressRearCamera", [&](std::string eventName){isInRearCam = true;});
	gameWrapper->HookEvent("Function TAGame.PlayerController_TA.ReleaseRearCamera", [&](std::string eventName){isInRearCam = false;});
	gameWrapper->HookEvent("Function TAGame.CameraState_BallCam_TA.BeginCameraState", [&](std::string eventName){isInBallCam = true;});
	gameWrapper->HookEvent("Function TAGame.CameraState_BallCam_TA.EndCameraState", [&](std::string eventName){isInBallCam = false;});
}
bool CelebrationPlugin::CanCreateValues()
{
	if(!enabled || IsCVarNull("CamControl_Swivel_READONLY") || IsCVarNull("CamControl_Focus") || IsCVarNull("CamControl_Rotation") || IsCVarNull("CamControl_Distance") || IsCVarNull("CamControl_FOV"))
		return false;
	else
		return true;
}
bool CelebrationPlugin::IsCVarNull(string cvarName)
{
    struct CastStructOne
    {
        struct CastStructTwo{void* address;};
        CastStructTwo* casttwo;
    };

	CVarWrapper cvar = cvarManager->getCvar(cvarName);
    CastStructOne* castone = (CastStructOne*)&cvar;
    return castone->casttwo->address == NULL;
}
void CelebrationPlugin::Enable()
{
	cvarManager->executeCommand("CamControl_Enable 1", false);
	enabled = true;
	CelebrationReset();
}
void CelebrationPlugin::Disable()
{
	enabled = false;
	cvarManager->executeCommand("CamControl_Enable 0", false);
	CarWrapper car = gameWrapper->GetLocalCar();
	if(!car.IsNull() && gameWrapper->IsInFreeplay())
		car.SetbPodiumMode(0);
}
void CelebrationPlugin::HandleValues()
{
	if(!CanCreateValues())
		return;
	
	//Reset values so that the game won't crash if the developer doesn't assign values to variables
	overrideValue[0] = true;//Focus X
	overrideValue[1] = true;//Focus Y
	overrideValue[2] = true;//Focus Z
	overrideValue[3] = true;//Rotation Pitch
	overrideValue[4] = true;//Rotation Yaw
	overrideValue[5] = true;//Rotation Roll
	overrideValue[6] = true;//Distance
	overrideValue[7] = true;//FOV

	SWIVEL = GetSwivel();
	FOCUS = Vector{0,0,0};
	ROTATION = Rotator{0,0,0};
	DISTANCE = 100;
	FOV = 90;

	//Get values from the developer
	CreateValues();

	//Send value requests to the parent mod
	string values[8];
	values[0] = to_string(FOCUS.X);
	values[1] = to_string(FOCUS.Y);
	values[2] = to_string(FOCUS.Z);
	values[3] = to_string(ROTATION.Pitch);
	values[4] = to_string(ROTATION.Yaw);
	values[5] = to_string(ROTATION.Roll);
	values[6] = to_string(DISTANCE);
	values[7] = to_string(FOV);
	
	for(int i=0; i<8; i++)
	{
		if(!overrideValue[i])
			values[i] = "NULL";
	}

	cvarManager->getCvar("CamControl_Focus").setValue(values[0] + "," + values[1] + "," + values[2]);
	cvarManager->getCvar("CamControl_Rotation").setValue(values[3] + "," + values[4] + "," + values[5]);
	cvarManager->getCvar("CamControl_Distance").setValue(values[6]);
	cvarManager->getCvar("CamControl_FOV").setValue(values[7]);
}
Rotator CelebrationPlugin::GetSwivel()
{
	if(IsCVarNull("CamControl_Swivel_READONLY"))
		return Rotator{0,0,0};

	string readSwivel = cvarManager->getCvar("CamControl_Swivel_READONLY").getStringValue();
	string swivelInputString;
	stringstream ssSwivel(readSwivel);

	Rotator SWIVEL = {0,0,0};

	getline(ssSwivel, swivelInputString, ',');
	SWIVEL.Pitch = stof(swivelInputString);
	getline(ssSwivel, swivelInputString, ',');
	SWIVEL.Yaw = stof(swivelInputString);
	getline(ssSwivel, swivelInputString, ',');
	SWIVEL.Roll = stof(swivelInputString);

	return SWIVEL;
}