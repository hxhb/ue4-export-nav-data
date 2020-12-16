// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#pragma once
#include "ExternRecastNavMeshGenetator.h"
#include "UE4RecastHelper.h"

#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "Misc/Paths.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FLibExportNavData.generated.h"

/**
 * 
 */
UCLASS()
class EXPORTNAVRUNTIME_API UFlibExportNavData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Editor only
	// UFUNCTION(Exec,BlueprintCallable,Category="ExportNav")
		static bool ExportRecastNavMesh(const FString& SavePath,EExportMode InExportMode);
	// Editor and Runtime
	// UFUNCTION(Exec,BlueprintCallable,Category="ExportNav")
		static bool ExportRecastNavData(const FString& InFilePath);

	static dtNavMesh* GetdtNavMeshInsByWorld(UWorld* InWorld);

	UFUNCTION(BlueprintCallable, BlueprintPure,Category="ExportNav", meta = (WorldContext = "WorldContextObject"))
		static bool IsValidNavigvationPointInWorld(UObject* WorldContextObject, const FVector& Point, const FVector InExtern = FVector::ZeroVector);
	UFUNCTION(BlueprintCallable, BlueprintPure,Category="ExportNav")
		static bool IsValidNavigationPointInNavbin(const FString& InNavBinPath, const FVector& Point, const FVector InExtern = FVector::ZeroVector);
	UFUNCTION(BlueprintCallable, BlueprintPure,Category="ExportNav")
		static bool IsValidNavigationPointInNavObj(class UdtNavMeshWrapper* InDtNavObject ,const FVector& Point, const FVector InExtern = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable,meta=(AutoCreateRefTerm="InStart,InEnd"))
		static bool FindDetourPathFromGameAxisByNavObject(class UdtNavMeshWrapper* InDtNavObject, const FVector& InStart, const FVector& InEnd, const FVector& InExternSize, TArray<FVector>& OutPaths);
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "InStart,InEnd"))
		static bool FindDetourPathFromRecastAxisByNavObject(class UdtNavMeshWrapper* InDtNavObject, const FVector& InStart, const FVector& InEnd, const FVector& InExternSize, TArray<FVector>& OutPaths);
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "InStart,InEnd"))
		static bool FindDetourPathByEngineNavMesh(const FVector& InStart, const FVector& InEnd, const FVector& InExternSize, TArray<FVector>& OutPaths);
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "InOrigin,InRedius"))
		static bool GetRandomPointByNavObject(class UdtNavMeshWrapper* InDtNavObject, const FVector& InOrigin, const FVector& InRedius, FVector& OutPoint);
		
	static bool FindDetourPathByRecastAxis(dtNavMesh* InNavMesh, const FVector& InStart, const FVector& InEnd, const FVector& ExternSize, TArray<FVector>& OutPaths);

	static bool FindDetourPathByGameAxis(dtNavMesh* InNavMesh, const FVector& InStart, const FVector& InEnd, const FVector& InExternSize, TArray<FVector>& OutPaths);
	

	static FString ConvPath_Slash2BackSlash(const FString& InPath);

	UFUNCTION(BlueprintCallable)
	static FVector Recast2UnrealPoint(const FVector& Vector);
	UFUNCTION(BlueprintCallable)
	static FVector Unreal2RecastPoint(const FVector& Vector);


	UFUNCTION(BlueprintCallable,meta=(WorldContext="WorldContextObject",CallableWithoutWorldContext))
		static float FindDistanceToWall(UObject* WorldContextObject, const FVector& StartLoc);
};
