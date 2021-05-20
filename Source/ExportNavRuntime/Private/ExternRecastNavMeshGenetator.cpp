// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#include "ExternRecastNavMeshGenetator.h"
#include "Navmesh/RecastHelpers.h"
#include "GenericPlatform/GenericPlatform.h"
#include "Detour/DetourNavMesh.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "HAL/FileManager.h"
#include "HAL/FileManagerGeneric.h"
#include "Resources/Version.h"

FExternRecastGeometryCache::FExternRecastGeometryCache(const uint8* Memory)
{
	Header = *((FHeader*)Memory);
	Verts = (float*)(Memory + sizeof(FExternRecastGeometryCache));
	Indices = (int32*)(Memory + sizeof(FExternRecastGeometryCache) + (sizeof(float) * Header.NumVerts * 3));
}

void FExternExportNavMeshGenerator::ExternExportNavigationData(const FString& FileName, EExportMode InExportMode)
{
	const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	const FNavigationOctree* NavOctree = NavSys ? NavSys->GetNavOctree() : NULL;
	if (NavOctree == NULL)
	{
		UE_LOG(LogNavigation, Error, TEXT("Failed to export navigation data due to %s being NULL"), NavSys == NULL ? TEXT("NavigationSystem") : TEXT("NavOctree"));
		return;
	}

	const double StartExportTime = FPlatformTime::Seconds();

	FString CurrentTimeStr = FDateTime::Now().ToString();
	for (int32 Index = 0; Index < NavSys->NavDataSet.Num(); ++Index)
	{
		// feed data from octtree and mark for rebuild				
		TNavStatArray<float> CoordBuffer;
		TNavStatArray<int32> IndexBuffer;
		const ARecastNavMesh* NavData = Cast<const ARecastNavMesh>(NavSys->NavDataSet[Index]);
		if (NavData)
		{
			struct FAreaExportData
			{
				FConvexNavAreaData Convex;
				uint8 AreaId;
			};
			TArray<FAreaExportData> AreaExport;
#if ENGINE_MINOR_VERSION >=26
			NavOctree->FindElementsWithBoundsTest(TotalNavBounds, [&](const FNavigationOctreeElement& Element)
#else
			for (FNavigationOctree::TConstElementBoxIterator<FNavigationOctree::DefaultStackAllocator> It(*NavOctree, TotalNavBounds);
				It.HasPendingElements();
				It.Advance())
#endif
			{
#if ENGINE_MINOR_VERSION < 26
				const FNavigationOctreeElement& Element = It.GetCurrentElement();
#endif
				const bool bExportGeometry = Element.Data->HasGeometry() && Element.ShouldUseGeometry(DestNavMesh->GetConfig());

				if (bExportGeometry && Element.Data->CollisionData.Num())
				{
					FExternRecastGeometryCache CachedGeometry(Element.Data->CollisionData.GetData());
					IndexBuffer.Reserve(IndexBuffer.Num() + (CachedGeometry.Header.NumFaces * 3));
					CoordBuffer.Reserve(CoordBuffer.Num() + (CachedGeometry.Header.NumVerts * 3));
					for (int32 i = 0; i < CachedGeometry.Header.NumFaces * 3; i++)
					{
						IndexBuffer.Add(CachedGeometry.Indices[i] + CoordBuffer.Num() / 3);
					}

					switch (InExportMode)
					{
						case EExportMode::Metre:
							{
								//// Export unit metre
								for (int32 i = 0; i < CachedGeometry.Header.NumVerts * 3; i += 3)
								{
									FVector Corrd = FVector{
										CachedGeometry.Verts[i] / 100.f,
										CachedGeometry.Verts[i + 2] / 100.f,
										CachedGeometry.Verts[i + 1] / 100.f,
									};
									// CoordBuffer.Add(CachedGeometry.Verts[i]);
									CoordBuffer.Add(Corrd.X);
									CoordBuffer.Add(Corrd.Z);
									CoordBuffer.Add(Corrd.Y);
								}
								break;
							};
							case EExportMode::Centimeter:
								{
									// Export unit centimeters 
									for (int32 i = 0; i < CachedGeometry.Header.NumVerts * 3; i++)
									{
										CoordBuffer.Add(CachedGeometry.Verts[i]);
									}
									break;
								};
							}

						}
						else


						{
							const TArray<FAreaNavModifier>& AreaMods = Element.Data->Modifiers.GetAreas();
							for (int32 i = 0; i < AreaMods.Num(); i++)
							{
								FAreaExportData ExportInfo;
								ExportInfo.AreaId = NavData->GetAreaID(AreaMods[i].GetAreaClass());

								if (AreaMods[i].GetShapeType() == ENavigationShapeType::Convex)
								{
									AreaMods[i].GetConvex(ExportInfo.Convex);

									TArray<FVector> ConvexVerts;
									GrowConvexHull(NavData->AgentRadius, ExportInfo.Convex.Points, ConvexVerts);
									ExportInfo.Convex.MinZ -= NavData->CellHeight;
									ExportInfo.Convex.MaxZ += NavData->CellHeight;
									ExportInfo.Convex.Points = ConvexVerts;

									AreaExport.Add(ExportInfo);
								}
							}
						}


					UWorld* NavigationWorld = GetWorld();
					for (int32 LevelIndex = 0; LevelIndex < NavigationWorld->GetNumLevels(); ++LevelIndex)
					{
						const ULevel* const Level = NavigationWorld->GetLevel(LevelIndex);
						if (Level == NULL)
						{
							continue;
						}

						const TArray<FVector>* LevelGeom = Level->GetStaticNavigableGeometry();
						if (LevelGeom != NULL && LevelGeom->Num() > 0)
						{
							TNavStatArray<FVector> Verts;
							TNavStatArray<int32> Faces;
							// For every ULevel in World take its pre-generated static geometry vertex soup
							TransformVertexSoupToRecast(*LevelGeom, Verts, Faces);

							IndexBuffer.Reserve(IndexBuffer.Num() + Faces.Num());
							CoordBuffer.Reserve(CoordBuffer.Num() + Verts.Num() * 3);
							for (int32 i = 0; i < Faces.Num(); i++)
							{
								IndexBuffer.Add(Faces[i] + CoordBuffer.Num() / 3);
							}
							for (int32 i = 0; i < Verts.Num(); i++)
							{
								CoordBuffer.Add(Verts[i].X);
								CoordBuffer.Add(Verts[i].Y);
								CoordBuffer.Add(Verts[i].Z);
							}
						}
					}


					FString AreaExportStr;
					for (int32 i = 0; i < AreaExport.Num(); i++)
					{
						const FAreaExportData& ExportInfo = AreaExport[i];
						AreaExportStr += FString::Printf(TEXT("\nAE %d %d %f %f\n"),
							ExportInfo.AreaId, ExportInfo.Convex.Points.Num(), ExportInfo.Convex.MinZ, ExportInfo.Convex.MaxZ);

						for (int32 iv = 0; iv < ExportInfo.Convex.Points.Num(); iv++)
						{
							FVector Pt = Unreal2RecastPoint(ExportInfo.Convex.Points[iv]);
							AreaExportStr += FString::Printf(TEXT("Av %f %f %f\n"), Pt.X, Pt.Y, Pt.Z);
						}
					}

					FString AdditionalData;

					if (AreaExport.Num())
					{
						AdditionalData += "# Area export\n";
						AdditionalData += AreaExportStr;
						AdditionalData += "\n";
					}

					AdditionalData += "# RecastDemo specific data\n";
		#if 0
					// use this bounds to have accurate navigation data bounds
					const FVector Center = Unreal2RecastPoint(NavData->GetBounds().GetCenter());
					FVector Extent = FVector(NavData->GetBounds().GetExtent());
					Extent = FVector(Extent.X, Extent.Z, Extent.Y);
		#else
					// this bounds match navigation bounds from level
					FBox RCNavBounds = Unreal2RecastBox(TotalNavBounds);
					const FVector Center = RCNavBounds.GetCenter();
					const FVector Extent = RCNavBounds.GetExtent();
		#endif
					const FBox Box = FBox::BuildAABB(Center, Extent);
					AdditionalData += FString::Printf(
						TEXT("rd_bbox %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f\n"),
						Box.Min.X, Box.Min.Y, Box.Min.Z,
						Box.Max.X, Box.Max.Y, Box.Max.Z
					);

					const FRecastNavMeshGenerator* CurrentGen = static_cast<const FRecastNavMeshGenerator*>(NavData->GetGenerator());
					check(CurrentGen);

					AdditionalData += FString::Printf(TEXT("# Cell Size\n"));
					AdditionalData += FString::Printf(TEXT("rd_cs %5.5f\n"), CurrentGen->GetConfig().cs);
					AdditionalData += FString::Printf(TEXT("# Cell Height\n"));
					AdditionalData += FString::Printf(TEXT("rd_ch %5.5f\n"), CurrentGen->GetConfig().ch);
					AdditionalData += FString::Printf(TEXT("# Agent max slope\n"));
					AdditionalData += FString::Printf(TEXT("rd_ams %5.5f\n"), CurrentGen->GetConfig().walkableSlopeAngle);
					AdditionalData += FString::Printf(TEXT("# warkable height\n"));
					AdditionalData += FString::Printf(TEXT("rd_wh %d\n"), CurrentGen->GetConfig().walkableHeight);
					AdditionalData += FString::Printf(TEXT("# warkable Climb\n"));
					AdditionalData += FString::Printf(TEXT("rd_wc %d\n"), CurrentGen->GetConfig().walkableClimb);
					AdditionalData += FString::Printf(TEXT("# warkable radius\n"));
					AdditionalData += FString::Printf(TEXT("rd_wr %d\n"), CurrentGen->GetConfig().walkableRadius);

					AdditionalData += FString::Printf(TEXT("# AgentHeight\n"));
					AdditionalData += FString::Printf(TEXT("rd_agh %5.5f\n"), CurrentGen->GetConfig().AgentHeight);
					AdditionalData += FString::Printf(TEXT("# AgentRadius\n"));
					AdditionalData += FString::Printf(TEXT("rd_agr %5.5f\n"), CurrentGen->GetConfig().AgentRadius);
					AdditionalData += FString::Printf(TEXT("# Agent max climb\n"));
					AdditionalData += FString::Printf(TEXT("rd_amc %5.5f\n"), (int)CurrentGen->GetConfig().AgentMaxClimb);
			
					AdditionalData += FString::Printf(TEXT("# border size\n"));
					AdditionalData += FString::Printf(TEXT("rd_bs %d\n"), (int)CurrentGen->GetConfig().borderSize);
					AdditionalData += FString::Printf(TEXT("# Max edge len\n"));
					AdditionalData += FString::Printf(TEXT("rd_mel %d\n"), CurrentGen->GetConfig().maxEdgeLen);
					AdditionalData += FString::Printf(TEXT("# maxSimplificationError\n"));
					AdditionalData += FString::Printf(TEXT("rd_mse %5.5f\n"), CurrentGen->GetConfig().maxSimplificationError);


					AdditionalData += FString::Printf(TEXT("# Region min size\n"));
					AdditionalData += FString::Printf(TEXT("rd_rmis %d\n"), (uint32)FMath::Sqrt(CurrentGen->GetConfig().minRegionArea));
					AdditionalData += FString::Printf(TEXT("# Region merge size\n"));
					AdditionalData += FString::Printf(TEXT("rd_rmas %d\n"), (uint32)FMath::Sqrt(CurrentGen->GetConfig().mergeRegionArea));

					AdditionalData += FString::Printf(TEXT("# maxVertsPerPoly\n"));
					AdditionalData += FString::Printf(TEXT("rd_mvpp %d\n"), CurrentGen->GetConfig().maxVertsPerPoly);
					AdditionalData += FString::Printf(TEXT("# detailSampleDist\n"));
					AdditionalData += FString::Printf(TEXT("rd_dsd %5.5f\n"), CurrentGen->GetConfig().detailSampleDist);
					AdditionalData += FString::Printf(TEXT("# detailSampleMaxError\n"));
					AdditionalData += FString::Printf(TEXT("rd_dsm %5.5f\n"), CurrentGen->GetConfig().detailSampleMaxError);
					AdditionalData += FString::Printf(TEXT("# PolyMaxHeight\n"));
		#if ENGINE_MINOR_VERSION < 24
					AdditionalData += FString::Printf(TEXT("rd_pmh %d\n"), CurrentGen->GetConfig().PolyMaxHeight);
		#endif
					AdditionalData += FString::Printf(TEXT("# minRegionArea\n"));
					AdditionalData += FString::Printf(TEXT("rd_minra %d\n"), CurrentGen->GetConfig().minRegionArea);
					AdditionalData += FString::Printf(TEXT("# mergeRegionArea\n"));
					AdditionalData += FString::Printf(TEXT("rd_mergera %d\n"), CurrentGen->GetConfig().mergeRegionArea);


					AdditionalData += FString::Printf(TEXT("# Perform Voxel Filtering\n"));
					AdditionalData += FString::Printf(TEXT("rd_pvf %d\n"), CurrentGen->GetConfig().bPerformVoxelFiltering);
					AdditionalData += FString::Printf(TEXT("# bMarkLowHeightAreas\n"));
					AdditionalData += FString::Printf(TEXT("rd_mlha %d\n"), CurrentGen->GetConfig().bMarkLowHeightAreas);
					AdditionalData += FString::Printf(TEXT("# bFilterLowSpanSequences\n"));
					AdditionalData += FString::Printf(TEXT("rd_flss %d\n"), CurrentGen->GetConfig().bFilterLowSpanSequences);
					AdditionalData += FString::Printf(TEXT("# bFilterLowSpanFromTileCache\n"));
					AdditionalData += FString::Printf(TEXT("rd_flstc %d\n"), CurrentGen->GetConfig().bFilterLowSpanFromTileCache);

					AdditionalData += FString::Printf(TEXT("# AgentIndex\n"));
					AdditionalData += FString::Printf(TEXT("rd_ai %d\n"), CurrentGen->GetConfig().AgentIndex);
					AdditionalData += FString::Printf(TEXT("# Tile size\n"));
					AdditionalData += FString::Printf(TEXT("rd_ts %d\n"), CurrentGen->GetConfig().tileSize);

					AdditionalData += FString::Printf(TEXT("# regionChunkSize\n"));
					AdditionalData += FString::Printf(TEXT("rd_rcs %d\n"), CurrentGen->GetConfig().regionChunkSize);
					AdditionalData += FString::Printf(TEXT("# TileCacheChunkSize\n"));
					AdditionalData += FString::Printf(TEXT("rd_tccs %d\n"), CurrentGen->GetConfig().TileCacheChunkSize);
					AdditionalData += FString::Printf(TEXT("# regionPartitioning\n"));
					AdditionalData += FString::Printf(TEXT("rd_rp %d\n"), CurrentGen->GetConfig().regionPartitioning);
					AdditionalData += FString::Printf(TEXT("# TileCachePartitionType\n"));
					AdditionalData += FString::Printf(TEXT("rd_tcpt %d\n"), CurrentGen->GetConfig().TileCachePartitionType);

					AdditionalData += FString::Printf(TEXT("# Generate Detailed Mesh\n"));
					AdditionalData += FString::Printf(TEXT("rd_gdm %d\n"), CurrentGen->GetConfig().bGenerateDetailedMesh);
					AdditionalData += FString::Printf(TEXT("# MaxPolysPerTile\n"));
					AdditionalData += FString::Printf(TEXT("rd_mppt %d\n"), CurrentGen->GetConfig().MaxPolysPerTile);

					AdditionalData += FString::Printf(TEXT("\n"));

					const FString FilePathName = FileName;// FString::Printf(TEXT("_NavDataSet%d_%s.obj"), Index, *CurrentTimeStr);
					ExportGeomToOBJFile(FilePathName, CoordBuffer, IndexBuffer, AdditionalData);
				}
		#if ENGINE_MINOR_VERSION >=26
				);
#endif
		}
		// UE_LOG(LogNavigation, Log, TEXT("ExportNavigation time: %.3f sec ."), FPlatformTime::Seconds() - StartExportTime);
	}
}

