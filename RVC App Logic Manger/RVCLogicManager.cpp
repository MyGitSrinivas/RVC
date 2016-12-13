/*#############################################################################
 *  (C) 2014  Fujitsu Ten Limited. All Rights Reserved.
 * =============================================================================
 *  HARDWARE         : INFO3_LOW
 *  MODULE           : RVC
 *  VERSION          : v0.0.1
 *  FILE NAME        : RVCLogicManager.cpp
 *  FILE DESCRIPTION : Manages all the Logic for screens of RVC.
 *  NOTE				: N/A
 *############################################################################*/

#include "RVCLogicManager.h"
#include "logger.h"
#include <Utility.h>


RVCLogicManager::RVCLogicManager() :
m_UpdatableView(NULL),
m_SMObj(NULL),
m_RVSState(RVS_STATE_UNKNOWN),
m_IsOverylayPresent(false),
m_Rvs(NULL),
m_cachedData(NULL),
m_bus(NULL),
m_pMainloop(NULL),
m_pHardKeyStateProvider(NULL),
m_KeyStateChangedId(-1),
m_DiagnosticManager(NULL),
m_showGuidelines(false),
m_showSmartTowGuidelines(false),
m_VehicleMovementState(VEHICLE_MOVEMENT_STATE_INVALID),
m_VehiclePowerMode(RVS_VPM_OFF),
m_VehicleSpeed(0),
m_CurAppState(false),
m_SmartTowState(-1)
{
    //Read Calibration and set respectively
   // InitialiseCalibration();
}

RVCLogicManager::~RVCLogicManager()
{
	m_UpdatableView = NULL;
	m_SMObj = NULL;
	m_cachedData = NULL;
	m_bus = NULL;
	m_pHardKeyStateProvider = NULL;
	m_pMainloop = NULL;
	m_DiagnosticManager = NULL;
	if (NULL != m_Rvs)
	{
		delete m_Rvs;
		m_Rvs = NULL;
	}
	else{}//do nothing
	RELEASE_PERSISTENCE
    // Release calibration
   // FREE_FRONTREARCAMERASTATIC();
}

void RVCLogicManager::Init(IUpdatableView* updatableView, IHMIF* hmif, IRVCScreenManager* screnMgr, map<unsigned int, ScreenElement*>& elementMap){

	m_DataElementsMap = elementMap;
	m_SMObj = screnMgr ;
    LOG1(("Init : Initializing RVC Application..."));
	m_UpdatableView = updatableView;
	m_bus  = hmif->GetIPC();
	m_pMainloop = hmif->GetMainLoop();
	m_pHardKeyStateProvider = hmif->GetHardKeyStateProvider();
	m_DiagnosticManager = hmif->GetDiagnosticsManager();
}

void RVCLogicManager::Start(){
    LOG1(("Start : Initializing RVC Application..."));
	HMIResolution_t resolution;
	resolution.width = 800 ;
	resolution.height = 480 ;

#ifdef PLATFORM_X86
	LOG1((TEXT("Init: Create  Platform Fatory for Host.")));

	IAppSimulationProvider* simulator = SimulationProvider::GetSimulationProvider();
	m_Rvs = new RVCSimulation(this);
	simulator->Register(APP_RVC, dynamic_cast<IFeatureSimulater*>(m_Rvs));

#else
    //m_Rvs = new Rvs();
    m_Rvs = new RVSStub();
#endif
	if (NULL != m_Rvs){
        m_Rvs->Init(this,resolution, BOTTOM_LEFT,m_bus,m_pMainloop,m_DiagnosticManager);
		m_Rvs->GetRvsState(&m_RVSState);
		LOG1((TEXT(" The state from RVS lib: %d \n"), m_RVSState));
	}
	else{
		LOGERR(("IRvs is NULL \n"));
	}

	LoadData();
	if (NULL != m_cachedData){
		RVSConfiguration rvsConfig;
		m_showGuidelines =  m_cachedData->RVC_ShowGuideline;
        m_showSmartTowGuidelines = m_cachedData->RVC_ShowSmartTowGuideline;
		m_showSymbols = m_cachedData->RVC_ShowSymbols;

		m_SMObj->SymbolState(m_showSymbols);
		rvsConfig.isGuidelineEnabled = m_cachedData->RVC_ShowGuideline;
		rvsConfig.isUPAEnabled= m_showSymbols;
		rvsConfig.isRCTAEnabled= m_showSymbols;
		if (NULL != m_Rvs){
			if (RVS_STATUS_OK == m_Rvs->SetRVSConfiguration(rvsConfig)){
				UpdateGuidelines();
			}
			else{}//do nothing
		}
		else{
			LOGERR(("IRvs is NULL \n"));
		}

		if (true == m_cachedData->RVC_ShowGuideline){

			if(m_UpdatableView) {

				m_UpdatableView->UpdateData(RVC_GUIDELINES_UPDATED, &m_GuidelinePoints) ;
			}
			else{}//do nothing
		}
		else{}//do nothing
	}
	else{}//do nothing
}


