#include "FlibExportNevChunk.h"
#include "UE4RecastHelper.h"
#include "FlibExportNavData.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HACK_PRIVATE_MEMBER_UTILS.hpp"
#include "Detour/DetourNavMesh.h"

DEFINE_LOG_CATEGORY(LogExportNavMesh);

bool UFlibExportNevChunk::ExportNavAreaByRef(UWorld* World, TArray<FBox> Areas,const FString& InFilePath)
{
	if(!World)
	{
		World = UFlibExportNevChunk::GetGWorld();
	}
	ARecastNavMesh* RecastNavMesh = UFlibExportNavData::GetMainRecastNavMesh(World);
	dtNavMesh* MainRecastNavMesh = UFlibExportNavData::GetdtNavMeshInsByWorld(World);

	TArray<dtTileRef> TileIndexs;
	UFlibExportNevChunk::GetNavMeshTilesRefInArea(MainRecastNavMesh,Areas,TileIndexs,World);

	dtNavMesh* NavMesh = dtAllocNavMesh();
	dtNavMeshParams TiledMeshParameters;
	FMemory::Memzero(TiledMeshParameters);
	TiledMeshParameters = *MainRecastNavMesh->getParams();

	int32 MaxTiles=0;
	int32 MaxPolys=0;
	CalcNavMeshProperties(Areas,MaxTiles,MaxPolys);
	
	TiledMeshParameters.maxTiles = MaxTiles;
	TiledMeshParameters.maxPolys = MaxPolys;

	const dtStatus status = NavMesh->init(&TiledMeshParameters);
	
	auto dtNavMesh_GetTile=GET_PRIVATE_MEMBER_FUNCTION(dtNavMesh, getTile);
	
	for(int32 tileIndex =0;tileIndex < TileIndexs.Num();++tileIndex)
	{
		const dtMeshTile* CurrentTile = MainRecastNavMesh->getTileByRef(TileIndexs[tileIndex]); //MainRecastNavMesh->getTile(tileIndex);
		dtTileRef TileRef = TileIndexs[tileIndex];
		int32 TileDataSize = CurrentTile->dataSize;
		char* TileData = UE4RecastHelper::DuplicateRecastRawData((char*)CurrentTile->data,TileDataSize);
		if(TileData)
		{
			dtTileRef AddtedTile;
			dtStatus AddStatus =  NavMesh->addTile((unsigned char*)TileData,TileDataSize,DT_TILE_FREE_DATA,TileRef,&AddtedTile);

			if (!dtStatusSucceed(AddStatus))
			{
				UE_LOG(LogExportNavMesh,Error,TEXT("Add tile index(%d) faild!"),tileIndex);
				// return false;
			}
			else
			{
				FBox TileBounds = Recast2UnrealBox(CurrentTile->header->bmin, CurrentTile->header->bmax);
				if(World)
				{
					UKismetSystemLibrary::DrawDebugBox(World,TileBounds.GetCenter(),TileBounds.GetExtent(),FLinearColor::Red,FRotator::ZeroRotator,5.0f);
				}
			}
		}
	}
	UE4RecastHelper::SerializedtNavMesh(TCHAR_TO_ANSI(*InFilePath), NavMesh);
	dtFreeNavMesh(NavMesh);
	return true;
}

