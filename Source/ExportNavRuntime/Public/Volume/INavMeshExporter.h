#pragma once

#include "CoreMinimal.h"

struct INavMeshExporter
{
	virtual ~INavMeshExporter(){}
	virtual TArray<FBox> GetAreas()const=0;
	virtual void ExportNavData()=0;
	virtual class UNavMeshChunker* GetNavMeshChunker()const=0;
};