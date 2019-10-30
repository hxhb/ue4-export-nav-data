// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DetourNavMesh.h"
#include "dtNavMeshWrapper.generated.h"

/**
 * 
 */
UCLASS(BlueprintType,Blueprintable)
class EXPORTNAV_API UdtNavMeshWrapper : public UObject
{
	GENERATED_UCLASS_BODY()

	virtual ~UdtNavMeshWrapper()override;

	UFUNCTION(BlueprintCallable)
	UdtNavMeshWrapper* LoadNavData(const FString& NavDataBinPath=TEXT(""));
	UFUNCTION(BlueprintCallable)
		bool IsAvailableNavData()const;

	dtNavMesh* GetNavData()const;
private:
	dtNavMesh* NavmeshIns;
};
