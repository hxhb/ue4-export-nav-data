// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Detour/DetourNavMesh.h"
#include "NavMeshChunker.generated.h"

/**
* 
*/
UCLASS(BlueprintType,EditInlineNew,EditInlineNew)
class EXPORTNAVRUNTIME_API UNavMeshChunker : public UObject
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,Category = "Nav|Export")
	FDirectoryPath SavePath;
	UPROPERTY(EditAnywhere,Category = "Nav|Export")
	FString Name;
	
	UPROPERTY(EditAnywhere,Category = "Nav|Path Finding")
	AActor* BeginActor;
	UPROPERTY(EditAnywhere,Category = "Nav|Path Finding")
	AActor* EndActor;
	UPROPERTY(EditAnywhere,Category = "Nav|Path Finding")
	FVector Extern = FVector{10.f,10.f,10.f};
	UPROPERTY(EditAnywhere,Category = "Nav|Path Finding")
	TArray<FFilePath> NavMeshFiles;

public:
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetNavMeshFiles()const;

	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void ExportNav(FBox Area);
	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void FindPathByEngineNav();
	UFUNCTION(BlueprintCallable,CallInEditor,Category = "Nav")
	virtual void FindPathByNavFiles();
};
