// Copyright 2022 ReadSpeaker AB. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;
using System.IO;
using System.Linq;

public class ReadSpeakerTTS : ModuleRules
{

	enum AndroidArchitecture
    {
		arm64,
		armeabi
    }

	public ReadSpeakerTTS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// WARNING! DO NOT REMOVE THE FOLLOWING LINE
		//PREPROCESS_ADDITIONS

		// WARNING! DO NOT REMOVE THE FOLLOWING LINE
		//POSTPROCESS_ADDITIONS

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"InputCore",
				"CoreUObject",
				"Engine",
				"TTSLibrary",
				"Projects",
				"SignalProcessing"
			}
			);

		PrivateDependencyModuleNames.AddRange(
				new string[]
				{
				"Slate",
				"SlateCore"
				}
				);

		RuntimeDependencies.Add("$(PluginDir)/Resources/TTSSettings.ini");

		if (Target.Type == TargetType.Editor)
		{
            PrivateDependencyModuleNames.AddRange(
		new string[]
		{
				"UnrealEd",
				"PropertyEditor"
		}
		);
        }

		// Additional Frameworks and Libraries for Android
		if (Target.Platform == UnrealTargetPlatform.Android)
		{

			PublicIncludePathModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Launch"
			}
			);

			string generatedPath = GenerateUPL();
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", generatedPath);
		}
	}

	string GenerateUPL()
    {
		string generatedPath = Path.Combine(ModuleDirectory, "ReadSpeakerTTS_UPL_generated.xml");
		List<string> upl = File.ReadAllLines(Path.Combine(ModuleDirectory, "ReadSpeakerTTS_UPL.xml")).ToList();
		List<string> libAdditions = new List<string>();

		libAdditions.AddRange(GenerateLibAdditionsFromDirectory(Path.Combine(PluginRoot, "Binaries", "ThirdParty", "TTSLibrary", "Android", "arm64-v8a")));
		libAdditions.AddRange(GenerateLibAdditionsFromDirectory(Path.Combine(PluginRoot, "Binaries", "ThirdParty", "TTSLibrary", "Android", "armeabi-v7a")));

		libAdditions = libAdditions.Distinct().ToList();
		int insertionIndex = upl.FindIndex((x) => x.Contains("*VTLIBS*"));
		upl.InsertRange(insertionIndex, libAdditions);
		File.WriteAllLines(generatedPath, upl);
		return generatedPath;
	}

	List<string> GenerateLibAdditionsFromDirectory(string directory)
    {
		List<string> libAdditions = new List<string>();
		List<string> files = Directory.GetFiles(directory).ToList();
		foreach(string file in files)
        {
			if (file.Contains("libvt_"))
			{
				int k = file.IndexOf("libvt_");
				string tmp = file.Substring(k);
				string libName = tmp.Substring(3, tmp.Length - 6);
				string libAddition = "System.loadLibrary(\"" + libName + "\");";
				libAdditions.Add(libAddition);
			}
		}
		return libAdditions;
	}

	public string ProjectRoot
    {
        get
        {
			return Path.GetFullPath(
				Path.Combine(ModuleDirectory, "..", "..", "..", "..")
			);
		}
    }

	public string PluginRoot
	{
		get
		{
			return Path.GetFullPath(
				Path.Combine(ModuleDirectory, "..", "..")
			);
		}
	}

	void PublicAddAdditionalLibrariesDirectory(string path, bool recursive)
	{
		foreach (string file in Directory.GetFiles(path))
		{
			if (file.EndsWith(".so")) 
			{
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
