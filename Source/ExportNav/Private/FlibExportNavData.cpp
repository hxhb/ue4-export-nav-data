
#include "FlibExportNavData.h"
#include "ExternRecastNavMeshGenetator.h"
#include "Editor.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "AI/NavDataGenerator.h"

bool UFlibExportNavData::ExecExportNavData(const FString& SaveFile)
{
	FString FinalSaveFile=SaveFile;

	UWorld* World = GEditor->GetEditorWorldContext(false).World();  

	if (World->GetNavigationSystem())
	{
		if (ANavigationData* NavData = Cast<ANavigationData>(World->GetNavigationSystem()->GetMainNavData()))
		{
			if (FExternExportNavMeshGenerator* Generator = static_cast<FExternExportNavMeshGenerator*>(NavData->GetGenerator()))
			{
				if (SaveFile.IsEmpty())
				{
					const FString Name = NavData->GetName();
					FinalSaveFile = FPaths::Combine(FPaths::ProjectSavedDir(), Name);
				}
				Generator->ExternExportNavigationData(FinalSaveFile);
				return true;
			}
		}
	}
	return false;
}