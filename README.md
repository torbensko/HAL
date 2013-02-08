# Overview

This project demonstrates a head tracking technique for the first-person shooter game, Half-Life 2. The technique has two aspects: 
- it allows the user to lean around corners using their head movements, and 
- it adds a shake to the camera to replicate the shaky-cam effect used in movies.

The source code for the project can be found on [GitHub](https://github.com/torbensko/HAL).

> *Please note:* it's best to consider this code as a prototype, albeit the second version. Per game cycle it performs no less than three line traces. It's the kind of stuff that would make Gabe cry. While I'm sure it's probably possible to delve a little deeper into the Source engine in order to make its integration more efficient, please understand, that wasn't the focus of my work. My objective as driven by my PhD, was to explore whether this stuff could be done, and if so, would anyone would care.



# Legal

My code, namely the files found in the HAL/src/game/shared/hal directory, is provided under a Creative Commons Attribution license (http://creativecommons.org/licenses/by/1.0/). As such, you are free to use the code for any purpose as long as you remember to mention my name (Torben Sko) at some point. Also please note that my code is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.



# Set up

Just to make your life difficult, I haven't included either the FaceAPI or the Source engine in this repo, so you'll need to do a bit of work before you have this puppy up and running. That said, if you follow these steps closely you should have it running pretty quickly.

HT  Grab Steam (http://store.steampowered.com/about/), install it, signup, grab Team Fortress 2 (it's free, yay) and then download the Source SDK (now listed under the Tools section of your Library)

1.  Open the Source SDK, select the Source SDK 2007 engine (note: whilst the 2009 engine is listed, at the time of writing this the Source SDK 2009 Base - i.e. the files it requires to run - was not available) and choose "Create a Mod"

1.  To make life easier (for me at least), we will set the mod up like I do. Choose to "Modify Half-Life 2 Single Player". For the name pick anything that suits you. For the directory, specify your Source Mods folder e.g. "C:\Program Files (x86)\Steam\steamapps\sourcemods\", except add to the end of this path the name of your mod in lowercase letters and stripped of spaces and special characters. So if you decide to call your mod "Head Tracking", you'll need to add to the end of your path "headtracking". From here on in, I will refer to this folder as your 'project folder'

1.  Navigate to this folder and clone this repo to it. If you're having trouble cloning to a non-empty directory, I would suggest grabbing the zip version from the website and then manually dragging the files over (you should be able to drag them all over in one operation). When you do this some of the game files will be overwritten, however for the most part it's pretty unobtrusive.

1.  In the src folder, open the properties.vsprops file and change the ModFolder so that it matches your project folder path

1.  Grab the non-commercial faceAPI from the Seeing Machines' website. At time of writing this 1.1.6 was the latest publicly available version, however code has also been provided for the upcoming v4 release (see the 'Other Steps' section below)

1.  Once you have installed the faceAPI, copy the DLLs and subfolders from "C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 1.2\API\bin" into a bin folder in the project folder - having not compiled the code yet, you will need to manually create this folder. Likewise, copy the smft1.def, smft1.exp and smft1.lib into a folder called "lib" in your "YOUR_PROJECT_FOLDER/src/game/shared/hal" folder

1.  Copy the "C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 1.2\API\include" folder into "YOUR_PROJECT_FOLDER/src/game/shared/hal"

1.  If you did all the above correctly, you should now have the following directory structure:

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
                                smft1.def
                                smft1.exp
                                smft1.lib
                            ...

1.  Now to build the mod. Open the Game_Episodic_HAL.sln file in your src folder (you'll also have a Game_Episodic-1.sln file, which is the original project file - feel free to delete that file). Note: that you may need to migrate this file, depending on your version of Visual Studio.

1.  Make sure the system is set to build under Release mode (Build > Configuration Manager > Configuration set to Release)

1.  Build the system.

1.  To run the system under the debug mode, open the 'Client Episodic' properties and navigate to the Debugging settings. Change the settings to the following:
    
    **COMMAND:**

        C:\Program Files (x86)\Steam\steamapps\YOUR_STEAM_USER_NAME\source sdk base 2007\hl1.exe
    
    **ARGUMENT:**

        -dev -window -novid -game "C:\Program Files (x86)\Steam\steamapps\SourceMods\YOUR_PROJECT_FOLDER"
    
    **DIRECTORY:**

        C:\Program Files (x86)\Steam\steamapps\YOUR_STEAM_USER_NAME\source sdk base 2007

