// Copyright 2022 ReadSpeaker AB. All Rights Reserved.

#include "ReadSpeakerTTS.h"
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>
#include <vector>
#if PLATFORM_ANDROID
#include <dlfcn.h>
#endif
#include "Async/Async.h"
#include "AudioDevice.h"
#include "Components/AudioComponent.h"
#include "Core.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ScopeTryLock.h"
#include "Modules/ModuleManager.h"
#include "RenderCore.h"
#include "SampleBuffer.h"
#include "Sound/SampleBufferIO.h"
#include "Sound/SoundSubmix.h"
#include "Sound/SoundWaveProcedural.h"
#include "Templates/Function.h"
#if WITH_EDITOR
#include "ClassIconFinder.h"
#include "DetailLayoutBuilder.h"
#include "Editor/EditorEngine.h"
#include "IDetailCustomization.h"
#include "LevelEditor.h"
#include "PropertyEditing.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#endif
#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

extern "C" {
#include "TTSLibrary/rsgame.h"
}

FReadSpeakerTTSModule::LoggingLevel FReadSpeakerTTSModule::Logging = FReadSpeakerTTSModule::LoggingLevel::Undefined;
FCriticalSection FReadSpeakerTTSModule::AndroidMutex;
FOnPauseAll FReadSpeakerTTSModule::FOnPauseAllDelegate;
FOnResumeAll FReadSpeakerTTSModule::FOnResumeAllDelegate;
FOnInterruptAll FReadSpeakerTTSModule::FOnInterruptAllDelegate;
static TArray<UTTSEngine*> Engines;

#define LOCTEXT_NAMESPACE "FReadSpeakerTTSModule"

#if WITH_EDITOR

TSharedRef< IDetailCustomization > FTTSSpeakerCustomization::MakeInstance()
{
    return MakeShareable(new FTTSSpeakerCustomization);
}

void FTTSSpeakerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray< TWeakObjectPtr< UObject > > Objects;
    DetailBuilder.GetObjectsBeingCustomized(Objects);

    TSharedRef< IPropertyHandle > EngineIDProp = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UTTSSpeaker, EngineID));
    TSharedRef< IPropertyHandle > VolumeProp = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UTTSSpeaker, Volume));
    TSharedRef< IPropertyHandle > PitchProp = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UTTSSpeaker, Pitch));
    TSharedRef< IPropertyHandle > SpeedProp = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UTTSSpeaker, Speed));
    TSharedRef< IPropertyHandle > PauseProp = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UTTSSpeaker, Pause));
    TSharedRef< IPropertyHandle > CommaPauseProp = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UTTSSpeaker, CommaPause));

    DetailBuilder.HideProperty(EngineIDProp);

    if (Objects.Num() != 1)
    {
        return;
    }

    TWeakObjectPtr<UTTSSpeaker> MyObject = Cast<UTTSSpeaker>(Objects[0].Get());
    IDetailCategoryBuilder& Cat = DetailBuilder.EditCategory(TEXT("Characteristics"));

    auto OnRegenerate = [&DetailBuilder]
    {
        DetailBuilder.ForceRefreshDetails();
    };

    Cat.AddCustomRow(LOCTEXT("TTSSpeaker", "Speaker"))
        .WholeRowContent()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("VoiceNameText", "Voice"))
        ]
    + SHorizontalBox::Slot()
        [
            SNew(STTSVoiceCombo).SpeechCharacteristics(MyObject).OnStateChanged_Lambda(OnRegenerate)
        ]
        ];

    UTTSEngine* Engine = FReadSpeakerTTSModule::GetEngineByID(MyObject->EngineID);
    if (Engine != NULL) {
        FString EngineName = Engine->Name;
        FString EngineType = Engine->Type;
        FString EngineLang = Engine->Language;
        FString EngineGender = Engine->Gender;

        Cat.AddCustomRow(LOCTEXT("TTSSpeaker", "Speaker")).WholeRowContent()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().Padding(5)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()[
                    SNew(STextBlock).Text(FText::FromString("Name: " + EngineName))
                ]
            + SHorizontalBox::Slot()[
                SNew(STextBlock).Text(FText::FromString("Type: " + EngineType))
            ]
            ]
        + SVerticalBox::Slot().Padding(5)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()[
                    SNew(STextBlock).Text(FText::FromString("Language: " + EngineLang))
                ]
            + SHorizontalBox::Slot()[
                SNew(STextBlock).Text(FText::FromString("Gender: " + EngineGender))
            ]
            ]
            ];
    }
    

    if (GEditor && !GEditor->PlayWorld) {
        Cat.AddProperty(VolumeProp);
        Cat.AddProperty(PitchProp);
        Cat.AddProperty(SpeedProp);
        Cat.AddProperty(PauseProp);
        Cat.AddProperty(CommaPauseProp);

        auto OnPreview = [MyObject]
        {
            if (MyObject.IsValid())
            {
                MyObject->Preview();
            }
            return FReply::Handled();
        };


        Cat.AddCustomRow(LOCTEXT("PreviewButton", "Preview"))
            .WholeRowContent()
            [
                SNew(SButton)
                .Text(LOCTEXT("PreviewButtonText", "Preview"))
            .OnClicked_Lambda(OnPreview)
            ];
    }
    else {
        DetailBuilder.HideProperty(VolumeProp);
        DetailBuilder.HideProperty(PitchProp);
        DetailBuilder.HideProperty(SpeedProp);
        DetailBuilder.HideProperty(PauseProp);
        DetailBuilder.HideProperty(CommaPauseProp);

        Cat.AddCustomRow(LOCTEXT("TTSSpeaker", "Speaker")).WholeRowContent()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().Padding(5)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()[
                    SNew(STextBlock).Text(FText::FromString("Exit Play Mode to customize Speaker properties."))
                ]
            ]
            ];
    }
}

