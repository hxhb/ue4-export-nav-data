// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "INavMeshExporter.h"

#include "Components/BoxComponent.h"
#include "CoreMinimal.h"

#include "NavMeshChunker.h"
#include "GameFramework/Actor.h"
#include "NavMeshAreaExporter.generated.h"

UCLASS()
class EXPORTNAVRUNTIME_API ANavMeshAreaExporter : public AActor,public INavMeshExporter
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANavMeshAreaExporter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual TArray<FBox> GetAreas()const override;
	virtual void ExportNavData() override;
	virtual class UNavMeshChunker* GetNavMeshChunker()const override;
public:
	UPROPERTY(EditAnywhere,Category = "Nav")
	FVector BoxExtent {500.f,500.f,500.f};
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

#if WITH_EDITOR
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
private:
	UPROPERTY()
	UBoxComponent* BoxComponent;
};
