#include <iostream>
#include <Windows.h>
#include "win_utils.h"
#include "xor.hpp"
#include <dwmapi.h>
#include "Main.h"
#include <vector>
#include "driver.h"
//#include "rwdefs.h"
#include "stdafx.h"
#include "define.h"
//#include "utils.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <Lmcons.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <chrono>
#include <ctime>
#include <iostream>
#include "imgui/imgui_internal.h"
#include "font1.h"

#include <cstdlib>
#include <iostream>
#include <chrono>
#include <random>


int crossmodepos = 7;
static const char* crossmodes[]
{
	"sqaure",

	"circle", 

	"lines",

	"distance lines"
};

FTransform GetBoneIndex(DWORD_PTR mesh, int index) {
	DWORD_PTR bonearray = read<DWORD_PTR>(mesh + 0x4B0);
	if (bonearray == NULL) {
		bonearray = read<DWORD_PTR>(mesh + 0x4B0 + 0x10);
	}
	return read<FTransform>(bonearray + (index * 0x30));
}
Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id) {
	FTransform bone = GetBoneIndex(mesh, id);
	FTransform ComponentToWorld = read<FTransform>(mesh + 0x1C0);
	D3DMATRIX Matrix;
	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());
	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}
D3DMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0)) {
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

extern Vector3 CameraEXT(0, 0, 0);
float FovAngle;

Vector3 ProjectWorldToScreen(Vector3 WorldLocation) {
	Vector3 Screenlocation = Vector3(0, 0, 0);
	Vector3 Camera;
	auto chain69 = read<uintptr_t>(Localplayer + 0xa8);
	uint64_t chain699 = read<uintptr_t>(chain69 + 8);
	Camera.x = read<float>(chain699 + 0x7E8);
	Camera.y = read<float>(Rootcomp + 0x12C);
	float test = asin(Camera.x);
	float degrees = test * (180.0 / M_PI);
	Camera.x = degrees;
	if (Camera.y < 0)
		Camera.y = 360 + Camera.y;
	D3DMATRIX tempMatrix = Matrix(Camera);
	Vector3 vAxisX, vAxisY, vAxisZ;
	vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	uint64_t chain = read<uint64_t>(Localplayer + 0x70);
	uint64_t chain1 = read<uint64_t>(chain + 0x98);
	uint64_t chain2 = read<uint64_t>(chain1 + 0x140);

	Vector3 vDelta = WorldLocation - read<Vector3>(chain2 + 0x10);
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));
	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	float zoom = read<float>(chain699 + 0x580);
	float FovAngle = 80.0f / (zoom / 1.19f);
	float ScreenCenterX = Width / 2;
	float ScreenCenterY = Height / 2;
	float ScreenCenterZ = Height / 2;
	Screenlocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.z = ScreenCenterZ - vTransformed.z * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	return Screenlocation;
}






