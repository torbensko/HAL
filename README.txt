author: Torben Sko (git@torbensko.com)

--- INTRO ---

HAL (Handycam and Leaning) is a head tracking technique for first-person shooter games. The technique has two purposes: it allows the user to lean around corners using their head movements and it adds a shake to the camera in order to make the game feel more realistic. For more details on how it works, please refer to the comments in the source code.


--- LEGAL STUFF ---

My code (i.e. the files found in the HAL/src/game/shared/hal) are provided under a Creative Commons Attribution license (http://creativecommons.org/licenses/by/3.0/) - so you're free to use them for any purpose. Just note that my code is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


--- SETTING IT ALL UP ---

Just to make your life difficult, I haven't included either the FaceAPI or the Source engine in this repo, so you'll need to do a bit of work before you have this puppy up and running. That said, if you follow these steps closely you should have it running pretty quickly.

1. Grab Steam (http://store.steampowered.com/about/), install it, signup, grab Team Fortress 2 (it's free, yay) and then download the Source SDK (now listed under the Tools section of your Library)

2.1 Open the Source SDK, select the Source SDK 2007 engine (note: whilst the 2009 engine is listed, at the time of writing this the Source SDK 2009 Base - i.e. the files it requires to run - was not available) and choose "Create a Mod"
2.2 To make life easier (for me at least), we will set the mod up like I do. Choose to "Modify Half-Life 2 Single Player". For the name pick anything that suits you. For the directory, specify your Source Mods folder e.g. "C:\Program Files (x86)\Steam\steamapps\sourcemods\", except add to the end of this path the name of your mod in lowercase letters and stripped of spaces and special characters. So if you decide to call your mod "Head Tracking", you'll need to add to the end of your path "headtracking". From here on in, I will refer to this folder as your 'project folder'

3 Navigate to this folder and clone this repo to it. If you're having trouble cloning to a non-empty directory, I would suggest just grabbing the zip copy and just manually dragging the files over (you should be able to drag them all over in one operation). When you do this some of the game files will be overwritten, however for the most part it's pretty unobtrusive.

4 In the src folder, open the properties.vsprops file and change the ModFolder so that it matches your project folder path

5.1 Grab the non-commercial faceAPI from the Seeing Machines' website. At time of writing this 3.2.6 was the latest publicly available version, however code has also been provided for the upcoming v4 release (see the 'Other Steps' section below)
5.2 Once you have installed the faceAPI, copy the DLLs and subfolders from "C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 3.2\API\bin" into a bin folder in the project folder - having not compiled the code yet, you will need to manually create this folder. Likewise, copy the smft32.def, smft32.exp and smft32.lib into a folder called "lib" in your "YOUR_PROJECT_FOLDER/src/game/shared/hal" folder
5.3 Copy the "C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 3.2\API\include" folder into "YOUR_PROJECT_FOLDER/src/game/shared/hal"
5.4 If you did all the above correctly, you should now have the following directory structure:
project_folder
	bin
		faceAPI Dlls
		resources
			...
		cal
			...
	src
		game
			shared
				hal
					include
						...
					lib
						smft32.def
						smft32.exp
						smft32.lib
					...

6.1 Now to build the mod. Open the Game_Episodic_HAL.sln file in your src folder (you'll also have a Game_Episodic-2005.sln file, which is the original project file - feel free to delete that file). Note: that you may need to migrate this file, depending on your version of Visual Studio.
6.2 Make sure the system is set to build under Release mode (Build > Configuration Manager > Configuration set to Release)
6.3 Build the system.
6.4 To run the system under the debug mode, open the 'Client Episodic' properties and navigate to the Debugging settings. Change the settings to the following:
COMMAND: C:\Program Files (x86)\Steam\steamapps\YOUR_STEAM_USER_NAME\source sdk base 2007\hl2.exe
ARGUMENT: -dev -window -novid -game "C:\Program Files (x86)\Steam\steamapps\SourceMods\YOUR_PROJECT_FOLDER"
DIRECTORY: C:\Program Files (x86)\Steam\steamapps\YOUR_STEAM_USER_NAME\source sdk base 2007

Other Steps: (optional)

A. To use the version 4 of the faceAPI, which at the timing of writing this wasn't publicly available, you will need to make the following changes:
A.1 Uncomment the USE_FACEAPI_4 definition in the faceapi.h file.
A.2 Replace all the faceAPI files mentioned earlier with the faceAPI 4 versions. The only real difference is that instead of copying files named smft32, you will find they are now called smft40 or something similar
A.3 Finally, open the 'Client Episodic' properties again and under Linker > Input > Additional Dependencies, change the last include from smft32 to smft40
A.4 Compile and run. Of note, the faceAPI might change between now and when it's publicly released, so there may be a few other issues for you to iron out.





