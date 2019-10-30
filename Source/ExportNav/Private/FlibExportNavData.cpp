
#include "FlibExportNavData.h"
#include "ExternRecastNavMeshGenetator.h"
#include "Editor.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "RecastQueryFilter.h"
#include "AI/NavDataGenerator.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HACK_PRIVATE_MEMBER_UTILS.hpp"

bool UFlibExportNavData::ExecExportNavMesh(const FString& SaveFile)
{
	FString FinalSaveFile=SaveFile;

	UWorld* World = GEditor->GetEditorWorldContext(false).World();  

	if (World->GetNavigationSystem())
	{
		if (ANavigationData* NavData = Cast<ANavigationData>(World->GetNavigationSystem()->GetMainNavData()))
		{
			if (FExternExportNavMeshGenerator* Generator = static_cast<FExternExportNavMeshGenerator*>(NavData->GetGenerator()))
			{
				if (SaveFile.IsEmpty())
				{
					const FString Name = NavData->GetName();
					FinalSaveFile = FPaths::Combine(FPaths::ProjectSavedDir(), Name);
				}
				Generator->ExternExportNavigationData(FinalSaveFile);
				return true;
			}
		}
	}
	return false;
}

bool UFlibExportNavData::IsValidNagivationPoint(UObject* WorldContextObject, const FVector& Point, const FVector InExtern)
{
	bool rSuccess = false;
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	FString PluginPath = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("ExportNav"))->GetBaseDir());
	FString NavDataPath = FPaths::Combine(PluginPath, TEXT("solo_navmesh.bin"));
	if (FPaths::FileExists(NavDataPath))
	{
		dtNavMesh* NavMeshData = NseRecastHelper::DeSerializedtNavMesh(TCHAR_TO_ANSI(*NavDataPath));
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::LogAndReturnNull);
		// dtNavMesh* NavMeshData = UFlibExportNavData::GetdtNavMeshInsByWorld(World);
		if (NavMeshData)
		{
			rSuccess = NseRecastHelper::dtIsValidNagivationPoint(World,NavMeshData, NseRecastHelper::FCustomVector(Point),NseRecastHelper::FCustomVector(InExtern));
		}
	}

	
	return rSuccess;
}

bool UFlibExportNavData::ExportNavData(const FString& InFilePath)
{
	UWorld* World = GEditor->GetEditorWorldContext(false).World();
	dtNavMesh* RecastdtNavMesh = UFlibExportNavData::GetdtNavMeshInsByWorld(World);
	
	if (RecastdtNavMesh)
	{
		NseRecastHelper::SerializedtNavMesh(TCHAR_TO_ANSI(*InFilePath), RecastdtNavMesh);
		return true;
	}
	else {
		return false;
	}
}

dtNavMesh* UFlibExportNavData::GetdtNavMeshInsByWorld(UWorld* InWorld)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(InWorld);
	check(NavSys);
	ANavigationData* MainNavDataIns = NavSys->GetDefaultNavDataInstance();
	ARecastNavMesh* RecastNavMeshIns = Cast<ARecastNavMesh>(MainNavDataIns);
	dtNavMesh* RecastdtNavMesh = RecastNavMeshIns->GetRecastMesh();
	return RecastdtNavMesh;
}

DECL_HACK_PRIVATE_NOCONST_FUNCTION(dtNavMesh, getTile, dtMeshTile*, int);

void NseRecastHelper::SerializedtNavMesh(const char* path, const dtNavMesh* mesh)
{
	using namespace NseRecastHelper;
	if (!mesh) return;

	FILE* fp = fopen(path, "wb");
	if (!fp)
		return;

	// Store header.
	NavMeshSetHeader header;
	header.magic = NAVMESHSET_MAGIC;
	header.version = NAVMESHSET_VERSION;
	header.numTiles = 0;
	auto dtNavMesh_getTile = GET_PRIVATE_MEMBER_FUNCTION(dtNavMesh, getTile);

	for (int i = 0; i < mesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = mesh->getTile(i);
		// const dtMeshTile* tile = CALL_MEMBER_FUNCTION(mesh, dtNavMesh_getTile, i);
		if (!tile || !tile->header || !tile->dataSize) continue;
		header.numTiles++;
	}
	memcpy(&header.params, mesh->getParams(), sizeof(dtNavMeshParams));
	fwrite(&header, sizeof(NavMeshSetHeader), 1, fp);

	// Store tiles.
	for (int i = 0; i < mesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = mesh->getTile(i);
		// const dtMeshTile* tile = CALL_MEMBER_FUNCTION(mesh, dtNavMesh_getTile, i);
		if (!tile || !tile->header || !tile->dataSize) continue;

		NavMeshTileHeader tileHeader;
		tileHeader.tileRef = mesh->getTileRef(tile);
		tileHeader.dataSize = tile->dataSize;
		fwrite(&tileHeader, sizeof(tileHeader), 1, fp);

		fwrite(tile->data, tile->dataSize, 1, fp);
	}

	fclose(fp);
}


