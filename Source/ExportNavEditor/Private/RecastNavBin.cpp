// Fill out your copyright notice in the Description page of Project Settings.


#include "RecastNavBin.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ARecastNavBin::ARecastNavBin(const FObjectInitializer& Initializer):Super(Initializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	dtNavMesh = Initializer.CreateDefaultSubobject<UdtNavMeshWrapper>(this,FName{TEXT("dtNavMesh")});
}

// Called when the game starts or when spawned
void ARecastNavBin::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARecastNavBin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARecastNavBin::ImportNavBin()
{
	if(UKismetSystemLibrary::IsValid(dtNavMesh) && FPaths::FileExists(dtNavMeshBin.FilePath))
	{
		dtNavMesh->LoadNavData(dtNavMeshBin.FilePath);
	}
}

UdtNavMeshWrapper* ARecastNavBin::GetDtNavMesh() const
{
	return dtNavMesh;
}

FVector ARecastNavBin::GetFindPathExternSize() const
{
	return ExternSize;
}