void UTTSSpeaker::Preview() {
    FReadSpeakerTTSModule::Init();
    Engine = FReadSpeakerTTSModule::GetEngineByID(EngineID);
    UTTSConverter* converter = NewObject<UTTSConverter>();
    converter->Text = "I sound like this";
    converter->Volume = Volume;
    converter->Pitch = Pitch;
    converter->Speed = Speed;
    converter->Pause = Pause;
    converter->CommaPause = CommaPause;
    converter->TextType = TTSTextType::Normal;
    converter->Engine = Engine;
    converter->ConvertToBuffer();
    USoundWaveProceduralTTS *SoundWave = NewObject<USoundWaveProceduralTTS>();
    SoundWave->TTSData = converter->GetAudioData();
    SoundWave->FillAudioQueue(converter->GetAudioData().Num());
    UGameplayStatics::PlaySound2D(GEditor->GetEditorWorldContext().World(), SoundWave, 1, 1, 0);
}
#endif

void FReadSpeakerTTSModule::StartupModule()
{
    FString BaseDir = IPluginManager::Get().FindPlugin("ReadSpeakerTTS")->GetBaseDir();
#if PLATFORM_WINDOWS
    FString LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/TTSLibrary/Binaries/Win64/libvtapi.dll"));
    VTAPILibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

    if (!VTAPILibraryHandle)
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load VTAPI."));
    }

    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/TTSLibrary/Binaries/Win64/rsgame.dll"));
    RSGameLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

    if (!RSGameLibraryHandle)
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load RSGame."));
    }

#elif PLATFORM_LINUX
    FString LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/TTSLibrary/Binaries/Linux/libvtapi.so"));
    VTAPILibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

    if (!VTAPILibraryHandle)
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load VTAPI."));
    }

    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/TTSLibrary/Binaries/Linux/librsgame.so"));
    RSGameLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

    if (!RSGameLibraryHandle)
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load RSGame."));
    }
#endif
#if WITH_EDITOR
    auto& PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(
        "TTSSpeaker",
        FOnGetDetailCustomizationInstance::CreateStatic(&FTTSSpeakerCustomization::MakeInstance)
    );
    PropertyModule.NotifyCustomizationModuleChanged();

    StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
    StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

    FString IconPath = FPaths::Combine(*BaseDir, TEXT("Resources/ButtonIcon_40x.png"));

    StyleSet->Set("ClassIcon.TTSSpeaker", new FSlateImageBrush(IconPath, FVector2D(16.0f, 16.0f)));

    FSlateStyleRegistry::RegisterSlateStyle(StyleSet.Get());
#endif
}


void FReadSpeakerTTSModule::ShutdownModule()
{
#if WITH_EDITOR
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomClassLayout("SpeechCharacteristics");
        PropertyModule.UnregisterCustomClassLayout("TTSSpeaker");
    }
    
    FSlateStyleRegistry::UnRegisterSlateStyle(StyleSet.Get());
    ensure(StyleSet.IsUnique());
#endif
    FPlatformProcess::FreeDllHandle(VTAPILibraryHandle);
    FPlatformProcess::FreeDllHandle(RSGameLibraryHandle);
    VTAPILibraryHandle = nullptr;
    RSGameLibraryHandle = nullptr;
}

void FReadSpeakerTTSModule::GetEngines(TArray<UTTSEngine*> *EngineArr) {
#if PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        static jmethodID GetEngineCountMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetEngineCountReadSpeaker", "()I", false);
        static jmethodID GetEngineInfoOfIndexMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetEngineInfoOfIndexReadSpeaker", "(I)[Ljava/lang/String;", false);
        int engineCount = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, GetEngineCountMethod);
        FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Number of installed engines is: %d"), engineCount));
        for (int i = 0; i < engineCount; i++) {
            jint index = i;
            jobjectArray arr = (jobjectArray)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, GetEngineInfoOfIndexMethod, index);
            std::vector<FString> engineFields;
            int fieldCount = Env->GetArrayLength(arr);
            for (int j = 0; j < fieldCount; j++) {
                jstring obj = (jstring)Env->GetObjectArrayElement(arr, j);
                const char* raw = Env->GetStringUTFChars(obj, 0);
                FString fstr = FString(UTF8_TO_TCHAR(raw));
                FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Engine field %d is %s"), j, *fstr));
                engineFields.push_back(fstr);
            }
            if (engineFields.size() < 4) {
                FReadSpeakerTTSModule::Log(TEXT("Corrupted VT engine information"));
            }
            else {
                RecieveEngineCallback((void*)EngineArr, TCHAR_TO_UTF8(*engineFields[0]), TCHAR_TO_UTF8(*engineFields[1]), TCHAR_TO_UTF8(*engineFields[2]), TCHAR_TO_UTF8(*engineFields[3]), NULL, NULL, -1, -1);
            }
        }
    }
#else
    RSGame_GetEngines(&RecieveEngineCallback, (void*)EngineArr);
#endif
}

void FReadSpeakerTTSModule::RecieveEngineCallback(void* context, char* speaker, char* type, char* language, char* gender, char* dbPath, char* version, int sampling, int channels) {
    TArray<UTTSEngine*>* EngineArr = reinterpret_cast<TArray<UTTSEngine*>*>(context);
    UTTSEngine* eng = NewObject<UTTSEngine>();
    eng->AddToRoot();
    eng->Name = speaker;
    eng->Type = type;
    eng->Language = language;
    eng->Gender = gender;
    eng->Version = version;
    eng->Sampling = sampling;
    eng->Channels = channels;
    std::string niceName(speaker);
    std::string niceType(type);
    niceName[0] = std::toupper(niceName[0], std::locale());
    niceType[0] = std::toupper(niceType[0], std::locale());
    std::string niceID = std::string(niceName + " " + niceType);
    eng->ID = niceID.c_str();
    for (int i = 0; i < Engines.Num(); i++) {
        UTTSEngine* comp = Engines[i];
        if (comp->Name.Equals(eng->Name) && comp->Type.Equals(eng->Type))
            return;
    }
    EngineArr->Add(eng);
}


