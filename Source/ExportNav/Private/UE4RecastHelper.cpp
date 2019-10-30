#include "UE4RecastHelper.h"

#include "NavigationData.h"
#include "RecastQueryFilter.h"

#include "HACK_PRIVATE_MEMBER_UTILS.hpp"

bool UE4RecastHelper::dtIsValidNagivationPoint(dtNavMesh* InNavMeshData, const UE4RecastHelper::FCustomVector& InPoint,const UE4RecastHelper::FCustomVector& InExtent)
{
	bool bSuccess=false;
	
	using namespace UE4RecastHelper;

	if (!InNavMeshData) return bSuccess;

	FCustomVector RcPoint = UE4RecastHelper::Unreal2RecastPoint(InPoint);
	const FCustomVector ModifiedExtent = InExtent;
	FCustomVector RcExtent= UE4RecastHelper::Unreal2RecastPoint(ModifiedExtent).GetAbs();
	FCustomVector ClosestPoint;

	dtQuerySpecialLinkFilter LinkFilter;
	dtNavMeshQuery NavQuery;
	NavQuery.init(InNavMeshData, 0, &LinkFilter);
	dtPolyRef PolyRef;
	dtQueryFilter QueryFilter;
	NavQuery.findNearestPoly2D(&RcPoint.X, &RcExtent.X, &QueryFilter, &PolyRef, (float*)(&ClosestPoint));

	if (PolyRef > 0)
	{
		const FCustomVector& UnrealClosestPoint = UE4RecastHelper::Recast2UnrealPoint(ClosestPoint);
		const FCustomVector ClosestPointDelta = UnrealClosestPoint - InPoint;
		if (-ModifiedExtent.X <= ClosestPointDelta.X && ClosestPointDelta.X <= ModifiedExtent.X
			&& -ModifiedExtent.Y <= ClosestPointDelta.Y && ClosestPointDelta.Y <= ModifiedExtent.Y
			&& -ModifiedExtent.Z <= ClosestPointDelta.Z && ClosestPointDelta.Z <= ModifiedExtent.Z)
		{
			bSuccess = true;
		}
	}

	return bSuccess;
}

DECL_HACK_PRIVATE_NOCONST_FUNCTION(dtNavMesh, getTile, dtMeshTile*, int);

void UE4RecastHelper::SerializedtNavMesh(const char* path, const dtNavMesh* mesh)
{
	using namespace UE4RecastHelper;
	if (!mesh) return;

	FILE* fp = NULL;
	fopen_s(&fp, path, "wb");
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
	memcpy(&header.params, mesh->getParams(), sizeof(dtNavMeshParams));
	fwrite(&header, sizeof(NavMeshSetHeader), 1, fp);

	// Store tiles.
	for (int i = 0; i < mesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = mesh->getTile(i);
		// const dtMeshTile* tile = CALL_MEMBER_FUNCTION(mesh, dtNavMesh_getTile, i);
		if (!tile || !tile->header || !tile->dataSize) continue;

		NavMeshTileHeader tileHeader;
		tileHeader.tileRef = mesh->getTileRef(tile);
		tileHeader.dataSize = tile->dataSize;
		fwrite(&tileHeader, sizeof(tileHeader), 1, fp);

		fwrite(tile->data, tile->dataSize, 1, fp);
	}

	fclose(fp);
}

dtNavMesh* UE4RecastHelper::DeSerializedtNavMesh(const char* path)
{
	
	FILE* fp;
	fopen_s(&fp, path, "rb");
	if (!fp) return 0;

	using namespace UE4RecastHelper;
	// Read header.
	NavMeshSetHeader header;
	size_t sizenum = sizeof(NavMeshSetHeader);
	size_t readLen = fread(&header, sizenum, 1, fp);
	if (readLen != 1)
	{
		fclose(fp);
		return 0;
	}
	if (header.magic != NAVMESHSET_MAGIC)
	{
		fclose(fp);
		return 0;
	}
	if (header.version != NAVMESHSET_VERSION)
	{
		fclose(fp);
		return 0;
	}

	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh)
	{
		fclose(fp);
		return 0;
	}
	dtStatus status = mesh->init(&header.params);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return 0;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		NavMeshTileHeader tileHeader;
		readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (readLen != 1)
		{
			fclose(fp);
			return 0;
		}

		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		readLen = fread(data, tileHeader.dataSize, 1, fp);
		if (readLen != 1)
		{
			dtFree(data);
			fclose(fp);
			return 0;
		}

		mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	fclose(fp);

	return mesh;
}

UE4RecastHelper::FCustomVector UE4RecastHelper::Recast2UnrealPoint(const FCustomVector& Vector)
{
	return FCustomVector(-Vector.X, -Vector.Z, Vector.Y);
}

UE4RecastHelper::FCustomVector UE4RecastHelper::Unreal2RecastPoint(const FCustomVector& Vector)
{
	return FCustomVector(-Vector.X, Vector.Z, -Vector.Y);
}