HRESULT DirectXInit(HWND hWnd)
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.Windowed = TRUE;
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_Params.hDeviceWindow = hWnd;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_Params.BackBufferWidth = Width;
	p_Params.BackBufferHeight = Height;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	p_Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
	{
		p_Object->Release();
		exit(4);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig font_config;
	font_config.OversampleH = 1;
	font_config.OversampleV = 1;
	font_config.PixelSnapH = true;

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF,
		0x0400, 0x044F,
		0,
	};



	
	int loaded = 0;
	if (loaded == 0) {
		title = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/ariblk.ttf", 40.f, &font_config, ranges);
		//title = io.Fonts->AddFontFromFileTTF(E("C:/windows/fonts/arial.ttf"), 40.f);
		//otherfont = io.Fonts->AddFontFromFileTTF(E("C:\\Windows\\Fonts\\timesbd.ttf"), 25.f);
		//otherotherfont = io.Fonts->AddFontFromFileTTF(E("C:\\Windows\\Fonts\\constanb.ttf"), 17.f);





		//title = io.Fonts->AddFontFromMemoryCompressedTTF(fontData, 50, 50.0f, NULL, io.Fonts->GetGlyphRangesDefault());
		tabfont = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/ariblk.ttf", 30.f, &font_config, ranges);
		othertitle = io.Fonts->AddFontFromMemoryCompressedTTF(fontData, 27, 27.0f, NULL, io.Fonts->GetGlyphRangesDefault());
		//otherfont = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/arialbi.tff", 20.f, &font_config, ranges);
		//title = io.Fonts->AddFontFromFileTTF(E("C:\\Windows\\Fonts\\Tahoma.ttf"), 40.f);
		//tabfont = io.Fonts->AddFontFromFileTTF(("C:\\Windows\\Fonts\\Tahoma.ttf"), 30.0f);
		//otherfont = io.Fonts->AddFontFromFileTTF(("C:\\Windows\\Fonts\\Tahoma.ttf"), 18.0f);
		loaded = 1;
	}
	
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(p_Device);
	ImGuiStyle& Style = ImGui::GetStyle();
	auto Color = Style.Colors;

	Style.WindowBorderSize = 0;

	Style.ChildRounding = 0;
	Style.FrameRounding = 0;
	Style.ScrollbarRounding = 0;
	Style.GrabRounding = 0;
	Style.PopupRounding = 0;
	Style.WindowRounding = 0;
	Style.WindowTitleAlign.x = 0.50f;
	Style.WindowPadding.x = 20.0f;

	Color[ImGuiCol_WindowBg] = ImColor(18, 18, 18, 255);
	Color[ImGuiCol_ChildBg] = ImColor(5, 5, 5, 255);
	Color[ImGuiCol_FrameBg] = ImColor(31, 31, 31, 255);
	Color[ImGuiCol_FrameBgActive] = ImColor(41, 41, 41, 255);
	Color[ImGuiCol_FrameBgHovered] = ImColor(41, 41, 41, 255);

	Color[ImGuiCol_Button] = ImColor(29, 29, 29, 255);
	Color[ImGuiCol_ButtonActive] = ImColor(32, 32, 32, 255);
	Color[ImGuiCol_ButtonHovered] = ImColor(36, 36, 36, 255);

	Color[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
	Color[ImGuiCol_Separator] = ImColor(36, 36, 36, 255);

	Color[ImGuiCol_ResizeGrip] = ImColor(30, 30, 30, 255);
	Color[ImGuiCol_ResizeGripActive] = ImColor(30, 30, 30, 255);
	Color[ImGuiCol_ResizeGripHovered] = ImColor(30, 30, 30, 255);

	Color[ImGuiCol_ChildBg] = ImColor(26, 26, 26, 255);

	Color[ImGuiCol_ScrollbarBg] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrab] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);

	Color[ImGuiCol_Header] = ImColor(39, 39, 39, 255);
	Color[ImGuiCol_HeaderActive] = ImColor(39, 39, 39, 255);
	Color[ImGuiCol_HeaderHovered] = ImColor(39, 39, 39, 255);
	Color[ImGuiCol_CheckMark] = ImColor(255, 255, 255, 255);

	p_Object->Release();
	return S_OK;
}
bool IsVec3Valid(Vector3 vec3)
{
	return !(vec3.x == 0 && vec3.y == 0 && vec3.z == 0);
}
void SetupWindow()
{
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetWindowToTarget, 0, 0, 0);

	WNDCLASSEXA wcex = {
		sizeof(WNDCLASSEXA),
		0,
		DefWindowProcA,
		0,
		0,
		nullptr,
		LoadIcon(nullptr, IDI_APPLICATION),
		LoadCursor(nullptr, IDC_ARROW),
		nullptr,
		nullptr,
		("X"),
		LoadIcon(nullptr, IDI_APPLICATION)
	};

	RECT Rect;
	GetWindowRect(GetDesktopWindow(), &Rect);

	RegisterClassExA(&wcex);

	MyWnd = CreateWindowExA(NULL, E("X"), E("X"), WS_POPUP, Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wcex.hInstance, NULL);
	SetWindowLong(MyWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	SetLayeredWindowAttributes(MyWnd, RGB(0, 0, 0), 255, LWA_ALPHA);
	//SetWindowDisplayAffinity(MyWnd, 1);

	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(MyWnd, &margin);

	ShowWindow(MyWnd, SW_SHOW);
	UpdateWindow(MyWnd);
}
Vector3 AimbotCorrection(float bulletVelocity, float bulletGravity, float targetDistance, Vector3 targetPosition, Vector3 targetVelocity) {
	Vector3 recalculated = targetPosition;
	float gravity = fabs(bulletGravity);
	float time = targetDistance / fabs(bulletVelocity);
	float bulletDrop = (gravity / 250) * time * time;
	recalculated.z += bulletDrop * 120;
	recalculated.x += time * (targetVelocity.x);
	recalculated.y += time * (targetVelocity.y);
	recalculated.z += time * (targetVelocity.z);
	return recalculated;
}
bool rapidfiretrigger = false;
bool advaimbot;
void aimbot(float x, float y, float z) {
	float ScreenCenterX = (Width / 2);
	float ScreenCenterY = (Height / 2);
	float ScreenCenterZ = (Depth / 2);
	//if (advaimbot) {
	//	int AimSpeedX = item.Aim_SpeedX; //item.Aim_Speed
	//	int AimSpeedY = item.Aim_SpeedY;
	//	int AimSpeedZ = AimSpeedX;
	//}
	int AimSpeedX = item.Aim_SpeedNormal; //item.Aim_Speed
	int AimSpeedY = item.Aim_SpeedNormal;
	int AimSpeedZ = item.Aim_SpeedNormal;
	float TargetX = 0;
	float TargetY = 0;
	float TargetZ = 0;

	if (x != 0) {
		if (x > ScreenCenterX) {
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeedX;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX) {
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeedX;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}
	if (y != 0) {
		if (y > ScreenCenterY) {
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeedY;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY) {
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeedY;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}
	if (z != 0) {
		if (z > ScreenCenterZ) {
			TargetZ = -(ScreenCenterZ - z);
			TargetZ /= AimSpeedZ;
			if (TargetZ + ScreenCenterZ > ScreenCenterZ * 2) TargetZ = 0;
		}

		if (z < ScreenCenterZ) {
			TargetZ = z - ScreenCenterZ;
			TargetZ /= AimSpeedZ;
			if (TargetZ + ScreenCenterZ < 0) TargetZ = 0;
		}
	}
	mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(TargetX), static_cast<DWORD>(TargetY), NULL, NULL);
	if (rapidfiretrigger) {
		if (GetAsyncKeyState(VK_RBUTTON)) {
			mouse_event(MOUSEEVENTF_LEFTDOWN, DWORD(NULL), DWORD(NULL), DWORD(0x0002), ULONG_PTR(NULL));
			mouse_event(MOUSEEVENTF_LEFTUP, DWORD(NULL), DWORD(NULL), DWORD(0x0004), ULONG_PTR(NULL));
		}
	}
	return;
}
double GetCrossDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

typedef struct _FNlEntity {
	uint64_t Actor;
	int ID;
	uint64_t mesh;
}FNlEntity;

std::vector<FNlEntity> entityList;

void cache()
{
	while (true) {
		std::vector<FNlEntity> tmpList;

		Uworld = read<DWORD_PTR>(sdk::module_base + 0xb59bd30);
		printf("Uworld : 0x%llX\n", Uworld);
		DWORD_PTR Gameinstance = read<DWORD_PTR>(Uworld + 0x190);
		printf("Gameinstance : 0x%llX\n", Gameinstance);
		DWORD_PTR LocalPlayers = read<DWORD_PTR>(Gameinstance + 0x38);
	
		Localplayer = read<DWORD_PTR>(LocalPlayers);
		printf("Localplayer : 0x%llX\n", Localplayer);
		PlayerController = read<DWORD_PTR>(Localplayer + 0x30);
		printf("PlayerController : 0x%llX\n", PlayerController);
		LocalPawn = read<DWORD_PTR>(PlayerController + 0x2B0);
		printf("LocalPawn : 0x%llX\n", LocalPawn);


		PlayerState = read<DWORD_PTR>(LocalPawn + 0x240);
		printf("PlayerState : 0x%llX\n", PlayerState);
		Rootcomp = read<DWORD_PTR>(LocalPawn + 0x138); //old 130
		printf("Rootcomp : 0x%llX\n", Rootcomp);

		if (LocalPawn != 0) {
			localplayerID = read<int>(LocalPawn + 0x18);
			printf("localplayerID : 0x%llX\n", localplayerID);
		}

		Persistentlevel = read<DWORD_PTR>(Uworld + 0x30);
		printf("Persistentlevel : 0x%llX\n", Persistentlevel);
		DWORD ActorCount = read<DWORD>(Persistentlevel + 0xA0);
		printf("ActorCount : 0x%llX\n", ActorCount);
		DWORD_PTR AActors = read<DWORD_PTR>(Persistentlevel + 0x98);
		printf("AActors : 0x%llX\n", AActors);

		for (int i = 0; i < ActorCount; i++) {
			uint64_t CurrentActor = read<uint64_t>(AActors + i * 0x8);

			int curactorid = read<int>(CurrentActor + 0x18);

			if (curactorid == localplayerID || curactorid == localplayerID + 765) {
				FNlEntity fnlEntity{ };
				fnlEntity.Actor = CurrentActor;
				fnlEntity.mesh = read<uint64_t>(CurrentActor + 0x288);
				fnlEntity.ID = curactorid;
				tmpList.push_back(fnlEntity);
			}
		}
		entityList = tmpList;
		Sleep(1);
	}
}

void AimAt(DWORD_PTR entity) {
	uint64_t currentactormesh = read<uint64_t>(entity + 0x288);
	auto rootHead = GetBoneWithRotation(currentactormesh, 98);


	if (item.Aim_Prediction) {
		float distance = localactorpos.Distance(rootHead) / 250;
		uint64_t CurrentActorRootComponent = read<uint64_t>(entity + 0x130);
		Vector3 vellocity = read<Vector3>(CurrentActorRootComponent + 0x140);
		Vector3 Predicted = AimbotCorrection(30000, -504, distance, rootHead, vellocity);
		Vector3 rootHeadOut = ProjectWorldToScreen(Predicted);
		if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0) {
			if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, Width / 2, Height / 2, Depth / 2) <= item.AimFOV * 1)) {
				aimbot(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z);

			}
		}
	}
	else {
		Vector3 rootHeadOut = ProjectWorldToScreen(rootHead);
		if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0) {
			if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, Width / 2, Height / 2, Depth / 2) <= item.AimFOV * 1)) {
				aimbot(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z);
			}
		}
	}
}
void AimAt2(DWORD_PTR entity) {
	uint64_t currentactormesh = read<uint64_t>(entity + 0x288);
	auto rootHead = GetBoneWithRotation(currentactormesh, 98);

	if (item.Aim_Prediction) {
		float distance = localactorpos.Distance(rootHead) / 250;
		uint64_t CurrentActorRootComponent = read<uint64_t>(entity + 0x130);
		Vector3 vellocity = read<Vector3>(CurrentActorRootComponent + 0x140);
		Vector3 Predicted = AimbotCorrection(30000, -504, distance, rootHead, vellocity);
		Vector3 rootHeadOut = ProjectWorldToScreen(Predicted);
		if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0) {
			if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, Width / 2, Height / 2, Depth / 2) <= item.AimFOV * 1)) {
				if (item.Lock_Line) {
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height / 2), ImVec2(rootHeadOut.x, rootHeadOut.y), ImGui::GetColorU32({ item.LockLine[0], item.LockLine[1], item.LockLine[2], 1.0f }), item.Thickness);

				}
			}
		}
	}
	else {
		Vector3 rootHeadOut = ProjectWorldToScreen(rootHead);
		if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0) {
			if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, Width / 2, Height / 2, Depth / 2) <= item.AimFOV * 1)) {
				if (item.Lock_Line) {
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height / 2), ImVec2(rootHeadOut.x, rootHeadOut.y), ImGui::GetColorU32({ item.LockLine[0], item.LockLine[1], item.LockLine[2], 1.0f }), item.Thickness);
				}
			}
		}
	}
}



