// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#include "UE4RecastHelper.h"
#include "Detour/DetourStatus.h"
#include "Detour/DetourNavMeshQuery.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

#ifdef USE_DETOUR_BUILT_INTO_UE4
	#include "Resources/Version.h"
#endif

static const long RCN_NAVMESH_VERSION = 1;
static const int INVALID_NAVMESH_POLYREF = 0;
static const int MAX_POLYS = 256;
static const int NAV_ERROR_NEARESTPOLY = -2;
#pragma warning (disable:4996)

bool UE4RecastHelper::dtIsValidNavigationPoint(dtNavMesh* InNavMeshData, const UE4RecastHelper::FVector3& InPoint, const UE4RecastHelper::FVector3& InExtent)
{
	bool bSuccess = false;

	using namespace UE4RecastHelper;

	if (!InNavMeshData) return bSuccess;

	FVector3 RcPoint = UE4RecastHelper::Unreal2RecastPoint(InPoint);
	const FVector3 ModifiedExtent = InExtent;
	FVector3 RcExtent = UE4RecastHelper::Unreal2RecastPoint(ModifiedExtent).GetAbs();
	FVector3 ClosestPoint;


	dtNavMeshQuery NavQuery;
#ifdef USE_DETOUR_BUILT_INTO_UE4
	dtQuerySpecialLinkFilter LinkFilter;
	NavQuery.init(InNavMeshData, 0, &LinkFilter);
	// UE_LOG(LogTemp, Warning, TEXT("CALL NavQuery.init(InNavMeshData, 0, &LinkFilter);"));
#else
	NavQuery.init(InNavMeshData, 0);
#endif

	dtPolyRef PolyRef;
	dtQueryFilter QueryFilter;

#ifdef USE_DETOUR_BUILT_INTO_UE4
	NavQuery.findNearestPoly2D(&RcPoint.X, &RcExtent.X, &QueryFilter, &PolyRef, (float*)(&ClosestPoint));
	UE_LOG(LogTemp, Log, TEXT("dtIsValidNavigationPoint PolyRef is %ud."), PolyRef);
#else
	NavQuery.findNearestPoly(&RcPoint.X, &RcExtent.X, &QueryFilter, &PolyRef, (float*)(&ClosestPoint));
#endif

	if (PolyRef > 0)
	{
		const FVector3& UnrealClosestPoint = UE4RecastHelper::Recast2UnrealPoint(ClosestPoint);
		const FVector3 ClosestPointDelta = UnrealClosestPoint - InPoint;
		if (-ModifiedExtent.X <= ClosestPointDelta.X && ClosestPointDelta.X <= ModifiedExtent.X
			&& -ModifiedExtent.Y <= ClosestPointDelta.Y && ClosestPointDelta.Y <= ModifiedExtent.Y
			&& -ModifiedExtent.Z <= ClosestPointDelta.Z && ClosestPointDelta.Z <= ModifiedExtent.Z)
		{
			bSuccess = true;
		}
	}

	return bSuccess;
}

