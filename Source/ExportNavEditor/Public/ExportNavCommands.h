// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ExportNavStyle.h"

class FExportNavCommands : public TCommands<FExportNavCommands>
{
public:

	FExportNavCommands()
		: TCommands<FExportNavCommands>(TEXT("ExportNav"), NSLOCTEXT("Contexts", "ExportNav", "ExportNav Plugin"), NAME_None, FExportNavStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
