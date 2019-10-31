// Fill out your copyright notice in the Description page of Project Settings.


#include "dtNavMeshWrapper.h"
#include "DetourNavMesh.h"
#include "UE4RecastHelper.h"

UdtNavMeshWrapper::UdtNavMeshWrapper(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer),NavmeshIns(NULL)
{
	UE_LOG(LogTemp, Warning, TEXT("UdtNavMeshWrapper::UdtNavMeshWrapper(const FObjectInitializer& ObjectInitializer)"));
}

UdtNavMeshWrapper::~UdtNavMeshWrapper()
{
	UE_LOG(LogTemp, Warning, TEXT("UdtNavMeshWrapper::~UdtNavMeshWrapper()"));

	if (NavmeshIns)
	{
		UE_LOG(LogTemp, Warning, TEXT("UdtNavMeshWrapper::~UdtNavMeshWrapper() free dtNavMesh"));
		dtFreeNavMesh(NavmeshIns);
	}
}

UdtNavMeshWrapper* UdtNavMeshWrapper::LoadNavData(const FString& NavDataBinPath)
{
	if (!NavDataBinPath.IsEmpty()&& FPaths::FileExists(NavDataBinPath))
	{
		if (NavmeshIns)
		{
			UE_LOG(LogTemp, Warning, TEXT("UdtNavMeshWrapper::LoadNavData Free old NavData"));
			dtFreeNavMesh(NavmeshIns);
			NavmeshIns = NULL;
		}

		UE_LOG(LogTemp, Warning, TEXT("UdtNavMeshWrapper::LoadNavData DeSerializedtNavMesh"));
		NavmeshIns = UE4RecastHelper::DeSerializedtNavMesh(TCHAR_TO_ANSI(*NavDataBinPath));
	}

	return this;
}

bool UdtNavMeshWrapper::IsAvailableNavData() const
{
	return !!NavmeshIns;
}

dtNavMesh* UdtNavMeshWrapper::GetNavData() const
{
	return NavmeshIns;
}
