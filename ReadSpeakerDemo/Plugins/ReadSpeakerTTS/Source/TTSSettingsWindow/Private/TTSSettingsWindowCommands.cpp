// Copyright 2022 ReadSpeaker AB. All Rights Reserved.

#include "TTSSettingsWindowCommands.h"

#define LOCTEXT_NAMESPACE "FTTSSettingsWindowModule"

void FTTSSettingsWindowCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ReadSpeaker TTS", "Bring up ReadSpeaker TTS Settings window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
