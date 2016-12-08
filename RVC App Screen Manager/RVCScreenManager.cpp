/*#############################################################################
 *  (C) 2014  Fujitsu Ten Limited. All Rights Reserved.
 * =============================================================================
 *  HARDWARE         : INFO3_LOW
 *  MODULE           : RVC
 *  VERSION          : v0.0.1
 *  FILE NAME        : RVCScreenManager.cpp
 *  FILE DESCRIPTION : Manages all the screens of RVC.
 *  NOTE				: N/A
 *############################################################################*/

#include "RVCScreenManager.h"
#include "ApplicationId.h"

RVCScreenManager::RVCScreenManager(DisplayType viewType,
		IRVCLogicManager* rvcLogicManager) :
		m_precedenceManger(NULL),
		m_LayerManager(NULL),
		m_AmIActive(false),
		m_ClientId(-1),
		m_AppDetails(RVC, "RearViewCamera", "",APP_RVC, "RearViewCamera", Native, HeadUnit),
		m_NotificationManager(NULL),
		m_rvsState(RVS_INACTIVE),
		m_rvsSymbolState(false)

{

	m_LMObj = rvcLogicManager;
	LOG5(("RVCScreenManager:Initializing RVC Application..."));
}


RVCScreenManager::~RVCScreenManager()
{

}

void RVCScreenManager::Init(IHMIF* hmif)
{
	LOG1(("Init : Initializing RVC Application..."));
	m_precedenceManger = hmif->GetPrecedenceManager();
	m_NotificationManager = hmif->GetNotificationManager();
	m_pEventConsumer = hmif->GetEventConsumer();
	// Register with layer manager
	m_LayerManager = hmif->GetLayerManager();
	m_LayerManager->LMRegister(APP_RVC,RVC,LM_PLANE_RVC,&m_ClientId);
	SetDataElements();
	RegisterScreens();
	setAudibleNotification();

	m_LMObj->Init(hmif->GetUpatableView(),hmif, this, m_ElementsMap);
}

void RVCScreenManager::Activate(const bool &isActive) {
	m_AmIActive = isActive;
}

const AppDetails* RVCScreenManager::GetAppInfo() {
	return  &m_AppDetails;
}


void RVCScreenManager::Pause() {
	/* do nothing */
}

void RVCScreenManager::Resume() {
	m_LMObj->Resume();
}

void RVCScreenManager::Start() {
	LOG1(("start : Initializing RVC Application..."));
	m_LMObj->Start();
}

bool RVCScreenManager::Stop() {
	return true;
}

bool RVCScreenManager::OnShuttingDown(){
	m_LMObj->Finalize();
	return true;
}

void RVCScreenManager::OnSuspend(lfc::lfc_reason_t reason){
	if (reason == E_LFC_REASON_SOFTWARE_UPDATE){
		m_LMObj->Finalize();
	}
	else{}//do nothing
}

void RVCScreenManager::OnResume(lfc::lfc_reason_t reason){
	if (reason == E_LFC_REASON_SOFTWARE_UPDATE){
		m_LMObj->Resume();
	}
	else{}//do nothing
}

void RVCScreenManager::OnHMIReady(){
	if(NULL != m_pEventConsumer) {
			m_pEventConsumer->HandleEvent(1, EVG_REARCAM_PARK_ASSIST_LAST_VALUE, Utility::IntToString(m_rvsSymbolState));
		}
		else {
			LOGERR((TEXT("m_pEventConsumer is NULL \n")));
		}
}

void RVCScreenManager::HandleEvent(unsigned int eventId, const std::string& eventArgs) {
	if (EVG_RVC_ENABLEGUIDELINE == eventId){
		if (NULL != m_LMObj){
			m_LMObj->ToggleShowGuideline();
		}
		else{}//do nothing
	}
	else if(EVG_RVC_EXIT == eventId){
		if (RVS_DISABLE == m_rvsState){
			m_LMObj->CancelHysteresisTimer();
		}
		else{}//do nothing
		//if(NULL != m_precedenceManger && NULL != m_NotificationManager){
		//m_precedenceManger->RemoveFromTop(this);
		//This is a patch as rvs lib is not sending overlay text changed as false when Reverse
		//m_NotificationManager->RemoveQuickNotice(this,SC_RVC_TRANSMISSION_FAIL_QUICK_NOTICE);
	}
	else if (EVG_REARCAM_PARK_ASSIST_VALUE == eventId){
		bool symbolState = (bool) atoi(eventArgs.c_str());
		m_LMObj->ShowSymbols(symbolState);
	}
	else{}//do nothing
}
//else{}//Not valid event ID


unsigned int RVCScreenManager::GetId() {
	return APP_RVC;
}

const ScreenElementsMap& RVCScreenManager::GetScreens() {
	return m_ScreenElementMap;
}

