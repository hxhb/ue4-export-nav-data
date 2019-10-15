// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Misc/Paths.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FLibExportNavData.generated.h"

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
};