void FExternExportNavMeshGenerator::GrowConvexHull(const float ExpandBy, const TArray<FVector>& Verts, TArray<FVector>& OutResult)
{
	if (Verts.Num() < 3)
	{
		return;
	}

	struct FSimpleLine
	{
		FVector P1, P2;

		FSimpleLine() {}

		FSimpleLine(FVector Point1, FVector Point2)
			: P1(Point1), P2(Point2)
		{

		}
		static FVector Intersection(const FSimpleLine& Line1, const FSimpleLine& Line2)
		{
			const float A1 = Line1.P2.X - Line1.P1.X;
			const float B1 = Line2.P1.X - Line2.P2.X;
			const float C1 = Line2.P1.X - Line1.P1.X;

			const float A2 = Line1.P2.Y - Line1.P1.Y;
			const float B2 = Line2.P1.Y - Line2.P2.Y;
			const float C2 = Line2.P1.Y - Line1.P1.Y;

			const float Denominator = A2 * B1 - A1 * B2;
			if (Denominator != 0)
			{
				const float t = (B1*C2 - B2 * C1) / Denominator;
				return Line1.P1 + t * (Line1.P2 - Line1.P1);
			}

			return FVector::ZeroVector;
		}
	};

	TArray<FVector> AllVerts(Verts);
	AllVerts.Add(Verts[0]);
	AllVerts.Add(Verts[1]);

	const int32 VertsCount = AllVerts.Num();
	const FQuat Rotation90(FVector(0, 0, 1), FMath::DegreesToRadians(90));

	float RotationAngle = MAX_FLT;
	for (int32 Index = 0; Index < VertsCount - 2; ++Index)
	{
		const FVector& V1 = AllVerts[Index + 0];
		const FVector& V2 = AllVerts[Index + 1];
		const FVector& V3 = AllVerts[Index + 2];

		const FVector V01 = (V1 - V2).GetSafeNormal();
		const FVector V12 = (V2 - V3).GetSafeNormal();
		const FVector NV1 = Rotation90.RotateVector(V01);
		const float d = FVector::DotProduct(NV1, V12);

		if (d < 0)
		{
			// CW
			RotationAngle = -90;
			break;
		}
		else if (d > 0)
		{
			//CCW
			RotationAngle = 90;
			break;
		}
	}

	// check if we detected CW or CCW direction
	if (RotationAngle >= BIG_NUMBER)
	{
		return;
	}

	const float ExpansionThreshold = 2 * ExpandBy;
	const float ExpansionThresholdSQ = ExpansionThreshold * ExpansionThreshold;
	const FQuat Rotation(FVector(0, 0, 1), FMath::DegreesToRadians(RotationAngle));
	FSimpleLine PreviousLine;
	OutResult.Reserve(Verts.Num());
	for (int32 Index = 0; Index < VertsCount - 2; ++Index)
	{
		const FVector& V1 = AllVerts[Index + 0];
		const FVector& V2 = AllVerts[Index + 1];
		const FVector& V3 = AllVerts[Index + 2];

		FSimpleLine Line1;
		if (Index > 0)
		{
			Line1 = PreviousLine;
		}
		else
		{
			const FVector V01 = (V1 - V2).GetSafeNormal();
			const FVector N1 = Rotation.RotateVector(V01).GetSafeNormal();
			const FVector MoveDir1 = N1 * ExpandBy;
			Line1 = FSimpleLine(V1 + MoveDir1, V2 + MoveDir1);
		}

		const FVector V12 = (V2 - V3).GetSafeNormal();
		const FVector N2 = Rotation.RotateVector(V12).GetSafeNormal();
		const FVector MoveDir2 = N2 * ExpandBy;
		const FSimpleLine Line2(V2 + MoveDir2, V3 + MoveDir2);

		const FVector NewPoint = FSimpleLine::Intersection(Line1, Line2);
		if (NewPoint == FVector::ZeroVector)
		{
			// both lines are parallel so just move our point by expansion distance
			OutResult.Add(V2 + MoveDir2);
		}
		else
		{
			const FVector VectorToNewPoint = NewPoint - V2;
			const float DistToNewVector = VectorToNewPoint.SizeSquared2D();
			if (DistToNewVector > ExpansionThresholdSQ)
			{
				//clamp our point to not move to far from original location
				const FVector HelpPos = V2 + VectorToNewPoint.GetSafeNormal2D() * ExpandBy * 1.4142;
				OutResult.Add(HelpPos);
			}
			else
			{
				OutResult.Add(NewPoint);
			}
		}

		PreviousLine = Line2;
	}
}