int UE4RecastHelper::findStraightPath(dtNavMesh* InNavMeshData, dtNavMeshQuery* InNavmeshQuery, const FVector3& start, const FVector3& end, std::vector<FVector3>& paths)
{
	bool bSuccess = false;

	using namespace UE4RecastHelper;

	if (!InNavMeshData) return bSuccess;

	FVector3 RcStart = UE4RecastHelper::Unreal2RecastPoint(start);
	FVector3 RcEnd = UE4RecastHelper::Unreal2RecastPoint(end);
	FVector3 RcExtent{ 10.f,10.f,10.f };

	dtNavMeshQuery NavQuery;
#ifdef USE_DETOUR_BUILT_INTO_UE4
	dtQuerySpecialLinkFilter LinkFilter;
	NavQuery.init(InNavMeshData, 0, &LinkFilter);
	// UE_LOG(LogTemp, Warning, TEXT("CALL NavQuery.init(InNavMeshData, 0, &LinkFilter);"));
#else
	NavQuery.init(InNavMeshData, 0);
#endif


	FVector3 StartClosestPoint;
	FVector3 EndClosestPoint;

	dtPolyRef StartPolyRef;
	dtPolyRef EndPolyRef;
	dtQueryFilter QueryFilter;

#ifdef USE_DETOUR_BUILT_INTO_UE4
	NavQuery.findNearestPoly2D(&RcStart.X, &RcExtent.X, &QueryFilter, &StartPolyRef, (float*)(&StartClosestPoint));
	// UE_LOG(LogTemp, Warning, TEXT("CALL findNearestPoly2D"));
#else
	NavQuery.findNearestPoly(&RcStart.X, &RcExtent.X, &QueryFilter, &StartPolyRef, (float*)(&StartClosestPoint));
#endif

#ifdef USE_DETOUR_BUILT_INTO_UE4
	NavQuery.findNearestPoly2D(&RcEnd.X, &RcExtent.X, &QueryFilter, &EndPolyRef, (float*)(&EndClosestPoint));
	// UE_LOG(LogTemp, Warning, TEXT("CALL findNearestPoly2D"));
#else
	NavQuery.findNearestPoly(&RcEnd.X, &RcExtent.X, &QueryFilter, &EndPolyRef, (float*)(&EndClosestPoint));
#endif

#ifdef USE_DETOUR_BUILT_INTO_UE4
	UE_LOG(LogTemp, Log, TEXT("FindDetourPath StartPolyRef is %u."), StartPolyRef);
	UE_LOG(LogTemp, Log, TEXT("FindDetourPath EndPolyRef is %u."), EndPolyRef);

	UE_LOG(LogTemp, Log, TEXT("FindDetourPath StartClosestPoint is %s."), *FVector(StartClosestPoint.X,StartClosestPoint.Y,StartClosestPoint.Z).ToString());
	UE_LOG(LogTemp, Log, TEXT("FindDetourPath EndClosestPoint is %s."), *FVector(EndClosestPoint.X, EndClosestPoint.Y, EndClosestPoint.Z).ToString());
#endif

	dtQueryResult Result;

#if ENGINE_MINOR_VERSION < 24
	dtStatus FindPathStatus = NavQuery.findPath(StartPolyRef, EndPolyRef, (float*)(&StartClosestPoint), (float*)(&EndClosestPoint), &QueryFilter, Result, NULL);
#else
	const float CostLimit = FLT_MAX;
	dtStatus FindPathStatus = NavQuery.findPath(StartPolyRef, EndPolyRef, (float*)(&StartClosestPoint), (float*)(&EndClosestPoint), CostLimit, &QueryFilter, Result, NULL);
#endif

#ifdef USE_DETOUR_BUILT_INTO_UE4
	UE_LOG(LogTemp, Log, TEXT("FindDetourPath FindPath return status is %u."), FindPathStatus);

	UE_LOG(LogTemp, Log, TEXT("FindDetourPath dtQueryResult size is %u."), Result.size());
#endif
//	InNavmeshQuery->findStraightPath(&StartNearestPt.X, &EndNearestPt.X, );
	return 0;
}

bool UE4RecastHelper::GetRandomPointInRadius(dtNavMeshQuery* InNavmeshQuery, dtQueryFilter* InQueryFilter, const FVector3& InOrigin, const FVector3& InRedius, FVector3& OutPoint)
{
	bool bStatus = false;
	dtNavMeshQuery* NavQuery = InNavmeshQuery;
	if (!NavQuery)
	{
		return false;
	}
	dtPolyRef OriginPolyRef;
	FVector3 ClosestPoint;
	FVector3 RcPoint = UE4RecastHelper::Unreal2RecastPoint(InOrigin);

#ifdef USE_DETOUR_BUILT_INTO_UE4
	InNavmeshQuery->findNearestPoly2D(&RcPoint.X, &InRedius.X, InQueryFilter, &OriginPolyRef, (float*)(&ClosestPoint));
	// UE_LOG(LogTemp, Warning, TEXT("CALL findNearestPoly2D"));
#else
	NavQuery->findNearestPoly(&RcPoint.X, &InRedius.X, InQueryFilter, &OriginPolyRef, (float*)(&ClosestPoint));
#endif

	dtPolyRef ResultPoly;
	FVector3 ResultPoint;
	auto NormalRand = []()->float
	{
		return std::rand() / (float)RAND_MAX;
	};

	dtStatus Status = NavQuery->findRandomPointAroundCircle(OriginPolyRef, &RcPoint.X, InRedius.X, InQueryFilter, NormalRand, &ResultPoly, &ResultPoint.X);

	if (dtStatusSucceed(Status))
	{
		OutPoint = UE4RecastHelper::Recast2UnrealPoint(ResultPoint);
		bStatus = true;
	}
	return bStatus;
}

