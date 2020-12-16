// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RecastNavBin.h"
#include "GameFramework/Actor.h"
#include "RecastDetourTestingActor.generated.h"

UCLASS()
class EXPORTNAVEDITOR_API ARecastDetourTestingActor : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:	
	// Sets default values for this actor's properties
	// ARecastDetourTestingActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual bool ShouldTickIfViewportsOnly()const override;

	UFUNCTION(BlueprintCallable)
	void UpdatePathing();
	
#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished)override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	UPROPERTY(EditAnywhere,Category = "FindPathing")
	ARecastDetourTestingActor* OtherActor;
	UPROPERTY(EditAnywhere,Category = "FindPathing")
	float AgentRadius = 60;
	
	UPROPERTY()
	ARecastNavBin* RecastNavBin;
	UPROPERTY()
	class UCapsuleComponent* CapsuleComponent;
	TArray<FVector> Paths;
};
