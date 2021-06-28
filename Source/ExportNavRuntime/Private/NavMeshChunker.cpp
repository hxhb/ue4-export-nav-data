#include "NavMeshChunker.h"
#include "dtNavMeshWrapper.h"
#include "FlibExportNavData.h"
#include "FlibExportNevChunk.h"
#include "NavigationSystem.h"
#include "Builders/CubeBuilder.h"
#include "Kismet/KismetSystemLibrary.h"

TArray<FString> UNavMeshChunker::GetNavMeshFiles() const
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

void UNavMeshChunker::ExportNav(FBox Area)
{
	UFlibExportNevChunk::ExportNavAreaByRef(GetWorld(),Area,FPaths::Combine(SavePath.Path,Name));
}

void UNavMeshChunker::FindPathByEngineNav()
{
	if(BeginActor && EndActor)
	{
		UKismetSystemLibrary::PrintString(this,TEXT(""),true,true,FLinearColor(0.0, 0.66, 1.0),10.f);
		FVector BeginPos = BeginActor->K2_GetActorLocation();
		FVector EndPos = EndActor->K2_GetActorLocation();
		
		UKismetSystemLibrary::PrintString(this,FString::Printf(TEXT("Start:%s\nEnd:%s"),*BeginPos.ToString(),*EndPos.ToString()),true,true,FLinearColor::Red,10.f);
		
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

void UNavMeshChunker::FindPathByNavFiles()
{
	if(BeginActor && EndActor)
	{
		UKismetSystemLibrary::PrintString(this,TEXT(""),true,true,FLinearColor(0.0, 0.66, 1.0),10.f);
		FVector BeginPos = BeginActor->K2_GetActorLocation();
		FVector EndPos = EndActor->K2_GetActorLocation();

		UKismetSystemLibrary::PrintString(this,FString::Printf(TEXT("Start:%s\nEnd:%s"),*BeginPos.ToString(),*EndPos.ToString()),true,true,FLinearColor::Red,10.f);
		
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