const ScreenElementsList&  RVCScreenManager::GetScreenDataId(unsigned int  screenName) {
	return m_ScreenElementMap[screenName];
}

void RVCScreenManager::OnRVSStateChanged(RVSState_t rvsState){
	m_rvsState = rvsState;
	if(NULL != m_precedenceManger && NULL != m_NotificationManager){
		switch(rvsState)
		{
		case RVS_INACTIVE :
			LOG5(("OnRVSStateChanged : RVS_INACTIVE...\n"));
			if (false == m_LMObj->IsOverlayPresent()){
				LOGPERF((TEXT("RVC : OnRVSStateChanged RVS_INACTIVE \n")));
				m_precedenceManger->RemoveFromTop(this);
				m_LMObj->UnsubscribeHardKey();
			}
			else{}//do nothing
			break;

		case RVS_DISABLE:
			LOG5(("OnRVSStateChanged: RVS_DISABLE \n"));
			m_LMObj->SubscribeHardKey();
			break;

		case RVS_RVC_ACTIVE:
			LOGPERF((TEXT("RVC : OnRVSStateChanged RVS_ACTIVE \n")));
			m_precedenceManger->BringToForeground(this,SC_RVC_VIEW);
			break;

		default:
			LOGERR((TEXT("Invalid RVS State %d\n"), rvsState));
			break;

		}
	}
	else{}//do nothing
}

void RVCScreenManager::RegisterScreens() {

	std::list<ScreenElement *> screenElementRVC;
	screenElementRVC.push_back(m_ElementsMap[RVC_UPASYMBOLZONE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_UPASYMBOL_FLASHING]);
	screenElementRVC.push_back(m_ElementsMap[RVC_LEFTRCTASYMBOLSTATUS]);
	screenElementRVC.push_back(m_ElementsMap[RVC_RIGHTRCTASYMBOLSTATUS]);
	screenElementRVC.push_back(m_ElementsMap[RVC_SHOW_GUIDELINE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_LEFTRCTASYMBOL_FLASHING]);
	screenElementRVC.push_back(m_ElementsMap[RVC_RIGHTRCTASYMBOL_FLASHING]);
	screenElementRVC.push_back(m_ElementsMap[RVC_OVERLAY_TEXT_2_PRESENT]);
	screenElementRVC.push_back(m_ElementsMap[RVC_UPASYMBOLZONE1_XCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_UPASYMBOLZONE1_YCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_UPASYMBOLZONE2_XCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_UPASYMBOLZONE2_YCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_UPASYMBOLZONE3_XCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_UPASYMBOLZONE3_YCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_UPASYMBOLZONE4_XCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_UPASYMBOLZONE4_YCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_LEFTRCTASYMBOL_XCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_LEFTRCTASYMBOL_YCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_RIGHTRCTASYMBOL_XCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_RIGHTRCTASYMBOL_YCOORDINATE]);
	screenElementRVC.push_back(m_ElementsMap[RVC_OVERLAY_TEXT_2_POSITION]);
	m_ScreenElementMap[SC_RVC_VIEW]=screenElementRVC;

	PolicyMapper mapper;
	mapper[SC_RVC_VIEW] = 1669500928u;
	m_precedenceManger->RegisterScreens(this,mapper);
}

