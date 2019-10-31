// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "Misc/Paths.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UE4RecastHelper.h"
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
	UFUNCTION(Exec,BlueprintCallable)
		static bool ExportRecastNavMesh(const FString& SavePath);
	// Editor and Runtime
	UFUNCTION(Exec,BlueprintCallable)
		static bool ExportRecastNavData(const FString& InFilePath);

	static dtNavMesh* GetdtNavMeshInsByWorld(UWorld* InWorld);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static bool IsValidNagivationPointByBinPATH(UObject* WorldContextObject,const FString& InNavBinPath, const FVector& Point, const FVector InExtern = FVector::ZeroVector);
	UFUNCTION(BlueprintCallable, BlueprintPure)
		static bool IsValidNagivationPointByNavObj(class UdtNavMeshWrapper* InDtNavObject ,const FVector& Point, const FVector InExtern = FVector::ZeroVector);

	FORCEINLINE static FVector FCustomVec2FVector(const UE4RecastHelper::FCustomVector& InCustomVector)
	{
		return FVector{ InCustomVector.X,InCustomVector.Y,InCustomVector.Z };
	}
	FORCEINLINE static UE4RecastHelper::FCustomVector FVector2FCustomVec(const FVector& InVector)
	{
		return UE4RecastHelper::FCustomVector{ InVector.X,InVector.Y,InVector.Z };
	}

	static FString ConvPath_Slash2BackSlash(const FString& InPath);
};