void RVCLogicManager::Resume(){
	OnCancelShutDown();
}

void RVCLogicManager::Finalize(){

	if( NULL != m_cachedData){
		m_cachedData->Store();
		m_cachedData->HandleShutdown();
	}
	else{}//do nothing
}

void RVCLogicManager::OnCancelShutDown(){
	if( NULL != m_cachedData){
		m_cachedData->CancelShutdown();
	}
	else{}//do nothing
}
void RVCLogicManager::LoadData(){
	if (NULL == m_cachedData){
		DECLARE_PERSISTENCE(m_cachedData,m_bus)

			if ( NULL != m_cachedData ){
				m_cachedData->Load();
			}
			else{}//do nothing
		END_DECLARE_PERSISTENCE
	}
	else{}//do nothing
}

RVSState_t RVCLogicManager::GetRVSState(){
	return m_RVSState;
}

bool RVCLogicManager::IsOverlayPresent(){
	return m_IsOverylayPresent ;
}

//void RVCLogicManager::InitialiseCalibration()
//{
//    LOG1((TEXT("MY19 ==== InitialiseCalibration \n")));
//    USE_FRONTREARCAMERASTATIC();
//    m_RvsFeatureAvailSmartTow = I3L_CAL_RVS_FEATURE_AVAIL_SMART_TOW;
//    LOG1((TEXT("DBG CAL Smart Tow Feature %d\n"), m_RvsFeatureAvailSmartTow));
//    m_RvsDisableHysteresisSpeed = I3L_CAL_RVS_DISABLE_HYSTERESIS_SPEED;
//    LOG1((TEXT("DBG CAL Hysteresis Speed %d\n"), m_RvsDisableHysteresisSpeed));
//    m_RvsForwardExitTimer = I3L_CAL_RVS_SMART_TOW_EXIT_TIME;
//    LOG1((TEXT("DBG CAL Forward Gear RVC Exit Timer %d\n"), m_RvsForwardExitTimer));
//    m_RvsParkExitTimer = I3L_CAL_RVS_RVC_PARK_EXIT_TIME;
//    LOG1((TEXT("DBG CAL Reverse to Park RVC Exit Timer %d\n"), m_RvsParkExitTimer));
//}

bool RVCLogicManager::IsSmartTowFeatureEnabled()
{
    return true;
    //return m_RvsFeatureAvailSmartTow;
}

void RVCLogicManager::OnVehicleSpeedChanged(unsigned int vehicleSpeed)
{
    LOG1((TEXT("MY19 ==== OnVehicleSpeedChanged speed = %d \n"), vehicleSpeed));
    m_VehicleSpeed = vehicleSpeed;
}

void RVCLogicManager::OnPowerModeChanged(RVSVehiclePowerMode_t vehiclePowerMode)
{
    LOG1((TEXT("MY19 ==== OnPowerModeChanged = %d\n"), vehiclePowerMode));
    m_VehiclePowerMode = vehiclePowerMode;
}

unsigned int RVCLogicManager::getVehicleSpeed()
{
    return m_VehicleSpeed;
}

