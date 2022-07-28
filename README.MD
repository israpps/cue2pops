recompiled and tweaked by El_isra (thanks to the original author...)

**********************************************

# BIN/CUE to IMAGE0.VCD conversion tool v2.0
### Last modified : 2013/05/16                 

```
*****************************************************************************************
* BIN/CUE to IMAGE0.VCD conversion tool v2.0						*
* Last modified : 2013/05/16								*
*											*
* Usage :										*
* CUE2POPS_2_0.EXE input.cue <cmd_1> <cmd_2> <cmd_3> <output.vcd>			*
*											*
* Commands are :									*
* gap++ : Adds 2 seconds to all track indexes MSF					*
* gap-- : Substracts 2 seconds to all track indexes MSF					*
* vmode : Attempts to patch the video mode to NTSC and to fix the screen position	*
* trainer : Enable cheats								*
*											*
* Examples :										*
* CUE2POPS_2_0.EXE mygame.cue								*
* CUE2POPS_2_0.EXE mygame.cue IMAGE0.VCD						*
* CUE2POPS_2_0.EXE mygame.cue vmode							*
* CUE2POPS_2_0.EXE mygame.cue gap++							*
* CUE2POPS_2_0.EXE mygame.cue gap--							*
* CUE2POPS_2_0.EXE mygame.cue trainer							*
* CUE2POPS_2_0.EXE mygame.cue vmode IMAGE0.VCD						*
* CUE2POPS_2_0.EXE mygame.cue gap++ IMAGE0.VCD						*
* CUE2POPS_2_0.EXE mygame.cue gap-- IMAGE0.VCD						*
* CUE2POPS_2_0.EXE mygame.cue trainer IMAGE0.VCD					*
* CUE2POPS_2_0.EXE mygame.cue gap++ vmode						*
* CUE2POPS_2_0.EXE mygame.cue gap++ trainer						*
* CUE2POPS_2_0.EXE mygame.cue gap-- vmode						*
* CUE2POPS_2_0.EXE mygame.cue gap-- trainer						*
* CUE2POPS_2_0.EXE mygame.cue gap++ vmode IMAGE0.VCD					*
* CUE2POPS_2_0.EXE mygame.cue gap-- vmode IMAGE0.VCD					*
* CUE2POPS_2_0.EXE mygame.cue gap++ trainer IMAGE0.VCD					*
* CUE2POPS_2_0.EXE mygame.cue gap-- trainer IMAGE0.VCD					*
* CUE2POPS_2_0.EXE mygame.cue gap++ vmode trainer IMAGE0.VCD				*
* CUE2POPS_2_0.EXE mygame.cue gap-- vmode trainer IMAGE0.VCD				*
* Commands and output file are optional.						*
*****************************************************************************************
```


## PREGAP CORRECTORS (`gap++` / `gap--`)


Manual pregap correction is normally not needed.

If for some reason the audio tracks of your game don't start at their correct location, try to adjust their MSF with the command "gap++" or "gap--".

`gap++` adds 2 seconds to the MSF of your tracks. Example :

Incorrect MSF : 02:34:00 (2 minutes, 34 seconds, 0 frames)

Corrected MSF : 02:36:00 (2 minutes, 36 seconds, 0 frames)

`gap--` substracts 2 seconds to the MSF of your tracks. Example :

Incorrect MSF : 02:34:00 (2 minutes, 34 seconds, 0 frames)

Corrected MSF : 02:32:00 (2 minutes, 32 seconds, 0 frames)


__Using POPStarter r10+ and CodeBreaker OSD.BIN you can listen to your game CDDA tracks and verify that it's properly fixed.__


## PAL -> NTSC PATCHER (vmode)

Does not resize the display and may be unable to fix all games.

The NTSC Patcher is activated when the command "vmode" is specified.

GS Mode Selector gives way better results.


## GAME FIXES AND CHEATS

Implemented since CUE2POPS v2.0.

Game fixes (of the internal function "GameFixer") are automatically applied and there's currently no way to enable/disable that feature.

Game cheats (of the function "GameTrainer") are patched to the output IMAGE0.VCD when the command "trainer" is specified, and only if the game was recognized by the title identification system.

You may find a list of integrated cheats & fixes, PATCHES.TXT, in this package.
 
## THE TITLE IDENTIFICATION SYSTEM

The title identification system searches for a certain sequence of bytes in your game dump for the game identification purpose.

If your game was improperly dumped or it's a homemade "compilation" of games in one disc, the identification may fail.

That game identification thing is used to set up parameters of :
- The GameFixer
- The GameTrainer
- NTSCpatcher (the deny_vmode exception, which disallow the alteration of the VMODE code for some games)

## BINARIES AND CUE SHEET STUFF

__CUE2POPS parses the cue sheet and proceeds in vomiting IMAGE0.VCD as long as the input CUE is properly formatted.__

Example of a CUE :
```cue
FILE "MY_GAME.BIN" BINARY
	TRACK 01 MODE2/2352
	INDEX 01 00:00:00
	TRACK 02 AUDIO
	INDEX 00 14:28:58
	INDEX 01 14:30:58
```

If you get the error message "Error: The cue sheet is not valid", please verify the syntax of your CUE.

Example of bad syntax : "TRACK 1" (incorrect) instead of "TRACK 01" (correct)...

The disc standard for the PlayStation/PSone and POPS is __Mode 2, 2352 bytes per sector (MODE2/2352)__, __NOT ISO Mode 1, 2048 bytes per sector (MODE1/2048)__.

If you get the error message "Error: Looks like your game dump is not MODE2/2352, or the cue is invalid." and if your cue sheet mentions "TRACK 01 MODE1/2048",

then your dump is a pile of trash.


CUE2POPS only supports stand alone BINs and does not work with dumps that have one separate file for each track.

If you get the error message "Error: Cue sheets of splitted dumps aren't supported" and if your game dump is splitted in multiple files for each track,

please reassemble it in a single BIN, using a software such as CDmage or [BinMerger](https://github.com/israpps/BINMerger/releases).

## CHANGELOG

- 2013/05/16 - v2.0
  + Fixed : dumpsize not known / daTrack_ptr set to zero when running a non-debug build
  + Fixed : False-positive Y-Pos pattern detections
  + Fixed : Incorrect Track Type in the descriptor A2
  + Blocked non-MODE2/2352 dumps
  + The NTSCpatcher console output is no longer restricted to the DEBUG build
  + Introduced the title identification system
  + Introduced NTSC Patcher exceptions (deny_vmode), for Crash Bandicoot [SCES-00344]
  + Added the Y-Pos fix for PAL VMODE
  + Introduced game cheats (command "trainer"), for Crash Bandicoot NTSC J, NTSC U/C and PAL
  + Introduced game fixes (automatic), for MGS Special Missions [SLES-02136]
  + Pregap correctors (gap++, gap-- and the unconditional one) were rewritten
  + The CUE2POPS version identifier is now 0x20

- 2013/04/22 - v1.2
  + Fixed : dumpsize not known / daTrack_ptr set to zero when trying to patch a single track game to NTSC
  + The CUE2POPS version identifier is now 0x02

- 2013/04/19 - v1.1
  + A bit of sanitization
  + Fixed : the INDEX 01 of TRACK 01 = 00:02:00 (mistakenly marked as 00:00:00 in the previous build, without a pregap)
  + Now adds 2 seconds to all tracks unconditionally
  + Writes a CUE2POPS version identifier in the POPS header, for interaction with POPStarter Revision 10

- 2013/04/06 - v1.0
  + Initial release
