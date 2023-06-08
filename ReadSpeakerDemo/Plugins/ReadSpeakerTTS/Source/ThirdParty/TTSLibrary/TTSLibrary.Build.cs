// Copyright 2022 ReadSpeaker AB. All Rights Reserved.

using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;

public class TTSLibrary : ModuleRules
{
	enum PlatformFlag
	{
		Unavailable = 0, Unused = 1, Used = 2
	}

	class ExportSetting
	{
		public string id;
		public PlatformFlag winx64flag;
		public PlatformFlag linuxx64flag;
		public PlatformFlag androidflag;
		public PlatformFlag ps4flag;
		public PlatformFlag ps5flag;

		public bool UsedForPlatform(string platform)
        {
			if (platform == "winx64")
				return winx64flag == PlatformFlag.Used;
			else if (platform == "linuxx64")
				return linuxx64flag == PlatformFlag.Used;
			else if (platform == "android")
				return androidflag == PlatformFlag.Used;
			else if (platform == "ps4")
				return ps4flag == PlatformFlag.Used;
			else if (platform == "ps5")
				return ps5flag == PlatformFlag.Used;
			else
				return false;
		}
	}

	public TTSLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		PublicDependencyModuleNames.AddRange(
			new string[]{
				"Core",
				"CoreUObject",
				"Engine",
				"Projects",
				"SignalProcessing"
			}
		);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Binaries", "Win64", "libvtapi.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Binaries", "Win64", "rsgame.lib"));

            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/rsgame.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libvtapi.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/librsttswrapper.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libvtconv.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libvteffect.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libvtplay.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libvtsave.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libvtssml.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/vcruntime140.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/msvcr120.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp874.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp932.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp936.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp949.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp950.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp1250.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp1251.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp1252.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp1254.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp1255.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp1256.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp1257.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp1258.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp28604.dll");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Win64/libcp57002.dll");
			RuntimeDependencies.Add("$(PluginDir)/Resources/ReadSpeaker/WinLinux/vtpath.ini");

			PublicDelayLoadDLLs.Add(Path.Combine("rsgame.dll"));
			PublicDelayLoadDLLs.Add(Path.Combine("libvtapi.dll"));

			AddVoicesForPlatform(Path.Combine(ModuleDirectory, "../../../", "Resources", "ReadSpeaker", "WinLinux"), "winx64");
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Binaries", "Linux", "libvtapi.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Binaries", "Linux", "libslicense.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Binaries", "Linux", "librsgame.a"));

            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/librsgame.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libvtapi.so");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libvtapi.so.4");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libvtapi.so.4.5");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/librsttswrapper.so");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libvtconv.so");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libvteffect.so");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libvtjni.so");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libvtsave.so");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libvtssml.so");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp874.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp932.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp936.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp949.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp950.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp1250.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp1251.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp1252.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp1254.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp1255.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp1256.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp1257.so");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp1258.so");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp28604.so");
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/TTSLibrary/Binaries/Linux/libcp57002.so");
            RuntimeDependencies.Add("$(PluginDir)/Resources/ReadSpeaker/WinLinux/vtpath.ini");

            PublicDelayLoadDLLs.Add(Path.Combine("librsgame.so"));
            PublicDelayLoadDLLs.Add(Path.Combine("libvtapi.so"));

            AddVoicesForPlatform(Path.Combine(PluginRoot, "Resources", "ReadSpeaker", "WinLinux"), "linuxx64");
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicAddAdditionalLibrariesDirectory(Path.Combine(ModuleDirectory, "Binaries", "Android", "arm64-v8a"), false);
			PublicAddAdditionalLibrariesDirectory(Path.Combine(ModuleDirectory, "Binaries", "Android", "armeabi-v7a"), false);
			RuntimeDependencies.Add("$(PluginDir)/Resources/ReadSpeaker/Android/vtpath.ini");
			AddVoicesForPlatform(Path.Combine(PluginRoot, "Resources", "ReadSpeaker", "Android"), "android");
        }
    }

    public string PluginRoot
	{
		get
		{
			return Path.GetFullPath(
				Path.Combine(ModuleDirectory, "..", "..", "..")
			);
		}
	}

	string FirstLetterToUpper(string str)
    {
		if (str == null)
			return null;

		if (str.Length > 1)
			return char.ToUpper(str[0]) + str.Substring(1);

		return str.ToUpper();
    }

	void AddDirectoryToRuntimeDependencies(string path, bool recursive, StagedFileType type)
    {
		foreach (string file in Directory.GetFiles(path))
		{
			RuntimeDependencies.Add(file, type);
		}
		if (recursive)
		{
			foreach (string dir in Directory.GetDirectories(path))
				AddDirectoryToRuntimeDependencies(dir, recursive, type);
		}
    }

	void AddVoicesForPlatform(string voicePath, string platform)
	{
		List<ExportSetting> exportSettings = ParseSetting(Path.Combine(PluginRoot, "Resources", "TTSSettings.ini"));
        if (exportSettings == null)
            return;
		foreach (string voiceDir in Directory.GetDirectories(voicePath))
		{
			foreach (string typeDir in Directory.GetDirectories(voiceDir))
			{
				string[] frags = typeDir.Split(Path.DirectorySeparatorChar);

				string id = FirstLetterToUpper(frags[frags.Length - 2]) + " " + frags[frags.Length - 1];
				foreach (ExportSetting setting in exportSettings)
				{
					if (setting.id == id && setting.UsedForPlatform(platform))
					{
						AddDirectoryToRuntimeDependencies(typeDir, true, StagedFileType.NonUFS);
					}
				}
			}
		}
	}

	List<ExportSetting> ParseSetting(string path)
    {
		if (!File.Exists(path))
			return null;
		string[] lines = File.ReadAllLines(path);
		List<ExportSetting> tmp = new List<ExportSetting>();
		foreach(string line in lines)
        {
			if (line.Contains("export"))
			{
				// Note
				// We don't use regex because of incompatibilities with UE5.
				string[] frags = line.Split('{');
				string exportData = frags[1].Substring(0, frags[1].Length - 1);
				string[] frags2 = exportData.Split(',');
				string[] frags3 = frags2[0].Split('=');
				string id = frags3[1].Trim();

				string[] frags4 = frags2[1].Split('=');
				string flags = frags4[1].Trim();

				ExportSetting setting = new ExportSetting();
				setting.id = id;

				for (int i = 0; i < flags.Length; i++)
				{
					PlatformFlag flag = PlatformFlag.Unavailable;
					char c = flags[i];
					switch (c)
					{
						case '0':
							flag = PlatformFlag.Unavailable;
							break;
						case '1':
							flag = PlatformFlag.Unused;
							break;
						case '2':
							flag = PlatformFlag.Used;
							break;
						default:
							throw new System.Exception("Faulty settingsfile");
					}
					if (i == 0)
					{
						setting.winx64flag = flag;
					} else if (i == 1)
					{
						setting.linuxx64flag = flag;
					} else if (i == 2)
					{
						setting.androidflag = flag;
					} else if (i == 3)
					{
						setting.ps4flag = flag;
					} else if (i == 4)
					{
						setting.ps5flag = flag;
					}
				}
				tmp.Add(setting);
			}
        }
		return tmp;
	}

	string PlatformFlagToString(PlatformFlag flag)
    {
        switch (flag)
        {
			case PlatformFlag.Unavailable:
				return "Unavailabale";
			case PlatformFlag.Unused:
				return "Unused";
			case PlatformFlag.Used:
				return "Used";
			default: return "";
        }
    }

	void PublicAddAdditionalLibrariesDirectory(string path, bool recursive)
    {
		foreach(string file in Directory.GetFiles(path))
        {
			if (file.EndsWith(".so")) {
				PublicAdditionalLibraries.Add(file);
			}
        }
		if (recursive)
		{
			foreach (string dir in Directory.GetDirectories(path))
			{
				PublicAddAdditionalLibrariesDirectory(dir, recursive);
			}
		}
    }

}
