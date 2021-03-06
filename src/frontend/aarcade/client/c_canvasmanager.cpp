#include "cbase.h"

#include "aa_globals.h"
#include "c_canvasmanager.h"
#include "c_anarchymanager.h"
#include "../../public/vtf/vtf.h"
#include "../public/bitmap/tgawriter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_CanvasManager::C_CanvasManager()
{
	DevMsg("CanvasManager: Constructor\n");
	//m_pRegen = null;

	//m_iCanvasWidth = 1280;
	//m_iCanvasHeight = 720;
	//m_iCanvasWidth = 1920;
	//m_iCanvasHeight = 1080;
	m_bUseDeferredTextureCleanUp = true;
	m_iLastAllowedRenderedFrame = -1;
	m_iLastAllowedPriorityRenderedFrame = -1;
	m_iVisibleCanvasesLastFrame = -1;
	m_iVisiblePriorityCanvasesLastFrame = -1;
	m_iVisibleCanvasesCurrentFrame = 0;
	m_iVisiblePriorityCanvasesCurrentFrame = 0;
	m_iLastRenderedFrame = -1;
	m_iLastPriorityRenderedFrame = -1;
	//m_pWebBrowser = null;
	m_pCanvasRegen = null;
	m_pDisplayInstance = null;
	//m_pSelectedEmbeddedInstance = null;

	/*
	m_iWebSurfaceWidth = 1280;
	m_iWebSurfaceHeight = 720;
	//m_iWebSurfaceWidth = 1920;
	//m_iWebSurfaceHeight = 1080;
	m_bHudPriority = true;// false;
	m_bSelectedPriority = true;// false;
	m_iVisibleWebTabsLastFrame = -1;
	m_iVisiblePriorityWebTabsLastFrame = -1;
	m_iVisibleWebTabsCurrentFrame = 0;
	m_iVisiblePriorityWebTabsCurrentFrame = 0;
	m_iLastRenderedFrame = -1;
	m_iLastPriorityRenderedFrame = -1;
	m_pWebBrowser = null;
	m_pWebSurfaceRegen = null;
	m_pSelectedWebTab = null;
	m_pFocusedWebTab = null;
	m_pHudWebTab = null;
	m_bHudReady = false;
	*/

	//g_pAnarchyManager->SetState(AASTATE_LIBRETROMANAGER);
}

C_CanvasManager::~C_CanvasManager()
{
	DevMsg("CanvasManager: Destructor\n");
}

void C_CanvasManager::Update()
{
	// Update the visible web tab estimates
	m_iVisibleCanvasesLastFrame = m_iVisibleCanvasesCurrentFrame;
	m_iVisibleCanvasesCurrentFrame = 0;

	m_iVisiblePriorityCanvasesLastFrame = m_iVisiblePriorityCanvasesCurrentFrame;
	m_iVisiblePriorityCanvasesCurrentFrame = 0;

	if (!engine->IsInGame())	// If we are in-game, this is done during PostRender instead.
		this->CleanupTextures();
}

CCanvasRegen* C_CanvasManager::GetOrCreateRegen()
{
	if (!m_pCanvasRegen)
	{
		m_pCanvasRegen = new CCanvasRegen;
		DevMsg("Creating NEW regen...\n");
	}

	return m_pCanvasRegen;
}

DeferredCleanupTexture_t* C_CanvasManager::FindDeferredCleanupTexture(ITexture* pTexture)
{
	unsigned int max = m_pendingTextureCleanup.size();
	for (unsigned int i = 0; i < max; i++)
	{
		if (m_pendingTextureCleanup[i]->pTexture == pTexture)
			return m_pendingTextureCleanup[i];
	}

	return null;

	/*
	auto it = m_pendingTextureCleanup.find(std::string(pTexture->GetName()));
	if (it != m_pendingTextureCleanup.end())
		return it->second;
	
	return null;
	*/
}
/*
DeferredCleanupTexture_t* C_CanvasManager::FindDeferredCleanupTexture(std::string textureName)
{
	auto it = m_pendingTextureCleanup.find(textureName);
	if (it != m_pendingTextureCleanup.end())
		return it->second;

	return null;
}
*/

