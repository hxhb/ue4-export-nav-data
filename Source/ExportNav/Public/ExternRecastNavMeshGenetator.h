#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"
#include "Navmesh/RecastNavMeshGenerator.h"

struct FExternRecastGeometryCache
{

	struct FHeader
	{
		FNavigationRelevantData::FCollisionDataHeader Validation;

		int32 NumVerts;
		int32 NumFaces;
		struct FWalkableSlopeOverride SlopeOverride;

		static uint32 StaticMagicNumber;
	};

	FHeader Header;

	/** recast coords of vertices (size: NumVerts * 3) */
	float* Verts;

	/** vert indices for triangles (size: NumFaces * 3) */
	int32* Indices;

	FExternRecastGeometryCache() {}
	FExternRecastGeometryCache(const uint8* Memory);

	static bool IsValid(const uint8* Memory, int32 MemorySize);
};


class EXPORTNAV_API FExternExportNavMeshGenerator : public FRecastNavMeshGenerator
{

public:
	void ExternExportNavigationData(const FString& FileName);

	static FVector ChangeDirectionOfPoint(FVector Coord);
	void GrowConvexHull(const float ExpandBy, const TArray<FVector>& Verts, TArray<FVector>& OutResult);
	void TransformVertexSoupToRecast(const TArray<FVector>& VertexSoup, TNavStatArray<FVector>& Verts, TNavStatArray<int32>& Faces);
	void ExportGeomToOBJFile(const FString& InFileName, const TNavStatArray<float>& GeomCoords, const TNavStatArray<int32>& GeomFaces, const FString& AdditionalData);

};
