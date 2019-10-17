
#include "FlibExportNavData.h"
#include "ExternRecastNavMeshGenetator.h"
#include "Editor.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "AI/NavDataGenerator.h"
#include "Kismet/KismetSystemLibrary.h"

bool UFlibExportNavData::ExecExportNavData(const FString& SaveFile)
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

bool UFlibExportNavData::IsValidNagivationPoint(UObject* WorldContextObject,const FVector& Point)
{
	//UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	//UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	//if (NavSys)
	//{
	//	FVector OutNavLocation;
	//	ANavigationData* UseNavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
	//	NavSys->ProjectPointToNavigation(WorldContextObject, Point, FVector{ 1.0,1.0,1.0 }, NULL, NULL);
	//}

	bool rSuccess = false;
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	FString PluginPath = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("ExportNav"))->GetBaseDir());
	FString NavDataPath= FPaths::Combine(PluginPath, TEXT("RecastDemo/solo_navmesh.bin"));
	if (FPaths::FileExists(NavDataPath))
	{
		dtNavMesh* NavMeshData = LoadNavData(TCHAR_TO_ANSI(*NavDataPath));
		if (NavMeshData)
		{
			rSuccess = dtIsValidNagivationPoint(NavMeshData, NseRecastHelper::FCustomVector(Point.X, Point.Y, Point.Z));
			UKismetSystemLibrary::PrintString(WorldContextObject, TEXT("Load Nav mesh data is success"));
		}
		else {
			UKismetSystemLibrary::PrintString(WorldContextObject, TEXT("Load Nav mesh data is faild"));
		}
	}

	
	return rSuccess;
}

bool UFlibExportNavData::dtIsValidNagivationPoint(dtNavMesh* NavMeshData, const NseRecastHelper::FCustomVector& InPoint)
{
	bool bSuccess=false;
	
	using namespace NseRecastHelper;

	if (!NavMeshData) return bSuccess;

	FCustomVector RcPoint = NseRecastHelper::Unreal2RecastPoint(InPoint);
	const FCustomVector ModifiedExtent = FCustomVector(1.f, 1.f, 1.f);
	FCustomVector RcExtent= NseRecastHelper::Unreal2RecastPoint(ModifiedExtent);
	
	FCustomVector ClosestPoint;
	 
	dtNavMeshQuery NavQuery;
	NavQuery.init(NavMeshData, 0,NULL);
	dtPolyRef PolyRef;

	NavQuery.findNearestPoly2D(&RcPoint.X, &RcPoint.X, NULL, &PolyRef, (float*)(&ClosestPoint));
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

dtNavMesh* UFlibExportNavData::LoadNavData(const char* Path)
{
	FILE* fp = fopen(Path, "rb");
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

NseRecastHelper::FCustomVector NseRecastHelper::Recast2UnrealPoint(const FCustomVector& Vector)
{
	return FCustomVector(-Vector.X, -Vector.Z, -Vector.Y);
}

NseRecastHelper::FCustomVector NseRecastHelper::Unreal2RecastPoint(const FCustomVector& Vector)
{
	return FCustomVector(-Vector.X, -Vector.Z, -Vector.Y);
}
