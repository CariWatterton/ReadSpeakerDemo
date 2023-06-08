// Copyright 2022 ReadSpeaker AB. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "CoreMinimal.h"
#include "Sound/SoundWaveProcedural.h"
#include "UObject/NoExportTypes.h"
#if WITH_EDITOR
#include "DetailLayoutBuilder.h"
#include "IDetailCustomization.h"
#include "Styling/SlateStyle.h"
#endif
#include "ReadSpeakerTTS.generated.h"

	class FToolBarBuilder;
	class FMenuBuilder;

	DECLARE_MULTICAST_DELEGATE(FOnPauseAll);
	DECLARE_MULTICAST_DELEGATE(FOnResumeAll);
	DECLARE_MULTICAST_DELEGATE(FOnInterruptAll);
	DECLARE_EVENT_TwoParams(UTTSSpeaker, FOnSpeakingStarted, FString, TTSTextType);
	DECLARE_EVENT_TwoParams(UTTSSpeaker, FOnSpeakingFinished, FString, TTSTextType);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConversionFinished);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWordEvent, int, startPos, int, endPos, float, time);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVisemeEvent, int, visemeId, float, time);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMarkEvent, FString, markName, float, time);

	UENUM(BlueprintType)
	enum class TTSTextType : uint8 {
		Normal = 0, ///< SSML tags won't be processed.
		SSML = 128 ///< SSML tags will be processed.
	};

	UENUM(BlueprintType)
	enum class TTSOutputFormat : uint8 {
		PCM16 = 0, ///< Linear 16-bit PCM format.
		PCM8 = 1 ///< Linear 8-bit PCM format.
	};

	class FReadSpeakerTTSModule : public IModuleInterface
	{
	public:

		enum LoggingLevel {
			Undefined = 0, ///< Set before module has been initialized.
			Off = 1, ///< Don't log debug messages.
			Verbose = 2 ///< Log debug messages.
		};

		/**
		 * Called when module loads.
		 */
		virtual void StartupModule() override;

		/**
		 * Called when module unloads.
		 */
		virtual void ShutdownModule() override;

		/**
		 * Updates the list of TTS Engines.
		 */
		READSPEAKERTTS_API static void GetEngines(TArray<UTTSEngine*> *EngineArr);

		/**
		 * Gets the TTS Engine with the specified name and type.
		 * @param {FString} name The name which is queried.
		 * @param {FString} type The type which is queried.
		 */
		READSPEAKERTTS_API static UTTSEngine* GetEngine(FString name, FString type);

		/**
		 * Gets the TTS Engine with the specified id.
		 * @param {FString} id The id which is queried.
		 * @returns {UTTSEngine*} A TTS engine with name name and type type. null if none exists.
		 */
		READSPEAKERTTS_API static UTTSEngine* GetEngineByID(FString id);

		/**
		 * Gets a TTS engine with a specified language.
		 * @param {FString} lang The language which is queried.
		 * @returns {UTTSEngine*} The TTS engine with ID id. null if none exists.
		 */
		READSPEAKERTTS_API static UTTSEngine* GetEngineWithLanguage(FString lang);

		/**
		 * Gets a TTS engine with a specified gender.
		 * @param {FString} gender The gender which is queried.
		 * @returns {UTTSEngine*} A TTS engine with language lang. null if none exists.
		 */
		READSPEAKERTTS_API static UTTSEngine* GetEngineWithGender(FString gender);

		/**
		 * Gets a TTS engine with a specified language and gender.
		 * @param {FString} lang The language which is queried.
		 * @param {FString} gender The gender which is queried.
		 * @returns {UTTSEngine*} A TTS engine with language lang and gender gender. null if none exists.
		 */
		READSPEAKERTTS_API static UTTSEngine* GetEngineWithLanguageAndGender(FString lang, FString gender);

		/**
		 * Gets all of the installed TTS engines.
		 * @returns {TArray<UTTSEngine*>} The currently installed TTS engines. Empty if none.
		 */
		READSPEAKERTTS_API static TArray<UTTSEngine*> GetInstalledEngines();

		/**
		 * Gets the TTS engines with a specified language.
		 * @param {FString} lang The language which is queried.
		 * @returns {TArray<UTTSEngine*>} All TTS engines with language lang. Empty if none.
		 */
		READSPEAKERTTS_API static TArray<UTTSEngine*> GetEnginesWithLanguage(FString lang);

		/**
		 * Gets the TTS engines with a specified gender.
		 * @param {FString} gender The gender which is queried.
		 * @returns {TArray<UTTSEngine*>} All TTS engines with gender gender. Empty if none.
		 */
		READSPEAKERTTS_API static TArray<UTTSEngine*> GetEnginesWithGender(FString gender);

		/**
		 * Gets the TTS engines with a specified language and gender.
		 * @param {FString} lang The language which is queried
		 * @param {FString} gender The gender which is queried.
		 * @returns {TArray<UTTSEngine*>} All TTS engines with language lang and gender gender. Empty if none.
		 */
		READSPEAKERTTS_API static TArray<UTTSEngine*> GetEnginesWithLanguageAndGender(FString lang, FString gender);

		/**
		 * Gets the available languages in the currently installed TTS engines.
		 * @returns {TArray<FString>} All available languages. Empty if none.
		 */
		READSPEAKERTTS_API static TArray<FString> GetAvailableLanguages();

		/**
		 * Gets the available genders for a specified language.
		 * @param {FString} lang The language which is queried.
		 * @returns {TArray<FString>} The available genders for language lang. Empty if none.
		 */
		READSPEAKERTTS_API static TArray<FString> GetAvailableGendersForLanguage(FString lang);

		/**
		 * Updates configuration files based on currently installed voices.
		 * Has to be called before a newly installed voice engine can be used.
		 */
		READSPEAKERTTS_API static void RefreshVoiceEngineInstallations();

		/**
		 * Initializes the text to speech system. Has to be called before any calls to associated functions.
		 * TODO Throw exception if platform not supported
		 * @throws not_supported The current platform is not supported.
		 */
		READSPEAKERTTS_API static int Init();

		/**
		 * Pauses playback of all active TTS speakers.
		 */
		READSPEAKERTTS_API static void PauseAll();

		/**
		 * Resumes playback of all active TTS speakers.
		 */
		READSPEAKERTTS_API static void ResumeAll();

		/**
		 * Interrupts playback of all active TTS speakers.
		 */
		READSPEAKERTTS_API static void InterruptAll();

		/**
		* Logs a message according if logging is set to verbose in TTS settings.
		* @param msg The message
		*/
		static void Log(FString msg);

		/**
		* Logs an error message.
		* @param msg The message
		*/
		static void LogError(FString msg);

		/**
		 * Binds playback functions to a TTS speaker.
		 */
		static void BindSpeaker(UTTSSpeaker* speaker);

		/**
		 * Unbinds playback functions from a TTS speaker.
		 */
		static void UnbindSpeaker(UTTSSpeaker* speaker);

		/**
		* Mutex to ensure thread safety on Android.
		*/
		static FCriticalSection AndroidMutex;

	private:
		void* VTAPILibraryHandle;
		void* RSGameLibraryHandle;
		static LoggingLevel Logging;
		static FOnPauseAll FOnPauseAllDelegate;
		static FOnResumeAll FOnResumeAllDelegate;
		static FOnInterruptAll FOnInterruptAllDelegate;
		static bool IsVerbose();
		static void ClearLastSession();
		static void RecieveEngineCallback(void* context, char* speaker, char* type, char* language, char* gender, char* dbPath, char* version, int sampling, int channels);
		static int LoadTTS(FString libPath, FString iniPath);
#if WITH_EDITOR
		TSharedRef<FSlateStyleSet> StyleSet = MakeShareable(new FSlateStyleSet("MyStyleSet"));
#endif	
};

	class READSPEAKERTTS_API VisemeEvent {
	public:
		float timestamp;
		int viseme_id;
	};

	UCLASS(ClassGroup = ReadSpeakerTTS, DefaultToInstanced)
	class READSPEAKERTTS_API USoundWaveProceduralTTS : public USoundWaveProcedural
	{
		GENERATED_BODY()
	public:

		USoundWaveProceduralTTS(const FObjectInitializer& ObjectInitializer);
		
		int TTSSampleRate; ///< The sample rate of the produced TTS audio.
		int TTSBitDepth; ///<  The bit depth of the produced TTS audio.
		double Length; ///< The length in seconds of the current audiodata.
		double RemainingDuration; ///< The remaining playback duration of the current audio in seconds.
		TArray<int16> SampleData; ///< The buffer which is fetched from when pushing data into the audio queue.
		TArray<int16> TTSData; ///< The buffer which is fetched from when pushing data into the audio queue.
		TQueue<VisemeEvent> VisemeEvents; ///< The queue of viseme events of the produced speech.

		/**
		 * Sets the data this soundwave will contain.
		 * @param data The audio data.
		 */
		void SetAudioData(TArray<int16> data);

		/**
		 * Pushes data from the TTSData buffer into the audio queue.
		 * @param InProceduralWave The sound wave to push data to
		 * @param SamplesRequired The number of samples to push.
		 */
		void FillAudioQueue(int32 SamplesRequired);

	private:
		double CalculateAudioDuration();
	};

	/**
	 * Represents a voice engine.
	 */
	UCLASS(ClassGroup = ReadSpeakerTTS, meta = (BlueprintSpawnableComponent))
		class READSPEAKERTTS_API UTTSEngine : public UObject {
		GENERATED_BODY()
		private:
			int32 ReferenceCount;
			bool KeepInMemory;
			friend class UTTSConverter;
		        int Acquire();
		        int Release();
		        FCriticalSection EngineMutex;
		public:
			FString ID; ///< The ID of the engine.
			FString Name; ///< The name of the engine.
			FString Type; ///< The type of the engine.
			FString Language; ///< The language of the engine.
			FString Gender; ///< The gender of the engine.
			FString Version; ///< The voice engine version.
			int Sampling; ///< The samplerate of the voice engine.
			int Channels; ///< The number of channels of the voice engine.

			/**
			 * Constructor
			 */
			UTTSEngine(const FObjectInitializer& ObjectInitializer);

			/**
			 * Loads this engine into memory. No-op if already loaded.
			 */
			UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Engine", meta = (Keywords = "LoadEngine"))
				int LoadEngine();

			/**
			 * Unloads the engine from memory. No-op if not loaded.
			 */
			UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Engine", meta = (Keywords = "UnloadEngine"))
				int UnloadEngine();

			/**
			 * Gets the ID of this engine.
			 */
			UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Engine", meta = (Keywords = "GetID"))
				FString GetID();
	};

	/**
	 * Blueprint wrapper for accessing static module functions.
	 */
	UCLASS(ClassGroup = ReadSpeakerTTS)
		class READSPEAKERTTS_API UTTSObject : public UObject
	{
		GENERATED_BODY()
	public:
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Global", meta = (Keywords = "Init"))
			static void Init();
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Global", meta = (Keywords = "FindEngine"))
			static UTTSEngine* FindEngine(FString name, FString type);
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Global", meta = (Keywords = "PauseAll"))
			static void PauseAll();
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Global", meta = (Keywords = "ResumeAll"))
			static void ResumeAll();
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Global", meta = (Keywords = "InterruptAll"))
			static void InterruptAll();
	};

	/**
	 * Encapsulates the text-to-speech conversion process.
	 */
	UCLASS(ClassGroup = ReadSpeakerTTS, meta = (BlueprintSpawnableComponent), DefaultToInstanced)
		class READSPEAKERTTS_API UTTSConverter : public UObject {
		GENERATED_BODY()
		public:
			UTTSEngine* Engine; ///< The voice engine to be used in synthesis.
			FString Text; ///< The text to be converted.
			int32 Volume; ///< The volume to be used in synthesis.
			int32 Pitch; ///< The pitch to be used in synthesis.
			int32 Speed; ///< The speed to be used in synthesis.
			int32 Pause; ///< The time in milliseconds to pause when encountering a delimiter in synthesis.
			int32 CommaPause; ///< The time in millieseconds to pause when encountering a ',' in synthesis.
			TTSTextType TextType; ///< Determines how the text should be processed in synthesis.
			TTSOutputFormat OutputFormat; ///< Gets or sets the format of the audio output.
			USoundWaveProceduralTTS *SoundWave; ///< Reference to the soundwave which the audio is output to.
			UAudioComponent *AudioComponent; ///< The audio component which should play the audio.
			FSoundAttenuationSettings *SoundAttenuationSettings;  ///< The sound attenuation to use during playback.
			bool FinishedConverting; ///<  true if conversion has both been started and finished, false otherwise.

			UTTSConverter(const FObjectInitializer& ObjectInitializer);

			/**
			 * Converts text to speech using the current handle values and stores the result in an audio buffer.
			 * \code
			 void SayHello(){
				 FReadSpeakerTTSModule::Init();
				 UTTSEngine *Engine = FReadSpeakerTTSModule::GetEngine("ashley", "d16");
				 UTTSConverter converter;
				 converter.Text = "Hello";
				 converter.Engine = Engine;
				 converter.Volume = 225;
				 converter.Pitch = 125;
				 converter.Speed = 125;
				 converter.Pause = 0;
				 converter.CommaPause = 0;
				 converter.EmphasisFactor = 0;
				 converter.ConvertToBuffer();
				 TArray<int16> result = converter.GetAudioData();
			 }
			 * \endcode
			 * Will result in the array result being filled with linear PCM data obtained from synthesizing the text
			 * "Hello" using the specified handle values.
			 */
			UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Converter", meta = (Keywords = "ConvertToBuffer"))
			void ConvertToBuffer();

			UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Converter", meta = (Keywords = "ConvertToBuffer_SyncInfo"))
			void ConvertToBuffer_SyncInfo();
			
			/**
			 * Converts text to speech using the current handle values and stores the result in an audio buffer.
			 * \code
			 void SayHello(){
				 FReadSpeakerTTSModule::Init();
				 UTTSEngine *Engine = FReadSpeakerTTSModule::GetEngine("ashley", "d16");
				 UTTSConverter converter;
				 converter.Text = "Hello";
				 converter.Engine = Engine;
				 converter.Volume = 225;
				 converter.Pitch = 125;
				 converter.Speed = 125;
				 converter.Pause = 0;
				 converter.CommaPause = 0;
				 converter.EmphasisFactor = 0;
				 converter.ConvertToBufferAsync();
			 }
			 * \endcode
			 * Will begin synthesizing the text "Hello" using the specified handle values. The resulting linear PCM
			 * data can be fetched with GetAudioData() once FinishedConverting() returns true.
			 */
			UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Converter", meta = (Keywords = "ConvertToBufferAsync"))
			void ConvertToBufferAsync();

			UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Converter", meta = (Keywords = "ConvertToBufferAsync_SyncInfo"))
			void ConvertToBufferAsync_SyncInfo();

			/**
			 * Gets the audio data that has been converted by ConverToBuffer() or ConvertToBufferAsync().
			 * @returns The audio data which has been converted. The complete data set if FinishedConverting() returns true, an incomplete data set otherwise.
			 */
			TArray<int16> GetAudioData();

			/**
			 * Plays the audio produced by this converter on the given AudioComponent using the given SoundAttenuationSettings.
			*/
			UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Converter", meta = (Keywords = "PlayAudioFromConverter"))
			void Play();

			UPROPERTY(BlueprintAssignable, Category = "ReadSpeaker|Converter")
			FOnConversionFinished OnConversionFinished;

			UPROPERTY(BlueprintAssignable, Category = "ReadSpeaker|Converter")
			FOnWordEvent OnWord;

			UPROPERTY(BlueprintAssignable, Category = "ReadSpeaker|Converter")
			FOnVisemeEvent OnViseme;

			UPROPERTY(BlueprintAssignable, Category = "ReadSpeaker|Converter")
			FOnMarkEvent OnMark;

		private:
			class FTTSSynthesizeTask : public FNonAbandonableTask {
			public:
				FTTSSynthesizeTask(TWeakObjectPtr<UTTSConverter> Converter, bool SyncInfo) {
					this->Converter = Converter;
					this->SyncInfo = SyncInfo;
				}

				FORCEINLINE TStatId GetStatId() const
				{
					RETURN_QUICK_DECLARE_CYCLE_STAT(FTTSSynthesizeTask, STATGROUP_ThreadPoolAsyncTasks);
				}

				void DoWork() {
					{
						{
#if PLATFORM_ANDROID
							FScopeLock ScopeLock(&(FReadSpeakerTTSModule::AndroidMutex));
#else
							FScopeLock ScopeLock(&(Converter->Engine->EngineMutex));
#endif
							if (SyncInfo) {
								Converter->ConvertToBuffer_SyncInfo();
							}
							else {
								Converter->ConvertToBuffer();
							}
						}
					}
				}

			private:
				TWeakObjectPtr<UTTSConverter> Converter;
				bool SyncInfo;
			};

			TArray<int16> AudioData;
			FAsyncTask<FTTSSynthesizeTask>* Task;

			static void audio_callback(void* context, char* data, int* length);
			static void word_callback(void* context, int* startPos, int* endPos, float* time, int* length);
			static void viseme_callback(void* context, short* visemeId, float* time, int* length);
			static void mark_callback(void* context, char* markName, float* time, int* length);
			void AddVisemeToQueue(int visemeId, float timestamp);
			void RecieveAudioCallback(char* data, int* length);
			void RecieveWordCallback(int* startPos, int* endPos, float* time, int* length);
			void RecieveVisemeCallback(short* visemeId, float* time, int* length);
			void RecieveMarkCallback(char* markName, float* time, int* length);
			void BeginDestroy() override;
	};

	/**
	 * An Actor Component which enables an Actor to speak using TTS.
	 */
	UCLASS(ClassGroup = ReadSpeakerTTS, meta = (BlueprintSpawnableComponent), DefaultToInstanced)
		class READSPEAKERTTS_API UTTSSpeaker : public UActorComponent
	{
		GENERATED_BODY()
	public:

		UTTSSpeaker(const FObjectInitializer& ObjectInitializer);

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Characteristics)
			FString EngineID; ///< The ID of the currently set voice engine.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Characteristics, meta = (ClampMin = "0", ClampMax = "500", UIMin = "0", UIMax = "500"))
			int32 Volume; ///< The volume this speaker should use during synthesis.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Characteristics, meta = (ClampMin = "50", ClampMax = "200", UIMin = "50", UIMax = "200"))
			int32 Pitch; ///< The pitch this speaker should use during synthesis.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Characteristics, meta = (ClampMin = "50", ClampMax = "400", UIMin = "50", UIMax = "400"))
			int32 Speed; ///< The speed this speaker should use during synthesis.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Characteristics, meta = (ClampMin = "0", ClampMax = "65535", UIMin = "0", UIMax = "65535"))
			int32 Pause; ///< The time in milliseconds which this speaker should pause when encountering a delimiter during synthesis.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Characteristics, meta = (ClampMin = "0", ClampMax = "65535", UIMin = "0", UIMax = "65535"))
			int32 CommaPause; ///< The time in milliseconds which this speaker should pause when encountering a ',' during synthesis.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation)
			FSoundAttenuationSettings SoundAttenuation; ///< The sound attenuation settings to be used by this speaker.

		/**
		* Delegate which is invoked when this speaker starts speaking.
		*/
		FOnSpeakingStarted OnSpeakingStarted;

		/**
		* Delegate which is invoked when this speaker finishes speaking.
		*/
		FOnSpeakingFinished OnSpeakingFinished;

		/**
		 * Previews how this speaker sounds in the Editor. No-op if not in Editor.
		 */
		void Preview();

		/**
		 * Sets the Audio Component used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "SetAudioComponent", DefaultToSelf))
		void SetAudioComponent(UAudioComponent* AudioComponent);

		/**
		 * Gets the Audio Component used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "GetAudioComponent", DefaultToSelf))
		UAudioComponent* GetAudioComponent();

		/**
		* Blueprint wrapper function to get the current viseme ID of this speaker.
		*/
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "GetCurrentViseme", DefaultToSelf))
			int GetCurrentViseme();

		/**
		 * Sets the Voluume used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "SetVolume", DefaultToSelf))
		void SetVolume(int NewVolume);

		/**
		 * Gets the Volume used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "GetVolume", DefaultToSelf))
		int GetVolume();

		/**
		 * Sets the Pitch used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "SetPitch", DefaultToSelf))
		void SetPitch(int NewPitch);
		
		/**
		 * Gets the Pitch used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "GetPitch", DefaultToSelf))
		int GetPitch();

		/**
		 * Sets the Speed used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "SetSpeed", DefaultToSelf))
		void SetSpeed(int NewSpeed);

		/**
		 * Gets the Speed used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "GetSpeed", DefaultToSelf))
		int GetSpeed();

		/**
		 * Sets the Pause time in ms used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "SetPause", DefaultToSelf))
		void SetPause(int NewPause);
		
		/**
		 * Gets the Pause time in ms used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "GetPause", DefaultToSelf))
		int GetPause();

		/**
		 * Sets the CommaPause time in ms used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "SetCommaPause", DefaultToSelf))
		void SetCommaPause(int NewCommaPause);

		/**
		 * Gets the CommaPause time in ms used by the Speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "GetCommaPause", DefaultToSelf))
		int GetCommaPause();

		/**
		 * Sets the engine to be used by this speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "SetEngine", DefaultToSelf))
		void SetEngine(UTTSEngine* Engine);

		/**
		 * Gets the Engine currently used by this speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "GetEngine", DefaultToSelf))
		UTTSEngine* GetEngine();

		/**
		 * Checks whether this speaker is currently speaking.
		 * @returns true if this speaker is currently speaking, false otherwise.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "IsSpeaking", DefaultToSelf))
		bool IsSpeaking();

		/**
		 * Reads a text aloud using the settings of this speaker.
		 * @param {UAudioComponent*} AudioComponent The audio component which should play the resulting speech.
		 * @param {FString} text The text to be read.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "Say", DefaultToSelf))
		void Say(FString text = "", TTSTextType textType = TTSTextType::Normal);

		/**
		 * Reads a text aloud using the settings of this speaker. Conversion happens asynchronously and plays
		 * the result upon completion.
		 * @param {UAudioComponent*} AudioComponent The audio component which should play the resulting speech.
		 * @param {FString} text The text to be read.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "SayAsync", DefaultToSelf))
		void SayAsync(FString text = "", TTSTextType textType = TTSTextType::Normal);

		/**
		 * Pauses playback of this speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "PauseSpeaking", DefaultToSelf))
		void PauseSpeaking();

		/**
		 * Resumes playback of this speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "ResumeSpeaking", DefaultToSelf))
		void ResumeSpeaking();

		/**
		 * Interrupts playback of this speaker.
		 */
		UFUNCTION(BlueprintCallable, Category = "ReadSpeaker|Speaker", meta = (Keywords = "InterruptSpeaking", DefaultToSelf))
		void InterruptSpeaking();

		/**
		 * Called when Speaking the current query is finished. Do not call manually.
		 */
		UFUNCTION()
		void FinishedSpeaking();
		
		/**
		* Called when Speaking the current query is started. Do not call manually.
		*/
		UFUNCTION()
		void StartedSpeaking();

	private:
		UTTSEngine* Engine;
		UTTSConverter* Converter;
		UAudioComponent* ThisAudioComponent;
		int CurrentVisemeID;
		void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		void BeginPlay() override;
		void BeginDestroy() override;
	};

