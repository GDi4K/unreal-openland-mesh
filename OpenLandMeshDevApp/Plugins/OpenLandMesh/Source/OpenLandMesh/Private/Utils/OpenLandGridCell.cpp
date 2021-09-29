#include "Utils/OpenLandGridCell.h"

#include "Misc/Crc.h"

#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FOpenLandGridCell& Thing)
{
	const FVector2D Pos = Thing.ToVector2D();
	return FCrc::MemCrc32(&Pos, sizeof(FVector2D));
}
#endif