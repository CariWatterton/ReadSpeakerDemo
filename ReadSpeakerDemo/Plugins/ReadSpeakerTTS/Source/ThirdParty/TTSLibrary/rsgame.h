// Copyright 2022 ReadSpeaker AB. All Rights Reserved.

#if !defined(PUBLIC_EX_API)
#if defined(__GNUC__) && defined(_WIN32)
#define PUBLIC_EX_API __attribute__((dllexport))
#elif defined(_WIN32)
#define PUBLIC_EX_API __declspec(dllexport)
#else
#define PUBLIC_EX_API
#endif
#endif

#if !defined(PUBLIC_IM_API)
#if defined(__GNUC__) && defined (_WIN32)
#define PUBLIC_IM_API __attribute__((dllimport)
#elif defined(_WIN32)
#define PUBLIC_IM_API __declspec(dllimport)
#else
#define PUBLIC_IM_API
#endif
#endif

typedef char* RSGAME_CONF_HANDLE;
typedef char* RSGAME_LIB_HANDLE;

typedef void(* RSGame_AudioCallback)(void*, char*, int*);
typedef void(* RSGame_EngineCallback)(void*, char*, char*, char*, char*, char*, char*, int, int);
typedef void(* RSGame_WordCallback)(void*, int*, int*, float*, int*);
typedef void(* RSGame_VisemeCallback)(void*, short*, float*, int*);
typedef void(* RSGame_MarkCallback)(void*, char*, float*, int*);

PUBLIC_EX_API RSGAME_LIB_HANDLE RSGame_CreateLibHandle(char* lib_path);
PUBLIC_EX_API RSGAME_CONF_HANDLE RSGame_CreateConfHandle(char* conf_path);
PUBLIC_EX_API int RSGame_CreateConfFile(char* voice_path, char* conf_path);
PUBLIC_EX_API void RSGame_FreeConfHandle(RSGAME_CONF_HANDLE conf_handle);
PUBLIC_EX_API void RSGame_FreeLibHandle(RSGAME_LIB_HANDLE lib_handle);
PUBLIC_EX_API int RSGame_Initialize(RSGAME_LIB_HANDLE lib_handle, RSGAME_CONF_HANDLE conf_handle);
PUBLIC_EX_API char* RSGame_GetVersion();
PUBLIC_EX_API char* RSGame_GetVTAPIVersion();
PUBLIC_EX_API int RSGame_RegisterEngines(RSGAME_CONF_HANDLE vtpath);
PUBLIC_EX_API void RSGame_Exit();
PUBLIC_EX_API int RSGame_GetEngines(RSGame_EngineCallback cb, void* context);
PUBLIC_EX_API int RSGame_AddEngine(char* pszSpeaker, char* pszType, int nSpeakerID, char* pszDBPath, char* pszLang, char* pszGender, int nCodePage, char* pszISOCode, char* pszVendor, int nSamplingRate);
PUBLIC_EX_API int RSGame_LoadEngine(char* speaker, char* type);
PUBLIC_EX_API int RSGame_UnloadEngine(char* name, char* type);
PUBLIC_EX_API int RSGame_TextToBuffer(char* text, char* name, char* type, RSGame_AudioCallback cb, int volume, int pitch, int speed, int pause, int comma_pause, int text_type, int output_format, void *context);
PUBLIC_EX_API int RSGame_TextToBuffer_SyncInfo(char* text, char* name, char* type, RSGame_AudioCallback acb, RSGame_WordCallback wcb, RSGame_VisemeCallback vcb, RSGame_MarkCallback mcb, int volume, int pitch, int speed, int pause, int comma_pause, int text_type, int output_format, void *context);
PUBLIC_EX_API int RSGame_TextToFile(char* text, char* name, char* type, char* path, int volume, int pitch, int speed, int pause, int comma_pause, int text_type, int output_format);