void FReadSpeakerTTSModule::RefreshVoiceEngineInstallations() {
    FString BaseDir = IPluginManager::Get().FindPlugin("ReadSpeakerTTS")->GetBaseDir();
    FString winLinuxPath = FPaths::Combine(*BaseDir, TEXT("Resources/ReadSpeaker/WinLinux"));
    FString androidPath = FPaths::Combine(*BaseDir, TEXT("Resources/ReadSpeaker/Android"));
#if PLATFORM_ANDROID
    // Not yet supported
    return;
#else
#if PLATFORM_WINDOWS
    char* absWinLinuxPath = _fullpath(NULL, TCHAR_TO_ANSI(*winLinuxPath), _MAX_PATH);
    char* absAndroidPath = _fullpath(NULL, TCHAR_TO_ANSI(*androidPath), _MAX_PATH);
#elif PLATFORM_LINUX
    char* absWinLinuxPath = realpath(TCHAR_TO_UTF8(*winLinuxPath), NULL);
    char* absAndroidPath = realpath(TCHAR_TO_UTF8(*androidPath), NULL);
#endif
    winLinuxPath = UTF8_TO_TCHAR(absWinLinuxPath);
    androidPath = UTF8_TO_TCHAR(absAndroidPath);
    RSGame_CreateConfFile(TCHAR_TO_ANSI(*winLinuxPath), TCHAR_TO_ANSI(*winLinuxPath));
    RSGame_CreateConfFile(TCHAR_TO_ANSI(*androidPath), TCHAR_TO_ANSI(*androidPath));
    free(absWinLinuxPath);
    free(absAndroidPath);
#endif
}

bool FReadSpeakerTTSModule::IsVerbose() {
    if (Logging == LoggingLevel::Undefined) {
        FString BaseDir = IPluginManager::Get().FindPlugin("ReadSpeakerTTS")->GetBaseDir();
        FString SavePath = FPaths::Combine(*BaseDir, TEXT("Resources/TTSSettings.ini"));
        std::ifstream inStream = std::ifstream(TCHAR_TO_UTF8(*SavePath));
        std::string result;
        while (std::getline(inStream, result)) {
            const std::regex pieces_regex("verboseDebug:([a-z]+)");
            std::smatch pieces_match;
            if (std::regex_search(result, pieces_match, pieces_regex) && pieces_match.size() == 2) {
                std::string value = pieces_match[1];
                if (value == "true") {
                    Logging = LoggingLevel::Verbose;
                }
                else {
                    Logging = LoggingLevel::Off;
                }
            }
        }
    }
    return Logging == LoggingLevel::Verbose;
}

void FReadSpeakerTTSModule::Log(FString msg) {
#if PLATFORM_ANDROID
    UE_LOG(LogTemp, Warning, TEXT("%s"), *msg);
#else
    if (IsVerbose()) {
        UE_LOG(LogTemp, Log, TEXT("READSPEAKER TTS: %s"), *msg);
    }
#endif
}

void FReadSpeakerTTSModule::LogError(FString msg) {
    UE_LOG(LogTemp, Error, TEXT("READSPEAKER TTS: %s"), *msg);
}

READSPEAKERTTS_API UTTSEngine* FReadSpeakerTTSModule::GetEngine(FString name, FString type)
{
    for (int i = 0; i < Engines.Num(); i++) {
        UTTSEngine* eng = Engines[i];
        if (eng->Name.Equals(name) && eng->Type.Equals(type))
            return eng;
    }
    return nullptr;
}

READSPEAKERTTS_API UTTSEngine* FReadSpeakerTTSModule::GetEngineByID(FString id)
{
    for (int i = 0; i < Engines.Num(); i++) {
        UTTSEngine* eng = Engines[i];
        if (eng->ID.Equals(id))
            return eng;
    }
    return nullptr;
}

READSPEAKERTTS_API UTTSEngine* FReadSpeakerTTSModule::GetEngineWithLanguage(FString lang)
{
    for (int i = 0; i < Engines.Num(); i++) {
        UTTSEngine* eng = Engines[i];
        if (eng->Language.Equals(lang))
            return eng;
    }
    return nullptr;
}

READSPEAKERTTS_API UTTSEngine* FReadSpeakerTTSModule::GetEngineWithGender(FString gender)
{
    for (int i = 0; i < Engines.Num(); i++) {
        UTTSEngine* eng = Engines[i];
        if (eng->Gender.Equals(gender))
            return eng;
    }
    return nullptr;
}

READSPEAKERTTS_API UTTSEngine* FReadSpeakerTTSModule::GetEngineWithLanguageAndGender(FString lang, FString gender)
{
    for (int i = 0; i < Engines.Num(); i++) {
        UTTSEngine* eng = Engines[i];
        if (eng->Language.Equals(lang) && eng->Gender.Equals(gender))
            return eng;
    }
    return nullptr;
}

READSPEAKERTTS_API TArray<UTTSEngine*> FReadSpeakerTTSModule::GetEnginesWithLanguage(FString lang)
{
    TArray<UTTSEngine*> tmp;
    for (int i = 0; i < Engines.Num(); i++) {
        UTTSEngine* eng = Engines[i];
        if (eng->Language.Equals(lang))
            tmp.Add(eng);
    }
    return tmp;
}

READSPEAKERTTS_API TArray<UTTSEngine*> FReadSpeakerTTSModule::GetEnginesWithGender(FString gender)
{
    TArray<UTTSEngine*> tmp;
    for (int i = 0; i < Engines.Num(); i++) {
        UTTSEngine* eng = Engines[i];
        if (eng->Gender.Equals(gender))
            tmp.Add(eng);
    }
    return tmp;
}

