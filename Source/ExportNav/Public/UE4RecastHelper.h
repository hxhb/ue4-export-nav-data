
#ifndef UE4_RECAST_HELPER__
#define UE4_RECAST_HELPER__

#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "Detour/DetourNavMeshQuery.h"
#include <math.h>
#include <inttypes.h>

namespace UE4RecastHelper
{
	struct NavMeshSetHeader
	{
		int32_t magic;
		int32_t version;
		int32_t numTiles;
		dtNavMeshParams params;
	};

	struct NavMeshTileHeader
	{
		dtTileRef tileRef;
		int32_t dataSize;
	};

	static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
	static const int NAVMESHSET_VERSION = 1;

	struct FCustomVector
	{
		float X;
		float Y;
		float Z;
	public:
		inline FCustomVector() :X(0.f), Y(0.f), Z(0.f) {}
		inline FCustomVector(float* InV) : X(InV[0]), Y(InV[1]), Z(InV[2]) {}
		inline FCustomVector(float px, float py, float pz) : X(px), Y(py), Z(pz) {}
		FCustomVector(const FCustomVector&) = default;

		inline FCustomVector operator-(const FCustomVector& V) const {
			return FCustomVector(X - V.X, Y - V.Y, Z - V.Z);
		}
		inline FCustomVector operator+(const FCustomVector& V)const {
			return FCustomVector(X + V.X, Y + V.Y, Z + V.Z);
		}
		inline FCustomVector operator-(const float& V)const {
			return FCustomVector(X - V, Y - V, Z - V);
		}
		inline FCustomVector operator+(const float& V)const {
			return FCustomVector(X + V, Y + V, Z + V);
		}
		inline FCustomVector GetAbs()const
		{
			return FCustomVector{ fabsf(X),fabsf(Y),fabsf(Z) };
		}
	};

	FCustomVector Recast2UnrealPoint(const FCustomVector& Vector);
	FCustomVector Unreal2RecastPoint(const FCustomVector& Vector);

	void SerializedtNavMesh(const char* path, const dtNavMesh* mesh);
	dtNavMesh* DeSerializedtNavMesh(const char* path);

	bool dtIsValidNagivationPoint(dtNavMesh* InNavMeshData, const FCustomVector& InPoint, const FCustomVector& InExtent = FCustomVector{ 10.f,10.f,10.f });

};


#endif