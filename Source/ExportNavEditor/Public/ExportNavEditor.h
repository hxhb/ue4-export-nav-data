// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#pragma once
#include "ExternRecastNavMeshGenetator.h"

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FExportNavEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

private:
	// export bin file
	void DoExportNavData(const FString& SaveToFile);
	// export recast obj file,Editor only
	void DoExportNavMesh(const FString& SaveToFile,EExportMode InExportMode);

	// if not fount any valid navigation data,show the message
	void NotFountAnyValidNavDataMsg();

	static void CreateSaveFileNotify(const FText& InMsg, const FString& InSavedFile);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
