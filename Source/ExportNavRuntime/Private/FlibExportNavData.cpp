// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#include "FlibExportNavData.h"

// #include "Editor.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "NavMesh/RecastQueryFilter.h"
#include "AI/NavDataGenerator.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UE4RecastHelper.h"
#include "dtNavMeshWrapper.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

DECLARE_STATS_GROUP(TEXT("ExportNav"), STATGROUP_ExportNav, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("ExportNav"), STAT_ExportNav, STATGROUP_ExportNav);

bool UFlibExportNavData::ExportRecastNavMesh(const FString& SaveFile,EExportMode InExportMode)
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
				Generator->ExternExportNavigationData(FinalSaveFile,InExportMode);
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
	if(RecastNavMeshIns && UKismetSystemLibrary::IsValid(RecastNavMeshIns))
	{
		dtNavMesh* RecastdtNavMesh = RecastNavMeshIns->GetRecastMesh();
		return RecastdtNavMesh;
	}
	return NULL;
}

bool UFlibExportNavData::IsValidNavigvationPointInWorld(UObject* WorldContextObject, const FVector& Point, const FVector InExtern /*= FVector::ZeroVector*/)
{
	bool rSuccess = false;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	dtNavMesh* NavMeshData = UFlibExportNavData::GetdtNavMeshInsByWorld(World);

	if (NavMeshData)
	{
		rSuccess = UE4RecastHelper::dtIsValidNavigationPoint(NavMeshData, Point, InExtern);
		dtFreeNavMesh(NavMeshData);
	}

	return rSuccess;
}

bool UFlibExportNavData::IsValidNavigationPointInNavObj(UdtNavMeshWrapper* InDtNavObject, const FVector& Point, const FVector InExtern)
{
	bool rSuccess = false;

	if (InDtNavObject && InDtNavObject->IsAvailableNavData())
	{
		dtNavMesh* NavMeshData = InDtNavObject->GetNavData();
		if (NavMeshData)
		{
			rSuccess = UE4RecastHelper::dtIsValidNavigationPoint(NavMeshData, Point, InExtern);
		}
	}

	return rSuccess;
}

bool UFlibExportNavData::FindDetourPathByNavObject(class UdtNavMeshWrapper* InDtNavObject, const FVector& InStart, const FVector& InEnd, TArray<FVector>& OutPaths)
{
	using namespace UE4RecastHelper;

	bool bstatus = false;
	if (InDtNavObject && InDtNavObject->IsAvailableNavData())
	{
		dtNavMesh* NavMeshData = InDtNavObject->GetNavData();
		if (NavMeshData)
		{
			bstatus = UFlibExportNavData::FindDetourPathByNavMesh(NavMeshData, InStart, InEnd, OutPaths);
		}
	}
	return bstatus;
}

bool UFlibExportNavData::FindDetourPathByEngineNavMesh(const FVector& InStart, const FVector& InEnd, TArray<FVector>& OutPaths)
{
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

	return FindDetourPathByNavMesh(RecastdtNavMesh, InStart, InEnd, OutPaths);
	
}


bool UFlibExportNavData::GetRandomPointByNavObject(class UdtNavMeshWrapper* InDtNavObject, const FVector& InOrigin, const FVector& InRedius, FVector& OutPoint)
{
	dtNavMeshQuery NavQuery;
	dtQueryFilter QueryFilter;

	NavQuery.init(InDtNavObject->GetNavData(), 1024);
	bool Status = false;
	UE4RecastHelper::FVector3 Result;
	{
		SCOPE_CYCLE_COUNTER(STAT_ExportNav);
		Status = UE4RecastHelper::GetRandomPointInRadius(&NavQuery, &QueryFilter, InOrigin, InRedius, Result);
	}
	OutPoint = Result.UE4Vector();
	return Status;
}

