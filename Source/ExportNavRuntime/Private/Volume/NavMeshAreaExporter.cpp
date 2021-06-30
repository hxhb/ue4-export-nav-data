// Fill out your copyright notice in the Description page of Project Settings.


#include "Volume/NavMeshAreaExporter.h"

#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ANavMeshAreaExporter::ANavMeshAreaExporter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	BoxComponent->SetBoxExtent(BoxExtent);
	NavMeshChunker = Cast<UNavMeshChunker>(UNavMeshChunker::StaticClass()->GetDefaultObject());
	SetRootComponent(BoxComponent);
}

// Called when the game starts or when spawned
void ANavMeshAreaExporter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANavMeshAreaExporter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<FBox> ANavMeshAreaExporter::GetAreas() const
{
	TArray<FBox> result;
	TArray<UActorComponent*> BoxComponents = this->K2_GetComponentsByClass(UBoxComponent::StaticClass());
	for(const auto& Component:BoxComponents)
	{
		UBoxComponent* BoxComp = Cast<UBoxComponent>(Component);
		if(BoxComp)
		{
			FVector Origin = BoxComp->K2_GetComponentLocation();
			FVector ScaledBoxExtent = BoxComp->GetScaledBoxExtent();
			result.Emplace(Origin - ScaledBoxExtent,Origin + ScaledBoxExtent);
		}
	}
	return result;
}

void ANavMeshAreaExporter::ExportNavData()
{
	GetNavMeshChunker()->ExportNav(GetAreas());
}

UNavMeshChunker* ANavMeshAreaExporter::GetNavMeshChunker() const
{
	return NavMeshChunker;
}

void ANavMeshAreaExporter::ExportNav()
{
	ExportNavData();
}

void ANavMeshAreaExporter::FindPathByEngineNav()
{
	GetNavMeshChunker()->FindPathByEngineNav();
}

void ANavMeshAreaExporter::FindPathByNavFiles()
{
	GetNavMeshChunker()->FindPathByNavFiles();
}

void ANavMeshAreaExporter::DrawNavMeshsArea()
{
	GetNavMeshChunker()->DrawNavMeshsArea();
}

#if WITH_EDITOR
void ANavMeshAreaExporter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (GIsEditor )
	{
		const FName PropName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : FName();
		const FName MemberName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : FName();

		if (MemberName == GET_MEMBER_NAME_CHECKED(ANavMeshAreaExporter, BoxExtent))
		{
			BoxComponent->SetBoxExtent(BoxExtent);
		}
	}	
}
#endif

