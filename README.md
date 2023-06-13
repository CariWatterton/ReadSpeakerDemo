# ReadSpeakerDemo 🔊
Free full Trial of the ReadSpeaker Plugin.
https://www.readspeaker.com/applications/gaming/


## ⚒️ You will need:
- Unreal Engine 5.1 https://www.unrealengine.com/en-US/download
- Visual Studio (I use Visual Studio 2019) https://visualstudio.microsoft.com/vs/older-downloads/


## Get the project

### 📁 Download a .zip
You can download the zip file by clicking the **green "<> code" button on the repo's github page**. It will open a small window; on the **local tab** at the bottom of that window is a button for Download ZIP. Unzip once downloaded.

### ⬇️ Clone with Git GUI
1. Open your source control programme. I use sourcetree: https://www.sourcetreeapp.com/ (It's free but it's not the most user-friendly).
2. **Clone a new repo**, following the instructions of your Git GUI. For sourcetree, this involves opening a new tab and hitting "clone" at the top. 
3. Enter the **github URL**. You can generate this by clicking the **green "<> code" button on github**; on the **local tab**, under HTTPS, copy the URL. 

Whichever way you choose, put the repo or zip somewhere easy to find for you in your files.


## ⚙️ Opening the project
1. Navigate to the **ReadSpeakerDemo folder** containing config, content, plugins and source. 
2. **Right click** on the **ReadSpeakerDemo uproject** (unreal engine project) file.
3. **Click "Generate Visual Studio project files"**. (You may need to click "Show more options").
4. Open the generated **ReadSpeakerDemo.sln with Visual Studio**.
5. Once loaded, locate the **"Solution Explorer" tab**. It is usually on the right and may be collapsed, if so then expand it.
6. In the Solution Explorer tab, directly under the "Games" folder, **right click on ReadSpeakerDemo and select "Rebuild"**. (Rebuild is like build but includes a clean first, which can stop common errors). You can watch the status of the rebuild by clicking on "Output" at the very bottom left of the programme. 
7. The project should build successfully. If it doesn't, click on the **Error List** tab at the bottom left of the window and screenshot all errors. Leave a comment in the **discussions tab of this repo** with those errors. Alternatively, you can try to fix it yourself by googling the error codes.
8. Close Visual Studio.
9. Open the **ReadSpeakerDemo uproject** (unreal engine project) file. (Ensure it is opening with UE 5.1. You can change the engine version by right clicking the uproject and selecting "Switch Unreal Engine Version".
10. Wait for shaders to compile 😔


## 🗒️ Extra Information & FAQs

### .dlls and .libs
You may notice on the commit tree that there is a commit that re-adds .dll and .lib files. Usually you wouldn't do this in a repo and you'd include .dll and .lib in your GitIgnore. The ReadSpeaker plugin used in this project needs the .dll and .lib files to work. Keep this in mind if you are using the ReadSpeaker plugin in your own projects.

### Pushing to the repo
You will not be able to push to this repo, and I don't intend on updating it all that much, so it doesn't really make any sense to clone the repo. Downloading the zip is just fine.

### What is this demo project?
This project was developed by me, Cari Watterton, as part of Project BlackKat; a project focusing on blind accessibility in video games. ReadSpeaker were lovely enough to work with me in exchange for a demo project of their plugin. As one of the BlackKat projects main goals is knowledge sharing, this aligned really well with what I wanted to achieve. 

This project is a demo of how the ReadSpeaker plugin can be used to:
- Provide menu & HUD narration
- Provide Audio Description

It shows off a variety of different voices and includes ways to set up common settings players who use screen readers would expect to have access to (speed and pitch).

### What can I do with this project?
This is a knowledge sharing project. It's whole purpose is to give you the tools and knowledge to make games more accessible using the ReadSpeaker plugin. You can use the blueprint code you find here as a base, directly copy, modify and redistribute as you please. Keep in mind the ReadSpeaker plugin included here is their free demo version which includes an audio watermark while the speech occurs. If releasing a game, you would get in touch to pay for a license to the non-audio watermarked version. This is great because it means you get full access to all of the functionality and voices of the plugin for as long as you like, and only have to worry about a license fee when you go to release. https://www.readspeaker.com/applications/gaming/ 
