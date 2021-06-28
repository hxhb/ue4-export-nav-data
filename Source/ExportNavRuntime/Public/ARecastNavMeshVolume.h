// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "NavMeshChunker.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "ARecastNavMeshVolume.generated.h"

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType)
class EXPORTNAVRUNTIME_API AARecastNavMeshVolume : public ANavMeshBoundsVolume
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly,Instanced,Category = "Nav")
	UNavMeshChunker* NavMeshChunker;

	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void ExportNav();
	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void FindPathByEngineNav();
	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void FindPathByNavFiles();

};
