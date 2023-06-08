// Copyright 2022 ReadSpeaker AB. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "TTSSettingsWindowStyle.h"

class FTTSSettingsWindowCommands : public TCommands<FTTSSettingsWindowCommands>
{
public:

	FTTSSettingsWindowCommands()
		: TCommands<FTTSSettingsWindowCommands>(TEXT("TTSSettingsWindow"), NSLOCTEXT("Contexts", "ReadSpeaker TTS Settings", "TTSSettingsWindow Plugin"), NAME_None, FTTSSettingsWindowStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};