void RVCScreenManager::SetDataElements()
{
	m_ElementsMap[RVC_UPASYMBOLZONE]= SetDataElement(RVC_UPASYMBOLZONE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_UPASYMBOL_FLASHING]= SetDataElement(RVC_UPASYMBOL_FLASHING,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_LEFTRCTASYMBOLSTATUS]= SetDataElement(RVC_LEFTRCTASYMBOLSTATUS,SinkHeadUnit, NUMERIC,"");
	m_ElementsMap[RVC_RIGHTRCTASYMBOLSTATUS]= SetDataElement(RVC_RIGHTRCTASYMBOLSTATUS,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_SHOW_GUIDELINE]= SetDataElement(RVC_SHOW_GUIDELINE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_LEFTRCTASYMBOL_FLASHING]= SetDataElement(RVC_LEFTRCTASYMBOL_FLASHING,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_RIGHTRCTASYMBOL_FLASHING]= SetDataElement(RVC_RIGHTRCTASYMBOL_FLASHING,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_GUIDELINES_UPDATED]= SetDataElement(RVC_GUIDELINES_UPDATED,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_OVERLAY_TEXT_2_PRESENT]= SetDataElement(RVC_OVERLAY_TEXT_2_PRESENT,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_UPASYMBOLZONE1_XCOORDINATE]= SetDataElement(RVC_UPASYMBOLZONE1_XCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_UPASYMBOLZONE1_YCOORDINATE]= SetDataElement(RVC_UPASYMBOLZONE1_YCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_UPASYMBOLZONE2_XCOORDINATE]= SetDataElement(RVC_UPASYMBOLZONE2_XCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_UPASYMBOLZONE2_YCOORDINATE]= SetDataElement(RVC_UPASYMBOLZONE2_YCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_UPASYMBOLZONE3_XCOORDINATE]= SetDataElement(RVC_UPASYMBOLZONE3_XCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_UPASYMBOLZONE3_YCOORDINATE]= SetDataElement(RVC_UPASYMBOLZONE3_YCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_UPASYMBOLZONE4_XCOORDINATE]= SetDataElement(RVC_UPASYMBOLZONE4_XCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_UPASYMBOLZONE4_YCOORDINATE]= SetDataElement(RVC_UPASYMBOLZONE4_YCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_LEFTRCTASYMBOL_XCOORDINATE]= SetDataElement(RVC_LEFTRCTASYMBOL_XCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_LEFTRCTASYMBOL_YCOORDINATE]= SetDataElement(RVC_LEFTRCTASYMBOL_YCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_RIGHTRCTASYMBOL_XCOORDINATE]= SetDataElement(RVC_RIGHTRCTASYMBOL_XCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_RIGHTRCTASYMBOL_YCOORDINATE]= SetDataElement(RVC_RIGHTRCTASYMBOL_YCOORDINATE,SinkHeadUnit,NUMERIC,"");
	m_ElementsMap[RVC_OVERLAY_TEXT_2_POSITION]= SetDataElement(RVC_OVERLAY_TEXT_2_POSITION,SinkHeadUnit,NUMERIC,"");
}

ScreenElement* RVCScreenManager::SetDataElement(unsigned int id, DisplaySink source, ScreenElementType type, string value)
{
	ScreenElement *screenElement = new ScreenElement;
	screenElement->m_Id= id;
	screenElement->m_Source= source;
	screenElement->m_Type= type;
	screenElement->m_Value= value;
	return screenElement;
}

void RVCScreenManager::OnTextLineChanged(bool visible,RVSText_t messageId){
	unsigned int qnId = 0;
	if(NULL != m_NotificationManager){
		switch(messageId){
		case RVS_TEXT_MSG_FAILED_SYNC:
			qnId = SC_RVC_SYNC_FAIL_QUICK_NOTICE;
			break;

		case RVS_TEXT_MSG_FAILED_TRANSMISSION:
			qnId = SC_RVC_TRANSMISSION_FAIL_QUICK_NOTICE;
			break;

		case RVS_TEXT_MSG_GUIDELINES_FAIL:
			qnId = SC_RVC_GUIDELINES_FAIL_QUICK_NOTICE;
			break;

		case RVS_TEXT_MSG_CALIB_STEERING:
			qnId = SC_RVC_STEERING_CALIBRATION_QUICK_NOTICE;
			break;

		case RVS_TEXT_MSG_PARK_ASST_FAIL:
			qnId = SC_RVC_PARK_ASSIST_FAIL_QUICK_NOTICE;
			break;
		default:
			LOGERR(("Invalid RVS Text %d", messageId));
			break;
		}

		LOG5((TEXT("OnTextLineChanged - visible=%d, messageId=%d, qnId=%d\n"), visible, messageId, qnId));
		if (0 != qnId){
			if (visible){
				m_precedenceManger->BringToForeground(this,SC_RVC_VIEW);
				m_NotificationManager->DisplayQuickNotice(this,qnId);
			}
			else{
				m_NotificationManager->RemoveQuickNotice(this,qnId);
				if (RVS_RVC_ACTIVE != m_LMObj->GetRVSState()){
					m_precedenceManger->RemoveFromTop(this);
				}
				else{}//do nothing...
			}
		}
	}
	else{
		//Do nothing
	}
}

void RVCScreenManager::setAudibleNotification()
{
	NotificationMapper quickNoticeMap;
	quickNoticeMap[SC_RVC_SYNC_FAIL_QUICK_NOTICE] = Notification(1669500928u,24,NTO_None);
	quickNoticeMap[SC_RVC_TRANSMISSION_FAIL_QUICK_NOTICE] = Notification(1669500928u,24,NTO_None);
	quickNoticeMap[SC_RVC_GUIDELINES_FAIL_QUICK_NOTICE] = Notification(1669500928u,24,NTO_None);
	quickNoticeMap[SC_RVC_STEERING_CALIBRATION_QUICK_NOTICE] = Notification(1669500928u,24,NTO_None);
	quickNoticeMap[SC_RVC_PARK_ASSIST_FAIL_QUICK_NOTICE] = Notification(1669500928u,24,NTO_None);
	m_NotificationManager->RegisterQuickNotice(this,quickNoticeMap);
}

void RVCScreenManager::SymbolState(bool symbolstate)
{
	m_rvsSymbolState = symbolstate;

}
