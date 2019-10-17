// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "Misc/Paths.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FLibExportNavData.generated.h"


namespace NseRecastHelper
{
	struct NavMeshSetHeader
	{
		int magic;
		int version;
		int numTiles;
		dtNavMeshParams params;
	};

	struct NavMeshTileHeader
	{
		dtTileRef tileRef;
		int dataSize;
	};

	static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
	static const int NAVMESHSET_VERSION = 1;

	struct FCustomVector
	{
		float X;
		float Y;
		float Z;
	public:
		FCustomVector() :X(0.f), Y(0.f), Z(0.f) {}
		FCustomVector(float px, float py, float pz) :X(px), Y(py), Z(pz) {}
		FCustomVector(const FCustomVector&) = default;

		FORCEINLINE FCustomVector operator-(const FCustomVector& V) const{
			return FCustomVector(X - V.X, Y - V.Y, Z - V.Z);
		}
		FORCEINLINE FCustomVector operator+(const FCustomVector& V)const {
			return FCustomVector(X + V.X, Y + V.Y, Z + V.Z);
		}
		FORCEINLINE FCustomVector operator-(const float& V)const {
			return FCustomVector(X - V, Y - V, Z - V);
		}
		FORCEINLINE FCustomVector operator+(const float& V)const {
			return FCustomVector(X + V, Y + V, Z + V);
		}

	};
	static FCustomVector Recast2UnrealPoint(const FCustomVector& Vector);
	static FCustomVector Unreal2RecastPoint(const FCustomVector& Vector);
};

/**
 * 
 */
UCLASS()
class EXPORTNAV_API UFlibExportNavData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(Exec,BlueprintCallable)
		static bool ExecExportNavData(const FString& SavePath);


	UFUNCTION(BlueprintCallable,meta=(ContextObject="WorldContextObject"))
		static bool IsValidNagivationPoint(UObject* WorldContextObject,const FVector& Point);
	static bool dtIsValidNagivationPoint(dtNavMesh* NavMeshData, const NseRecastHelper::FCustomVector& InPoint);
	static dtNavMesh* LoadNavData(const char* Path);
};
