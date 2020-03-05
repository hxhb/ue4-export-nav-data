// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.


#include "dtNavMeshWrapper.h"
#include "Detour/DetourNavMesh.h"
#include "UE4RecastHelper.h"
#include "Misc/Paths.h"

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