void UFlibExportNevChunk::GetNavMeshTilesRefInArea(dtNavMesh* DetourNavMesh,const TArray<FBox>& InclusionBounds, TArray<dtTileRef>& Indices,UWorld* World)
{
	if (DetourNavMesh)
	{
		const float* NavMeshOrigin = DetourNavMesh->getParams()->orig;
		const float TileSize = DetourNavMesh->getParams()->tileWidth;

		// Generate a set of all possible tile coordinates that belong to requested bounds
		TSet<FIntPoint>	TileCoords;	
		for (const FBox& Bounds : InclusionBounds)
		{
			const FBox RcBounds = Unreal2RecastBox(Bounds);
			const int32 XMin = FMath::FloorToInt((RcBounds.Min.X - NavMeshOrigin[0]) / TileSize);
			const int32 XMax = FMath::FloorToInt((RcBounds.Max.X - NavMeshOrigin[0]) / TileSize);
			const int32 YMin = FMath::FloorToInt((RcBounds.Min.Z - NavMeshOrigin[2]) / TileSize);
			const int32 YMax = FMath::FloorToInt((RcBounds.Max.Z - NavMeshOrigin[2]) / TileSize);

			for (int32 y = YMin; y <= YMax; ++y)
			{
				for (int32 x = XMin; x <= XMax; ++x)
				{
					TileCoords.Add(FIntPoint(x, y));
				}
			}
		}

		// We guess that each tile has 3 layers in average
		Indices.Reserve(TileCoords.Num()*3);

		TArray<const dtMeshTile*> MeshTiles;
		MeshTiles.Reserve(3);

		for (const FIntPoint& TileCoord : TileCoords)
		{
			int32 MaxTiles = DetourNavMesh->getTileCountAt(TileCoord.X, TileCoord.Y);
			if (MaxTiles > 0)
			{
				MeshTiles.SetNumZeroed(MaxTiles, false);
				
				const int32 MeshTilesCount = DetourNavMesh->getTilesAt(TileCoord.X, TileCoord.Y, MeshTiles.GetData(), MaxTiles);
				for (int32 i = 0; i < MeshTilesCount; ++i)
				{
					const dtMeshTile* MeshTile = MeshTiles[i];
					dtTileRef TileRef = DetourNavMesh->getTileRef(MeshTile);
					if (TileRef)
					{
						// Consider only mesh tiles that actually belong to a requested bounds
						FBox TileBounds = Recast2UnrealBox(MeshTile->header->bmin, MeshTile->header->bmax);
						for (const FBox& RequestedBounds : InclusionBounds)
						{
							// if (TileBounds.Intersect(RequestedBounds))
							if (RequestedBounds.IsInside(TileBounds) || TileBounds.Intersect(RequestedBounds))
							{
								// if(World)
								// {
								// 	UKismetSystemLibrary::DrawDebugBox(World,TileBounds.GetCenter(),TileBounds.GetExtent(),FLinearColor::Green,FRotator::ZeroRotator,10.0f);
								// }
								// int32 TileIndex = (int32)DetourNavMesh->decodePolyIdTile(TileRef);
								Indices.Add(TileRef);
								break;
							}
							else
							{
								continue;
							}
						}
					}
				}
			}
		}
	}
}

UWorld* UFlibExportNevChunk::GetGWorld()
{
	UWorld* World = nullptr;
	auto WorldList = GEngine->GetWorldContexts();
	for (int32 i = 0; i < WorldList.Num(); ++i)
	{
		UWorld* local_World = WorldList[i].World();
		if (local_World && UKismetSystemLibrary::IsValid(local_World))
		{
			World = local_World;
			break;
		}
	}
	return World;
}

FBox UFlibExportNevChunk::GetNavMeshTileBounds(dtNavMesh* DetourNavMesh,int32 TileIndex)
{
	FBox Bbox(ForceInit);

	if (DetourNavMesh && TileIndex >= 0 && TileIndex < DetourNavMesh->getMaxTiles())
	{
		// workaround for privacy issue in the recast API
		dtNavMesh const* const ConstRecastNavMesh = DetourNavMesh;

		dtMeshTile const* const Tile = ConstRecastNavMesh->getTile(TileIndex);
		if (Tile)
		{
			dtMeshHeader const* const Header = Tile->header;
			if (Header)
			{
				Bbox = Recast2UnrealBox(Header->bmin, Header->bmax);
			}
		}
	}
	return Bbox;
}

bool UFlibExportNevChunk::GetNavMeshTileXY(dtNavMesh* DetourNavMesh,const FVector& Point, int32& OutX, int32& OutY)
{
	if (DetourNavMesh)
	{
		// workaround for privacy issue in the recast API
		dtNavMesh const* const ConstRecastNavMesh = DetourNavMesh;

		const FVector RecastPt = Unreal2RecastPoint(Point);
		int32 TileX = 0;
		int32 TileY = 0;

		ConstRecastNavMesh->calcTileLoc(&RecastPt.X, &TileX, &TileY);
		OutX = TileX;
		OutY = TileY;
		return true;
	}

	return false;
}

