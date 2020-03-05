// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Detour/DetourNavMesh.h"
#include "dtNavMeshWrapper.generated.h"

/**
 * 
 */
UCLASS(BlueprintType,Blueprintable)
class EXPORTNAVRUNTIME_API UdtNavMeshWrapper : public UObject
{
	GENERATED_UCLASS_BODY()

	virtual ~UdtNavMeshWrapper()override;

	UFUNCTION(BlueprintCallable,Category="ExportNav")
	UdtNavMeshWrapper* LoadNavData(const FString& NavDataBinPath=TEXT(""));
	UFUNCTION(BlueprintCallable,Category="ExportNav")
		bool IsAvailableNavData()const;

	dtNavMesh* GetNavData()const;
private:
	dtNavMesh* NavmeshIns;
};