void C_CanvasManager::DoOrDeferTextureCleanup(ITexture* pTexture)
{
	if (m_bUseDeferredTextureCleanUp)
	{
		DeferredCleanupTexture_t* pDeferredCleanupTexture = this->FindDeferredCleanupTexture(pTexture);
		if (pDeferredCleanupTexture)
		{
			DevMsg("WARNING: Texture attempted to add itself twice to the deferred texture cleanup list: %s\n", pTexture->GetName());

			if (pDeferredCleanupTexture->pTexture != pTexture)
				DevMsg("CRITICAL ERROR: The previous texture for this texture name does not match this pointer!\n");

			// update the death time
			pDeferredCleanupTexture->startTime = g_pAnarchyManager->GetTimeNumber();
		}
		else
		{
			pDeferredCleanupTexture = new DeferredCleanupTexture_t();
			pDeferredCleanupTexture->pTexture = pTexture;
			pDeferredCleanupTexture->startTime = g_pAnarchyManager->GetTimeNumber();
			m_pendingTextureCleanup.push_back(pDeferredCleanupTexture);
			//m_pendingTextureCleanup[std::string(pTexture->GetName())] = pDeferredCleanupTexture;
		}
	}
	else
		this->CleanupTexture(pTexture);
}

void C_CanvasManager::TextureNotDeferred(ITexture* pTexture)
{
	unsigned int max = m_pendingTextureCleanup.size();
	for (unsigned int i = 0; i < max; i++)
	{
		if (m_pendingTextureCleanup[i]->pTexture == pTexture)
		{
			m_pendingTextureCleanup.erase(m_pendingTextureCleanup.begin() + i);
			break;
		}
	}
	//auto it = m_pendingTextureCleanup.find(std::string(pTexture->GetName()));
	//if (it != m_pendingTextureCleanup.end())
	//	m_pendingTextureCleanup.erase(it);
}

void C_CanvasManager::CleanupTexture(ITexture* pTexture)
{
	DevMsg("Cleaning up texture:\n");
	DevMsg("\tTexturePionter: %i\n", pTexture);
	DevMsg("\tIsErrorTexture: %i\n", pTexture->IsError());	

	std::string texName = pTexture->GetName();
	DevMsg("\tName: %s\n", texName);

	pTexture->DecrementReferenceCount();
	pTexture->DeleteIfUnreferenced();
}

void C_CanvasManager::CleanupTextures()
{
	if (!m_bUseDeferredTextureCleanUp)
		return;

	uint64 delay = 3;
	uint64 difference;
	uint64 currentTime = g_pAnarchyManager->GetTimeNumber();
	DeferredCleanupTexture_t* pDeferredCleanupTexture;
	ITexture* pTexture;

	unsigned int max = m_pendingTextureCleanup.size();
	for (unsigned int i = max; i > 0; i--)
	{
		pDeferredCleanupTexture = m_pendingTextureCleanup[i-1];
		difference = currentTime - pDeferredCleanupTexture->startTime;
		if (difference > delay)
		{
			pTexture = pDeferredCleanupTexture->pTexture;
			this->CleanupTexture(pTexture);
			m_pendingTextureCleanup.erase(m_pendingTextureCleanup.begin() + (i - 1));
		}
	}
}

bool C_CanvasManager::IsPriorityEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance)
{
	if (pEmbeddedInstance->GetId() == "images" || pEmbeddedInstance->GetId() == "network")
		return false;

	//if (pEmbeddedInstance == (C_EmbeddedInstance*)g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud") || pEmbeddedInstance == g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance())
	if (pEmbeddedInstance->GetId() == "hud" && g_pAnarchyManager->GetInputManager()->GetInputMode())
		return true;

	///*
	if (pEmbeddedInstance == g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() && pEmbeddedInstance->GetId() != "hud" && (g_pAnarchyManager->GetSelectedEntity() || g_pAnarchyManager->GetInputManager()->GetFullscreenMode()))
		return true;
	//*/
	
	return false;
}