READSPEAKERTTS_API TArray<UTTSEngine*> FReadSpeakerTTSModule::GetEnginesWithLanguageAndGender(FString lang, FString gender)
{
    TArray<UTTSEngine*> tmp;
    for (int i = 0; i < Engines.Num(); i++) {
        UTTSEngine* eng = Engines[i];
        if (eng->Language.Equals(lang) && eng->Gender.Equals(gender))
            tmp.Add(eng);
    }
    return tmp;
}

READSPEAKERTTS_API TArray<FString> FReadSpeakerTTSModule::GetAvailableLanguages()
{
    TArray<FString> tmp;
    for (int i = 0; i < Engines.Num(); i++) {
        FString lang = Engines[i]->Language;
        if (!tmp.Contains(lang))
            tmp.Add(lang);
    }
    return tmp;
}

READSPEAKERTTS_API TArray<FString> FReadSpeakerTTSModule::GetAvailableGendersForLanguage(FString lang)
{
    TArray<FString> tmp;
    for (int i = 0; i < Engines.Num(); i++) {
        UTTSEngine* eng = Engines[i];
        if (eng->Language.Equals(lang)) 
        {
            if (!tmp.Contains(eng->Gender))
                tmp.Add(eng->Gender);
        }
    }
    return tmp;
}

int FReadSpeakerTTSModule::LoadTTS(FString libPath, FString iniPath) {
#if PLATFORM_ANDROID
    return -1;
#else
    int ret = -1;
    RSGAME_CONF_HANDLE conf_handle = RSGame_CreateConfHandle(TCHAR_TO_ANSI(*iniPath));
    RSGAME_LIB_HANDLE lib_handle = RSGame_CreateLibHandle(TCHAR_TO_ANSI(*libPath));

    FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Conf_handle: %s, lib_hande: %s"), UTF8_TO_TCHAR(conf_handle), UTF8_TO_TCHAR(lib_handle)));

    ret = RSGame_Initialize(lib_handle, conf_handle);

    if (ret != 0) {
        FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("Failed to load TTS modules at %s, return code: %d"), *libPath, ret));
    }
    else {
        FString rsgame_version = FString(RSGame_GetVersion());
        FString vtapi_version = FString(RSGame_GetVTAPIVersion());
        FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Initialized succesfully. RSGame version=%s, VTAPI version=%s"), *rsgame_version, *vtapi_version));
    }
    return ret;
#endif
}

void FReadSpeakerTTSModule::ClearLastSession() {
#if PLATFORM_ANDROID
    return;
#else
    RSGame_Exit();
#endif
}

int FReadSpeakerTTSModule::Init() {

    Logging = LoggingLevel::Undefined;

    FReadSpeakerTTSModule::RefreshVoiceEngineInstallations();

#if PLATFORM_ANDROID

    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        extern FString GFilePathBase;
#if ENGINE_MAJOR_VERSION == 4
        FString BaseDir = GFilePathBase + FString("/UE4Game/") + FApp::GetName() + "/" + FApp::GetName() + FString("/Plugins/ReadSpeakerTTS");
#else
        FString BaseDir = GFilePathBase + FString("/UnrealGame/") + FApp::GetName() + "/" + FApp::GetName() + FString("/Plugins/ReadSpeakerTTS");
#endif
        FString iniPath = FPaths::Combine(*BaseDir, TEXT("Resources/ReadSpeaker/Android"));
        jstring iniPathFinal = Env->NewStringUTF(TCHAR_TO_UTF8(*iniPath));
        static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_InitReadSpeaker", "(Ljava/lang/String;Ljava/lang/String;Z)V", false);
        FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, iniPathFinal, iniPathFinal, true);
        return 0;
    }
    else {
        return -1;
    }
#else

    FString BaseDir = IPluginManager::Get().FindPlugin("ReadSpeakerTTS")->GetBaseDir();
    int ret = -1;

    FString libraryPath;
    FString iniPath;

#if PLATFORM_WINDOWS
    libraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/TTSLibrary/Binaries/Win64/"));
    iniPath = FPaths::Combine(*BaseDir, TEXT("Resources/ReadSpeaker/WinLinux/"));
    char* absLibraryPath = _fullpath(NULL, TCHAR_TO_ANSI(*libraryPath), 1000);
    char* absIniPath = _fullpath(NULL, TCHAR_TO_ANSI(*iniPath), 1000);
    libraryPath = UTF8_TO_TCHAR(absLibraryPath);
    iniPath = UTF8_TO_TCHAR(absIniPath);
#elif PLATFORM_LINUX
    FString relativeIniPath = FPaths::Combine(*BaseDir, TEXT("Resources/ReadSpeaker/WinLinux"));
    FString relativeLibPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/TTSLibrary/Binaries/Linux"));
    char* absLibraryPath = realpath(TCHAR_TO_UTF8(*relativeLibPath), NULL);
    char* absIniPath = realpath(TCHAR_TO_UTF8(*relativeIniPath), NULL);
    libraryPath = UTF8_TO_TCHAR(absLibraryPath);
    iniPath = UTF8_TO_TCHAR(absIniPath);
#endif

    FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Loading TTS with path: %s and %s"), *libraryPath, *iniPath));

    ret = LoadTTS(libraryPath, iniPath);

    Engines = TArray<UTTSEngine*>();
    FReadSpeakerTTSModule::GetEngines(&Engines);

#if PLATFORM_LINUX || PLATFORM_WINDOWS
    free(absLibraryPath);
    free(absIniPath);
#endif
    return ret;
#endif
}

void FReadSpeakerTTSModule::PauseAll() {
    FOnPauseAllDelegate.Broadcast();
    FReadSpeakerTTSModule::Log(FString::Printf(TEXT("----PAUSING ALL----")));
}