void RVCLogicManager::ToggleShowSmartTowGuideLine()
{
    LOG1((TEXT("MY19 ==== ToggleShowStandardGuideline \n")));
    m_showSmartTowGuidelines = !m_showSmartTowGuidelines;
    if (NULL != m_cachedData){
        m_cachedData->RVC_ShowSmartTowGuideline = !m_cachedData->RVC_ShowSmartTowGuideline;
        RVSConfiguration rvsConfig;
        rvsConfig.isGuidelineEnabled = m_cachedData->RVC_ShowSmartTowGuideline;
        rvsConfig.isUPAEnabled = false;
        rvsConfig.isRCTAEnabled = false;
        if (NULL != m_Rvs){
            if (RVS_STATUS_OK == m_Rvs->SetRVSConfiguration(rvsConfig)){
                UpdateSmartTowGuidelines();
            }
            else{
                //restore old state
                m_cachedData->RVC_ShowSmartTowGuideline = !m_cachedData->RVC_ShowSmartTowGuideline;
                m_showSmartTowGuidelines = !m_showSmartTowGuidelines;
            }
        }
        else{
            LOGERR(("IRvs is NULL \n"));
        }
        if (true == m_cachedData->RVC_ShowSmartTowGuideline){
            if(m_UpdatableView) {
                m_UpdatableView->UpdateData(RVC_GUIDELINES_UPDATED, &m_GuidelinePoints) ; }
        }
        else{}//do nothing
    }
    else{}
}

void RVCLogicManager::ToggleShowGuideline(){
    LOG1((TEXT("MY19 ==== ToggleShowStandardGuideline \n")));
	m_showGuidelines = !m_showGuidelines;
	if (NULL != m_cachedData){
		m_cachedData->RVC_ShowGuideline = !m_cachedData->RVC_ShowGuideline;
		RVSConfiguration rvsConfig;
		rvsConfig.isGuidelineEnabled = m_cachedData->RVC_ShowGuideline;
		rvsConfig.isUPAEnabled= m_showSymbols;
		rvsConfig.isRCTAEnabled= m_showSymbols;
		if (NULL != m_Rvs){
			if (RVS_STATUS_OK == m_Rvs->SetRVSConfiguration(rvsConfig)){
				UpdateGuidelines();
			}
			else{
				//restore old state
				m_cachedData->RVC_ShowGuideline = !m_cachedData->RVC_ShowGuideline;
				m_showGuidelines = !m_showGuidelines;
			}
		}
		else{
			LOGERR(("IRvs is NULL \n"));
		}

		if (true == m_cachedData->RVC_ShowGuideline){

			if(m_UpdatableView) {

				m_UpdatableView->UpdateData(RVC_GUIDELINES_UPDATED, &m_GuidelinePoints) ; }
		}
		else{}//do nothing
	}
	else{}//do nothing

}

void RVCLogicManager::UpdateRVSData(unsigned int RVSDataIdentifier,std::string m_ActualValue){

	if(m_UpdatableView)
	{
		m_UpdatableView->UpdateData(m_DataElementsMap[RVSDataIdentifier],  m_ActualValue);
	}
	else{}//do nothing
}

void RVCLogicManager::setHitchViewCameraState(RVSCameraStatus_t RVSCameraStatus)
{
    LOG1((TEXT("MY19 ==== setHitchViewCameraState: Hitch Camera View status: %d\n"), RVSCameraStatus));
    m_Rvs->setHitchViewCameraOn(RVSCameraStatus);
}

void RVCLogicManager::setRearCameraState(RVSCameraStatus_t RVSCameraStatus)
{
    LOG1((TEXT("MY19 ==== setRearCameraState: Rear Camera View status: %d\n"), RVSCameraStatus));
    m_Rvs->setRearCameraOn(RVSCameraStatus);
}


void RVCLogicManager::setCameraAppIconStatus(bool appState)
{
    LOG1((TEXT("MY19 ==== setCameraAppIconStatus: Camera App Icon status: %d\n"), appState));
    m_CurAppState = appState;
    if (m_CurAppState != false)
    {
        m_SmartTowState = 0;
        handleState();
    }
    else
    {
        LOG1((TEXT("MY19 ==== setCameraAppIconStatus: Camera Icon not clicked %d\n"), appState));
    }
}

