/*#############################################################################
 *  (C) 2014  Fujitsu Ten Limited. All Rights Reserved.
 * =============================================================================
 *  HARDWARE         : INFO3_LOW
 *  MODULE           : RVC
 *  VERSION          : v0.0.1
 *  FILE NAME        : RVCScreenManager.h
 *  FILE DESCRIPTION : Manages all the screens for RVC.
 *  NOTE				: N/A
 *############################################################################*/

#ifndef inc_RVCScreenManager_H
#define inc_RVCScreenManager_H

#include "RVCGlobalTypes.h"
#include "IPrecedenceManager.h"

#include "HMIFTypes.h"
#include "IApplication.h"
#include "IHMIF.h"

#include "Utility.h"
#include <map>
#include "ITestableApp.h"
#include "IEventHandler.h"
#include "IScreenProvider.h"
#include "INotificationManager.h"
#include "IUpdatableView.h"
#include "IEventConsumer.h"

#include "logger.h"
#include "ILMServer.h"
#include "AppDetails.h"
#include "IEventHandler.h"
#include "IRVCScreenManager.h"
#include "IRVCLogicManager.h"
#include "HMIFEvents.h"

using namespace i3l::rvs;

class RVCScreenManager : public IApplication, public IRVCScreenManager {

public:

	RVCScreenManager(DisplayType viewType, IRVCLogicManager * rvcLogicManager);	 //Constructor

	virtual ~RVCScreenManager();	//virtual destructor

	//IApplication
	void Init(IHMIF * hmif);

	void Activate(const bool &isActive);

	const AppDetails * GetAppInfo();

	unsigned int GetId();

	const ScreenElementsList & GetScreenDataId(unsigned int screenName);

	const ScreenElementsMap & GetScreens();

	void HandleEvent(unsigned int eventId, const std::string & eventArgs);

	void Start();

	void Pause();

	void Resume();

	bool Stop();

	bool OnShuttingDown();

	void OnSuspend(lfc::lfc_reason_t reason);

	void OnResume(lfc::lfc_reason_t reason);

	void OnHMIReady();

	void OnRVSStateChanged(RVSState_t rvsState_t);

	void OnTextLineChanged(bool visible,RVSText_t messageId);

	void SymbolState(bool symbolstate);

private:

	void RegisterScreens();
	ScreenElement* SetDataElement(unsigned int id, DisplaySink source, ScreenElementType type, string value);
	void SetDataElements();
	void setAudibleNotification();
private:
	IPrecedenceManager* m_precedenceManger; // Precedence
	ILMServer* m_LayerManager;
	bool m_AmIActive;
	int m_ClientId;
	IRVCLogicManager* m_LMObj;
	AppDetails m_AppDetails;
	map<unsigned int, ScreenElement*> m_ElementsMap;
	ScreenElementsMap m_ScreenElementMap;
	PolicyMapper m_ScreenPolicyMap;
	INotificationManager* m_NotificationManager;
	RVSState_t m_rvsState;
	IEventConsumer* m_pEventConsumer;
	bool m_rvsSymbolState;

};


#endif
