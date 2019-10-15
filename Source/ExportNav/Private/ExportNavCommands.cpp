// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ExportNavCommands.h"

#define LOCTEXT_NAMESPACE "FExportNavModule"

void FExportNavCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Export Nav", "Export Navigation Data", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