void RVCLogicManager::handleState()
{
    LOG1((TEXT("MY19 ====  handleState:\n")));
    bool isSTGuidelinesEnabled = m_cachedData->RVC_ShowSmartTowGuideline;
    LOG1((TEXT("isSTGuidelinesEnabled - %d\n"), isSTGuidelinesEnabled));
    switch(m_SmartTowState)
    {
    case 0: // Case when event pressed from HMI Camera Icon
        switch(m_VehicleMovementState)
        {
        case VEHICLE_MOVEMENT_STATE_PARKED:
        case VEHICLE_MOVEMENT_STATE_NEUTRAL:
            m_RVSState = RVS_RVC_ACTIVE;
            break;
        case VEHICLE_MOVEMENT_STATE_FORWARD:
            if (m_VehicleSpeed > 7)
            {
                m_RVSState = RVS_RVC_ACTIVE;
                // start timer for 8 secs
            }
            else
            {
               while(m_VehicleSpeed > 20) //m_RvsDisableHysteresisSpeed)
               {
                   if(m_RVSState != RVS_RVC_ACTIVE)
                   {
                        m_RVSState = RVS_RVC_ACTIVE;
                        setRearCameraState(REAR_CAMERA_ON );
                        m_SMObj->OnRVSStateChanged(m_RVSState);
                   }
               }
               m_RVSState = RVS_INACTIVE;
            }
            break;
        case VEHICLE_MOVEMENT_STATE_REVERSE:
            // do nothing
            break;
        default:
            break;
            //
        }
    case 1: // Case when event from sensor
        switch(m_VehicleMovementState)
        {
        case VEHICLE_MOVEMENT_STATE_REVERSE:
            m_RVSState = RVS_RVC_ACTIVE;
            break;
        case VEHICLE_MOVEMENT_STATE_PARKED:
            m_RVSState = RVS_RVC_ACTIVE;
            // start timer for 3 secs
            break;
        case VEHICLE_MOVEMENT_STATE_FORWARD:
        case VEHICLE_MOVEMENT_STATE_NEUTRAL:
            m_RVSState = RVS_INACTIVE;
            break;
        default:
            break;
        }
    }
    if (RVS_RVC_ACTIVE == m_RVSState)
    {
        setRearCameraState(REAR_CAMERA_ON );
        m_SMObj->OnRVSStateChanged(m_RVSState);
    }
    else if (RVS_INACTIVE == m_RVSState)
    {
        setRearCameraState(REAR_CAMERA_OFF);
        m_SMObj->OnRVSStateChanged(m_RVSState);
    }
    else
    {
        //do nothing
    }
}

void RVCLogicManager::OnRVSStateChanged(RVSState_t rvsState){

	LOG5((TEXT("OnRVSStateChanged: The state from RVS lib: %d=%d\n"), m_RVSState , rvsState ));
	if (m_RVSState != rvsState ){
		m_RVSState = rvsState ;

		if (NULL != m_SMObj){
			LOG1((TEXT("OnRVSStateChanged:LM rvsState %d\n"), rvsState));
            //m_SMObj->OnRVSStateChanged(rvsState);
		}
		else{
			LOGERR(("RVCScreenManager is NULL \n"));
		}//Do nothing
	}
	else{}//we have same state.. no change .. no logic
}

void RVCLogicManager::OnOverlayTextLine1Changed(bool visible, RVSText_t messageId,  RVSTextHPos_t hPosition, RVSTextVPos_t vPosition){

	m_SMObj->OnTextLineChanged(visible,messageId);

}

void RVCLogicManager::OnOverlayTextLine2Changed(bool visible, RVSText_t messageId,  RVSTextHPos_t hPosition, RVSTextVPos_t vPosition){
	LOG3((TEXT("OnOverlayTextLine2Changed visible %d  messageId %d hPosition %d vPosition %d \n"), visible,messageId,hPosition,vPosition));
	m_IsOverylayPresent = visible ;
	if (true == visible){
		UpdateRVSData(RVC_OVERLAY_TEXT_2_PRESENT,"1");
		UpdateRVSData(RVC_OVERLAY_TEXT_2_POSITION,(RVS_TEXT_VPOS_TOP == vPosition) ? "1" : "0");
	}
	else{
		UpdateRVSData(RVC_OVERLAY_TEXT_2_PRESENT,"0");
	}

	m_SMObj->OnTextLineChanged(visible,messageId);
}

