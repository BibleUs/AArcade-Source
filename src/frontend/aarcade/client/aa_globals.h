#ifndef AA_MAX_STRING
#define AA_MAX_STRING 1024
#endif

#ifndef AA_LIBRETRO_3D
#define AA_LIBRETRO_3D false
#endif

#ifndef AA_EMBEDDED_INSTANCE_WIDTH
#define AA_EMBEDDED_INSTANCE_WIDTH 1280
#endif

#ifndef AA_EMBEDDED_INSTANCE_HEIGHT
#define AA_EMBEDDED_INSTANCE_HEIGHT 720
#endif

#ifndef AA_HUD_INSTANCE_WIDTH
#define AA_HUD_INSTANCE_WIDTH 1920
#endif

#ifndef AA_HUD_INSTANCE_HEIGHT
#define AA_HUD_INSTANCE_HEIGHT 1080
#endif

#ifndef AA_NETWORK_INSTANCE_WIDTH
#define AA_NETWORK_INSTANCE_WIDTH 32
#endif

#ifndef AA_NETWORK_INSTANCE_HEIGHT
#define AA_NETWORK_INSTANCE_HEIGHT 32
#endif

#ifndef AA_MASTER_INSTANCE_WIDTH
#define AA_MASTER_INSTANCE_WIDTH 32
#endif

#ifndef AA_MASTER_INSTANCE_HEIGHT
#define AA_MASTER_INSTANCE_HEIGHT 32
#endif

#ifndef AA_THUMBNAIL_SIZE
#define AA_THUMBNAIL_SIZE 512
#endif

#ifndef AA_PLATFORM_ID
#define AA_PLATFORM_ID "-KJvcne3IKMZQTaG7lPo"
#endif

#ifndef AA_DEFAULT_TYPEID
#define AA_DEFAULT_TYPEID ""//"-KKa1MHJTls2KqNphWFM"
#endif

#ifndef AA_LIBRARY_VERSION
#define AA_LIBRARY_VERSION 1
#endif

#ifndef AA_IMPORT_INFO
#define AA_IMPORT_INFO
#include <string>
#include <vector>
enum aaImportType
{
	AAIMPORT_NONE = 0,
	AAIMPORT_MODELS = 1
};

enum aaImportStatus
{
	AAIMPORTSTATUS_NONE = 0,
	AAIMPORTSTATUS_WAITING_TO_START = 1,
	AAIMPORTSTATUS_WORKING = 2,
	AAIMPORTSTATUS_COMPLETE = 3,
	AAIMPORTSTATUS_ABORTED = 4,
	AAIMPORTSTATUS_WAITING_FOR_PROCESSING = 5,
	AAIMPORTSTATUS_PROCESSING = 6,
	AAIMPORTSTATUS_WAITING_FOR_ADDING = 7,
	AAIMPORTSTATUS_ADDING = 8
};

struct importInfo_t {
	unsigned int count;
	aaImportType type;
	aaImportStatus status;
	std::vector<std::string> data;
	std::vector<unsigned int> duplicates;
};
#endif

/*
#ifndef AA_LIBRETRO_PATH
#define AA_LIBRETRO_PATH "D:\\Projects\\AArcade-Source\\game\\frontend\\libretro\\cores"
#endif
*/
//#include "cbase.h"

//#include "c_liveView.h"
//#include "c_anarchymanager.h"
/*
struct DynamicImage {
	int status;
	std::string file;
	ITexture* texture;
//	C_LiveView* ownedBy;
};
*/

//#include <vector>
//#include <string>
//#include "cbase.h"