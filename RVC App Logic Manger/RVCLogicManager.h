/*#############################################################################
 *  (C) 2014  Fujitsu Ten Limited. All Rights Reserved.
 * =============================================================================
 *  HARDWARE         : INFO3_LOW
 *  MODULE           : RVC
 *  VERSION          : v0.0.1
 *  FILE NAME        : RVCLogicManager.h
 *  FILE DESCRIPTION : Implementation for RVCLogicManager.
 *  NOTE				: N/A
 *############################################################################*/

#ifndef inc_RVCLogicManager_H
#define inc_RVCLogicManager_H

#include "IHMIF.h"
#include "RVCGlobalTypes.h"
#include "vdac/vehicle.h"
#include "sensordataaccess.h"
#include "sensordatamanager.h"
#include "IRvsEvent.h"
#include "IRvs.h"
#include "IRVCScreenManager.h"
#include "IRVCLogicManager.h"
#include "IUpdatableView.h"
#include "config.h"
//#include "FrontRearCameraStaticCaldef.h"
//#include "CalDefines.h"
//#include "BaseCaldef.h"
#ifdef PLATFORM_X86
#include "RVCSimulation.h"
#include "IAppSimulationProvider.h"
#include "SimulationProvider.h"
#endif

#include "PersistableParameters.h"

//#ifdef PLATFORM_X86
//#endif
#include "RVSStub.h"
//#include "Rvs.h"

//using namespace kivi::sensormanagement;
using namespace i3l::rvs;
using namespace persistence_HMIF_RVC;
using namespace i3l::hmi::core;
class RVCLogicManager : public IRvsEvent, public IRVCLogicManager{

public:

	RVCLogicManager();	//Constructor

	virtual ~RVCLogicManager();	//Destructor

	void Init(IUpdatableView* updatableView, IHMIF* hmif, IRVCScreenManager* screnMgr, map<unsigned int, ScreenElement*>& elementMap);
	void Start();
	void Resume();

	void Finalize();
	void ToggleShowGuideline();
	inline RVSState_t GetRVSState();
	inline bool IsOverlayPresent();
	void SubscribeHardKey();
	void UnsubscribeHardKey();
	void OnRVSStateChanged(RVSState_t rvsState);
	void OnOverlayTextLine1Changed(bool visible, RVSText_t messageId,  RVSTextHPos_t hPosition, RVSTextVPos_t vPosition);
	void OnOverlayTextLine2Changed(bool visible, RVSText_t messageId,  RVSTextHPos_t hPosition, RVSTextVPos_t vPosition);


	void OnUpaSymbolChanged(bool visible, RVSUPAZone_t zone, RVSUvCoord_t coordinate,
			RVSColor_t color, RVSUpaSize_t size,
			RVSAnimation_t animation);

	void OnLeftRctaChanged(bool visible, RVSUvCoord_t coordinate, RVSAnimation_t animation);

	void OnRightRctaChanged(bool visible, RVSUvCoord_t coordinate, RVSAnimation_t animation);

	void OnGuideLineChanged(bool visible, const RVSGuidelinePaths& guidelinePoints);
	void CancelHysteresisTimer();
	void OnHardKeyChangeStatus(i3l::hmi::core::hardkey_keycode keyCode, i3l::hmi::core::key_state keyState);
	void OnCancelShutDown();
	void ShowSymbols(bool symbolState);
    void setRearCameraState(RVSCameraStatus_t RVSCameraStatus);
    void setHitchViewCameraState(RVSCameraStatus_t RVSCameraStatus);
    void OnHitchViewGuideLineChanged(bool visible, const RVSGuidelinePaths& guidelinePoints);
    void ToggleShowSmartTowGuideLine();
    void OnVehicleMovementStateChanged(VehicleMovementState_t vehicleMovementState);
    VehicleMovementState_t getVehicleMovementState();
    bool IsSmartTowFeatureEnabled();
    void handleState();
    void InitialiseCalibration();
    void setCameraAppIconStatus(bool appState);
    void OnVehicleSpeedChanged(unsigned int vehicleSpeed);
    void OnPowerModeChanged(RVSVehiclePowerMode_t vehiclePowerMode);
    unsigned int getVehicleSpeed();

private:
	void RemoveAnimationPlane();
	void UpdateRVSData(unsigned int RVSDataIdentifier,std::string m_ActualValue);
	void print_guidelines_points(vector<RVSUvCoord_t> dataPoints, string message);
	void LoadData();
	void UpdateGuidelines();
    void UpdateSmartTowGuidelines();

private:
	IUpdatableView* m_UpdatableView;
	IRVCScreenManager* m_SMObj;
	RVSState_t m_RVSState;
	bool m_IsOverylayPresent;
	IRvs* m_Rvs;
	map<unsigned int, ScreenElement*> m_DataElementsMap;
	RVSGuidelinePaths m_GuidelinePoints;
	CachableData* m_cachedData;
	kivi::ipc::IIPC* m_bus;
	KMainLoop* m_pMainloop;
	IHardKeyStateProvider* m_pHardKeyStateProvider;
	handler_id m_KeyStateChangedId;
	IDiagnosticsManager* m_DiagnosticManager;
	bool m_showGuidelines ;
	bool m_showSymbols;
    bool m_showSmartTowGuidelines;
    VehicleMovementState_t m_VehicleMovementState;
    unsigned long m_RvsFeatureAvailSmartTow;
    unsigned int m_VehiclePowerMode;
    unsigned int m_VehicleSpeed;
    unsigned char m_RvsDisableHysteresisSpeed;
    unsigned int m_RvsForwardExitTimer;
    unsigned int m_RvsParkExitTimer;
    bool m_CurAppState;
    int m_SmartTowState;
};

#endif