/*
bool C_CanvasManager::ShouldRender(C_EmbeddedInstance* pEmbeddedInstance)
{
	// don't render more than 1 web tab per frame
	if (m_iLastRenderedFrame == gpGlobals->framecount || m_iLastPriorityRenderedFrame == gpGlobals->framecount)
		return false;

	bool bIsPriorityEmbeddedInstance = this->IsPriorityEmbeddedInstance(pEmbeddedInstance);
	if (bIsPriorityEmbeddedInstance)
	{
		int iLastRenderedFrame = pEmbeddedInstance->GetLastRenderedFrame();

		// always render the 1st time
		if (iLastRenderedFrame <= 0)
			return true;

		// render if we're the only visible (priority) web tab
		//	if (m_iVisiblePriorityWebTabsLastFrame <= 1)
		//		return true;

		// render if we've waited long enough for all other (priority) web views to render

		// Need to wait an extra tick if there are any non-priority web tabs waiting.
		int iExtraOne = 0;
		if (m_iVisibleCanvasesLastFrame > 0)
			iExtraOne = 1;

		if (gpGlobals->framecount - iLastRenderedFrame >= m_iVisiblePriorityCanvasesLastFrame + iExtraOne)
			return true;
	}
	else
	{
		//DevMsg("num priority web views: %i\n", m_iVisiblePriorityWebTabsLastFrame);
		int iLastRenderedFrame = pEmbeddedInstance->GetLastRenderedFrame();

		//	int frameCount = m_iLastPriorityRenderedFrame;// gpGlobals->framecount - m_iLastPriorityRenderedFrame;
		// we are a regular web tab

		// don't render if there are still priority web tabs waiting to render
		//	if (m_iVisiblePriorityWebTabsLastFrame > 0 && frameCount - m_iLastPriorityRenderedFrame >= m_iVisiblePriorityWebTabsLastFrame)
		//	return false;

		// always render the 1st time
		if (iLastRenderedFrame <= 0)
			return true;

		// render if we're the only visible (regular) web tab
		//	if (m_iVisibleWebTabsLastFrame <= 1)
		//		return true;

		// render if we've waited long enough for all other (regular) web views to render
		//if (frameCount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame)

		// Need to wait an extra tick if there are any priority web tabs waiting.
		//int iExtraOne = 0;
		bool bExtraOne = false;
		if (m_iVisiblePriorityCanvasesLastFrame > 0)
			bExtraOne = true;
		//iExtraOne = 1;

		//if (bExtraOne && m_iLastPriorityRenderedFrame - iLastRenderedFrame >= m_iVisiblePriorityCanvasesLastFrame + m_iVisibleCanvasesLastFrame - 1)
		if (bExtraOne && m_iLastPriorityRenderedFrame - iLastRenderedFrame >= 1 + m_iVisibleCanvasesLastFrame - 1)
			return true;
		else if (!bExtraOne && gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleCanvasesLastFrame - 1)
			return true;
	}

	//	if (m_iLastRenderedFrame != gpGlobals->framecount && (iLastRenderedFrame <= 0 || m_iVisibleWebTabsLastFrame <= 1 || ((pWebTab == m_pHudWebTab || pWebTab == m_pSelectedWebTab) && m_iVisiblePriorityWebTabsLastFrame <= 1) || (((pWebTab == m_pHudWebTab || pWebTab == m_pSelectedWebTab) && m_iVisiblePriorityWebTabsLastFrame <= 1) || gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame)))
	//		return true;

	return false;
}
*/