bool NseRecastHelper::dtIsValidNagivationPoint(UWorld* InWorld,dtNavMesh* InNavMeshData, const NseRecastHelper::FCustomVector& InPoint,const NseRecastHelper::FCustomVector& InExtent)
{
	bool bSuccess=false;
	
	using namespace NseRecastHelper;

	if (!InNavMeshData) return bSuccess;

	FCustomVector RcPoint = NseRecastHelper::Unreal2RecastPoint(InPoint);
	const FCustomVector ModifiedExtent = InExtent;
	FCustomVector RcExtent= NseRecastHelper::Unreal2RecastPoint(ModifiedExtent).GetAbs();
	
	float ClosestPoint[3] = {0.0f};

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(InWorld);
	check(NavSys);
	ANavigationData* MainNavDataIns = NavSys->GetDefaultNavDataInstance();
	ARecastNavMesh* RecastNavMeshIns = Cast<ARecastNavMesh>(MainNavDataIns);

	// FRecastSpeciaLinkFilter LinkFilter(NavSys, RecastNavMeshIns);
	dtQuerySpecialLinkFilter LinkFilter;
	dtNavMeshQuery NavQuery;
	NavQuery.init(InNavMeshData, 0, &LinkFilter);
	dtPolyRef PolyRef;
	dtQueryFilter QueryFilter;
	NavQuery.findNearestPoly2D(&RcPoint.X, &RcExtent.X, &QueryFilter, &PolyRef, ClosestPoint);

	if (PolyRef > 0)
	{
		const FCustomVector& UnrealClosestPoint = NseRecastHelper::Recast2UnrealPoint(ClosestPoint);
		const FCustomVector ClosestPointDelta = UnrealClosestPoint - InPoint;
		if (-ModifiedExtent.X <= ClosestPointDelta.X && ClosestPointDelta.X <= ModifiedExtent.X
			&& -ModifiedExtent.Y <= ClosestPointDelta.Y && ClosestPointDelta.Y <= ModifiedExtent.Y
			&& -ModifiedExtent.Z <= ClosestPointDelta.Z && ClosestPointDelta.Z <= ModifiedExtent.Z)
		{
			bSuccess = true;
		}
	}

	return bSuccess;
}

dtNavMesh* NseRecastHelper::LoadNavData(const char* Path)
{
	FILE* fp;
	fopen_s(&fp,Path, "rb");
	if (!fp) return NULL;

	NseRecastHelper::NavMeshSetHeader header;
	size_t readLen = fread(&header, sizeof(NseRecastHelper::NavMeshSetHeader), 1, fp);
	if (readLen != 1)
	{
		fclose(fp);
		return NULL;
	}
	if (header.magic != NseRecastHelper::NAVMESHSET_MAGIC)
	{
		fclose(fp);
		return NULL;
	}
	if (header.version != NseRecastHelper::NAVMESHSET_VERSION)
	{
		fclose(fp);
		return NULL;
	}
	dtNavMesh* NavMeshData = dtAllocNavMesh();
	if (!NavMeshData)
	{
		fclose(fp);
		return NULL;
	}

	dtStatus status = NavMeshData->init(&header.params);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return 0;
	}

	for (int index = 0; index < header.numTiles; ++index)
	{
		NseRecastHelper::NavMeshTileHeader tileHeader;
		readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (readLen != 1)
		{
			fclose(fp);
			return NULL;
		}
		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;
		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		readLen = fread(data, tileHeader.dataSize,1,fp);
		if (readLen != 1)
		{
			dtFree(data);
			fclose(fp);
			return 0;
		}
		NavMeshData->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef,NULL);
	
	}
	fclose(fp);
	return NavMeshData;
}


dtNavMesh* NseRecastHelper::DeSerializedtNavMesh(const char* path)
{
	
	FILE* fp;
	fopen_s(&fp, path, "rb");
	if (!fp) return 0;

	using namespace NseRecastHelper;
	// Read header.
	NavMeshSetHeader header;
	size_t sizenum = sizeof(NavMeshSetHeader);
	size_t readLen = fread(&header, sizenum, 1, fp);
	if (readLen != 1)
	{
		fclose(fp);
		return 0;
	}
	if (header.magic != NAVMESHSET_MAGIC)
	{
		fclose(fp);
		return 0;
	}
	if (header.version != NAVMESHSET_VERSION)
	{
		fclose(fp);
		return 0;
	}

	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh)
	{
		fclose(fp);
		return 0;
	}
	dtStatus status = mesh->init(&header.params);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return 0;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		NavMeshTileHeader tileHeader;
		readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (readLen != 1)
		{
			fclose(fp);
			return 0;
		}

		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		readLen = fread(data, tileHeader.dataSize, 1, fp);
		if (readLen != 1)
		{
			dtFree(data);
			fclose(fp);
			return 0;
		}

		mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	fclose(fp);

	return mesh;
}

NseRecastHelper::FCustomVector NseRecastHelper::Recast2UnrealPoint(const FCustomVector& Vector)
{
	return FCustomVector(-Vector.X, -Vector.Z, Vector.Y);
}

NseRecastHelper::FCustomVector NseRecastHelper::Unreal2RecastPoint(const FCustomVector& Vector)
{
	return FCustomVector(-Vector.X, Vector.Z, -Vector.Y);
}