bool UFlibExportNavData::FindDetourPathByNavMesh(dtNavMesh* InNavMesh, const FVector& InStart, const FVector& InEnd, TArray<FVector>& OutPaths)
{
	dtNavMeshQuery NavQuery;
	dtQueryFilter QueryFilter;

	NavQuery.init(InNavMesh, 1024);

	FVector RcStart = UFlibExportNavData::Unreal2RecastPoint(InStart);
	FVector RcEnd = UFlibExportNavData::Unreal2RecastPoint(InEnd);
	float Extern[3]{ 10.f,10.f,10.f };

	float StartPoint[3]{ RcStart.X,RcStart.Y,RcStart.Z };
	dtPolyRef StartPolyRef;
	float StartNarestPt[3]{ 0.f };
	dtStatus StartStatus = NavQuery.findNearestPoly(StartPoint, Extern, &QueryFilter, &StartPolyRef, StartNarestPt);

	float EndPoint[3]{ RcEnd.X,RcEnd.Y,RcEnd.Z };
	dtPolyRef EndPolyRef;
	float EndNarestPt[3]{ 0.f };
	dtStatus EndStatus = NavQuery.findNearestPoly(EndPoint, Extern, &QueryFilter, &EndPolyRef, EndNarestPt);

	UE_LOG(LogTemp, Log, TEXT("Start Point FindNearestPoly status is %u,PolyRef is %u."), StartStatus, StartPolyRef);
	UE_LOG(LogTemp, Log, TEXT("End Point FindNearestPoly status is %u.,PolyRef is %u."), EndStatus, EndPolyRef);
	UE_LOG(LogTemp, Log, TEXT("Start Point FindNearestPoly narestpt is %s."), *FVector(StartPoint[0], StartPoint[1], StartPoint[2]).ToString());
	UE_LOG(LogTemp, Log, TEXT("End Point FindNearestPoly narestpt is %s."), *FVector(EndPoint[0], EndPoint[1], EndPoint[2]).ToString());

	dtQueryResult result;
	float totalcost[1024 * 3];
	dtStatus FindPathStatus = NavQuery.findPath(StartPolyRef, EndPolyRef, StartNarestPt, EndNarestPt, &QueryFilter, result, totalcost);

	UE_LOG(LogTemp, Log, TEXT("findPath status is %u.,result size is %u."), FindPathStatus, result.size());
	std::vector<dtPolyRef> path;

	for (int index = 0; index < result.size(); ++index)
	{
		UE_LOG(LogTemp, Log, TEXT("Find Path index is %d ref is %u."), index, result.getRef(index));
		path.push_back(result.getRef(index));
		float currentpos[3]{ 0.f };
		result.getPos(index, currentpos);
		UE_LOG(LogTemp, Log, TEXT("Find Path index is %d pos is %s."), index, *UFlibExportNavData::Recast2UnrealPoint(FVector(currentpos[0], currentpos[1], currentpos[2])).ToString());
		// OutPaths.Add(UFlibExportNavData::Recast2UnrealPoint(FVector(currentpos[0], currentpos[1], currentpos[2])));
	}
	dtQueryResult findStraightPathResult;
	NavQuery.findStraightPath(StartNarestPt, EndNarestPt, path.data(), path.size(), findStraightPathResult);


	UE_LOG(LogTemp, Log, TEXT("findStraightPath size is %u."), findStraightPathResult.size());
	for (int index = 0; index < findStraightPathResult.size(); ++index)
	{
		float currentpos[3]{ 0.f };
		findStraightPathResult.getPos(index, currentpos);
		UE_LOG(LogTemp, Log, TEXT("findStraightPath index is %d ref is %u."), index, findStraightPathResult.getRef(index));
		UE_LOG(LogTemp, Log, TEXT("findStraightPath index is %d pos is %s."), index, *UFlibExportNavData::Recast2UnrealPoint(FVector(currentpos[0], currentpos[1], currentpos[2])).ToString());
		OutPaths.Add(UFlibExportNavData::Recast2UnrealPoint(FVector(currentpos[0], currentpos[1], currentpos[2])));
	}
	return true;
}

bool UFlibExportNavData::IsValidNavigationPointInNavbin(const FString& InNavBinPath,const FVector& Point, const FVector InExtern)
{
	bool rSuccess = false;
	
	if (InNavBinPath.IsEmpty() || !FPaths::FileExists(InNavBinPath))
		return false;

	dtNavMesh* NavMeshData = UE4RecastHelper::DeSerializedtNavMesh(TCHAR_TO_ANSI(*InNavBinPath));
		
	if (NavMeshData)
	{
		rSuccess = UE4RecastHelper::dtIsValidNavigationPoint(NavMeshData, Point, InExtern);
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

FVector UFlibExportNavData::Recast2UnrealPoint(const FVector& Vector)
{
	return FVector(-Vector.X, -Vector.Z, Vector.Y);
}

FVector UFlibExportNavData::Unreal2RecastPoint(const FVector& Vector)
{
	return FVector(-Vector.X, Vector.Z, -Vector.Y);
}