bool C_CanvasManager::ShouldRender(C_EmbeddedInstance* pEmbeddedInstance, bool bPreTest)
{
	bool bIsPriorityEmbeddedInstance = this->IsPriorityEmbeddedInstance(pEmbeddedInstance);

//	DevMsg("Rendering: %i %s %s\n", bIsPriorityEmbeddedInstance, pEmbeddedInstance->GetId().c_str(), pEmbeddedInstance->GetTexture()->GetName());

	// don't render more than 1 web tab per frame
//	if (m_iLastRenderedFrame == gpGlobals->framecount || m_iLastPriorityRenderedFrame == gpGlobals->framecount)
//		return false;

	if (m_iLastPriorityRenderedFrame == gpGlobals->framecount)
		return false;
	else if (m_iLastRenderedFrame == gpGlobals->framecount)
	{
//		if (bIsPriorityEmbeddedInstance)
//		{
//			if (pEmbeddedInstance->GetId() == "hud")
//				DevMsg("WARNING: HUD webview aborted due to non-priority web view stealing its spot in the render queue.\n");
//			else
//				DevMsg("WARNING: Priority webview aborted due to non-priority web view stealing its spot in the render queue.\n");
//		}

		return false;
	}

	// if this is NOT a priority web view, and the last PRIORITY web view was NOT render in the PREVIOUS frame, and there IS a priority web view THIS frame, BORT

	// FIXME: m_iLastPriorityRenderedFrame should be last time the hud was ALLOWED to render, not the last time it ACTUALLY rendered.
	/*
	if (!bIsPriorityEmbeddedInstance && m_iLastPriorityRenderedFrame < gpGlobals->framecount - 1 && m_iVisiblePriorityCanvasesLastFrame > 0)
	{
		DevMsg("Abort A: %s %s\n", pEmbeddedInstance->GetId().c_str(), pEmbeddedInstance->GetTexture()->GetName());
		return false;
	}
	*/

//	/*
	
//	DevMsg("INFOS: %i %i\n", m_iVisibleCanvasesLastFrame, m_iVisiblePriorityCanvasesLastFrame);

	if (bIsPriorityEmbeddedInstance)
	{
		int iLastRenderedFrame = pEmbeddedInstance->GetLastRenderedFrame();

		// always render the 1st time
		if (m_iLastPriorityRenderedFrame <= 0)
		{
			if (!bPreTest)
				m_iLastAllowedPriorityRenderedFrame = gpGlobals->framecount;
//			if (m_iVisibleCanvasesLastFrame > 0 && m_iLastAllowedRenderedFrame < gpGlobals->framecount - 1)
//				return false;
//			else
				return true;
		}

		// Need to wait an extra tick if there are any non-priority web tabs waiting.
		int factor = 1;
		if (m_iVisibleCanvasesLastFrame > 0)
		{
			factor = 2;
	//		DevMsg("Extra one: %i\n", iExtraOne);
		}

//		if (gpGlobals->framecount - m_iLastPriorityRenderedFrame >= m_iVisiblePriorityCanvasesLastFrame + iExtraOne)
		if (gpGlobals->framecount - iLastRenderedFrame >= (m_iVisiblePriorityCanvasesLastFrame * factor) - 1)
		{
			if (!bPreTest)
				m_iLastAllowedPriorityRenderedFrame = gpGlobals->framecount;
		//	if (m_iVisibleCanvasesLastFrame > 0 && m_iLastAllowedRenderedFrame < gpGlobals->framecount - 1)
	//			return false;
//			else
				return true;
		}

		/*
		// render if we're the only visible (priority) web tab
		//	if (m_iVisiblePriorityWebTabsLastFrame <= 1)
		//		return true;

		// render if we've waited long enough for all other (priority) web views to render

		// Need to wait an extra tick if there are any non-priority web tabs waiting.
		int iExtraOne = 0;
		if (m_iVisibleCanvasesLastFrame > 0)
			iExtraOne = 1;

		int iFactor = 1;
		if (m_iVisiblePriorityCanvasesLastFrame > 0)
			iFactor = m_iVisiblePriorityCanvasesLastFrame;

		if (gpGlobals->framecount - m_iLastRenderedFrame >= m_iVisiblePriorityCanvasesLastFrame + (iExtraOne * iFactor))
			return true;
		*/

	//	DevMsg("Abort B: %s %s out of %i\n", pEmbeddedInstance->GetId().c_str(), pEmbeddedInstance->GetTexture()->GetName(), m_iVisiblePriorityCanvasesLastFrame);
		//return true;
	}
	else
	{
//	*/
		//DevMsg("num priority web views: %i\n", m_iVisiblePriorityWebTabsLastFrame);
		int iLastRenderedFrame = pEmbeddedInstance->GetLastRenderedFrame();

		//	int frameCount = m_iLastPriorityRenderedFrame;// gpGlobals->framecount - m_iLastPriorityRenderedFrame;
		// we are a regular web tab

		// don't render if there are still priority web tabs waiting to render
		//	if (m_iVisiblePriorityWebTabsLastFrame > 0 && frameCount - m_iLastPriorityRenderedFrame >= m_iVisiblePriorityWebTabsLastFrame)
		//	return false;

		// always render the 1st time
		if (m_iLastRenderedFrame <= 0)
		{
			if (!bPreTest)
				m_iLastAllowedRenderedFrame = gpGlobals->framecount;

			if (m_iVisiblePriorityCanvasesLastFrame > 0 && m_iLastAllowedPriorityRenderedFrame < gpGlobals->framecount - 1)
				return false;
			else
				return true;
		}

		// render if we're the only visible (regular) web tab
		//	if (m_iVisibleWebTabsLastFrame <= 1)
		//		return true;

		// render if we've waited long enough for all other (regular) web views to render
		//if (frameCount - m_iLastRenderedFrame >= m_iVisibleWebTabsLastFrame)
		/*
		if (m_iVisiblePriorityCanvasesLastFrame > 0 && m_iLastPriorityRenderedFrame - m_iLastRenderedFrame >= m_iVisibleCanvasesLastFrame - 1)// + this->GetNumPriorityEmbeddedInstances())//1)
			return true;
		else if (m_iVisiblePriorityCanvasesLastFrame <= 0 && gpGlobals->framecount - m_iLastRenderedFrame >= m_iVisibleCanvasesLastFrame)
			return true;
		*/

		if (m_iVisiblePriorityCanvasesLastFrame > 0 && m_iLastAllowedPriorityRenderedFrame < gpGlobals->framecount - 1)//m_iLastAllowedPriorityRenderedFrame != gpGlobals->framecount - 1)
		{
			if (!bPreTest)
				m_iLastAllowedRenderedFrame = gpGlobals->framecount;
			return false;
		}


		int factor = 1;
		if (m_iVisiblePriorityCanvasesLastFrame > 0)
			factor = 2;// m_iVisiblePriorityCanvasesLastFrame;

		/*
		if (m_iVisiblePriorityCanvasesLastFrame > 0 && m_iLastPriorityRenderedFrame - iLastRenderedFrame >= m_iVisiblePriorityCanvasesLastFrame + m_iVisibleCanvasesLastFrame - 1) //iExtraOne + m_iVisibleCanvasesLastFrame - 1)
			return true;
		else 
		*/




		//if (gpGlobals->framecount - iLastRenderedFrame >= (m_iVisibleCanvasesLastFrame * factor) - 1)
		if (gpGlobals->framecount - iLastRenderedFrame >= (m_iVisibleCanvasesLastFrame * factor) - 1)
		{
			if (!bPreTest)
				m_iLastAllowedRenderedFrame = gpGlobals->framecount;
			return true;
		}





		/*
		if (m_iVisiblePriorityCanvasesLastFrame > 0 && m_iLastPriorityRenderedFrame - iLastRenderedFrame >= m_iVisiblePriorityCanvasesLastFrame + m_iVisibleCanvasesLastFrame - 1) //iExtraOne + m_iVisibleCanvasesLastFrame - 1)
			return true;
		else if (m_iVisiblePriorityCanvasesLastFrame <= 0 && gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleCanvasesLastFrame)
			return true;
		*/
	}

	//	if (m_iLastRenderedFrame != gpGlobals->framecount && (m_iLastRenderedFrame <= 0 || m_iVisibleWebTabsLastFrame <= 1 || ((pWebTab == m_pHudWebTab || pWebTab == m_pSelectedWebTab) && m_iVisiblePriorityWebTabsLastFrame <= 1) || (((pWebTab == m_pHudWebTab || pWebTab == m_pSelectedWebTab) && m_iVisiblePriorityWebTabsLastFrame <= 1) || gpGlobals->framecount - iLastRenderedFrame >= m_iVisibleWebTabsLastFrame)))
	//		return true;

	//DevMsg("Abort C: %s %s\n", pEmbeddedInstance->GetId().c_str(), pEmbeddedInstance->GetTexture()->GetName());
	return false;
}