bool instares;
bool noanim;
bool rapidfire;
bool airstruct;
bool adsair;

void DrawSkeleton(DWORD_PTR mesh)
{
	Vector3 vHeadBone = GetBoneWithRotation(mesh, 98);
	Vector3 vHip = GetBoneWithRotation(mesh, 2);
	Vector3 vNeck = GetBoneWithRotation(mesh, 66);
	Vector3 vUpperArmLeft = GetBoneWithRotation(mesh, 93);
	Vector3 vUpperArmRight = GetBoneWithRotation(mesh, 9);
	Vector3 vLeftHand = GetBoneWithRotation(mesh, 62);
	Vector3 vRightHand = GetBoneWithRotation(mesh, 33);
	Vector3 vLeftHand1 = GetBoneWithRotation(mesh, 100);
	Vector3 vRightHand1 = GetBoneWithRotation(mesh, 99);
	Vector3 vRightThigh = GetBoneWithRotation(mesh, 69);
	Vector3 vLeftThigh = GetBoneWithRotation(mesh, 76);
	Vector3 vRightCalf = GetBoneWithRotation(mesh, 72);
	Vector3 vLeftCalf = GetBoneWithRotation(mesh, 79);
	Vector3 vLeftFoot = GetBoneWithRotation(mesh, 85);
	Vector3 vRightFoot = GetBoneWithRotation(mesh, 84);
	Vector3 vHeadBoneOut = ProjectWorldToScreen(vHeadBone);
	Vector3 vHipOut = ProjectWorldToScreen(vHip);
	Vector3 vNeckOut = ProjectWorldToScreen(vNeck);
	Vector3 vUpperArmLeftOut = ProjectWorldToScreen(vUpperArmLeft);
	Vector3 vUpperArmRightOut = ProjectWorldToScreen(vUpperArmRight);
	Vector3 vLeftHandOut = ProjectWorldToScreen(vLeftHand);
	Vector3 vRightHandOut = ProjectWorldToScreen(vRightHand);
	Vector3 vLeftHandOut1 = ProjectWorldToScreen(vLeftHand1);
	Vector3 vRightHandOut1 = ProjectWorldToScreen(vRightHand1);
	Vector3 vRightThighOut = ProjectWorldToScreen(vRightThigh);
	Vector3 vLeftThighOut = ProjectWorldToScreen(vLeftThigh);
	Vector3 vRightCalfOut = ProjectWorldToScreen(vRightCalf);
	Vector3 vLeftCalfOut = ProjectWorldToScreen(vLeftCalf);
	Vector3 vLeftFootOut = ProjectWorldToScreen(vLeftFoot);
	Vector3 vRightFootOut = ProjectWorldToScreen(vRightFoot);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vHipOut.x, vHipOut.y), ImVec2(vNeckOut.x, vNeckOut.y), ImColor(0,0,0), 2.0f );
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ImVec2(vNeckOut.x, vNeckOut.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ImVec2(vNeckOut.x, vNeckOut.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vLeftHandOut.x, vLeftHandOut.y), ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vRightHandOut.x, vRightHandOut.y), ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vLeftHandOut.x, vLeftHandOut.y), ImVec2(vLeftHandOut1.x, vLeftHandOut1.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vRightHandOut.x, vRightHandOut.y), ImVec2(vRightHandOut1.x, vRightHandOut1.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vLeftThighOut.x, vLeftThighOut.y), ImVec2(vHipOut.x, vHipOut.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vRightThighOut.x, vRightThighOut.y), ImVec2(vHipOut.x, vHipOut.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ImVec2(vLeftThighOut.x, vLeftThighOut.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vRightCalfOut.x, vRightCalfOut.y), ImVec2(vRightThighOut.x, vRightThighOut.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vLeftFootOut.x, vLeftFootOut.y), ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ImColor(0,0,0), 2.0f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(vRightFootOut.x, vRightFootOut.y), ImVec2(vRightCalfOut.x, vRightCalfOut.y), ImColor(0,0,0), 2.0f);
}
void DrawESP() {

	
	auto entityListCopy = entityList;
	float closestDistance = FLT_MAX;
	DWORD_PTR closestPawn = NULL;

	DWORD_PTR AActors = read<DWORD_PTR>(Ulevel + 0x98);

	int ActorTeamId = read<int>(0xF30);

	int curactorid = read<int>(0x18);
	//printf("test1\n");
	if (curactorid == localplayerID || curactorid == 20328438 || curactorid == 20328753 || curactorid == 9343426 || curactorid == 9875120 || curactorid == 9877254 || curactorid == 22405639 || curactorid == 9874439 || curactorid == 14169230)

		if (AActors == (DWORD_PTR)nullptr)
			return;
//	printf("test2\n");
	for (unsigned long i = 0; i < entityListCopy.size(); ++i) {
		//printf("test3\n");
		FNlEntity entity = entityListCopy[i];
		//printf("test4\n");
		uint64_t CurrentActor = read<uint64_t>(AActors + i * 0x8);

		uint64_t CurActorRootComponent = read<uint64_t>(entity.Actor + 0x138);
		if (CurActorRootComponent == (uint64_t)nullptr || CurActorRootComponent == -1 || CurActorRootComponent == NULL)
			continue;

		Vector3 actorpos = read<Vector3>(CurActorRootComponent + 0x11C);
		Vector3 actorposW2s = ProjectWorldToScreen(actorpos);

		DWORD64 otherPlayerState = read<uint64_t>(entity.Actor + 0x240);
		if (otherPlayerState == (uint64_t)nullptr || otherPlayerState == -1 || otherPlayerState == NULL)
			continue;
		//printf("test5\n");
		localactorpos = read<Vector3>(Rootcomp + 0x11C);

		Vector3 bone66 = GetBoneWithRotation(entity.mesh, 98);
		Vector3 bone0 = GetBoneWithRotation(entity.mesh, 0);

		Vector3 top = ProjectWorldToScreen(bone66);
		Vector3 chest = ProjectWorldToScreen(bone66);
		Vector3 aimbotspot = ProjectWorldToScreen(bone66);
		Vector3 bottom = ProjectWorldToScreen(bone0);

		Vector3 Head = ProjectWorldToScreen(Vector3(bone66.x, bone66.y, bone66.z + 15));


		Vector3 chestnone = GetBoneWithRotation(entity.mesh, 66);
		Vector3 chest1 = ProjectWorldToScreen(chestnone);


		float distance = localactorpos.Distance(bone66) / 100.f;
		float BoxHeight = (float)(Head.y - bottom.y);
		float CornerHeight = abs(Head.y - bottom.y);
		float CornerWidth = BoxHeight * 0.46;

		int MyTeamId = read<int>(PlayerState + 0xF30);
		int ActorTeamId = read<int>(otherPlayerState + 0xF30);
		int curactorid = read<int>(CurrentActor + 0x98);
		uintptr_t CurrentWeapon = read<uintptr_t>(LocalPawn + 0x5E8); //CurrentWeapon Offset
	//	printf("test6\n");
		if (MyTeamId != ActorTeamId) {
			//printf("test7\n");
			if (distance < item.VisDist) {
			//	printf("test8\n");
				if (instares) {
					write<float>(LocalPawn + 0x3758, 1);
				}
				if (noanim) {
					if (CurrentWeapon) {
						write<bool>(CurrentWeapon + 0x2B3, true); //bDisableEquipAnimation Offset (update this yourself)
					}
				}
				if (rapidfire) {
					float a = 0;
					float b = 0;
					if (CurrentWeapon) {
						a = read<float>(CurrentWeapon + 0x9E4); //LastFireTime Offset
						b = read<float>(CurrentWeapon + 0x9E8); //LastFireTimeVerified Offset
						write<float>(CurrentWeapon + 0x9E4, a + b - 0.0003); //LastFireTime Offset
					}
				}
				if (airstruct) {
					if (GetAsyncKeyState(VK_MENU)) { //Alt Keybind
						write<float>(LocalPawn + 0x98, 0); //CustomTimeDilation Offset
					}
					else {
						write<float>(LocalPawn + 0x98, 1); //CustomTimeDilation Offset
					}
				}
				if (adsair) {
					write<bool>(LocalPawn + 0x3E51, true);
				}
				//printf("test work\n");
				if (item.Esp_line) {
					ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height / 2), ImVec2(Head.x, Head.y), ImGui::GetColorU32({ item.LineESP[0], item.LineESP[1], item.LineESP[2], 1.0f }), item.Thickness);
				}
			
				
				if (item.Esp_box_fill) {
					ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(Head.x - (CornerWidth / 2), Head.y), ImVec2(bottom.x + (CornerWidth / 2), bottom.y), ImGui::GetColorU32({ item.Espboxfill[1], item.Espboxfill[1], item.Espboxfill[1], 0.75f }));
				}
				if (item.Esp_box) {
					
				
					ImGui::GetOverlayDrawList()->AddRect(ImVec2(Head.x - (CornerWidth / 2), Head.y + ((Head.y - chest1.y) / 3)), ImVec2(bottom.x + (CornerWidth / 2), bottom.y - ((Head.y - chest1.y) / 4)), ImColor(24, 63, 100), 0, 0, 4);
					ImGui::GetOverlayDrawList()->AddRect(ImVec2(Head.x - (CornerWidth / 2), Head.y + ((Head.y - chest1.y) / 3)), ImVec2(bottom.x + (CornerWidth / 2), bottom.y - ((Head.y - chest1.y) / 4)), ImColor(50, 130, 202), 0, 0, item.Thickness);
					
				}


				if (item.Esp_Corner_Box) {
					DrawCornerBox(Head.x - (CornerWidth / 2), Head.y, CornerWidth, CornerHeight, ImGui::GetColorU32({ item.BoxCornerESP[0], item.BoxCornerESP[1], item.BoxCornerESP[2], 1.0f }), item.Thickness);
				}


				if (item.Distance_Esp) {
					char dist[64];
					sprintf_s(dist, "[%.f] M", distance);
					ImGui::PushFont(otherfont);
					ImGui::GetOverlayDrawList()->AddText(ImVec2(bottom.x - 20, bottom.y), ImGui::GetColorU32({ color.Red[0], color.Red[1], color.Red[2], 4.0f }), dist);
					ImGui::PopFont();
				}
				if (item.skeleton) {
					DrawSkeleton(entity.mesh);
				}

				if (item.Aimbot) {
					if (distance < item.AimDistance) {
						auto dx = aimbotspot.x - (Width / 2);
						auto dy = aimbotspot.y - (Height / 2);
						auto dz = aimbotspot.z - (Depth / 2);
						auto dist = sqrtf(dx * dx + dy * dy + dz * dz) / 100.0f;
						if (dist < item.AimFOV && dist < closestDistance) {
							closestDistance = dist;
							closestPawn = entity.Actor;

						}
					}
				}
			}
		}
		else if (curactorid == 18391356) {
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height / 1), ImVec2(bottom.x, bottom.y), ImGui::GetColorU32({ color.Black[0], color.Black[1], color.Black[2], 4.0f }), item.Thickness);
		}
	}

	if (item.Aimbot) {
		if (closestPawn != 0) {
			if (item.Aimbot && closestPawn && GetAsyncKeyState(VK_RBUTTON) < 0) {
				AimAt(closestPawn);

				if (item.Auto_Bone_Switch) {

					item.boneswitch += 1;
					if (item.boneswitch == 700) {
						item.boneswitch = 0;
					}

					if (item.boneswitch == 0) {
						item.hitboxpos = 0;
					}
					else if (item.boneswitch == 50) {
						item.hitboxpos = 1;
					}
					else if (item.boneswitch == 100) {
					}
					else if (item.boneswitch == 150) {
						item.hitboxpos = 3;
					}
					else if (item.boneswitch == 200) {
						item.hitboxpos = 4;
					}
					else if (item.boneswitch == 250) {
						item.hitboxpos = 5;
					}
					else if (item.boneswitch == 300) {
						item.hitboxpos = 6;
					}
					else if (item.boneswitch == 350) {
						item.hitboxpos = 7;
					}
					else if (item.boneswitch == 400) {
						item.hitboxpos = 6;
					}
					else if (item.boneswitch == 450) {
						item.hitboxpos = 5;
					}
					else if (item.boneswitch == 500) {
						item.hitboxpos = 4;
					}
					else if (item.boneswitch == 550) {
						item.hitboxpos = 3;
					}
					else if (item.boneswitch == 600) {
						item.hitboxpos = 2;
					}
					else if (item.boneswitch == 650) {
						item.hitboxpos = 1;



					}
				}
			}
			else {
				isaimbotting = false;
				AimAt2(closestPawn);
			}
		}
	}
}