#if WITH_EDITOR
	class FTTSSpeakerCustomization : public IDetailCustomization {
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
		static TSharedRef<IDetailCustomization> MakeInstance();
	};

	DECLARE_DELEGATE(OnValueChangedDelegate);

	/** Delegate that is executed when the check box state changes */
	DECLARE_DELEGATE(FOnStateChanged);

	class STTSVoiceCombo : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(STTSVoiceCombo) 
			: _SpeechCharacteristics()
			, _OnStateChanged()
		{
		}

		SLATE_ARGUMENT(TWeakObjectPtr<class UTTSSpeaker>, SpeechCharacteristics);

		/** Called when the checked state has changed */
		SLATE_EVENT(FOnStateChanged, OnStateChanged)

		SLATE_END_ARGS()

		typedef TSharedPtr<FString> FComboItemType;
		void Construct(const FArguments& InArgs);
		TSharedRef<SWidget> MakeWidgetForOption(FComboItemType InOption);
		void OnSelectionChanged(FComboItemType NewValue, ESelectInfo::Type);
		FText GetCurrentItemLabel() const;

		FComboItemType CurrentItem;
		TArray<FComboItemType> Options;
		TWeakObjectPtr<class UTTSSpeaker> SpeechCharacteristics;
		/** Delegate called when the check box changes state */
		FOnStateChanged OnStateChanged;
	};
#undef LOCTEXT_NAMESPACE
#endif