void C_CanvasManager::RegisterProxy(CWebSurfaceProxy* pProxy)
{
	m_webSurfaceProxies.push_back(pProxy);
}

void C_CanvasManager::SetMaterialEmbeddedInstanceId(IMaterial* pMaterial, std::string id)
{
	m_materialEmbeddedInstanceIds[pMaterial] = id;
}

C_EmbeddedInstance* C_CanvasManager::FindEmbeddedInstance(IMaterial* pMaterial)
{
	C_EmbeddedInstance* pEmbeddedInstance = null;
	auto foundId = m_materialEmbeddedInstanceIds.find(pMaterial);
	if (foundId != m_materialEmbeddedInstanceIds.end())
	{
		std::string id = foundId->second;

		pEmbeddedInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance(id);
		if (!pEmbeddedInstance)
			pEmbeddedInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(id);

		if (!pEmbeddedInstance)
			pEmbeddedInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(id);

		if (pEmbeddedInstance)
			return pEmbeddedInstance;
	}

	return null;
}

C_EmbeddedInstance* C_CanvasManager::FindEmbeddedInstance(std::string id)
{
	C_EmbeddedInstance* pEmbeddedInstance = null;

	auto foundInstance = m_materialEmbeddedInstanceIds.begin();
	while (foundInstance != m_materialEmbeddedInstanceIds.end())
	{
		pEmbeddedInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance(id);
		if (!pEmbeddedInstance)
			pEmbeddedInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(id);

		if (!pEmbeddedInstance)
			pEmbeddedInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(id);

		if (pEmbeddedInstance)
			return pEmbeddedInstance;
		else
			foundInstance++;
	}

	return null;
}

