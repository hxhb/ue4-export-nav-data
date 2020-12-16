// Fill out your copyright notice in the Description page of Project Settings.


#include "RecastDetourTestingActor.h"
#include "RecastNavBin.h"
#include "FlibExportNavData.h"

// engine
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ARecastDetourTestingActor::ARecastDetourTestingActor(const FObjectInitializer& Initializer):Super(Initializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCylinder"));
	CapsuleComponent->InitCapsuleSize(AgentRadius, AgentRadius / 2);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	RootComponent = CapsuleComponent;

}

// Called when the game starts or when spawned
void ARecastDetourTestingActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARecastDetourTestingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(Paths.Num())
	{
		FVector LastPoint = Paths[0];
		for(int32 index =1;index<Paths.Num();++index)
		{
			FHitResult result;
			UKismetSystemLibrary::LineTraceSingle(this,LastPoint,Paths[index],ETraceTypeQuery::TraceTypeQuery1,true,TArray<AActor*>{},EDrawDebugTrace::ForOneFrame,result,true,FLinearColor::Red,FLinearColor::Green,1.0);
			LastPoint = Paths[index];
		}
	}

}

bool ARecastDetourTestingActor::ShouldTickIfViewportsOnly() const
{
	return true;
}

void ARecastDetourTestingActor::UpdatePathing()
{
	if(!HasAnyFlags(RF_ClassDefaultObject))
	{
		TArray<AActor*> AllRecastNavActors;
		UGameplayStatics::GetAllActorsOfClass(this,ARecastNavBin::StaticClass(),AllRecastNavActors);
		if(AllRecastNavActors.Num() == 0)
		{
			RecastNavBin = Cast<ARecastNavBin>(GetWorld()->SpawnActor(ARecastNavBin::StaticClass()));
		}
		else
		{
			RecastNavBin = Cast<ARecastNavBin>(AllRecastNavActors[0]);
		}
	}
	
	if(UKismetSystemLibrary::IsValid(RecastNavBin) && UKismetSystemLibrary::IsValid(OtherActor))
	{
		UdtNavMeshWrapper* NavMeshWrapper = RecastNavBin->GetDtNavMesh();
		if(NavMeshWrapper->GetNavData())
		{
			Paths.Empty();
			UFlibExportNavData::FindDetourPathByGameAxis(NavMeshWrapper->GetNavData(),GetActorLocation(),OtherActor->GetActorLocation(),RecastNavBin->GetFindPathExternSize(), Paths);
		}
	}
}

void ARecastDetourTestingActor::PostEditMove(bool bFinished)
{
	UpdatePathing();
}

void ARecastDetourTestingActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	static FName OtherTestingActorName = GET_MEMBER_NAME_CHECKED(ARecastDetourTestingActor,OtherActor);
	if(PropertyChangedEvent.GetPropertyName() == OtherTestingActorName)
	{
		if(OtherActor)
		{
			OtherActor->OtherActor = this;
			UpdatePathing();
		}
	}
}