#ifndef WEB_SURFACE_PROXY_H
#define WEB_SURFACE_PROXY_H

//#include <vector>
//#include <map>
//#include "c_baseentity.h"
#include "c_prop_shortcut.h"
#include <KeyValues.h>
#include "proxyentity.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/IMaterialProxy.h"
//#include "imaterialproxydict.h"
//#include "c_websurfaceregen.h"

//#include "c_webtab.h"
//#include <map>

#include "c_embeddedinstance.h"
#include "c_canvasregen.h"

class CWebSurfaceProxy : public CEntityMaterialProxy
{
public:
	CWebSurfaceProxy();
	~CWebSurfaceProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind(C_BaseEntity *pC_BaseEntity = null);
	virtual void Release();

	void ReleaseStuff();

	void StaticLevelShutdownPostEntity();
	void StaticLevelShutdownPreEntity();
	void LevelShutdownPreEntity();
	void ReleaseCurrent();

	void PrepareRefreshItemTextures(std::string itemId, std::string channel);
	void RefreshItemTextures(std::string itemId, std::string channel);

	void UnreferenceEmbeddedInstance(C_EmbeddedInstance* pEmbeddedInstance);
	void UnreferenceTexture(ITexture* pTexture);

	// mutators
	//void SetId(std::string id) { m_id = id; }

	// accessors
	IMaterial *GetMaterial() { return m_pMaterial; }
	static CCanvasRegen* GetTextureRegenerator() { return s_pCanvasRegen; };
	static void OnSimpleImageRendered(std::string channel, std::string itemId, std::string field, ITexture* pTexture);

private:
	int				m_iState;
	//std::string		m_id;
	C_EmbeddedInstance*		m_pCurrentEmbeddedInstance;
	C_EmbeddedInstance*		m_pEmbeddedInstance;
	IMaterial*		m_pMaterial;
	IMaterialVar*	m_pMaterialTextureVar;
	IMaterialVar*	m_pMaterialDetailBlendFactorVar;
	ITexture*		m_pOriginalTexture;
	ITexture*		m_pCurrentTexture;	// NOT ACTUALLY USED!!!! OBSOLETE!!
	std::string		m_originalId;
	int				m_iOriginalAutoCreate;
	std::string		m_originalUrl;

	std::string		m_originalSimpleImageChannel;
	//static std::map<std::string, std::map<std::string, ITexture*>> s_simpleImages;	// a map of shortcut to
	//static CUtlMap<std::string, CUtlMap<std::string, ITexture*>> s_simpleImages;
	static CUtlMap<std::string, ITexture*> s_screenSimpleImages;
	static CUtlMap<std::string, ITexture*> s_marqueeSimpleImages;
	static bool s_bLessThanReady;

//	static int				s_textureCount;	// probably needed for cleanup??
	static CCanvasRegen* s_pCanvasRegen;

	//ITexture* CreateTexture(C_BaseEntity* pEntity);
};

EXPOSE_INTERFACE(CWebSurfaceProxy, IMaterialProxy, "websurface" IMATERIAL_PROXY_INTERFACE_VERSION);

#endif //WEB_SURFACE_PROXY_H