void C_CanvasManager::CloseAllInstances()
{
	// all instances other than HUD and IMAGES should be removed at this time (until cross-map background item play gets re-enabled.)
	DevMsg("Closing instances...\n");
	g_pAnarchyManager->GetAwesomiumBrowserManager()->CloseAllInstances();	// but not hud and images
	DevMsg("Awesomium Browser instances closed.\n");
	g_pAnarchyManager->GetSteamBrowserManager()->CloseAllInstances();
	DevMsg("Steam Browser instances closed.\n");
	g_pAnarchyManager->GetLibretroManager()->CloseAllInstances();
	DevMsg("Librertro instances closed.\n");
}

void C_CanvasManager::CloseInstance(std::string id)
{
	C_EmbeddedInstance* pEmbeddedInstance = null;

	auto foundInstance = m_materialEmbeddedInstanceIds.begin();
	while (foundInstance != m_materialEmbeddedInstanceIds.end())
	{
		pEmbeddedInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance(id);
		if (pEmbeddedInstance)
		{
			g_pAnarchyManager->GetAwesomiumBrowserManager()->DestroyAwesomiumBrowserInstance(dynamic_cast<C_AwesomiumBrowserInstance*>(pEmbeddedInstance));
			return;
		}

		pEmbeddedInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(id);
		if (pEmbeddedInstance)
		{
			g_pAnarchyManager->GetLibretroManager()->DestroyLibretroInstance(dynamic_cast<C_LibretroInstance*>(pEmbeddedInstance));
			return;
		}

		pEmbeddedInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstance(id);
		if (pEmbeddedInstance)
		{
			g_pAnarchyManager->GetSteamBrowserManager()->DestroySteamBrowserInstance(dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance));
			return;
		}

		pEmbeddedInstance = g_pAnarchyManager->GetWindowManager()->FindWindowInstance(id);
		if (pEmbeddedInstance)
		{
			g_pAnarchyManager->GetWindowManager()->CloseWindowsWindow(id);	// FIXME: should use an overload that accepts the instance pointer directly
			g_pAnarchyManager->GetWindowManager()->DestroyWindowInstance(dynamic_cast<C_WindowInstance*>(pEmbeddedInstance));
			return;
		}

		foundInstance++;
	}

	DevMsg("Warning: Could not find instance w/ ID %s to close!\n", id.c_str());
}

