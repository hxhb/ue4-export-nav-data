#include "FlibExportNevChunk.h"
#include "UE4RecastHelper.h"
#include "FlibExportNavData.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HACK_PRIVATE_MEMBER_UTILS.hpp"
#include "Detour/DetourNavMesh.h"

DEFINE_LOG_CATEGORY(LogExportNavMesh);

DECL_HACK_PRIVATE_CONST_FUNCTION(dtNavMesh,getTile,const dtMeshTile*,int);

bool UFlibExportNevChunk::ExportNavArea(UWorld* World, FBox Area,const FString& InFilePath)
{
	if(!World)
	{
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
		if (!World) return false;
	}
	ARecastNavMesh* RecastNavMesh = UFlibExportNavData::GetMainRecastNavMesh(World);

	dtNavMesh* MainRecastNavMesh = UFlibExportNavData::GetdtNavMeshInsByWorld(World);
	dtNavMesh* NavMesh = dtAllocNavMesh();
	dtNavMeshParams TiledMeshParameters;
	FMemory::Memzero(TiledMeshParameters);
	TiledMeshParameters = *MainRecastNavMesh->getParams();
	const dtStatus status = NavMesh->init(&TiledMeshParameters);
	
	UE4RecastHelper::NavMeshSetHeader header;
	{
		// Store header.
		header.magic = UE4RecastHelper::NAVMESHSET_MAGIC;
		header.version = UE4RecastHelper::NAVMESHSET_VERSION;
		header.numTiles = 0;
		std::memcpy(&header.params, MainRecastNavMesh->getParams(), sizeof(dtNavMeshParams));
	}

	TArray<int32> TileIndexs;
	UFlibExportNevChunk::GetNavMeshTilesIn(MainRecastNavMesh,TArray<FBox>{Area},TileIndexs);
	header.numTiles = TileIndexs.Num();

	auto dtNavMesh_GetTile=GET_PRIVATE_MEMBER_FUNCTION(dtNavMesh, getTile);
	
	for(int32 tileIndex =0;tileIndex < TileIndexs.Num();++tileIndex)
	{
		const dtMeshTile* CurrentTile = CALL_MEMBER_FUNCTION(MainRecastNavMesh,dtNavMesh_GetTile,tileIndex); //MainRecastNavMesh->getTile(tileIndex);
		dtTileRef TileRef = MainRecastNavMesh->getTileRef(CurrentTile);
		int32 TileDataSize = CurrentTile->dataSize;
		unsigned char* TileData = UE4RecastHelper::DuplicateRecastRawData(CurrentTile->data,TileDataSize);
		if(TileData)
		{
			dtTileRef* AddtedTile = NULL;
			dtStatus AddStatus =  NavMesh->addTile(TileData,TileDataSize,CurrentTile->flags,TileRef,AddtedTile);

			if (!dtStatusSucceed(AddStatus) && !AddtedTile)
			{
				UE_LOG(LogExportNavMesh,Error,TEXT("Add tile index(%d) faild!"),tileIndex);
				return false;
			}
		}
	}
	UE4RecastHelper::SerializedtNavMesh(TCHAR_TO_ANSI(*InFilePath), NavMesh);
	return true;
}


void UFlibExportNevChunk::GetNavMeshTilesIn(dtNavMesh* DetourNavMesh,const TArray<FBox>& InclusionBounds, TArray<int32>& Indices)
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
							if (TileBounds.Intersect(RequestedBounds))
							// if (RequestedBounds.IsInside(TileBounds) || TileBounds.Intersect(RequestedBounds))
							{
								int32 TileIndex = (int32)DetourNavMesh->decodePolyIdTile(TileRef);
								Indices.Add(TileIndex);
								break;
							}
						}
					}
				}
			}
		}
	}
}