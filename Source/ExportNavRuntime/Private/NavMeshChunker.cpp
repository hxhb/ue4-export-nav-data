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

void UNavMeshChunker::ExportNav(const TArray<FBox>& Areas)
{
	UFlibExportNevChunk::ExportNavAreaByRef(GetWorld(),Areas,FPaths::Combine(SavePath.Path,Name));
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
			FVector PathSatrt = FVector::ZeroVector;
			for(const auto& Point:Path->PathPoints)
			{
				UKismetSystemLibrary::PrintString(this,Point.ToString(),true,true,FLinearColor(0.0, 0.66, 1.0),10.f);
				UKismetSystemLibrary::DrawDebugSphere(this,Point,100.f,12,FLinearColor::Green,10.f);
				if(PathSatrt != FVector::ZeroVector)
				{
					UKismetSystemLibrary::DrawDebugLine(this,PathSatrt,Point,FLinearColor::Green,10.f);
				}
				PathSatrt = Point;
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
			FVector PathSatrt = FVector::ZeroVector;
			for(const auto& Point:Paths)
			{
				UKismetSystemLibrary::PrintString(this,Point.ToString(),true,true,FLinearColor(0.0, 0.66, 1.0),10.f);
				UKismetSystemLibrary::DrawDebugSphere(this,Point,100.f,12,FLinearColor::Red,10.f);
				if(PathSatrt != FVector::ZeroVector)
				{
					UKismetSystemLibrary::DrawDebugLine(this,PathSatrt,Point,FLinearColor::Red,10.f);
				}
				PathSatrt = Point;
			}
		}
		else
		{
			UKismetSystemLibrary::PrintString(this,TEXT("FindPathByEngineNav Faild!"),true,true,FLinearColor::Blue,10.f);
		}
	}
}
#include "HACK_PRIVATE_MEMBER_UTILS.hpp"

void UNavMeshChunker::DrawNavMeshsArea()
{
	UdtNavMeshWrapper* NavMeshWrapper = NewObject<UdtNavMeshWrapper>();
	NavMeshWrapper->LoadNavData(GetNavMeshFiles());

	dtNavMesh* NavMeshData = NavMeshWrapper->GetNavData();
	auto dtNavMesh_GetTile=GET_PRIVATE_MEMBER_FUNCTION(dtNavMesh, getTile);

	UWorld* World = this->GetWorld();
	if(!World)
	{
		World = UFlibExportNevChunk::GetGWorld();
	}
	if(NavMeshData)
	{
		int max_tile_index = NavMeshData->getMaxTiles();
		for(int tile_index = 0;tile_index < max_tile_index;++tile_index)
		{
			const dtMeshTile* CurrentTile = CALL_MEMBER_FUNCTION(NavMeshData,dtNavMesh_GetTile,tile_index);
			
			if(CurrentTile && CurrentTile->header && CurrentTile->polys && strlen((const char*)CurrentTile->data) && CurrentTile->dataSize)
			{
				dtTileRef TileRef = NavMeshData->getTileRef(CurrentTile);
				if (TileRef && NavMeshData->isValidPolyRef(TileRef))
				{
					FBox TileBounds = Recast2UnrealBox(CurrentTile->header->bmin, CurrentTile->header->bmax);
					UKismetSystemLibrary::DrawDebugBox(World,TileBounds.GetCenter(),TileBounds.GetExtent(),FLinearColor::Green,FRotator::ZeroRotator,5.0f);
				}
				
			}
		}
	}
}