void C_CanvasManager::SwitchToInstance(std::string id)
{
	C_EmbeddedInstance* pEmbeddedInstance = null;

	auto foundInstance = m_materialEmbeddedInstanceIds.begin();
	while (foundInstance != m_materialEmbeddedInstanceIds.end())
	{
		pEmbeddedInstance = g_pAnarchyManager->GetWindowManager()->FindWindowInstance(id);
		if (pEmbeddedInstance)
		{
			g_pAnarchyManager->GetWindowManager()->SwitchToWindowInstance(id);	// FIXME: should use an overload that accepts the instance pointer directly
			return;
		}

		foundInstance++;
	}

	DevMsg("Warning: Could not find instance w/ ID %s to switch to!\n", id.c_str());
}

void C_CanvasManager::GetAllInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances)
{
	//g_pAnarchyManager->GetAwesomiumBrowserManager()->GetAllInstances(embeddedInstances);
	g_pAnarchyManager->GetSteamBrowserManager()->GetAllInstances(embeddedInstances);
	g_pAnarchyManager->GetLibretroManager()->GetAllInstances(embeddedInstances);
}

void C_CanvasManager::CaptureInstanceThumbnail(C_EmbeddedInstance* pEmbeddedInstance)
{
	ITexture* pTexture = pEmbeddedInstance->GetTexture();
	if (pTexture)
	{
		// aarcade_user/cache/tasks

		// TODO: Save a TGA or some sort of image out!

		/*
		g_pFullFileSystem->CreateDirHierarchy("cache/tasks", "DEFAULT_WRITE_PATH");

		std::string filename = g_pAnarchyManager->GetAArcadeUserFolder();
		filename += "\\cache\\tasks\\";
		filename += pEmbeddedInstance->GetId();
		filename += ".tga";
		*/

		//DevMsg("Saving task thumbnail %s\n", filename.c_str());

		/*
		// Write TGA format to buffer
		int textureWidth = pTexture->GetActualWidth();
		int textureHeight = pTexture->GetActualHeight();
		int textureDpeth = pTexture->GetActualDepth();

		int iMaxTGASize = 1024 + (textureWidth * textureHeight * 4);
		void *pTGA = malloc(iMaxTGASize);

		CUtlBuffer buffer;
		buffer.SetExternalBuffer(pTGA, iMaxTGASize, 0);
		*/

		//pTexture->SaveToFile(filename.c_str());

		
		//pTexture->ImageData

		//size_t byteSize;
		//void* result = pTexture->GetResourceData(0, &byteSize);
		//DevMsg("Done: %i\n", (result != null));
		//bool bWriteResult = TGAWriter::WriteToBuffer(, buffer, nSrcWidth, nSrcHeight, IMAGE_FORMAT_RGB888, IMAGE_FORMAT_RGB888);
		//pTexture->SaveToFile(filename.c_str());
	}
}