void RVCLogicManager::OnUpaSymbolChanged(bool visible, RVSUPAZone_t zone, RVSUvCoord_t coordinate,
		RVSColor_t color, RVSUpaSize_t size,
		RVSAnimation_t animation){
	UpdateRVSData(RVC_UPASYMBOLZONE,Utility::IntToString((unsigned int)zone));
	LOG5((TEXT("OnUpaSymbolChanged:UPA ZONE RVC Application...: %d\n"),(unsigned int) zone));
	if (visible){
		UpdateRVSData(RVC_UPASYMBOL_FLASHING,Utility::IntToString((unsigned int)animation));
		switch(zone){

		case RVS_UPA_ZONE_1:
			LOG5((TEXT("RVS_UPA_ZONE_1:LM coordinate.u %d coordinate.v %d \n"), coordinate.u,coordinate.v));
			UpdateRVSData(RVC_UPASYMBOLZONE1_XCOORDINATE,Utility::IntToString(coordinate.u));
			UpdateRVSData(RVC_UPASYMBOLZONE1_YCOORDINATE,Utility::IntToString(coordinate.v));
			break;

		case RVS_UPA_ZONE_2:
			LOG5((TEXT("RVS_UPA_ZONE_2:LM coordinate.u %d coordinate.v %d \n"), coordinate.u,coordinate.v));
			UpdateRVSData(RVC_UPASYMBOLZONE2_XCOORDINATE,Utility::IntToString(coordinate.u));
			UpdateRVSData(RVC_UPASYMBOLZONE2_YCOORDINATE,Utility::IntToString(coordinate.v));
			break;

		case RVS_UPA_ZONE_3:
			LOG5((TEXT("RVS_UPA_ZONE_3:LM coordinate.u %d coordinate.v %d \n"), coordinate.u,coordinate.v));
			UpdateRVSData(RVC_UPASYMBOLZONE3_XCOORDINATE,Utility::IntToString(coordinate.u));
			UpdateRVSData(RVC_UPASYMBOLZONE3_YCOORDINATE,Utility::IntToString(coordinate.v));
			break;

		case RVS_UPA_ZONE_4:
			LOG5((TEXT("RVS_UPA_ZONE_4:LM coordinate.u %d coordinate.v %d \n"), coordinate.u,coordinate.v));
			UpdateRVSData(RVC_UPASYMBOLZONE4_XCOORDINATE,Utility::IntToString(coordinate.u));
			UpdateRVSData(RVC_UPASYMBOLZONE4_YCOORDINATE,Utility::IntToString(coordinate.v));
			break;

		default:
			LOGERR((TEXT("Invalid RVS_UPA_ZONE %d\n"), zone));
			break;
		}
	}
	else{
		UpdateRVSData(RVC_UPASYMBOL_FLASHING,"-1");
	}

}

void RVCLogicManager::OnLeftRctaChanged(bool visible, RVSUvCoord_t coordinate, RVSAnimation_t animation){

	LOG3((TEXT("OnLeftRctaChanged:OnRearCrossTrafficAlertLeftStateChanged visible: %d\n"), visible));
	UpdateRVSData(RVC_LEFTRCTASYMBOLSTATUS,Utility::IntToString((unsigned int)visible));
	if (visible){
		UpdateRVSData(RVC_LEFTRCTASYMBOL_FLASHING,Utility::IntToString((unsigned int)animation));
		LOG5((TEXT("OnLeftRctaChanged:LM coordinate.u %d coordinate.v %d \n"), coordinate.u,coordinate.v));
		UpdateRVSData(RVC_LEFTRCTASYMBOL_XCOORDINATE,Utility::IntToString(coordinate.u));
		UpdateRVSData(RVC_LEFTRCTASYMBOL_YCOORDINATE,Utility::IntToString(coordinate.v));
	}
	else{}//do nothing
}
void RVCLogicManager::OnRightRctaChanged(bool visible, RVSUvCoord_t coordinate, RVSAnimation_t animation){

	LOG3((TEXT("OnRightRctaChanged:OnRearCrossTrafficAlertRightStateChanged visible: %d\n"), visible));
	UpdateRVSData(RVC_RIGHTRCTASYMBOLSTATUS,Utility::IntToString((unsigned int)visible));
	if (visible){
		UpdateRVSData(RVC_RIGHTRCTASYMBOL_FLASHING,Utility::IntToString((unsigned int)animation));
		LOG5((TEXT("OnRightRctaChanged:LM coordinate.u %d coordinate.v %d \n"), coordinate.u,coordinate.v));
		UpdateRVSData(RVC_RIGHTRCTASYMBOL_XCOORDINATE,Utility::IntToString(coordinate.u));
		UpdateRVSData(RVC_RIGHTRCTASYMBOL_YCOORDINATE,Utility::IntToString(coordinate.v));
	}
	else{}//do nothing
}