void GetKey() {
	if (item.hitboxpos == 0) {
		item.hitbox = 98;
	}
	else if (item.hitboxpos == 1) {
		item.hitbox = 66;
	}
	else if (item.hitboxpos == 2) {
		item.hitbox = 5;
	}

	else if (item.hitboxpos == 3) {
		item.hitbox = 2;
	}





	DrawESP();
}







std::wstring user_name_w()
{
	wchar_t buffer[UNLEN + 1]{};
	DWORD len = UNLEN + 1;
	if (::GetUserNameW(buffer, &len)) return buffer;
	else return {}; // or: return L"" ;
}

std::string WStringToUTF8(const wchar_t* lpwcszWString)
{
	char* pElementText;
	int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}
std::wstring MBytesToWString(const char* lpcszString)
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);
	wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
	std::wstring wString = (wchar_t*)pUnicode;
	delete[] pUnicode;
	return wString;
}
void DrawString(float fontSize, int x, int y, ImU32 color, bool bCenter, bool stroke, const char* pText, ...)
{
	va_list va_alist;
	char buf[1024] = { 0 };
	va_start(va_alist, pText);
	_vsnprintf_s(buf, sizeof(buf), pText, va_alist);
	va_end(va_alist);
	std::string text = WStringToUTF8(MBytesToWString(buf).c_str());
	if (bCenter)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		x = x - textSize.x / 2;
		y = y - textSize.y;
	}
	if (stroke)
	{
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
	}
	ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
}
int get_fps()
{
	using namespace std::chrono;
	static int count = 0;
	static auto last = high_resolution_clock::now();
	auto now = high_resolution_clock::now();
	static int fps = 0;

	count++;

	if (duration_cast<milliseconds>(now - last).count() > 1000) {
		fps = count;
		count = 0;
		last = now;
	}

	return fps;
}
typedef std::chrono::system_clock Clock;
bool Tab(const char* icon, const char* label, const ImVec2& size_arg, const bool selected)
{


	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	static float sizeplus = 0.f;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;

	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

	auto animcolor3 = ImColor(27, 27, 27, 255);
	auto animcolor2 = ImColor(115, 0, 255, 255);
	auto animcolor = ImColor(24, 24, 24, 255);

	//window->DrawList->AddRectFilled({ bb.Min.x,bb.Max.y }, { bb.Max.x,bb.Min.y }, animcolor);

	if (selected)
		//window->DrawList->AddRectFilled({ bb.Min.x,bb.Max.y }, { bb.Min.x + 2,bb.Min.y }, animcolor2);

	if (selected)
		//window->DrawList->AddRectFilled({ bb.Min.x + 2,bb.Max.y }, { bb.Min.x + 148,bb.Min.y }, animcolor3);

	//ImGui::PushFont(otherfont);

	if (selected)
		window->DrawList->AddText({ bb.Min.x + size_arg.x / 2 - ImGui::CalcTextSize(label).x / 2,bb.Min.y + size_arg.y / 2 - ImGui::CalcTextSize(label).y / 2 }, ImColor(50, 130, 202), label);

	if (!selected)
		window->DrawList->AddText({ bb.Min.x + size_arg.x / 2 - ImGui::CalcTextSize(label).x / 2,bb.Min.y + size_arg.y / 2 - ImGui::CalcTextSize(label).y / 2 }, ImColor(87, 218, 131), label);


	//ImGui::PopFont();

	return pressed;
}
//void dot_draw() { //Matrixs on menu
//	struct screen_size {
//		int x, y;
//	}; screen_size sc;
//	sc.x = GetSystemMetrics(0);
//	sc.y = GetSystemMetrics(1);
//	int s = rand() % 5;
//
//
//	if (s == 0) {
//		dots.push_back(new dot(ImVec2(rand() % (int)sc.x, -10), ImVec2((rand() * 1) - 3, rand() % 3 + 1)));
//	}
//	else if (s == 1) {
//		dets.push_back(new dot(ImVec2(rand() % (int)sc.x, (int)sc.y + 10), ImVec2((rand() % 1) - 3, -1 * (rand() % 3 + 1))));
//	}
//	else if (s == 2) {
//		dots.push_back(new dot(ImVec2(-10, rand() % (int)sc.y), ImVec2(rand() % 3 + 1, (rand() % 1) - 3)));
//	}
//	else if (s == 3) {
//		dots.push_back(new dot(ImVec2((int)sc.x + 10, rand() % (int)sc.y), ImVec2(-1 * (rand() * 1 + 1), (rand() % 1) - 3)));
//	}
//
//
//
//	for (auto i = dots.begin(); i < dots.end();) {
//		if ((*i)->n_pos.Y < -20 || (-i)->_pos.Y > sc.y + 20 || (-i)->_pos.x < -20 || (+i)->n_pos.X > sc.x + 2) {
//			delete (i);
//			i = dots.erase(i);
//		}
//		else {
//			(-i)->update();
//		}
//	}
//
//	for (auto i - dots.begin(); i < dots.end(); i++) {
//		(-i)->draw();
//	}
//}
//
//void dot_clear()
//{
//	for (auto i = dots.begin(); i < dots.end(); i++) {
//		delete (vi);
//	}
//	dots.clear();
//}