void C_CanvasManager::LevelShutdownPreEntity()
{
	// all instances other than HUD and IMAGES should be removed at this time (until cross-map background item play gets re-enabled.)
	this->CloseAllInstances();
	//g_pAnarchyManager->GetSteamBrowserManager()->CloseAllInstances();
	//g_pAnarchyManager->GetAwesomiumBrowserManager()->CloseAllInstances();	// but not hud and images
	//g_pAnarchyManager->GetLibretroManager()->CloseAllInstances();

	// empty out the static stuff
	unsigned int max = m_webSurfaceProxies.size();

	// revert all current textures to their original
	for (unsigned int i = 0; i < max; i++)
		m_webSurfaceProxies[i]->ReleaseCurrent();

	/*
	// call the static cleanup method
	for (unsigned int i = 0; i < max; i++)
	{
		m_webSurfaceProxies[i]->StaticLevelShutdownPostEntity();
		break;
	}
	*/
}

void C_CanvasManager::LevelShutdownPostEntity()
{
	// empty out the static stuff
	unsigned int max = m_webSurfaceProxies.size();

	for (unsigned int i = 0; i < max; i++)
	{
		//DevMsg("Yarrrr: %s\n", m_webSurfaceProxies[i]->GetMaterial()->GetName());
		m_webSurfaceProxies[i]->ReleaseStuff();
		if (i == max - 1)
		{
			// call the static cleanup method
			m_webSurfaceProxies[i]->StaticLevelShutdownPostEntity();
		}
	}
}

void C_CanvasManager::UnreferenceTexture(ITexture* pTexture)
{
	// tell ALL proxies that this texture is on its way out and should NOT be referenced anywhere (mostly the issue is when its still set as the texturevar of the proxy's material.)
	unsigned int max = m_webSurfaceProxies.size();
	for (unsigned int i = 0; i < max; i++)
	{
		m_webSurfaceProxies[i]->UnreferenceTexture(pTexture);
	}
}

void C_CanvasManager::UnreferenceEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance)
{
	this->GetOrCreateRegen()->NotifyInstanceAboutToDie(pEmbeddedInstance);

	// tell ALL proxies that this INSTANCE is on its way out and should NOT be referenced anywhere
	unsigned int max = m_webSurfaceProxies.size();
	for (unsigned int i = 0; i < max; i++)
	{
		m_webSurfaceProxies[i]->UnreferenceEmbeddedInstance(pEmbeddedInstance);
	}
}

void C_CanvasManager::SetDifferentDisplayInstance(C_EmbeddedInstance* pEmbeddedInstance)
{
	m_pDisplayInstance = null;

	// get all the tasks
	std::vector<C_EmbeddedInstance*> embeddedInstances;
	this->GetAllInstances(embeddedInstances);

	// cycle through them all and find the 1st non-hud non-images one
	std::string testId;
	for (unsigned int i = 0; i < embeddedInstances.size(); i++)
	{
		testId = embeddedInstances[i]->GetId();
		if (testId == "hud" || testId == "images" || testId == "network" || embeddedInstances[i] == pEmbeddedInstance)
			continue;

		m_pDisplayInstance = embeddedInstances[i];
		break;
	}
}

C_EmbeddedInstance* C_CanvasManager::GetFirstInstanceToDisplay()
{
	// get all the tasks
	std::vector<C_EmbeddedInstance*> embeddedInstances;
	this->GetAllInstances(embeddedInstances);

	// cycle through them all and find the 1st non-hud non-images one
	std::string testId;
	for (unsigned int i = 0; i < embeddedInstances.size(); i++)
	{
		testId = embeddedInstances[i]->GetId();
		if (testId == "hud" || testId == "images" || testId == "network")
			continue;

		return embeddedInstances[i];
	}

	return null;
}

void C_CanvasManager::RefreshItemTextures(std::string itemId, std::string channel)
{
	// tell ALL proxies that the textures for this item need to be re-loaded
	unsigned int max = m_webSurfaceProxies.size();
	for (unsigned int i = 0; i < max; i++)
	{
		m_webSurfaceProxies[i]->PrepareRefreshItemTextures(itemId, channel);
	}

	for (unsigned int i = 0; i < max; i++)
	{
		m_webSurfaceProxies[i]->RefreshItemTextures(itemId, channel);
		break;	// its a static member that will get modified.
	}
}