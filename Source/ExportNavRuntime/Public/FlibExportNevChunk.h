// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#pragma once
#include "ExternRecastNavMeshGenetator.h"
#include "UE4RecastHelper.h"

#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "Misc/Paths.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlibExportNevChunk.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogExportNavMesh,All,All)

USTRUCT()
struct FNavChunkArea
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector CenterPos;
	
	UPROPERTY(EditAnywhere)
	FVector Bounds;
};
/**
 * 
 */
UCLASS()
class EXPORTNAVRUNTIME_API UFlibExportNevChunk : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Editor only
	UFUNCTION(BlueprintCallable)
	static bool ExportNavArea(UWorld* World,FBox Area,const FString& InFilePath);
	static void GetNavMeshTilesIn(dtNavMesh* DetourNavMesh,const TArray<FBox>& InclusionBounds, TArray<int32>& Indices);
};
