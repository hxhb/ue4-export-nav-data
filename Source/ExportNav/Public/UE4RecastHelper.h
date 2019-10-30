#pragma once

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include <math.h>

namespace UE4RecastHelper
{
	struct NavMeshSetHeader
	{
		int32 magic;
		int32 version;
		int32 numTiles;
		dtNavMeshParams params;
	};

	struct NavMeshTileHeader
	{
		dtTileRef tileRef;
		int32 dataSize;
	};

	static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
	static const int NAVMESHSET_VERSION = 1;

	struct FCustomVector
	{
		float X;
		float Y;
		float Z;
	public:
		FORCEINLINE FCustomVector() :X(0.f), Y(0.f), Z(0.f) {}
		FORCEINLINE FCustomVector(float* InV) : X(InV[0]), Y(InV[1]), Z(InV[2]) {}
		FORCEINLINE FCustomVector(float px, float py, float pz) :X(px), Y(py), Z(pz) {}
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
		FORCEINLINE FCustomVector GetAbs()const
		{
			return FCustomVector{ fabsf(X),fabsf(Y),fabsf(Z) };
		}
	};

	static FCustomVector Recast2UnrealPoint(const FCustomVector& Vector);
	static FCustomVector Unreal2RecastPoint(const FCustomVector& Vector);

	static void SerializedtNavMesh(const char* path, const dtNavMesh* mesh);
	static dtNavMesh* DeSerializedtNavMesh(const char* path);

	static bool dtIsValidNagivationPoint(dtNavMesh* NavMeshData, const UE4RecastHelper::FCustomVector& InPoint, const UE4RecastHelper::FCustomVector& InExtent = FCustomVector{10.f,10.f,10.f});

};