void RVCLogicManager::OnHitchViewGuideLineChanged(bool visible, const RVSGuidelinePaths& guidelinePoints)
{
    LOG5((TEXT("OnHitchViewGuideLineChanged visible : %d\n"), visible));
    m_showSmartTowGuidelines = visible;
    m_cachedData->RVC_ShowSmartTowGuideline = m_showSmartTowGuidelines;
    UpdateSmartTowGuidelines();
    if(m_UpdatableView)
    {
        if (RVS_RVC_ACTIVE == m_RVSState)
        {
            lock_t guard(m_GuidelinePoints.m_DataLock);
            {
                m_GuidelinePoints.m_leftBoundaryColor = guidelinePoints.m_leftBoundaryColor;
                m_GuidelinePoints.m_leftBoundary = guidelinePoints.m_leftBoundary;
                m_GuidelinePoints.m_rightBoundaryColor = guidelinePoints.m_rightBoundaryColor;
                m_GuidelinePoints.m_rightBoundary= guidelinePoints.m_rightBoundary;
                m_GuidelinePoints.m_leftWedgeColor = guidelinePoints.m_leftWedgeColor;
                m_GuidelinePoints.m_leftWedge = guidelinePoints.m_leftWedge;
                m_GuidelinePoints.m_rightWedgeColor = guidelinePoints.m_rightWedgeColor;
                m_GuidelinePoints.m_rightWedge = guidelinePoints.m_rightWedge;
                m_GuidelinePoints.m_distanceMarkerColor = guidelinePoints.m_distanceMarkerColor;
                m_GuidelinePoints.m_distanceMarker = guidelinePoints.m_distanceMarker;
            }
            if ( m_showGuidelines ){
                m_UpdatableView->UpdateData(RVC_GUIDELINES_UPDATED, &m_GuidelinePoints);
            }
        }
        else{}//do nothing
    }
    else {}// do nothing
}

void RVCLogicManager::OnVehicleMovementStateChanged(VehicleMovementState_t vehicleMovementState)
{
    LOG1((TEXT("OnVehicleMovementStateChanged vehicleMovementState : %d\n"), vehicleMovementState));
    m_VehicleMovementState = vehicleMovementState;
    m_SmartTowState = 1;
    handleState();
}

VehicleMovementState_t RVCLogicManager::getVehicleMovementState()
{
    return m_VehicleMovementState;
}

void RVCLogicManager::OnGuideLineChanged(bool visible, const RVSGuidelinePaths& guidelinePoints){
	LOG5((TEXT("OnGuideLineChanged visible : %d\n"), visible));
	m_showGuidelines = visible;
	m_cachedData->RVC_ShowGuideline = m_showGuidelines;
	UpdateGuidelines();
	if(m_UpdatableView) {

		if (RVS_RVC_ACTIVE == m_RVSState){

			lock_t guard(m_GuidelinePoints.m_DataLock);

			{

				m_GuidelinePoints.m_leftBoundaryColor = guidelinePoints.m_leftBoundaryColor;

				m_GuidelinePoints.m_leftBoundary = guidelinePoints.m_leftBoundary;

				m_GuidelinePoints.m_rightBoundaryColor = guidelinePoints.m_rightBoundaryColor;

				m_GuidelinePoints.m_rightBoundary= guidelinePoints.m_rightBoundary;

				m_GuidelinePoints.m_leftWedgeColor = guidelinePoints.m_leftWedgeColor;

				m_GuidelinePoints.m_leftWedge = guidelinePoints.m_leftWedge;

				m_GuidelinePoints.m_rightWedgeColor = guidelinePoints.m_rightWedgeColor;

				m_GuidelinePoints.m_rightWedge = guidelinePoints.m_rightWedge;

				m_GuidelinePoints.m_distanceMarkerColor = guidelinePoints.m_distanceMarkerColor;

				m_GuidelinePoints.m_distanceMarker = guidelinePoints.m_distanceMarker;

			}

			if ( m_showGuidelines ){

				m_UpdatableView->UpdateData(RVC_GUIDELINES_UPDATED, &m_GuidelinePoints);

			}

		}

		else{}//do nothing
	}

	else {}//do nothing


	//		print_guidelines_points(guidelinePoints.m_leftWedge, "m_leftWedge " );
	//		print_guidelines_points(guidelinePoints.m_rightWedge, "m_rightWedge " );
	//		print_guidelines_points(guidelinePoints.m_leftBoundary, "m_leftBoundary " );
	//		print_guidelines_points(guidelinePoints.m_rightBoundary, "m_rightBoundary " );
	//
	//		LOG3((TEXT(" %s size = %d\n"), "Distance Markers", guidelinePoints.m_distanceMarker.size()));
	//		//int index = 0;
	//		for(unsigned int index = 0; index < guidelinePoints.m_distanceMarker.size(); index++){
	//		    LOG3((TEXT("Distance Marker @ %d \n"), index));
	//		    std::vector<RVSUvCoord_t> linePoints = guidelinePoints.m_distanceMarker.at(index);
	//		    print_guidelines_points(linePoints , " " );
	//		}
}