int start1num = 110;
int end1num = 200;

int start1 = 0;
int end1 = 0;
int loaded = 0;

void draw_dot() {
	struct screen_size {
		int x, y;
		}; screen_size sc;
		sc.x = GetSystemMetrics(0); //far right
		sc.y = GetSystemMetrics(1); //far bottom
	

		/*int i = 0;
		for (; i <= 500000000; i++) {
			if (i == 500000000) {
				start1 = rand() % start1num;
				end1 = rand() % end1num;
				
			}
			
		}*/
		
		//if (loaded == 0) {
		////srand(time(NULL));
		////srand(10000);
		//	start1 = rand() % start1num;
		//	end1 = rand() % end1num;
		//	//srand(0);
		//	loaded = 1;
		//}
		//start1 - 100;
		srand((unsigned)time(0));
		srand(1);
		start1 = (rand() % start1num);
		
		//start1 = rand() % start1num;

		end1 = rand() % end1num;
		
		
		ImGui::GetOverlayDrawList()->AddCircleFilled(ImVec2(sc.x - start1, sc.y - start1), 10, ImColor(255, 255, 255), 66);
	//	i = 0;
}


bool draw_abigsquare()
{
	
	ImGui::SetNextWindowSize({ 750, 550 });


	ImGui::StyleColorsClassic();
	ImGuiStyle* style = &ImGui::GetStyle();


	ImVec4* colors = style->Colors;

	ImVec4* colorz = ImGui::GetStyle().Colors;
	ImGui::StyleColorsClassic();
	style->WindowPadding = ImVec2(8, 8);
	style->WindowRounding = 0.0f;
	style->FramePadding = ImVec2(4, 2);
	style->FrameRounding = 0.0f;
	style->ItemSpacing = ImVec2(8, 4);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->IndentSpacing = 21.0f;
	style->ScrollbarSize = 14.0f;
	style->ScrollbarRounding = 0.0f;
	style->GrabMinSize = 10.0f;
	style->GrabRounding = 0.0f;
	style->TabRounding = 0.f;
	style->ChildRounding = 0.0f;
	style->WindowBorderSize = 0.f;
	style->ChildBorderSize = 0.f;
	style->PopupBorderSize = 0.f;
	style->FrameBorderSize = 0.f;
	style->TabBorderSize = 0.f;

	style->Colors[ImGuiCol_Text] = ImColor(255, 255, 255);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.0f, 0.0263f, 0.0357f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImColor(32, 31, 51);
	style->Colors[ImGuiCol_ChildWindowBg] = ImColor(20, 33, 45);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0263f, 0.0357f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImColor(1, 1, 1);
	style->Colors[ImGuiCol_BorderShadow] = ImColor(1, 1, 1);
	style->Colors[ImGuiCol_FrameBg] = ImColor(40, 40, 40);
	style->Colors[ImGuiCol_FrameBgHovered] = ImColor(40, 40, 40);
	style->Colors[ImGuiCol_FrameBgActive] = ImColor(40, 40, 40);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.263f, 0.357f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImColor(111, 3, 252);
	style->Colors[ImGuiCol_SliderGrab] = ImColor(111, 3, 252);
	style->Colors[ImGuiCol_SliderGrabActive] = ImColor(111, 3, 252);
	style->Colors[ImGuiCol_Button] = ImColor(32, 32, 32);
	style->Colors[ImGuiCol_ButtonHovered] = ImColor(32, 32, 32);
	style->Colors[ImGuiCol_ButtonActive] = ImColor(32, 32, 32);
	style->Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_Column] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

	style->WindowTitleAlign.x = 0.50f;
	style->FrameRounding = 0.0f;





	clock_t deltaTime = 0;
	unsigned int frames = 0;
	double  frameRate = 30;
	double  averageFrameTimeMilliseconds = 33.333;



	static int tabb = 1;
	ImColor colblack = ImColor(1, 1, 1);

	
	

	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y));
	ImGui::Begin(("Kingdom"), 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);
	draw_dot();
	/*ImGui::PushFont(title);
	ImGui::SetCursorPos(ImVec2(120, 15));
	ImGui::Text("Kingdom");
	ImGui::PopFont();*/

	//ImGui::SetCursorPos(ImVec2(20, 40));
	//ImGui::PushFont(tabfont);
	//if (Tab(("A"), ("Aimbot"), ImVec2(199, 60), tabb == 1 ? true : false))
	//	tabb = 1;
	//ImGui::SetCursorPos(ImVec2(130, 40));
	//if (Tab(("B"), ("Visuals"), ImVec2(199, 60), tabb == 2 ? true : false))
	//	tabb = 2;
	///*ImGui::SetCursorPos(ImVec2(1, 220));
	//if (Tab(("C"), ("Exploits"), ImVec2(199, 60), tabb == 3 ? true : false))
	//	tabb = 3;*/
	//ImGui::PopFont();



	//ImGui::SetCursorPos(ImVec2(20, 100));
	//ImGui::PushFont(otherfont);
	//if (tabb == 1) {
	//	if (ImGui::BeginChild(("##aimbot"), ImVec2(300, 373), true, ImGuiWindowFlags_NoScrollbar))
	//	{
	//		ImGui::Spacing();
	//		ImGui::Checkbox(" Aimbot", &item.Aimbot);
	//		ImGui::Spacing();
	//		ImGui::Checkbox(" Draw FOV", &item.Draw_FOV_Circle);
	//		ImGui::Spacing();
	//		ImGui::SliderFloat("Fov Size", &item.AimFOV, 20, 400);
	//		ImGui::Spacing();
	//		ImGui::SliderFloat(" Smoothing", &item.Aim_SpeedNormal, 5, 20);
	//		
	//		


	//		ImGui::EndChild();
	//	}
	//}
	//if (tabb == 2) {
	//	if (ImGui::BeginChild(("##esp"), ImVec2(300, 373), true, ImGuiWindowFlags_NoScrollbar))
	//	{
	//		ImGui::Spacing();
	//		ImGui::Checkbox(" Boxes", &item.Esp_box);
	//		ImGui::Spacing();
	//		ImGui::Checkbox(" Skeleton", &item.skeleton); 				
	//		ImGui::Spacing();
	//		ImGui::Checkbox(" Distance", &item.Distance_Esp);



	//		ImGui::EndChild();
	//	}
	//}
	//if (tabb == 3) {
	//	if (ImGui::BeginChild(("##exploits"), ImVec2(300, 350), true, ImGuiWindowFlags_NoScrollbar))
	//	{
	//		ImGui::Spacing();
	//		ImGui::Checkbox(" Quik Wepon Switch", &noanim);
	//		ImGui::Spacing();
	//		ImGui::Checkbox(" Insta Revive", &instares);
	//		ImGui::Spacing();
	//		ImGui::Checkbox(" Rapid Fire", &rapidfire);
	//		ImGui::Spacing();
	//		ImGui::Checkbox(" Airstruct", &airstruct);
	//		ImGui::Spacing();
	//		ImGui::Checkbox(" Ads In Air", &adsair);


	//		ImGui::EndChild();
	//	}
	//}

	//ImGui::PopFont();









	//cout << "Current user name is " << getUserName() << "." << endl;
	char fpsinfo[64];
	char usernamez[128];
	char day[128];
	TCHAR username[UNLEN + 1];
	DWORD size = UNLEN + 1;
	GetUserName((TCHAR*)username, &size);
	
	
	sprintf(usernamez, "Welcome, %S", username);
	
	//DrawString(14, 100, 35, ImColor(1, 1, 1), true, true, username);
	//auto now = Clock::now();
	//std::time_t now_c = Clock::to_time_t(now);
	//struct tm* parts = std::localtime(&now_c);

	time_t now = time(0);

	// convert now to string form
	char* dt = ctime(&now);

	//cout << "The local date and time is: " << dt << endl
