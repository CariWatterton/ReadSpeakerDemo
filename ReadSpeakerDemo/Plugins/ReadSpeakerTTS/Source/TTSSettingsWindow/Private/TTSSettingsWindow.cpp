// Copyright 2022 ReadSpeaker AB. All Rights Reserved.

#include "TTSSettingsWindow.h"
#include <stdlib.h>
#include <regex>
#include <fstream>
#include <sstream>
#include "TTSSettingsWindowStyle.h"
#include "TTSSettingsWindowCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Images/SImage.h"
#include "../../../../ReadSpeakerTTS/Source/ReadSpeakerTTS/Public/ReadSpeakerTTS.h"
#include "ToolMenus.h"
#include "Interfaces/IPluginManager.h"

static const FName TTSSettingsWindowTabName("ReadSpeaker TTS");

#define LOCTEXT_NAMESPACE "FTTSSettingsWindowModule"


void FTTSSettingsWindowModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FTTSSettingsWindowStyle::Initialize();
	FTTSSettingsWindowStyle::ReloadTextures();

	FTTSSettingsWindowCommands::Register();

	FTTSSettingsWindowModule savedModule = Load();
	savedModule.Save();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FTTSSettingsWindowCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FTTSSettingsWindowModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FTTSSettingsWindowModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TTSSettingsWindowTabName, FOnSpawnTab::CreateRaw(this, &FTTSSettingsWindowModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FTTSSettingsWindowTabTitle", "ReadSpeaker TTS"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FTTSSettingsWindowModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FTTSSettingsWindowStyle::Shutdown();

	FTTSSettingsWindowCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TTSSettingsWindowTabName);
}

