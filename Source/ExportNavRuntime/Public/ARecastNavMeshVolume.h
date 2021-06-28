// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "ARecastNavMeshVolume.generated.h"

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType)
class EXPORTNAVRUNTIME_API AARecastNavMeshVolume : public ANavMeshBoundsVolume
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere,Category = "Nav|Export")
	FDirectoryPath SavePath;
	UPROPERTY(EditAnywhere,Category = "Nav|Export")
	FString Name;

	UPROPERTY(EditAnywhere,Category = "Nav|Path Finding")
	AActor* BeginActor;
	UPROPERTY(EditAnywhere,Category = "Nav|Path Finding")
	AActor* EndActor;
	UPROPERTY(EditAnywhere,Category = "Nav|Path Finding")
	FVector Extern = FVector{10.f,10.f,10.f};
	UPROPERTY(EditAnywhere,Category = "Nav|Path Finding")
	TArray<FFilePath> NavMeshFiles;

	

	UFUNCTION(BlueprintCallable)
	FBox GetAreaBox()const;
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetNavMeshFiles()const;

	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void ExportNav();
	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void FindPathByEngineNav();
	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void FindPathByNavFiles();
	
// #if WITH_EDITOR
// 	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent) override;
// 	virtual void OnBuilderUpdated();
// #endif
};
