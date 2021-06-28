// Fill out your copyright notice in the Description page of Project Settings.

#include "ARecastNavMeshVolume.h"
#include "FlibExportNevChunk.h"

void AARecastNavMeshVolume::ExportNav()
{
	NavMeshChunker->ExportNav(GetBounds().GetBox());
}

void AARecastNavMeshVolume::FindPathByEngineNav()
{
	NavMeshChunker->FindPathByEngineNav();
}

void AARecastNavMeshVolume::FindPathByNavFiles()
{
	NavMeshChunker->FindPathByNavFiles();
}
