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
	static bool ExportNavAreaByRef(UWorld* World, TArray<FBox> Areas,const FString& InFilePath);
	static FBox GetNavMeshTileBounds(dtNavMesh* DetourNavMesh,int32 TileIndex);
	static bool GetNavMeshTileXY(dtNavMesh* DetourNavMesh,const FVector& Point, int32& OutX, int32& OutY);
	static void GetNavMeshTilesAt(dtNavMesh* DetourNavMesh,int32 TileX, int32 TileY, TArray<int32>& Indices);
	static void GetNavMeshTilesRefInArea(dtNavMesh* DetourNavMesh,const TArray<FBox>& InclusionBounds, TArray<dtTileRef>& Indices,UWorld* World=NULL);

	static UWorld* GetGWorld();
};