void FReadSpeakerTTSModule::ResumeAll() {
    FOnResumeAllDelegate.Broadcast();
    FReadSpeakerTTSModule::Log(FString::Printf(TEXT("----RESUMING ALL----")));
}

void FReadSpeakerTTSModule::InterruptAll() {
    FOnInterruptAllDelegate.Broadcast();
    FReadSpeakerTTSModule::Log(FString::Printf(TEXT("----INTERRUPTING ALL----")));
}

void FReadSpeakerTTSModule::BindSpeaker(UTTSSpeaker *speaker)
{
    FOnPauseAllDelegate.AddUObject(speaker, &UTTSSpeaker::PauseSpeaking);
    FOnResumeAllDelegate.AddUObject(speaker, &UTTSSpeaker::ResumeSpeaking);
    FOnInterruptAllDelegate.AddUObject(speaker, &UTTSSpeaker::InterruptSpeaking);
}

void FReadSpeakerTTSModule::UnbindSpeaker(UTTSSpeaker* speaker) {
    FOnPauseAllDelegate.RemoveAll(speaker);
    FOnResumeAllDelegate.RemoveAll(speaker);
    FOnInterruptAllDelegate.RemoveAll(speaker);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FReadSpeakerTTSModule, ReadSpeakerTTS)

UTTSEngine::UTTSEngine(const FObjectInitializer& ObjectInitializer) : UObject(ObjectInitializer) { }

int UTTSEngine::Acquire()
{
#if PLATFORM_ANDROID
    return -1;
#else
    int ret = 0;
    if (ReferenceCount == 0 && !KeepInMemory) {
        ret = RSGame_LoadEngine(TCHAR_TO_UTF8(*Name), TCHAR_TO_UTF8(*Type));
        if (ret != 0) {
            FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("Engine %s failed to load, return code %d"), *Name, ret));
        }
        else {
            FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Engine %s was loaded"), *Name));
        }
    }
    ReferenceCount++;
    return ret;
#endif
}

int UTTSEngine::Release()
{
#if PLATFORM_ANDROID
    return -1;
#else
    int ret = 0;
    ReferenceCount--;

    if (ReferenceCount == 0 && !KeepInMemory) {
        ret = RSGame_UnloadEngine(TCHAR_TO_UTF8(*Name), TCHAR_TO_UTF8(*Type));
        if (ret != 0) {
            FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("Engine %s failed to unload, return code %d"), *Name, ret));
        }
        else {
            FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Engine %s was unloaded"), *Name));
        }
    }
    return ret;
#endif
}

int UTTSEngine::LoadEngine()
{
#if PLATFORM_ANDROID
    return -1;
#else
    {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 1
        FScopeLock ScopeLock = FScopeLock(&(this->EngineMutex));
#else
        FScopeLock(&(this->EngineMutex));
#endif

        int ret = -1;
        if (ReferenceCount == 0) {
            ret = RSGame_LoadEngine(TCHAR_TO_UTF8(*Name), TCHAR_TO_UTF8(*Type));
            if (ret != 0) {
                FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("Engine %s failed load, return code %d"), *Name, ret));
            }
            else {
                FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Engine %s was loaded"), *Name));
            }
        }
        KeepInMemory = true;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 1
        ScopeLock.Unlock();
#endif
        return ret;
    }
#endif
}

int UTTSEngine::UnloadEngine()
{
#if PLATFORM_ANDROID
    return -1;
#else
    {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 1
        FScopeLock ScopeLock = FScopeLock(&(this->EngineMutex));
#else
        FScopeLock(&(this->EngineMutex));
#endif

        int ret = -1;
        if (ReferenceCount == 0) {
            ret = RSGame_UnloadEngine(TCHAR_TO_UTF8(*Name), TCHAR_TO_UTF8(*Type));
            if (ret != 0) {
                FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("Engine %s failed to unload, return code %d"), *Name, ret));
            }
            else {
                FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Engine %s was loaded"), *Name));
            }
        }
        KeepInMemory = false;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 1
        ScopeLock.Unlock();
#endif
        return 0;
    }
#endif
}

READSPEAKERTTS_API FString UTTSEngine::GetID() {
    return ID;
}

USoundWaveProceduralTTS::USoundWaveProceduralTTS(const FObjectInitializer& ObjectInitializer) : USoundWaveProcedural(ObjectInitializer)
{
    NumChannels = 1;
    bCanProcessAsync = false;
    bLooping = false;
    SampleRate = 16000;
    NumSamplesToGeneratePerCallback = 32768;
    bProcedural = true;
}

void USoundWaveProceduralTTS::SetAudioData(TArray<int16> data) {
    TTSData = data;
    Length = CalculateAudioDuration();
    RemainingDuration = Length;
}

double USoundWaveProceduralTTS::CalculateAudioDuration() {
    return (double)((double)TTSData.Num() / ((double)TTSSampleRate / (double)NumChannels / ((double)TTSBitDepth / 16)));
}

void USoundWaveProceduralTTS::FillAudioQueue(int32 SamplesRequested)
{
    const int32 QueuedSamples = GetAvailableAudioByteCount() / sizeof(int16);
    const int32 SamplesNeeded = SamplesRequested - QueuedSamples;

    SampleData.Reset(SamplesNeeded);

    for (int32 i = 0; i < SamplesNeeded && i < TTSData.Num(); ++i) {
        int16 sample = TTSData[i];
        SampleData.Add(sample);
    }

    QueueAudio((uint8*)(SampleData.GetData()), SampleData.Num() * sizeof(int16));
}

UTTSConverter::UTTSConverter(const FObjectInitializer& ObjectInitializer) : UObject(ObjectInitializer) {
    SoundWave = NewObject<USoundWaveProceduralTTS>();
}

void UTTSConverter::BeginDestroy() {
    if (Task != NULL) {
        Task->EnsureCompletion();
    }
    Super::BeginDestroy();
}

