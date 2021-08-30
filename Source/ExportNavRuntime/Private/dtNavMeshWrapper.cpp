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
	// UE_LOG(LogTemp, Warning, TEXT("UdtNavMeshWrapper::~UdtNavMeshWrapper()"));
	//
	// if (NavmeshIns)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("UdtNavMeshWrapper::~UdtNavMeshWrapper() free dtNavMesh"));
	// 	dtFreeNavMesh(NavmeshIns);
	// }
}

UdtNavMeshWrapper* UdtNavMeshWrapper::LoadNavData(const TArray<FString>& NavDataBinPaths)
{
	ReleaseNavData();
	std::vector<std::string> binpaths;
	for(const auto&NavDataPath:NavDataBinPaths)
	{
		if (!NavDataPath.IsEmpty()&& FPaths::FileExists(NavDataPath))
		{
			binpaths.push_back(TCHAR_TO_ANSI(*NavDataPath));
		}
	}
	ReleaseNavData();
	UE_LOG(LogTemp, Warning, TEXT("UdtNavMeshWrapper::LoadNavData DeSerializedtNavMesh"));
	NavmeshIns = UE4RecastHelper::DeSerializeMultidtNavMesh(binpaths);
	return this;
}

void UdtNavMeshWrapper::ReleaseNavData()
{
	if(IsAvailableNavData())
	{
		UE_LOG(LogTemp, Warning, TEXT("UdtNavMeshWrapper::LoadNavData Free old NavData"));
        	dtFreeNavMesh(NavmeshIns);
        	NavmeshIns = NULL;
	}
}

bool UdtNavMeshWrapper::IsAvailableNavData() const
{
	return !!NavmeshIns;
}

dtNavMesh* UdtNavMeshWrapper::GetNavData() const
{
	return NavmeshIns;
}