//void RVCLogicManager::print_guidelines_points(vector<RVSUvCoord_t> dataPoints, string message){
//
//	LOG3((TEXT("print_guidelines_points %s size = %d\n"), message.c_str(), dataPoints.size()));
//	for ( vector<RVSUvCoord_t>::iterator point = dataPoints.begin(); point != dataPoints.end();point++){
//		LOG3((TEXT("[%5d, %5d] \n"), 800 - (*point).u, (*point).v));
//	}
//}

void RVCLogicManager::CancelHysteresisTimer(){

	LOG5((TEXT("CancelHysteresisTimer \n")));
	m_Rvs->CancelHysteresis();

}

void RVCLogicManager::SubscribeHardKey(){


	if((NULL != m_pHardKeyStateProvider) && (-1 == m_KeyStateChangedId)){
		LOG5((TEXT("SubscribeHardKey::m_KeyStateChangedId= %d \n"),m_KeyStateChangedId));
		m_KeyStateChangedId = m_pHardKeyStateProvider->KeyStateChanged.subscribe(this, &RVCLogicManager::OnHardKeyChangeStatus);
	}
	else{}//do nothing
}

void RVCLogicManager::OnHardKeyChangeStatus(hardkey_keycode keyCode, key_state keyState){


	if (HARDKEY_NONE != keyCode){
		LOG5((TEXT("OnHardKeyChangeStatus \n")));
		CancelHysteresisTimer();
	}
	else{}//do nothing

}
void RVCLogicManager::UnsubscribeHardKey(){

	if((NULL != m_pHardKeyStateProvider) && (-1 != m_KeyStateChangedId)){
		LOG5((TEXT("HardKeyUnsubscription \n")));
		m_pHardKeyStateProvider->KeyStateChanged.unsubscribe(m_KeyStateChangedId);
		m_KeyStateChangedId = -1;
	}
	else{}//do nothing
}

void RVCLogicManager::ShowSymbols(bool symbolState){
	LOG5((TEXT("SymbolState:%d\n"),symbolState));
	m_cachedData->RVC_ShowSymbols = symbolState;
	m_showSymbols = symbolState;
	RVSConfiguration rvsConfig;

	rvsConfig.isGuidelineEnabled = m_showGuidelines;
	rvsConfig.isUPAEnabled= m_showSymbols;
	rvsConfig.isRCTAEnabled= m_showSymbols;
	if (NULL != m_Rvs){
		m_Rvs->SetRVSConfiguration(rvsConfig);
	}

}

void RVCLogicManager::UpdateSmartTowGuidelines(){

    LOG1((TEXT("MY19 === UpdateSmartTowGuidelines: \n")));
    UpdateRVSData(RVC_SHOW_GUIDELINE,m_cachedData->RVC_ShowSmartTowGuideline ? "1" : "0");

    if(m_UpdatableView) {

        m_UpdatableView->UpdateData(RVC_SHOW_GUIDELINE, &m_cachedData->RVC_ShowSmartTowGuideline) ;
    }
    else{}//do nothing
}


void RVCLogicManager::UpdateGuidelines(){

    LOG1((TEXT("MY19 === UpdateGuidelines: \n")));
    UpdateRVSData(RVC_SHOW_GUIDELINE,m_cachedData->RVC_ShowGuideline ? "1" : "0");

    if(m_UpdatableView) {

        m_UpdatableView->UpdateData(RVC_SHOW_GUIDELINE, &m_cachedData->RVC_ShowGuideline) ;
    }
    else{}//do nothing
}
