// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#include "ExportNavEditor.h"
#include "ExportNavStyle.h"
#include "ExportNavCommands.h"

#include "FlibExportNavData.h"

#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "DesktopPlatformModule.h"
#include "LevelEditor.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

static const FName ExportNavTabName("ExportNav");

#define LOCTEXT_NAMESPACE "FExportNavEditorModule"

void FExportNavEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FExportNavStyle::Initialize();
	FExportNavStyle::ReloadTextures();

	FExportNavCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FExportNavCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FExportNavEditorModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FExportNavEditorModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FExportNavEditorModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FExportNavEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FExportNavStyle::Shutdown();

	FExportNavCommands::Unregister();
}

void FExportNavEditorModule::PluginButtonClicked()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();

	if (!UFlibExportNavData::GetdtNavMeshInsByWorld(World))
	{
		NotFountAnyValidNavDataMsg();
		return;
	}

	FString MapName = World->GetMapName();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	FString PluginPath = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("ExportNav"))->GetBaseDir());

	FString OutPath;
	if (DesktopPlatform)
	{
		const bool bOpened = DesktopPlatform->OpenDirectoryDialog(
			nullptr,
			LOCTEXT("SaveNav", "Save Recast Navigation NavMesh & NavData").ToString(),
			PluginPath,
			OutPath
		);
		if (!OutPath.IsEmpty() && FPaths::DirectoryExists(OutPath))
		{
			FString CurrentTime = FDateTime::Now().ToString();

			FText NavMeshMsg = LOCTEXT("SaveNavMeshMesh", "Successd to Export the NavMesh.");

#if EXPORT_NAV_MESH_AS_CM
			FString NavMeshFileCM = FPaths::Combine(OutPath, MapName + TEXT("-NavMesh-CM-") + CurrentTime+TEXT(".obj"));
			DoExportNavMesh(NavMeshFileCM,EExportMode::Centimeter);
			CreateSaveFileNotify(NavMeshMsg, NavMeshFileCM);
#endif
#if EXPORT_NAV_MESH_AS_M
			FString NavMeshFileM = FPaths::Combine(OutPath, MapName + TEXT("-NavMesh-M-") + CurrentTime + TEXT(".obj"));
			DoExportNavMesh(NavMeshFileM, EExportMode::Metre);
			CreateSaveFileNotify(NavMeshMsg, NavMeshFileM);
#endif
			FString NavDataFile = FPaths::Combine(OutPath, MapName + TEXT("-NavData-") + CurrentTime+TEXT(".bin"));
			DoExportNavData(NavDataFile);

			FText NavDataMsg = LOCTEXT("SaveNavMeshData", "Successd to Export the RecastNavigation data.");
			CreateSaveFileNotify(NavDataMsg, NavDataFile);
		}
	}
}

void FExportNavEditorModule::DoExportNavMesh(const FString& SaveToFile,EExportMode InExportMode)
{
	UFlibExportNavData::ExportRecastNavMesh(SaveToFile,InExportMode);
}

void FExportNavEditorModule::NotFountAnyValidNavDataMsg()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	FText DialogText = FText::Format(
		LOCTEXT("NotFoundValidNavDialogText", "Not found any valid Navigation data in {0} Map!"),
		FText::FromString(World->GetMapName())
	);
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FExportNavEditorModule::CreateSaveFileNotify(const FText& InMsg, const FString& InSavedFile)
{
	auto Message = InMsg;
	FNotificationInfo Info(Message);
	Info.bFireAndForget = true;
	Info.ExpireDuration = 5.0f;
	Info.bUseSuccessFailIcons = false;
	Info.bUseLargeFont = false;

	const FString HyperLinkText = InSavedFile;
	Info.Hyperlink = FSimpleDelegate::CreateStatic(
		[](FString SourceFilePath)
	{
		FPlatformProcess::ExploreFolder(*SourceFilePath);
	},
		HyperLinkText
		);
	Info.HyperlinkText = FText::FromString(HyperLinkText);

	FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Success);
}

void FExportNavEditorModule::DoExportNavData(const FString& SaveToFile)
{
	UFlibExportNavData::ExportRecastNavData(SaveToFile);

}

void FExportNavEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FExportNavCommands::Get().PluginAction);
}



void FExportNavEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FExportNavCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FExportNavEditorModule, ExportNavEditor)