void UTTSConverter::audio_callback(void* context, char* data, int* length)
{
    UTTSConverter* converter = reinterpret_cast<UTTSConverter*>(context);
    converter->RecieveAudioCallback(data, length);
}

void UTTSConverter::word_callback(void* context, int* startPos, int* endPos, float* time, int* length) {
    UTTSConverter* converter = reinterpret_cast<UTTSConverter*>(context);
    converter->RecieveWordCallback(startPos, endPos, time, length);
}

void UTTSConverter::viseme_callback(void* context, short* visemeId, float* time, int* length) {
    UTTSConverter* converter = reinterpret_cast<UTTSConverter*>(context);
    converter->RecieveVisemeCallback(visemeId, time, length);
}

void UTTSConverter::mark_callback(void* context, char* markName, float* time, int* length) {
    UTTSConverter* converter = reinterpret_cast<UTTSConverter*>(context);
    converter->RecieveMarkCallback(markName, time, length);
}

void UTTSConverter::ConvertToBufferAsync() {
    Task = new FAsyncTask<FTTSSynthesizeTask>(TWeakObjectPtr<UTTSConverter>(this), false);
    Task->StartBackgroundTask();
}

void UTTSConverter::ConvertToBufferAsync_SyncInfo() {
    Task = new FAsyncTask<FTTSSynthesizeTask>(TWeakObjectPtr<UTTSConverter>(this), true);
    Task->StartBackgroundTask();
}

void UTTSConverter::ConvertToBuffer()
{
#if PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        jmethodID TextToBufferMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_TextToBufferReadSpeaker", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIIIII)[B", false);
        jstring text = Env->NewStringUTF(TCHAR_TO_UTF8(*Text));
        jstring engName = Env->NewStringUTF(TCHAR_TO_UTF8(*(Engine->Name)));
        jstring engType = Env->NewStringUTF(TCHAR_TO_UTF8(*(Engine->Type)));
        jbyteArray arr = (jbyteArray)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, TextToBufferMethod, text, engName, engType, Volume, Pitch, Speed, Pause, CommaPause, (int)TextType);
        int count = Env->GetArrayLength(arr);
        jbyte* b = Env->GetByteArrayElements(arr, 0);
        RecieveAudioCallback((char*)b, &count);
    }
#else
    int ret = -1;
    if (Engine == NULL) {
        FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("TTSEngine undefined")));
        return;
    }
    else {
        FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Synthesizing with voice %s"), *(Engine->ID)));

        ret = Engine->Acquire();

        if (ret != 0) {
            FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("Acquiring Engine %s failed, return code: %d"), *(Engine->ID), ret));
        }
        else {
            FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Acquiring Engine %s succeeded, current reference count: %d"), *(Engine->ID), Engine->ReferenceCount));
        }

        ret = RSGame_TextToBuffer(TCHAR_TO_UTF8(*Text), TCHAR_TO_UTF8(*Engine->Name), TCHAR_TO_UTF8(*Engine->Type), &audio_callback, Volume, Pitch, Speed, Pause, CommaPause, (int)TextType, (int)OutputFormat, (void*)this);

        if (ret != 0) {
            FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("TextToBuffer failed Engine=%s, Text=%s, return code: %d"), *(Engine->ID), *Text, ret));
        }
        else {
            FReadSpeakerTTSModule::Log(TEXT("TextToBuffer success"));
        }

        ret = Engine->Release();

        if (ret != 0) {
            FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("Release Engine %s failed, return code: %d"), *(Engine->ID), ret));
        }
        else {
            FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Releasing Engine %s succeeded, current reference count: %d"), *(Engine->ID), Engine->ReferenceCount));
        }
    }
#endif

    FinishedConverting = true;

    // Report back to gamethread when done.
    FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
        OnConversionFinished.Broadcast();
    }
    , TStatId(), nullptr, ENamedThreads::GameThread);
}

void UTTSConverter::ConvertToBuffer_SyncInfo()
{
#if PLATFORM_ANDROID
    FReadSpeakerTTSModule::LogError(TEXT("SyncInfo not supported on Android."));
    return;
#else
    int ret = -1;
    if (Engine == NULL) {
        FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("TTSEngine undefined")));
        return;
    }
    else {
        FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Synthesizing with voice %s"), *Engine->ID));

        ret = Engine->Acquire();

        if (ret != 0) {
            FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("Acquiring Engine %s failed, return code: %d"), *(Engine->ID), ret));
        }
        else {
            FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Acquiring Engine %s succeeded, current reference count: %d"), *(Engine->ID), Engine->ReferenceCount));
        }

        ret = RSGame_TextToBuffer_SyncInfo(TCHAR_TO_UTF8(*Text), TCHAR_TO_UTF8(*Engine->Name), TCHAR_TO_UTF8(*Engine->Type), &audio_callback, &word_callback, &viseme_callback, &mark_callback, Volume, Pitch, Speed, Pause, CommaPause, (int)TextType, (int)OutputFormat, (void*)this);

        if (ret != 0) {
            FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("TextToBuffer_SyncInfo failed Engine=%s, Text=%s, return code: %d"), *(Engine->ID), *Text, ret));
        }
        else {
            FReadSpeakerTTSModule::Log(TEXT("TextToBuffer success"));
        }

        ret = Engine->Release();

        if (ret != 0) {
            FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("Release Engine %s failed, return code: %d"), *(Engine->ID), ret));
        }
        else {
            FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Releasing Engine %s succeeded, current reference count: %d"), *(Engine->ID), Engine->ReferenceCount));
        }

        FinishedConverting = true;

        // Report back to gamethread when done.
        FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
            OnConversionFinished.Broadcast();
        }
        , TStatId(), nullptr, ENamedThreads::GameThread);
    }
#endif
}

void UTTSConverter::RecieveAudioCallback(char* data, int* length) {
    TArray<int16> RecievedData;
    RecievedData.Append((int16*)data, *length / sizeof(int16));
    AudioData.Append(RecievedData);
}

