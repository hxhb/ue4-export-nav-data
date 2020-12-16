// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/EngineTypes.h"
#include "CoreMinimal.h"

#include "dtNavMeshWrapper.h"
#include "GameFramework/Actor.h"
#include "RecastNavBin.generated.h"

UCLASS()
class EXPORTNAVEDITOR_API ARecastNavBin : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:	
	// Sets default values for this actor's properties
	// ARecastNavBin();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere,Category="ImportNavData")
	FFilePath dtNavMeshBin;
	UPROPERTY(EditAnywhere,Category="ImportNavData")
	FVector ExternSize{10.f,10.f,10.f};
	UFUNCTION(BlueprintCallable,CallInEditor,Category="ImpoerNavData")
	void ImportNavBin();

	UFUNCTION(BlueprintCallable)
	UdtNavMeshWrapper* GetDtNavMesh()const;
	UFUNCTION(BlueprintCallable)
	FVector GetFindPathExternSize()const;
	

	
	UdtNavMeshWrapper* dtNavMesh;
};
