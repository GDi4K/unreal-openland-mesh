#include "Utils/OpenLandGridCell.h"

#include "Misc/Crc.h"

#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FOpenLandGridCell& Thing)
{
	return FCrc::MemCrc32(&Thing, sizeof(FOpenLandGridCell));
}
#endif