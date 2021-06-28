// Fill out your copyright notice in the Description page of Project Settings.


#include "ARecastNavMeshVolume.h"

#include "dtNavMeshWrapper.h"
#include "FlibExportNavData.h"
#include "FlibExportNevChunk.h"
#include "NavigationSystem.h"
#include "Builders/CubeBuilder.h"
#include "Kismet/KismetSystemLibrary.h"

AARecastNavMeshVolume::AARecastNavMeshVolume(const FObjectInitializer& Initializer):Super(Initializer)
{
	
}

FBox AARecastNavMeshVolume::GetAreaBox() const
{
	return GetBounds().GetBox();
}

TArray<FString> AARecastNavMeshVolume::GetNavMeshFiles() const
{
	TArray<FString> result;
	for(const auto& File:NavMeshFiles)
	{
		if(FPaths::FileExists(File.FilePath))
		{
			result.AddUnique(FPaths::ConvertRelativePathToFull(File.FilePath));
		}
	}
	return result;
}

void AARecastNavMeshVolume::ExportNav()
{
	UFlibExportNevChunk::ExportNavAreaByRef(GetWorld(),GetAreaBox(),FPaths::Combine(SavePath.Path,Name));
}

void AARecastNavMeshVolume::FindPathByEngineNav()
{
	if(BeginActor && EndActor)
	{
		UKismetSystemLibrary::PrintString(this,TEXT(""),true,true,FLinearColor(0.0, 0.66, 1.0),10.f);
		FVector BeginPos = BeginActor->K2_GetActorLocation();
		FVector EndPos = EndActor->K2_GetActorLocation();
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(this,BeginPos,EndPos);
		if(Path)
		{
			for(const auto& Point:Path->PathPoints)
			{
				UKismetSystemLibrary::PrintString(this,Point.ToString(),true,true,FLinearColor(0.0, 0.66, 1.0),10.f);
				UKismetSystemLibrary::DrawDebugSphere(this,Point,100.f,12,FLinearColor::Green,10.f);
			}
		}
		else
		{
			UKismetSystemLibrary::PrintString(this,TEXT("FindPathByEngineNav Faild!"),true,true,FLinearColor::Red,10.f);
		}
	}
}

void AARecastNavMeshVolume::FindPathByNavFiles()
{
	if(BeginActor && EndActor)
	{
		UKismetSystemLibrary::PrintString(this,TEXT(""),true,true,FLinearColor(0.0, 0.66, 1.0),10.f);
		FVector BeginPos = BeginActor->K2_GetActorLocation();
		FVector EndPos = EndActor->K2_GetActorLocation();

		UdtNavMeshWrapper* NavMeshWrapper = NewObject<UdtNavMeshWrapper>();
		NavMeshWrapper->LoadNavData(GetNavMeshFiles());

		TArray<FVector> Paths;
		UFlibExportNavData::FindDetourPathFromGameAxisByNavObject(NavMeshWrapper,BeginPos,EndPos,Extern,Paths);
		if(!!Paths.Num())
		{
			for(const auto& Point:Paths)
			{
				UKismetSystemLibrary::PrintString(this,Point.ToString(),true,true,FLinearColor(0.0, 0.66, 1.0),10.f);
				UKismetSystemLibrary::DrawDebugSphere(this,Point,100.f,12,FLinearColor::Red,10.f);
			}
		}
		else
		{
			UKismetSystemLibrary::PrintString(this,TEXT("FindPathByEngineNav Faild!"),true,true,FLinearColor::Blue,10.f);
		}
	}
	
}

// #if WITH_EDITOR
// void AARecastNavMeshVolume::PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent)
// {
// 	Super::PostEditChangeProperty(PropertyChangedEvent);
//
// 	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
// 	if (GIsEditor && NavSys)
// 	{
// 		const FName PropName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : FName();
// 		const FName MemberName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
//
// 		if (PropName == GET_MEMBER_NAME_CHECKED(ABrush, BrushBuilder)
// 			|| MemberName == GET_MEMBER_NAME_CHECKED(ANavMeshBoundsVolume, SupportedAgents)
// 			|| MemberName == USceneComponent::GetRelativeLocationPropertyName()
// 			|| MemberName == USceneComponent::GetRelativeRotationPropertyName()
// 			|| MemberName == USceneComponent::GetRelativeScale3DPropertyName())
// 		{
// 			OnBuilderUpdated();
// 		}
// 	}
// }
//
// void AARecastNavMeshVolume::OnBuilderUpdated()
// {
// 	FBoxSphereBounds CurrBounds = GetBounds();
// 	if(Max)
// 	{
// 		Max->SetWorldLocation(K2_GetActorLocation());
// 		Max->AddRelativeLocation(CurrBounds.BoxExtent);
// 	}
// 	if(Min)
// 	{
// 		Min->SetWorldLocation(K2_GetActorLocation());
// 		Min->AddRelativeLocation(FVector::ZeroVector-CurrBounds.BoxExtent);
// 	}
// 	
// }
// #endif
