// Fill out your copyright notice in the Description page of Project Settings.

#include "Volume/ARecastNavMeshVolume.h"
#include "FlibExportNevChunk.h"

AARecastNavMeshVolume::AARecastNavMeshVolume(const FObjectInitializer& Initializer):Super(Initializer)
{
	NavMeshChunker = Cast<UNavMeshChunker>(UNavMeshChunker::StaticClass()->GetDefaultObject());
}


TArray<FBox> AARecastNavMeshVolume::GetAreas() const
{
	return TArray<FBox>{GetBounds().GetBox()};
}

void AARecastNavMeshVolume::ExportNavData()
{
	GetNavMeshChunker()->ExportNav(GetAreas());
}

UNavMeshChunker* AARecastNavMeshVolume::GetNavMeshChunker() const
{
	return NavMeshChunker;
}

void AARecastNavMeshVolume::ExportNav()
{
	ExportNavData();
}

void AARecastNavMeshVolume::FindPathByEngineNav()
{
	GetNavMeshChunker()->FindPathByEngineNav();
}

void AARecastNavMeshVolume::FindPathByNavFiles()
{
	GetNavMeshChunker()->FindPathByNavFiles();
}

void AARecastNavMeshVolume::DrawNavMeshsArea()
{
	GetNavMeshChunker()->DrawNavMeshsArea();
}