void UTTSConverter::RecieveWordCallback(int* startPos, int* endPos, float* time, int* length) {
    OnWord.Broadcast(*startPos, *endPos, *time);
}

void UTTSConverter::RecieveVisemeCallback(short* visemeId, float* time, int* length) {
    AddVisemeToQueue(*visemeId, *time);
    OnViseme.Broadcast(*visemeId, *time);
}

void UTTSConverter::RecieveMarkCallback(char* markName, float* time, int* length) {
    OnMark.Broadcast(FString(markName), *time);
}

void UTTSConverter::AddVisemeToQueue(int visemeId, float timestamp) {
    VisemeEvent viseme_event;
    viseme_event.timestamp = timestamp;
    viseme_event.viseme_id = visemeId;
    SoundWave->VisemeEvents.Enqueue(viseme_event);
}

TArray<int16> UTTSConverter::GetAudioData()
{
    if (FinishedConverting) {
        return AudioData;
    }
    else {
        return TArray<int16>();
    }
}

void UTTSConverter::Play()
{
    SoundWave->TTSSampleRate = Engine->Sampling;
    SoundWave->TTSBitDepth = OutputFormat == TTSOutputFormat::PCM16 ? 16 : 8;
    SoundWave->SetAudioData(GetAudioData());
    SoundWave->FillAudioQueue(GetAudioData().Num());;
    if (AudioComponent->IsValidLowLevel()) {
        AudioComponent->AdjustAttenuation(*SoundAttenuationSettings);
        AudioComponent->Sound = SoundWave;
        AudioComponent->Play(0.0f);
    }
    else {
        FReadSpeakerTTSModule::LogError(FString::Printf(TEXT("No audio component set, set AudioComponent on UTTSConverter first.")));
    }
}

UTTSSpeaker::UTTSSpeaker(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UTTSSpeaker::BeginPlay() {
    FReadSpeakerTTSModule::BindSpeaker(this);
    Super::BeginPlay();
}

void UTTSSpeaker::BeginDestroy() {
    FReadSpeakerTTSModule::UnbindSpeaker(this);
    Super::BeginDestroy();
}

void UTTSSpeaker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
    // Check if we are currently playing a sound, if we're not paused, reduce the remaining duration.
    // If finished playing, broadcast it.
    if (ThisAudioComponent != NULL && ThisAudioComponent->Sound != NULL && ThisAudioComponent->GetPlayState() == EAudioComponentPlayState::Playing) {
        USoundWaveProceduralTTS* CurrentSound = (USoundWaveProceduralTTS*)ThisAudioComponent->Sound;
        CurrentSound->RemainingDuration -= DeltaTime;
        float elapsed = CurrentSound->Length - CurrentSound->RemainingDuration;

        VisemeEvent* top_event = CurrentSound->VisemeEvents.Peek();

        if (top_event != nullptr && top_event->timestamp <= elapsed) {
            VisemeEvent ev;
            if (CurrentSound->VisemeEvents.Dequeue(ev)) {
                CurrentVisemeID = ev.viseme_id;
            }
        }
        if (CurrentSound->RemainingDuration <= 0) {
            FinishedSpeaking();
        }
    }
}

void UTTSSpeaker::SetEngine(UTTSEngine* NewEngine) {
    Engine = NewEngine;
    EngineID = NewEngine->ID;
}

UTTSEngine* UTTSSpeaker::GetEngine()
{
    if (Engine == NULL || Engine->ID != EngineID) {
        Engine = FReadSpeakerTTSModule::GetEngineByID(EngineID);
    }
    return Engine;
}

void UTTSSpeaker::SetAudioComponent(UAudioComponent* AudioComponent) {
    ThisAudioComponent = AudioComponent;
}

UAudioComponent* UTTSSpeaker::GetAudioComponent() {
    return ThisAudioComponent;
}

void UTTSSpeaker::SetVolume(int NewVolume)
{
    Volume = NewVolume;
}

int UTTSSpeaker::GetVolume()
{
    return Volume;
}

void UTTSSpeaker::SetPitch(int NewPitch)
{
    Pitch = NewPitch;
}

int UTTSSpeaker::GetPitch()
{
    return Pitch;
}

void UTTSSpeaker::SetSpeed(int NewSpeed)
{
    Speed = NewSpeed;
}

int UTTSSpeaker::GetSpeed()
{
    return Speed;
}

void UTTSSpeaker::SetPause(int NewPause)
{
    Pause = NewPause;
}

int UTTSSpeaker::GetPause()
{
    return Pause;
}

void UTTSSpeaker::SetCommaPause(int NewCommaPause)
{
    CommaPause = NewCommaPause;
}

int UTTSSpeaker::GetCommaPause()
{
    return CommaPause;
}

int UTTSSpeaker::GetCurrentViseme() {
    return CurrentVisemeID;
}

bool UTTSSpeaker::IsSpeaking() {
    return ThisAudioComponent != NULL && ThisAudioComponent->Sound != NULL && ThisAudioComponent->GetPlayState() == EAudioComponentPlayState::Playing;
}

void UTTSSpeaker::StartedSpeaking() {
    FReadSpeakerTTSModule::Log(FString::Printf(TEXT("%s started speaking"), *(GetOwner()->GetName())));
    FString SpokenText = Converter->Text;
    TTSTextType Type = Converter->TextType;
    OnSpeakingStarted.Broadcast(SpokenText, Type);
}

void UTTSSpeaker::FinishedSpeaking() {
    FReadSpeakerTTSModule::Log(FString::Printf(TEXT("%s finished speaking"), *(GetOwner()->GetName())));
    
    FString SpokenText = Converter->Text;
    TTSTextType Type = Converter->TextType;
    
    if (Converter != NULL && Converter->IsValidLowLevel()) {
        Converter = NULL;
    }

    ThisAudioComponent->Sound = NULL;

    OnSpeakingFinished.Broadcast(SpokenText, Type);
}

