// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#include "ExportNavCommands.h"

#define LOCTEXT_NAMESPACE "FExportNavModule"

void FExportNavCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Export Nav", "Export Navigation Data", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
