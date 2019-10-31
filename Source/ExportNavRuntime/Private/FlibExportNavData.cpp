
#include "FlibExportNavData.h"
#include "ExternRecastNavMeshGenetator.h"
#include "Editor.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "RecastQueryFilter.h"
#include "AI/NavDataGenerator.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UE4RecastHelper.h"
#include "dtNavMeshWrapper.h"

bool UFlibExportNavData::ExportRecastNavMesh(const FString& SaveFile)
{
#if WITH_EDITOR
	
	FString FinalSaveFile=SaveFile;

	// UWorld* World = GEditor->GetEditorWorldContext(false).World();  

	UWorld* World=NULL;

	auto WorldList = GEngine->GetWorldContexts();
	for (int32 i=0;i < WorldList.Num();++i)
	{
		UWorld* local_World = WorldList[i].World();

		if (UKismetSystemLibrary::IsValid(local_World) && local_World->WorldType == EWorldType::Editor)
		{
			World = local_World;
			break;
		}
	
	}

	if (World && World->GetNavigationSystem())
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
#else
	return false;
#endif
}

bool UFlibExportNavData::ExportRecastNavData(const FString& InFilePath)
{
	// UWorld* World = GEditor->GetEditorWorldContext(false).World();
	UWorld* World = NULL;

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

	dtNavMesh* RecastdtNavMesh = UFlibExportNavData::GetdtNavMeshInsByWorld(World);

	if (RecastdtNavMesh)
	{
		UE4RecastHelper::SerializedtNavMesh(TCHAR_TO_ANSI(*InFilePath), RecastdtNavMesh);
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

bool UFlibExportNavData::IsValidNagivationPointByNavObj(UdtNavMeshWrapper* InDtNavObject, const FVector& Point, const FVector InExtern)
{
	bool rSuccess = false;

	if (InDtNavObject && InDtNavObject->IsAvailableNavData())
	{
		dtNavMesh* NavMeshData = InDtNavObject->GetNavData();
		if (NavMeshData)
		{
			rSuccess = UE4RecastHelper::dtIsValidNagivationPoint(NavMeshData, UFlibExportNavData::FVector2FCustomVec(Point), UFlibExportNavData::FVector2FCustomVec(InExtern));
		}
	}

	return rSuccess;
}

bool UFlibExportNavData::IsValidNagivationPointByBinPATH(UObject* WorldContextObject, const FString& InNavBinPath,const FVector& Point, const FVector InExtern /*= FVector::ZeroVector*/)
{
	bool rSuccess = false;
	
	if (InNavBinPath.IsEmpty() || !FPaths::FileExists(InNavBinPath))
		return false;

	dtNavMesh* NavMeshData = UE4RecastHelper::DeSerializedtNavMesh(TCHAR_TO_ANSI(*InNavBinPath));
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	// dtNavMesh* NavMeshData = UFlibExportNavData::GetdtNavMeshInsByWorld(World);
		
	if (NavMeshData)
	{
		rSuccess = UE4RecastHelper::dtIsValidNagivationPoint(NavMeshData, UFlibExportNavData::FVector2FCustomVec(Point), UFlibExportNavData::FVector2FCustomVec(InExtern));
		dtFreeNavMesh(NavMeshData);
	}
	
	return rSuccess;
}

FString UFlibExportNavData::ConvPath_Slash2BackSlash(const FString& InPath)
{
	FString ResaultPath;
	TArray<FString> OutArray;
	InPath.ParseIntoArray(OutArray, TEXT("\\"));
	if (OutArray.Num() == 1 && OutArray[0] == InPath)
	{
		InPath.ParseIntoArray(OutArray, TEXT("/"));
	}
	for (const auto& item : OutArray)
	{
		if (FPaths::DirectoryExists(ResaultPath + item))
		{
			ResaultPath.Append(item);
			ResaultPath.Append(TEXT("\\"));
		}
	}
	return ResaultPath;
}