void UTTSSpeaker::Say(FString text, TTSTextType textType)
{
    Engine = FReadSpeakerTTSModule::GetEngineByID(EngineID);

    if (Engine == NULL) {
        FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Could not find requested engine: %s"), *EngineID));
        return;
    }

    Converter = NewObject<UTTSConverter>();
    Converter->Text = text;
    Converter->Engine = Engine;
    Converter->Volume = Volume;
    Converter->Pitch = Pitch;
    Converter->Speed = Speed;
    Converter->Pause = Pause;
    Converter->CommaPause = CommaPause;
    Converter->TextType = textType;
    Converter->OutputFormat = TTSOutputFormat::PCM16;
    Converter->AudioComponent = ThisAudioComponent;
    Converter->SoundAttenuationSettings = &SoundAttenuation;

    Converter->ConvertToBuffer_SyncInfo();

    Converter->Play();
    StartedSpeaking();
}

void UTTSSpeaker::SayAsync(FString text, TTSTextType textType)
{
    Engine = FReadSpeakerTTSModule::GetEngineByID(EngineID);

    if (Engine == NULL) {
        FReadSpeakerTTSModule::Log(FString::Printf(TEXT("Could not find requested engine: %s"), *EngineID));
        return;
    }

    Converter = NewObject<UTTSConverter>();
    Converter->Text = text;
    Converter->Engine = Engine;
    Converter->Volume = Volume;
    Converter->Pitch = Pitch;
    Converter->Speed = Speed;
    Converter->Pause = Pause;
    Converter->CommaPause = CommaPause;
    Converter->TextType = textType;
    Converter->OutputFormat = TTSOutputFormat::PCM16;
    Converter->AudioComponent = ThisAudioComponent;
    Converter->SoundAttenuationSettings = &SoundAttenuation;
    
    Converter->OnConversionFinished.AddDynamic(Converter, &UTTSConverter::Play);
    Converter->OnConversionFinished.AddDynamic(this, &UTTSSpeaker::StartedSpeaking);

    Converter->ConvertToBufferAsync_SyncInfo();
}

void UTTSSpeaker::PauseSpeaking() {
    if (ThisAudioComponent != NULL && !ThisAudioComponent->bIsPaused) {
        ThisAudioComponent->SetPaused(true);
    }
}

void UTTSSpeaker::ResumeSpeaking() {
    if (ThisAudioComponent != NULL && ThisAudioComponent->bIsPaused) {
        ThisAudioComponent->SetPaused(false);
    }
}

void UTTSSpeaker::InterruptSpeaking() {
    if (ThisAudioComponent != NULL && ThisAudioComponent->IsPlaying()) {
        ThisAudioComponent->Stop();
    }
}

void UTTSObject::Init() {
    FReadSpeakerTTSModule::Init();
}

UTTSEngine *UTTSObject::FindEngine(FString name, FString type) {
    return FReadSpeakerTTSModule::GetEngine(name, type);
}

void UTTSObject::PauseAll() {
    FReadSpeakerTTSModule::PauseAll();
}

void UTTSObject::ResumeAll() {
    FReadSpeakerTTSModule::ResumeAll();
}

void UTTSObject::InterruptAll() {
    FReadSpeakerTTSModule::InterruptAll();
}

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "STTSVoiceCombo"

void STTSVoiceCombo::Construct(const FArguments& InArgs)
{
    SpeechCharacteristics = InArgs._SpeechCharacteristics;
    OnStateChanged = InArgs._OnStateChanged;
    if (GEditor && !GEditor->PlayWorld) {
        FReadSpeakerTTSModule::Init();
    }
    int current = 0;
    if (SpeechCharacteristics == NULL)
        return;

    if (Engines.Num() > 0) {
        for (int i = 0; i < Engines.Num(); i++) {
            if (Engines[i]->ID == SpeechCharacteristics->EngineID) {
                current = i;
            }
            Options.Add(MakeShareable(new FString(Engines[i]->ID)));
        }


        CurrentItem = Options[current];
        SpeechCharacteristics->EngineID = *CurrentItem.Get();
    }
    else {
        Options.Add(MakeShareable(new FString("No voices available")));
        CurrentItem = Options[0];
    }

    ChildSlot
        [
            SNew(SComboBox<FComboItemType>)
            .OptionsSource(&Options)
            .OnSelectionChanged(this, &STTSVoiceCombo::OnSelectionChanged)
            .OnGenerateWidget(this, &STTSVoiceCombo::MakeWidgetForOption)
            .InitiallySelectedItem(CurrentItem)
        [
            SNew(STextBlock)
            .Text(this, &STTSVoiceCombo::GetCurrentItemLabel)
        ]
        ];
}

TSharedRef<SWidget> STTSVoiceCombo::MakeWidgetForOption(FComboItemType InOption)
{
    return SNew(STextBlock).Text(FText::FromString(*InOption));
}

void STTSVoiceCombo::OnSelectionChanged(FComboItemType NewValue, ESelectInfo::Type)
{
    CurrentItem = NewValue;
    TSharedPtr<FString> TheName = (TSharedPtr<FString>)CurrentItem;
    FString IDString = *(TheName).Get();
    if (SpeechCharacteristics != NULL) {
        UTTSEngine* Engine = FReadSpeakerTTSModule::GetEngineByID(IDString);
        SpeechCharacteristics->SetEngine(Engine);
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("TTSSpeaker is NULL"));
    }

    OnStateChanged.ExecuteIfBound();
}

FText STTSVoiceCombo::GetCurrentItemLabel() const
{
    if (CurrentItem.IsValid())
    {
        return FText::FromString(*CurrentItem);
    }

    return LOCTEXT("InvalidComboEntryText", "<<Invalid option>>");
}
#endif