void UE4RecastHelper::SerializedtNavMesh(const char* path, const dtNavMesh* mesh)
{
	using namespace UE4RecastHelper;
	if (!mesh) return;

	std::FILE* fp = std::fopen(path, "wb");
	if (!fp)
		return;

	// Store header.
	NavMeshSetHeader header;
	header.magic = NAVMESHSET_MAGIC;
	header.version = NAVMESHSET_VERSION;
	header.numTiles = 0;
	// auto dtNavMesh_getTile = GET_PRIVATE_MEMBER_FUNCTION(dtNavMesh, getTile);

	for (int i = 0; i < mesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = mesh->getTile(i);
		// const dtMeshTile* tile = CALL_MEMBER_FUNCTION(mesh, dtNavMesh_getTile, i);
		if (!tile || !tile->header || !tile->dataSize) continue;
		header.numTiles++;
	}
	std::memcpy(&header.params, mesh->getParams(), sizeof(dtNavMeshParams));
	std::fwrite(&header, sizeof(NavMeshSetHeader), 1, fp);

	// Store tiles.
	for (int i = 0; i < mesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = mesh->getTile(i);
		// const dtMeshTile* tile = CALL_MEMBER_FUNCTION(mesh, dtNavMesh_getTile, i);
		if (!tile || !tile->header || !tile->dataSize) continue;

		NavMeshTileHeader tileHeader;
		tileHeader.tileRef = mesh->getTileRef(tile);
		tileHeader.dataSize = tile->dataSize;
		std::fwrite(&tileHeader, sizeof(tileHeader), 1, fp);

		std::fwrite(tile->data, tile->dataSize, 1, fp);
	}

	std::fclose(fp);
}

dtNavMesh* UE4RecastHelper::DeSerializedtNavMesh(const char* path)
{

	std::FILE* fp = std::fopen(path, "rb");
	if (!fp) return 0;

	using namespace UE4RecastHelper;
	// Read header.
	NavMeshSetHeader header;
	size_t sizenum = sizeof(NavMeshSetHeader);
	size_t readLen = std::fread(&header, sizenum, 1, fp);
	if (readLen != 1)
	{
		std::fclose(fp);
		return 0;
	}
	if (header.magic != NAVMESHSET_MAGIC)
	{
		std::fclose(fp);
		return 0;
	}
	if (header.version != NAVMESHSET_VERSION)
	{
		std::fclose(fp);
		return 0;
	}

	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh)
	{
		std::fclose(fp);
		return 0;
	}
	dtStatus status = mesh->init(&header.params);
	if (dtStatusFailed(status))
	{
		std::fclose(fp);
		return 0;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		NavMeshTileHeader tileHeader;
		readLen = std::fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (readLen != 1)
		{
			std::fclose(fp);
			return 0;
		}

		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		std::memset(data, 0, tileHeader.dataSize);
		readLen = fread(data, tileHeader.dataSize, 1, fp);
		if (readLen != 1)
		{
			dtFree(data);
			fclose(fp);
			return 0;
		}

		mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	std::fclose(fp);

	return mesh;
}

namespace UE4RecastHelper
{
	FVector3 Recast2UnrealPoint(const FVector3& Vector)
	{
		return FVector3(-Vector.X, -Vector.Z, Vector.Y);
	}

	FVector3 Unreal2RecastPoint(const FVector3& Vector)
	{
		return FVector3(-Vector.X, Vector.Z, -Vector.Y);
	}
};