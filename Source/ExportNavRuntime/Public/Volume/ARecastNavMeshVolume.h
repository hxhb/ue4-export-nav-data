// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Volume/INavMeshExporter.h"
#include "CoreMinimal.h"

#include "NavMeshChunker.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "ARecastNavMeshVolume.generated.h"

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType)
class EXPORTNAVRUNTIME_API AARecastNavMeshVolume : public ANavMeshBoundsVolume,public INavMeshExporter
{
	GENERATED_UCLASS_BODY()

	virtual TArray<FBox> GetAreas()const override;
	virtual void ExportNavData() override;
	virtual class UNavMeshChunker* GetNavMeshChunker()const override;
	
	UPROPERTY(EditInstanceOnly,Instanced,Category = "Nav")
	UNavMeshChunker* NavMeshChunker;

	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void ExportNav();
	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void FindPathByEngineNav();
	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void FindPathByNavFiles();
	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void DrawNavMeshsArea();
};