void UFlibExportNevChunk::GetNavMeshTilesAt(dtNavMesh* DetourNavMesh,int32 TileX, int32 TileY, TArray<int32>& Indices)
{
	if (DetourNavMesh)
	{
		// workaround for privacy issue in the recast API
		dtNavMesh const* const ConstRecastNavMesh = DetourNavMesh;

		const int32 MaxTiles = ConstRecastNavMesh->getTileCountAt(TileX, TileY);
		TArray<const dtMeshTile*> Tiles;
		Tiles.AddZeroed(MaxTiles);

		const int32 NumTiles = ConstRecastNavMesh->getTilesAt(TileX, TileY, Tiles.GetData(), MaxTiles);
		for (int32 i = 0; i < NumTiles; i++)
		{
			dtTileRef TileRef = ConstRecastNavMesh->getTileRef(Tiles[i]);
			if (TileRef)
			{
				const int32 TileIndex = (int32)ConstRecastNavMesh->decodePolyIdTile(TileRef);
				Indices.Add(TileIndex);
			}
		}
	}
}

void UFlibExportNevChunk::CalcPolyRefBits(ARecastNavMesh* NavMeshOwner, int32& MaxTileBits, int32& MaxPolyBits)
{
	static const int32 TotalBits = (sizeof(dtPolyRef) * 8);
#if USE_64BIT_ADDRESS
	MaxTileBits = NavMeshOwner ? FMath::CeilToFloat(FMath::Log2(NavMeshOwner->GetTileNumberHardLimit())) : 20;
	MaxPolyBits = FMath::Min<int32>(32, (TotalBits - DT_MIN_SALT_BITS) - MaxTileBits);
#else
	MaxTileBits = 14;
	MaxPolyBits = (TotalBits - DT_MIN_SALT_BITS) - MaxTileBits;
#endif//USE_64BIT_ADDRESS
}

int32 UFlibExportNevChunk::CaclulateMaxTilesCount(const TArray<FBox>& NavigableAreas, float TileSizeinWorldUnits, float AvgLayersPerGridCell)
{
	int32 GridCellsCount = 0;
	for (FBox AreaBounds : NavigableAreas)
	{
		// TODO: need more precise calculation, currently we don't take into account that volumes can be overlapped
		FBox RCBox = Unreal2RecastBox(AreaBounds);
		int32 XSize = FMath::CeilToInt(RCBox.GetSize().X/TileSizeinWorldUnits) + 1;
		int32 YSize = FMath::CeilToInt(RCBox.GetSize().Z/TileSizeinWorldUnits) + 1;
		GridCellsCount+= (XSize*YSize);
	}
	
	return FMath::CeilToInt(GridCellsCount * AvgLayersPerGridCell);
}
#include "NavigationSystem.h"
void UFlibExportNevChunk::CalcNavMeshProperties(const TArray<FBox> Boxs,int32& MaxTiles, int32& MaxPolys)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(UFlibExportNevChunk::GetGWorld());
	ANavigationData* MainNavDataIns = NavSys->GetDefaultNavDataInstance();
	ARecastNavMesh* RecastNavMeshIns = Cast<ARecastNavMesh>(MainNavDataIns);
	const FRecastNavMeshGenerator* CurrentGen = static_cast<const FRecastNavMeshGenerator*>(RecastNavMeshIns->GetGenerator());
	FRecastBuildConfig Config = CurrentGen->GetConfig();
	
	int32 MaxTileBits = -1;
	int32 MaxPolyBits = -1;

	// limit max amount of tiles
	CalcPolyRefBits(RecastNavMeshIns, MaxTileBits, MaxPolyBits);
	
	const int32 MaxTilesFromMask = (1 << MaxTileBits);
	int32 MaxRequestedTiles = 0;

	MaxRequestedTiles = CaclulateMaxTilesCount(Boxs, Config.tileSize * Config.cs, 8.0f);


	if (MaxRequestedTiles < 0 || MaxRequestedTiles > MaxTilesFromMask)
	{
		// UE_LOG(LogNavigation, Error, TEXT("Navmesh bounds are too large! Limiting requested tiles count (%d) to: (%d) for %s"), MaxRequestedTiles, MaxTilesFromMask, *GetFullNameSafe(DestNavMesh));
		MaxRequestedTiles = MaxTilesFromMask;
	}

	// Max tiles and max polys affect how the tile IDs are calculated.
	// There are (sizeof(dtPolyRef)*8 - DT_MIN_SALT_BITS) bits available for 
	// identifying a tile and a polygon.
	#if USE_64BIT_ADDRESS
	MaxPolys = (MaxPolyBits >= 32) ? INT_MAX : (1 << MaxPolyBits);
#else
	MaxPolys = 1 << ((sizeof(dtPolyRef) * 8 - DT_MIN_SALT_BITS) - MaxTileBits);
#endif // USE_64BIT_ADDRESS
	MaxTiles = MaxRequestedTiles;
}