TSharedRef<SDockTab> FTTSSettingsWindowModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = LOCTEXT("WindowWidgetText", "Scan for installed voice engines");
	
	FTTSSettingsWindowModule savedModule = Load();

	TSharedPtr<FTTSSettingsWindowModule> modulePtr = MakeShared<FTTSSettingsWindowModule>(savedModule);

	FString BaseDir = IPluginManager::Get().FindPlugin("ReadSpeakerTTS")->GetBaseDir();
	FString logoPath = FPaths::Combine(*BaseDir, TEXT("Resources/logo_readspeaker-1.png"));
	FSlateImageBrush img(TCHAR_TO_UTF8(*logoPath), FVector2D(256, 256), FSlateColor());

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(10)[
				SNew(STextBlock).Text(LOCTEXT("GeneralSettingsText", "General Settings"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(20,0)
				[
					SNew(SHorizontalBox) + SHorizontalBox::Slot().VAlign(VAlign_Top).HAlign(HAlign_Left).AutoWidth()[
						SNew(STextBlock).Text(LOCTEXT("VerboseDebugText", "Verbose Debug"))
					]
					+ SHorizontalBox::Slot().VAlign(VAlign_Top).HAlign(HAlign_Left).Padding(15, 0)[
						SNew(SCheckBox).IsChecked(modulePtr.ToSharedRef().Get().verboseDebug ? ECheckBoxState::Checked : ECheckBoxState::Unchecked).OnCheckStateChanged_Lambda([&, modulePtr](ECheckBoxState st) -> void {
							if (st == ECheckBoxState::Checked) {
								modulePtr.ToSharedRef().Get().verboseDebug = true;
							}
							else {
								modulePtr.ToSharedRef().Get().verboseDebug = false;
							}
							modulePtr.ToSharedRef().Get().Save();
						})
					]	
				]
	+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(20, 0)
		[
			SNew(SBox).HeightOverride(25)[
				SNew(SButton)
					.Text(WidgetText)
					.OnClicked_Lambda([]()->FReply {return FTTSSettingsWindowModule::ScanForVoiceEngines(); })
			]
		]
	+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(10)[
		SNew(STextBlock).Text(LOCTEXT("ExportSettingText", "Export Settings"))
	]
			+ SVerticalBox::Slot().AutoHeight().Padding(20, 0)[
				SNew(SBox).HAlign(HAlign_Left).VAlign(VAlign_Top).HeightOverride(100)[
					SNew(STTSExportGrid).TTSSettingsWindowModule(modulePtr)
				]
			]
		];
}

FReply FTTSSettingsWindowModule::ScanForVoiceEngines() {
	FReadSpeakerTTSModule::RefreshVoiceEngineInstallations();
	FTTSSettingsWindowModule currentSettings = FTTSSettingsWindowModule::Load();
	TArray<FTTSVoiceExportSettings> preliminaryExportSettings = FTTSSettingsWindowModule::GetPreliminaryExportSettings();
	currentSettings.SynchronizeExportSettings(preliminaryExportSettings);
	currentSettings.Save();
	FGlobalTabmanager::Get()->FindExistingLiveTab(TTSSettingsWindowTabName)->RequestCloseTab();
	FGlobalTabmanager::Get()->TryInvokeTab(TTSSettingsWindowTabName);
	return FReply::Handled();
}

TArray<FTTSVoiceExportSettings> FTTSSettingsWindowModule::GetPreliminaryExportSettings() {

	TArray<FTTSVoiceExportSettings> PreliminarySettings;

	FString BaseDir = IPluginManager::Get().FindPlugin("ReadSpeakerTTS")->GetBaseDir();
	FString SavePath = FPaths::Combine(*BaseDir, TEXT("Resources/TTSSettings.ini"));

	FString WinLinuxVtPath = FPaths::Combine(*BaseDir, TEXT("Resources/ReadSpeaker/WinLinux/vtpath.ini"));
	FString AndroidVtPath = FPaths::Combine(*BaseDir, TEXT("Resources/ReadSpeaker/Android/vtpath.ini"));
	FString PS4Path = FPaths::Combine(*BaseDir, TEXT("Resources/ReadSpeaker/PS4/vtpath.ini"));
	
	std::ifstream winLinuxVtPathInStream = std::ifstream(TCHAR_TO_UTF8(*WinLinuxVtPath));
	std::ifstream androidPathInStream = std::ifstream(TCHAR_TO_UTF8(*AndroidVtPath));

	std::string result;
	
	while (std::getline(winLinuxVtPathInStream, result)) {
		const std::regex pieces_regex("packname (.+)_(.+)_setup_engine");
		std::smatch pieces_match;
		if (std::regex_search(result, pieces_match, pieces_regex)) {
			if (pieces_match.size() == 3) {
				std::string id = pieces_match[1].str() + " " + pieces_match[2].str();
				std::string name = pieces_match[1].str();
				std::string type = pieces_match[2].str();
				name[0] = std::toupper(name[0]);
				type[0] = std::toupper(type[0]);
				std::string niceID = name + " " + type;
				FString EngineID(niceID.c_str());
				FTTSVoiceExportSettings setting = FTTSVoiceExportSettings(EngineID, PlatformFlag::Used, PlatformFlag::Used, PlatformFlag::Unavailable, PlatformFlag::Unavailable, PlatformFlag::Unavailable);
				PreliminarySettings.Add(setting);
			}
		}
	}
	while (std::getline(androidPathInStream, result)) {
		const std::regex pieces_regex("packname (.+)_(.+)_setup_engine");
		std::smatch pieces_match;
		if (std::regex_search(result, pieces_match, pieces_regex)) {
			if (pieces_match.size() == 3) {
				std::string id = pieces_match[1].str() + " " + pieces_match[2].str();
				std::string name = pieces_match[1].str();
				std::string type = pieces_match[2].str();
				name[0] = std::toupper(name[0]);
				type[0] = std::toupper(type[0]);
				std::string niceID = name + " " + type;
				FString EngineID(niceID.c_str());
				int index = -1;
				for (int i = 0; i < PreliminarySettings.Num(); i++) {
					if (PreliminarySettings[i].id == EngineID) {
						index = i;
					}
				}
				if (index >= 0) {
					PreliminarySettings[index].android = PlatformFlag::Used;
				}
				else {
					FTTSVoiceExportSettings setting = FTTSVoiceExportSettings(EngineID, PlatformFlag::Unavailable, PlatformFlag::Unavailable, PlatformFlag::Used, PlatformFlag::Unavailable, PlatformFlag::Unavailable);
					PreliminarySettings.Add(setting);
				}
			}
		}
	}
	winLinuxVtPathInStream.close();
	androidPathInStream.close();
	return PreliminarySettings;
}

void FTTSSettingsWindowModule::SynchronizeExportSettings(TArray<FTTSVoiceExportSettings> preliminaryExportSettings) {
	for (int i = 0; i < preliminaryExportSettings.Num(); i++) {
		for (int j = 0; j < exportSettings.Num(); j++) {
			FTTSVoiceExportSettings preliminarySetting = preliminaryExportSettings[i];
			FTTSVoiceExportSettings currentSetting = exportSettings[j];

			if (currentSetting.id == preliminarySetting.id) {
				if (preliminarySetting.winx64 != PlatformFlag::Unavailable && currentSetting.winx64 != PlatformFlag::Unavailable) {
					preliminarySetting.winx64 = currentSetting.winx64;
				}
				if (preliminarySetting.linuxx64 != PlatformFlag::Unavailable && currentSetting.linuxx64 != PlatformFlag::Unavailable) {
					preliminarySetting.linuxx64 = currentSetting.linuxx64;
				}
				if (preliminarySetting.android != PlatformFlag::Unavailable && currentSetting.android != PlatformFlag::Unavailable) {
					preliminarySetting.android = currentSetting.android;
				}
			}
			preliminaryExportSettings[i] = preliminarySetting;
		}
	}

	// Sort settings alphabetically by first character in ID.
	preliminaryExportSettings.Sort([](const FTTSVoiceExportSettings& stn1, const FTTSVoiceExportSettings& stn2) {
		return stn1.id < stn2.id;
	});
	
	exportSettings = preliminaryExportSettings;
}

FTTSSettingsWindowModule FTTSSettingsWindowModule::Load()
{
	FString BaseDir = IPluginManager::Get().FindPlugin("ReadSpeakerTTS")->GetBaseDir();
	FString SavePath = FPaths::Combine(*BaseDir, TEXT("Resources/TTSSettings.ini"));
	FTTSSettingsWindowModule settingsModule;
	std::regex verbosedebug_regex("verboseDebug\\:([a-z]+)");
	std::regex exportflags_regex("export\\:\\{id\\=(.+)\\, flags\\=([0-2]+)\\}");
	std::ifstream inStream = std::ifstream(TCHAR_TO_UTF8(*SavePath));
	std::string result;
	while (std::getline(inStream, result)) {
		std::smatch pieces_match;
		if (std::regex_search(result, pieces_match, verbosedebug_regex) && pieces_match.size() == 2) {
			settingsModule.verboseDebug = pieces_match[1].str() == "true";
		}
		else if (std::regex_search(result, pieces_match, exportflags_regex)) {
			FString ID = FString(pieces_match[1].str().c_str());
			std::string flags = pieces_match[2].str();
			TArray<PlatformFlag> flagsToSet;
			for (int i = 0; i < flags.length(); i++) {
				char flag = flags[i];
				PlatformFlag toSet = PlatformFlag::Unavailable;
				if (flag == '0') {
					toSet = PlatformFlag::Unavailable;
				}
				else if (flag == '1') {
					toSet = PlatformFlag::Unused;
				}
				else if (flag == '2') {
					toSet = PlatformFlag::Used;
				}
				flagsToSet.Add(toSet);
			}
			settingsModule.exportSettings.Add(FTTSVoiceExportSettings(ID, flagsToSet[0], flagsToSet[1], flagsToSet[2], (PlatformFlag)0, (PlatformFlag)0));
		}
	}

	inStream.close();

	return settingsModule;
}

void FTTSSettingsWindowModule::PrintSetting(FTTSVoiceExportSettings setting) {
	UE_LOG(LogTemp, Warning, TEXT("READSPEAKER SETTING MODULE, SETTING=%s, %d, %d, %d"), *setting.id, setting.winx64, setting.linuxx64, setting.android);
}

FTTSVoiceExportSettings::FTTSVoiceExportSettings(FString id, PlatformFlag winx64flag, PlatformFlag linuxx64flag, PlatformFlag androidflag, PlatformFlag ps4flag, PlatformFlag ps5flag) {
	this->id = id;
	this->winx64 = winx64flag;
	this->linuxx64 = linuxx64flag;
	this->android = androidflag;
	this->ps4 = ps4flag;
	this->ps5 = ps5flag;
}

void FTTSSettingsWindowModule::Save()
{
	FString BaseDir = IPluginManager::Get().FindPlugin("ReadSpeakerTTS")->GetBaseDir();
	FString SavePath = FPaths::Combine(*BaseDir, TEXT("Resources/TTSSettings.ini"));

	std::ofstream outStream = std::ofstream(TCHAR_TO_UTF8(*SavePath));

	outStream << (verboseDebug ? "verboseDebug:true" : "verboseDebug:false") << std::endl;

	for (int i = 0; i < exportSettings.Num(); i++) {
		FTTSVoiceExportSettings exportSetting = exportSettings[i];
		FString flagString = FString::Printf(TEXT("%d%d%d%d%d"), (int)exportSetting.winx64, (int)exportSetting.linuxx64, (int)exportSetting.android, (int)exportSetting.ps4, (int)exportSetting.ps5);
		FString theString = FString::Printf(TEXT("export:{id=%s, flags=%s}"), *exportSetting.id, *flagString);
		outStream << TCHAR_TO_UTF8(*theString) << std::endl;
	}
	outStream.close();
}

void FTTSSettingsWindowModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(TTSSettingsWindowTabName);
}

void FTTSSettingsWindowModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FTTSSettingsWindowCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FTTSSettingsWindowCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTTSSettingsWindowModule, TTSSettingsWindow)
