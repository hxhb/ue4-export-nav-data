// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ExportNav.h"
#include "FlibExportNavData.h"
#include "ExportNavStyle.h"
#include "ExportNavCommands.h"


#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "DesktopPlatformModule.h"
#include "LevelEditor.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"

static const FName ExportNavTabName("ExportNav");

#define LOCTEXT_NAMESPACE "FExportNavModule"

void FExportNavModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FExportNavStyle::Initialize();
	FExportNavStyle::ReloadTextures();

	FExportNavCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FExportNavCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FExportNavModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FExportNavModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FExportNavModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FExportNavModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FExportNavStyle::Shutdown();

	FExportNavCommands::Unregister();
}

void FExportNavModule::PluginButtonClicked()
{
	DoExportNavData();
}

void FExportNavModule::DoExportNavData()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	FString PluginPath = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("ExportNav"))->GetBaseDir());
	if (DesktopPlatform)
	{
		TArray<FString> SaveFilenames;
		FString SaveName= World->GetMapName()+FString(TEXT("-RecastNavMesh-"))+FDateTime::Now().ToString();
		const bool bOpened = DesktopPlatform->SaveFileDialog(
			nullptr,
			LOCTEXT("SaveNav", "Save Navigation Data").ToString(),
			FPaths::Combine(PluginPath, TEXT("RecastDemo/Meshes")),
			SaveName,
			TEXT("*.obj|*.obj"),
			EFileDialogFlags::None,
			SaveFilenames
		);

		if (SaveFilenames.Num() > 0)
		{
			FString SaveToFole = FPaths::ConvertRelativePathToFull(SaveFilenames[0]);
			UFlibExportNavData::ExecExportNavMesh(SaveToFole);

			
			// FString RecastDemoProc = FPaths::Combine(PluginPath,TEXT("RecastDemo/RecastDemo.exe"));
			/*FString EndCommand = TEXT("cmd /c start /D ") + RecastDemoProc;
			system(TCHAR_TO_ANSI(*EndCommand));*/
			// FPlatformProcess::CreateProc(*RecastDemoProc, NULL, false, false ,false, NULL, NULL, NULL, NULL);
		}
	}
	
	
}
void FExportNavModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FExportNavCommands::Get().PluginAction);
}

void FExportNavModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FExportNavCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FExportNavModule, ExportNav)