//	std::cout << 1900 + parts->tm_year << std::endl;
//	std::cout << 1 + parts->tm_mon << std::endl;
//	std::cout << parts->tm_mday << std::endl;
	//sprintf(day, "Today is, %d/%d/%d", parts->tm_mon, parts->tm_mday, parts->tm_year);
	
	

	
	
		

	ImGui::End();
	//title text
	// ImGui::PushFont(othertitle);
	//ImGui::GetOverlayDrawList()->AddText(ImVec2(ImGui::GetWindowPos().x + 205, ImGui::GetWindowPos().y + 40), ImColor(15, 173, 86), usernamez);
	//ImGui::GetOverlayDrawList()->AddText(ImVec2(ImGui::GetWindowPos().x + 460, ImGui::GetWindowPos().y + 40), ImColor(15, 173, 86), "Date: ");
	//ImGui::GetOverlayDrawList()->AddText(ImVec2(ImGui::GetWindowPos().x + 520, ImGui::GetWindowPos().y + 40), ImColor(15, 173, 86), dt);
	//ImGui::PopFont();
	//lines
//	ImGui::GetOverlayDrawList()->AddLine(ImVec2(ImGui::GetWindowPos().x + 1, ImGui::GetWindowPos().y + 70), ImVec2(ImGui::GetWindowPos().x + 798, ImGui::GetWindowPos().y + 70), colblack, 3);
	//ImGui::GetOverlayDrawList()->AddLine(ImVec2(ImGui::GetWindowPos().x + 200, ImGui::GetWindowPos().y + 1), ImVec2(ImGui::GetWindowPos().x + 200, ImGui::GetWindowPos().y + 599), colblack, 3);

	return true;
}
void shortcurts()
{
	if (Key.IsKeyPushing(VK_INSERT))
	{
		if (menu_key == false)
		{
			menu_key = true;
		}
		else if (menu_key == true)
		{
			menu_key = false;
		}
		Sleep(200);
	}
}
void render() {

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	DrawESP();
	

	if (item.Draw_FOV_Circle) {
		

		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(Width / 2, Height / 2), float(item.AimFOV), ImColor(87, 218, 131), 66, 2.f);
	}
	if (item.Cross_Hair) {
		int centerx = (Width / 2); 
		int centery = (Height / 2);


		if (crossmodepos == 0) {//sqaure 
			ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(centerx - 4, centery - 4), ImVec2(centerx + 4, centery + 4), ImColor(50, 130, 202), 0, 4);
		}
		if (crossmodepos == 1) {//circle
			ImGui::GetOverlayDrawList()->AddCircleFilled(ImVec2(centerx, centery), 5, ImColor(50, 130, 202));
		}
		if (crossmodepos == 2) {//line
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(centerx - 10, centery), ImVec2(centerx + 10, centery), ImColor(50, 130, 202), 3);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(centerx, centery - 10), ImVec2(centerx, centery + 10), ImColor(50, 130, 202), 3);
		}
		if (crossmodepos == 3) {//line dist

			ImGui::GetOverlayDrawList()->AddLine(ImVec2(centerx - 5, centery), ImVec2(centerx - 12, centery), ImColor(50, 130, 202), 3);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(centerx + 5, centery), ImVec2(centerx + 12, centery), ImColor(50, 130, 202), 3);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(centerx, centery + 5), ImVec2(centerx, centery + 12), ImColor(50, 130, 202), 3);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(centerx, centery - 5), ImVec2(centerx, centery - 12), ImColor(50, 130, 202), 3);
		}
	
		
	}

	shortcurts();
	if (menu_key)
	{
		//background();
		draw_abigsquare();
		ImGui::GetIO().MouseDrawCursor = 1;
	}
	else {
		ImGui::GetIO().MouseDrawCursor = 0;
	}

	//char fpsinfo[64];
	//sprintf(fpsinfo, E("FPS: %03d"), get_fps());
	//	DrawString(14, 100, 35, &Col.green, true, true, fpsinfo);

	ImGui::EndFrame();
	p_Device->SetRenderState(D3DRS_ZENABLE, false);
	p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (p_Device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		p_Device->EndScene();
	}
	HRESULT result = p_Device->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		p_Device->Reset(&p_Params);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}