void FExternExportNavMeshGenerator::TransformVertexSoupToRecast(const TArray<FVector>& VertexSoup, TNavStatArray<FVector>& Verts, TNavStatArray<int32>& Faces)
{
	if (VertexSoup.Num() == 0)
	{
		return;
	}

	check(VertexSoup.Num() % 3 == 0);

	const int32 StaticFacesCount = VertexSoup.Num() / 3;
	int32 VertsCount = Verts.Num();
	const FVector* Vertex = VertexSoup.GetData();

	for (int32 k = 0; k < StaticFacesCount; ++k, Vertex += 3)
	{
		Verts.Add(Unreal2RecastPoint(Vertex[0]));
		Verts.Add(Unreal2RecastPoint(Vertex[1]));
		Verts.Add(Unreal2RecastPoint(Vertex[2]));
		Faces.Add(VertsCount + 2);
		Faces.Add(VertsCount + 1);
		Faces.Add(VertsCount + 0);

		VertsCount += 3;
	}
}

void FExternExportNavMeshGenerator::ExportGeomToOBJFile(const FString& InFileName, const TNavStatArray<float>& GeomCoords, const TNavStatArray<int32>& GeomFaces, const FString& AdditionalData)
{
#define USE_COMPRESSION 0

#if ALLOW_DEBUG_FILES
	//SCOPE_CYCLE_COUNTER(STAT_Navigation_TileGeometryExportToObjAsync);

	FString FileName = InFileName;

#if USE_COMPRESSION
	FileName += TEXT("z");
	struct FDataChunk
	{
		TArray<uint8> UncompressedBuffer;
		TArray<uint8> CompressedBuffer;
		void CompressBuffer()
		{
			const int32 HeaderSize = sizeof(int32);
			const int32 UncompressedSize = UncompressedBuffer.Num();
			CompressedBuffer.Init(0, HeaderSize + FMath::Trunc(1.1f * UncompressedSize));

			int32 CompressedSize = CompressedBuffer.Num() - HeaderSize;
			uint8* DestBuffer = CompressedBuffer.GetData();
			FMemory::Memcpy(DestBuffer, &UncompressedSize, HeaderSize);
			DestBuffer += HeaderSize;

			FCompression::CompressMemory(NAME_Zlib, (void*)DestBuffer, CompressedSize, (void*)UncompressedBuffer.GetData(), UncompressedSize, COMPRESS_BiasMemory);
			CompressedBuffer.SetNum(CompressedSize + HeaderSize, false);
		}
	};
	FDataChunk AllDataChunks[3];
	const int32 NumberOfChunks = sizeof(AllDataChunks) / sizeof(FDataChunk);
	{
		FMemoryWriter ArWriter(AllDataChunks[0].UncompressedBuffer);
		for (int32 i = 0; i < GeomCoords.Num(); i += 3)
		{
			FVector Vertex(GeomCoords[i + 0], GeomCoords[i + 1], GeomCoords[i + 2]);
			ArWriter << Vertex;
		}
	}

	{
		FMemoryWriter ArWriter(AllDataChunks[1].UncompressedBuffer);
		for (int32 i = 0; i < GeomFaces.Num(); i += 3)
		{
			FVector Face(GeomFaces[i + 0] + 1, GeomFaces[i + 1] + 1, GeomFaces[i + 2] + 1);
			ArWriter << Face;
		}
	}

	{
		auto AnsiAdditionalData = StringCast<ANSICHAR>(*AdditionalData);
		FMemoryWriter ArWriter(AllDataChunks[2].UncompressedBuffer);
		ArWriter.Serialize((ANSICHAR*)AnsiAdditionalData.Get(), AnsiAdditionalData.Length());
	}

	FArchive* FileAr = IFileManager::Get().CreateDebugFileWriter(*FileName);
	if (FileAr != NULL)
	{
		for (int32 Index = 0; Index < NumberOfChunks; ++Index)
		{
			AllDataChunks[Index].CompressBuffer();
			int32 BufferSize = AllDataChunks[Index].CompressedBuffer.Num();
			FileAr->Serialize(&BufferSize, sizeof(int32));
			FileAr->Serialize((void*)AllDataChunks[Index].CompressedBuffer.GetData(), AllDataChunks[Index].CompressedBuffer.Num());
		}
		UE_LOG(LogNavigation, Error, TEXT("UncompressedBuffer size:: %d "), AllDataChunks[0].UncompressedBuffer.Num() + AllDataChunks[1].UncompressedBuffer.Num() + AllDataChunks[2].UncompressedBuffer.Num());
		FileAr->Close();
	}

#else
	FArchive* FileAr = IFileManager::Get().CreateDebugFileWriter(*FileName);
	if (FileAr != NULL)
	{
		for (int32 Index = 0; Index < GeomCoords.Num(); Index += 3)
		{
			FString LineToSave = FString::Printf(TEXT("v %f %f %f\n"), GeomCoords[Index + 0], GeomCoords[Index + 1], GeomCoords[Index + 2]);
			auto AnsiLineToSave = StringCast<ANSICHAR>(*LineToSave);
			FileAr->Serialize((ANSICHAR*)AnsiLineToSave.Get(), AnsiLineToSave.Length());
		}

		for (int32 Index = 0; Index < GeomFaces.Num(); Index += 3)
		{
			FString LineToSave = FString::Printf(TEXT("f %d %d %d\n"), GeomFaces[Index + 0] + 1, GeomFaces[Index + 1] + 1, GeomFaces[Index + 2] + 1);
			auto AnsiLineToSave = StringCast<ANSICHAR>(*LineToSave);
			FileAr->Serialize((ANSICHAR*)AnsiLineToSave.Get(), AnsiLineToSave.Length());
		}

		auto AnsiAdditionalData = StringCast<ANSICHAR>(*AdditionalData);
		FileAr->Serialize((ANSICHAR*)AnsiAdditionalData.Get(), AnsiAdditionalData.Length());
		FileAr->Close();
	}
#endif

#undef USE_COMPRESSION
#endif
}
