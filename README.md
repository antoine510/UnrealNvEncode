# NvEncode plugin for Unreal Engine

## Install

Clone as "NvEncode" in your project's `Plugin` folder

## Usage

On a clean project:

### Setup Starburst Gaming

Install the Starburst Engine from https://gitlab.inria.fr/starburst/engine-go

Install the SBGUnrealAdapter plugin from https://gitlab.inria.fr/Simbiot/common/team-only/demos/ue-plugins/satellite-adapter-unreal

Copy the `SBG` folder to your Content folder, replacing if needed.

When you know which Satellite will use your video feed(s), add its video receiving component to your session template, open `SBG/MySessionTemplate.stpl` and add:  
```
- componentPath: SBG/ExternalComponents/<MyVideoReceiverCompName>
  versionConstraint: "^1.0"
```

### Launch Unreal Engine

Launch your project. It should compile automatically (provided you have Visual Studio 2019 installed).


### Setup NvEncode

Next set your map gamemode to the NvQuickGamemodeBP class.

Next add a NvVideoSource to your pawn, this will capture the video.
If you want the source to follow the camera, create the video source as a child of your camera and select `Match Camera FOV` in its properties.

### Setup the receiving Satellite

You're on your own there ¯\\_(ツ)_/¯.  
Just know that it has to receive frames on the `videoFrame` Topic and send readiness status on the `frameSlotReady` Topic.

### Play

- Launch Starburst Engine.
- Launch your image receiving Satellite.
- Launch the game (in editor or standalone).  
???
- Enjoy