WPARAM MainLoop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, MyWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();
		if (GetAsyncKeyState(0x23) & 1)
			exit(8);

		if (hwnd_active == GameWnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(MyWnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(GameWnd, &rc);
		ClientToScreen(GameWnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameWnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;
		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{

			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			p_Params.BackBufferWidth = Width;
			p_Params.BackBufferHeight = Height;
			SetWindowPos(MyWnd, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			p_Device->Reset(&p_Params);
		}
		render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanuoD3D();
	DestroyWindow(MyWnd);

	return Message.wParam;
}
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		CleanuoD3D();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (p_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_Params.BackBufferWidth = LOWORD(lParam);
			p_Params.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = p_Device->Reset(&p_Params);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}
//std::string GetNameFromFortniteGame(int key)
//{
//	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
//	uint16_t NameOffset = (uint16_t)key;
//
//	uint64_t NamePoolChunk = read<uint64_t>( (uintptr_t)base_address + 0xB7C9BC0 + ((ChunkOffset + 2) * 8));
//	uint64_t entryOffset = NamePoolChunk + (DWORD)(2 * NameOffset);
//	uint16_t nameEntry = read<uint16_t>( entryOffset);
//
//	int nameLength = nameEntry >> 6;
//	char buff[1028];
//
//	if ((uint32_t)nameLength && nameLength > 0)
//	{
//		driver->ReadProcessMemory( entryOffset + 2, buff, nameLength);
//
//		char* v2 = buff; // rdi
//		unsigned __int16* v3; // rbx
//		signed int v4 = nameLength; // ebx
//		__int64 result; // rax
//		signed int v6; // er8
//		unsigned int v7; // ecx
//		char v8; // dl
//
//		result = read<unsigned int>( (uintptr_t)base_address + 0xB672840) >> 5;
//		v6 = 0;
//		if (v4)
//		{
//			do
//			{
//				v7 = *v2++;
//				v8 = result ^ 16 * v7 ^ (result ^ (v7 >> 4)) & 0xF;
//				result = (unsigned int)(result + 4 * v6++);
//				*(v2 - 1) = v8;
//			} while (v6 < v4);
//		}
//
//		buff[nameLength] = '\0';
//		return std::string(buff);
//	}
//	else
//	{
//		return "";
//	}
//}
//void lootespniggerrat()
//{
//	std::cout << GetNameFromFortniteGame;
//}
void CleanuoD3D()
{
	if (p_Device != NULL)
	{
		p_Device->EndScene();
		p_Device->Release();
	}
	if (p_Object != NULL)
	{
		p_Object->Release();
	}
}
int isTopwin()
{
	HWND hWnd = GetForegroundWindow();
	if (hWnd == GameWnd)
		return TopWindowGame;
	if (hWnd == MyWnd)
		return TopWindowMvoe;

	return 0;
}
void SetWindowToTarget()
{
	while (true)
	{
		GameWnd = get_process_wnd(sdk::process_id);
		if (GameWnd)
		{
			ZeroMemory(&GameRect, sizeof(GameRect));
			GetWindowRect(GameWnd, &GameRect);
			Width = GameRect.right - GameRect.left;
			Height = GameRect.bottom - GameRect.top;
			DWORD dwStyle = GetWindowLong(GameWnd, GWL_STYLE);
			if (dwStyle & WS_BORDER)
			{
				GameRect.top += 32;
				Height -= 39;
			}
			ScreenCenterX = Width / 2;
			ScreenCenterY = Height / 2;
			MoveWindow(MyWnd, GameRect.left, GameRect.top, Width, Height, true);
		}
	}
}


int main() {
	printf(("[!] Open driver connection: "));
	//printf(("test1\n"));
	if (driver->Init(FALSE)) {
		//	printf(("test2\n"));
		printf(("Success!\n"));
		driver->Attach((L"FortniteClient-Win64-Shipping.exe"));
		//	printf(("test3\n"));
		// Create and open a text file
		
		while (true)
		{

			SetupWindow();
			//printf(("test4\n"));

			DirectXInit(MyWnd);

			//printf(("test5\n"));
			sdk::process_id = driver->GetProcessId((L"FortniteClient-Win64-Shipping.exe")); //Spotify - Premium.exe
			//sdk::process_id = driver->GetProcessId((L"Discord.exe"));
			sdk::module_base = driver->GetModuleBase((L"FortniteClient-Win64-Shipping.exe")); //FortniteClient-Win64-Shipping.exe

			printf(("BaseAddress :0x%llX\n"), sdk::module_base);
			//lootespniggerrat();
			HANDLE handle = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(cache), nullptr, NULL, nullptr);
			CloseHandle(handle);

			MainLoop();
		}
		return 0;
	}
	printf(("Failed!\n"));
	system(("pause"));
	return 1;
}