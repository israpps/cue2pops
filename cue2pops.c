/* 	BIN/CUE to IMAGE0.VCD conversion tool v2.0
	Last modified : 2013/05/16
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <inttypes.h>
#include <time.h>

#ifdef DEBUG
  int debug = 1;
#else
  int debug = 0;
#endif

const int SECTORSIZE = 2352; // Sector size
const int HEADERSIZE = 0x100000; // POPS header size. Also used as buffer size for caching BIN data in file output operations

int pregap_count = 0; // Number of "PREGAP" occurrences in the cue
int postgap_count = 0; // Number of "POSTGAP" occurrences in the cue

typedef struct {
	int vmode; // User command status (vmode)
	int trainer; // User command status (trainer)

	int gap_more; // User command status (gap++)
	int gap_less; // User command status (gap--)

	int deny_vmode; 	// 2013/05/16 - v2.0 : Triggered by GameIdentifier... Makes NTSCpatcher skip the PAL->NTSC patch.
	int fix_game;		// 2013/05/16 - v2.0 : Triggered by GameIdentifier... Enables GameFixer .

	int game_has_cheats;	// 2013/05/16 - v2.0 : Triggered by GameIdentifier... .
	int game_title;
	int game_trained;
	int game_fixed;
} parameters;


extern int errno;

void game_identifier(unsigned char *inbuf, parameters *p)
{
	int ptr;

	if(debug != 0) {
		if(p->vmode == 0) {
			printf("----------------------------------------------------------------------------------\n");
		}
		printf("Hello from game_identifier!\n");
	}

	if(p->game_title == 0) {
		for(ptr = 0; ptr < HEADERSIZE; ptr += 4) {
			if(inbuf[ptr] == 'S' && inbuf[ptr+1] == 'C' && inbuf[ptr+2] == 'E' && inbuf[ptr+3] == 'S' && inbuf[ptr+4] == '-' && inbuf[ptr+5] == '0' && inbuf[ptr+6] == '0' && inbuf[ptr+7] == '3' && inbuf[ptr+8] == '4' && inbuf[ptr+9] == '4' && inbuf[ptr+10] == ' ' && inbuf[ptr+11] == ' ' && inbuf[ptr+12] == ' ' && inbuf[ptr+13] == ' ' && inbuf[ptr+14] == ' ' && inbuf[ptr+15] == ' ') {
				if(debug == 0) {
					printf("----------------------------------------------------------------------------------\n");
				}
				printf("Crash Bandicoot [SCES-00344]\n");
				p->deny_vmode++; // 2013/05/16 - v2.0 : The NTSC patch fucks up the framerate badly, so now it's skipped
				p->game_title = 1;
				p->game_has_cheats = 1;
				p->fix_game = 0;
				break;
			}
			/* -------------------------------- */
			if(inbuf[ptr] == 'S' && inbuf[ptr+1] == 'C' && inbuf[ptr+2] == 'U' && inbuf[ptr+3] == 'S' && inbuf[ptr+4] == '-' && inbuf[ptr+5] == '9' && inbuf[ptr+6] == '4' && inbuf[ptr+7] == '9' && inbuf[ptr+8] == '0' && inbuf[ptr+9] == '0' && inbuf[ptr+10] == ' ' && inbuf[ptr+11] == ' '&& inbuf[ptr+12] == ' ' && inbuf[ptr+13] == ' ' && inbuf[ptr+14] == ' ' && inbuf[ptr+15] == ' ') {
				if(debug == 0) {
					printf("----------------------------------------------------------------------------------\n");
				}
				printf("Crash Bandicoot [SCUS-94900]\n");
				p->game_title = 2;
				p->game_has_cheats = 1;
				p->fix_game = 0;
				break;
			}
			/* -------------------------------- */
			if(inbuf[ptr] == 'S' && inbuf[ptr+1] == 'C' && inbuf[ptr+2] == 'P' && inbuf[ptr+3] == 'S' && inbuf[ptr+4] == '_' && inbuf[ptr+5] == '1' && inbuf[ptr+6] == '0' && inbuf[ptr+7] == '0' && inbuf[ptr+8] == '3' && inbuf[ptr+9] == '1' && inbuf[ptr+10] == ' ' && inbuf[ptr+11] == ' ' && inbuf[ptr+12] == ' ' && inbuf[ptr+13] == ' ' && inbuf[ptr+14] == ' ' && inbuf[ptr+15] == ' ') {
				if(debug == 0) {
					printf("----------------------------------------------------------------------------------\n");
				}
				printf("Crash Bandicoot [SCPS-10031]\n");
				p->game_title = 3;
				p->game_has_cheats = 1;
				p->fix_game = 0;
				break;
			}
			/* -------------------------------- */
			if(inbuf[ptr] == ' ' && inbuf[ptr+1] == '1' && inbuf[ptr+2] == '9' && inbuf[ptr+3] == '9' && inbuf[ptr+4] == '9' && inbuf[ptr+5] == '0' && inbuf[ptr+6] == '8' && inbuf[ptr+7] == '1' && inbuf[ptr+8] == '6' && inbuf[ptr+9] == '1' && inbuf[ptr+10] == '4' && inbuf[ptr+11] == '1' && inbuf[ptr+12] == '6' && inbuf[ptr+13] == '3' && inbuf[ptr+14] == '3' && inbuf[ptr+15] == '0' && inbuf[ptr+16] == '0' && inbuf[ptr+17] == '$') {
				if(debug == 0) {
					printf("----------------------------------------------------------------------------------\n");
				}
				printf("Metal Gear Solid : Special Missions [SLES-02136]\n");
				p->game_title = 4;
				p->game_has_cheats = 0;
				p->fix_game = 1;
				break;
			}
		}
	}

	if(p->game_title != 0 && p->fix_game == 1) {
		printf("GameFixer is ON\n");
	}
	if(p->game_title != 0 && p->trainer == 1 && p->game_has_cheats == 0) {
		printf("There is no cheat for this title\n");
	}
	if(p->game_title != 0 && p->deny_vmode != 0 && p->vmode == 1) {
		printf("VMODE patching is disabled for this title\n");
	}
	if(p->game_title != 0 && debug == 0) {
		printf("----------------------------------------------------------------------------------\n");
	}

	if(debug != 0) {
		printf("game_title     = %d\n", p->game_title);
		printf("fix_game      = %d\n", p->fix_game);
		printf("deny_vmode    = %d\n", p->deny_vmode);
		printf("game_has_cheats = %d\n", p->game_has_cheats);
		printf("game_identifier says goodbye.\n");
		printf("----------------------------------------------------------------------------------\n");
	}

	if(p->game_title == 0 && p->trainer == 1) {
		printf("Unknown game, no fix/trainer enabled\n");
		printf("Continuing...\n");
	}
}

void game_fixer(unsigned char *inbuf, parameters *p)
{
	int ptr;

	if(p->game_fixed == 0) {
		for(ptr = 0; ptr < HEADERSIZE; ptr += 4) {
			if(p->game_title == 4) {
				if(inbuf[ptr] == 0x78 && inbuf[ptr+1] == 0x26 && inbuf[ptr+2] == 0x43 && inbuf[ptr+3] == 0x8C) inbuf[ptr] = 0x74;
				if(inbuf[ptr] == 0xE8 && inbuf[ptr+1] == 0x75 && inbuf[ptr+2] == 0x06 && inbuf[ptr+3] == 0x80) {
					inbuf[ptr-8] = 0x07;
					printf("game_fixer : Disc Swap Patched\n");
					p->game_fixed = 1;
					printf("----------------------------------------------------------------------------------\n");
					break;
				}
			}
		}
	}
}

void game_trainer(unsigned char *inbuf, parameters *p)
{
	int ptr;

	if(p->game_trained == 0) {
		for(ptr = 0; ptr < HEADERSIZE; ptr += 4) {
			if(p->game_title == 1) {
				if(inbuf[ptr] == 0x7C && inbuf[ptr+1] == 0x16 && inbuf[ptr+2] == 0x20 && inbuf[ptr+3] == 0xAC) {
					inbuf[ptr+2] = 0x22;
					printf("game_trainer : Test Save System Enabled\n");
					p->game_trained = 1;
					printf("----------------------------------------------------------------------------------\n");
					break;
				}
			}
			if(p->game_title == 2) {
				if(inbuf[ptr] == 0x9C && inbuf[ptr+1] == 0x19 && inbuf[ptr+2] == 0x20 && inbuf[ptr+3] == 0xAC) {
					inbuf[ptr+2] = 0x22;
					printf("game_trainer : Test Save System Enabled\n");
					p->game_trained = 1;
					printf("----------------------------------------------------------------------------------\n");
					break;
				}
			}
			if(p->game_title == 3) {
				if(inbuf[ptr] == 0x84 && inbuf[ptr+1] == 0x19 && inbuf[ptr+2] == 0x20 && inbuf[ptr+3] == 0xAC) {
					inbuf[ptr+2] = 0x22;
					printf("game_trainer : Test Save System Enabled\n");
					p->game_trained = 1;
					printf("----------------------------------------------------------------------------------\n");
					break;
				}
			}
		}
	}
}

void NTSC_patcher(unsigned char *inbuf, int tracker, parameters *p)
{
	int i;

	for(i = 0; i < HEADERSIZE; i += 4) {
		if((inbuf[i] == 0x13 && inbuf[i+1] == 0x00 && (inbuf[i+2] == 0x90 || inbuf[i+2] == 0x91) && inbuf[i+3] == 0x24) && (inbuf[i+4] == 0x10 && inbuf[i+5] == 0x00 && (inbuf[i+6] == 0x90 || inbuf[i+6] == 0x91) && inbuf[i+7] == 0x24)) {
			// ?? 00 90 24 ?? 00 90 24 || ?? 00 91 24 ?? 00 91 24 || ?? 00 91 24 ?? 00 90 24 || ?? 00 90 24 ?? 00 91 24
			printf("Y-Pos pattern found at dump offset 0x%X / LBA %d (VCD offset 0x%X)\n", tracker + i, (tracker + i) / SECTORSIZE, HEADERSIZE + tracker + i);
			inbuf[i] = 0xF8;		// 2013/05/16, v2.0 : Also apply the fix here, in case NTSC_patcher cannot find/patch the video mode
			inbuf[i+1] = 0xFF;	// 2013/05/16, v2.0 : //
			inbuf[i+4] = 0xF8;
			inbuf[i+5] = 0xFF;
			printf("----------------------------------------------------------------------------------\n");
		} else if(inbuf[i+2] != 0xBD && inbuf[i+3] != 0x27 && inbuf[i+4] == 0x08 && inbuf[i+5] == 0x00 && inbuf[i+6] == 0xE0 && inbuf[i+7] == 0x03 && inbuf[i+14] == 0x02 && inbuf[i+15] == 0x3C && inbuf[i+18] == 0x42 && inbuf[i+19] == 0x8C && inbuf[i+20] == 0x08 && inbuf[i+21] == 0x00 && inbuf[i+22] == 0xE0 && inbuf[i+23] == 0x03 && inbuf[i+24] == 0x00 && inbuf[i+25] == 0x00 && inbuf[i+26] == 0x00 && inbuf[i+27] == 0x00 && ((inbuf[i+2] == 0x24 && inbuf[i+3] == 0xAC) || (inbuf[i+6] == 0x24 && inbuf[i+7] == 0xAC) || (inbuf[i+10] == 0x24 && inbuf[i+11] == 0xAC))) {
			// ?? ?? ?? ?? 08 00 E0 03 ?? ?? ?? ?? ?? ?? 02 3C ?? ?? 42 8C 08 00 E0 03 00 00 00 00
			if(p->deny_vmode != 0) {
				printf("Skipped VMODE pattern at dump offset 0x%X / LBA %d (VCD offset 0x%X)\n", tracker + i, (tracker + i) / SECTORSIZE, HEADERSIZE + tracker + i);
			}
			if(p->deny_vmode == 0) {
				printf("VMODE pattern found at dump offset 0x%X / LBA %d (VCD offset 0x%X)\n", tracker + i, (tracker + i) / SECTORSIZE, HEADERSIZE + tracker + i);
				inbuf[i+12] = 0x00;
				inbuf[i+13] = 0x00;
				inbuf[i+14] = 0x00;
				inbuf[i+15] = 0x00;
				inbuf[i+16] = 0x00;
				inbuf[i+17] = 0x00;
				inbuf[i+18] = 0x02;
				inbuf[i+19] = 0x24;
				if(inbuf[i+2] == 0x24 && inbuf[i+3] == 0xAC) {
					inbuf[i+2] = 0x20;
				} else if(inbuf[i+6] == 0x24 && inbuf[i+7] == 0xAC) {
					inbuf[i+6] = 0x20;
				} else if(inbuf[i+10] == 0x24 && inbuf[i+11] == 0xAC) {
					inbuf[i+10] = 0x20;
				}
			}
			printf("----------------------------------------------------------------------------------\n");
		}
	}
}

int get_file_size(char *file_name)
{
	FILE *file_handle;
	int status;
	int size;

	if(!(file_handle = fopen(file_name, "rb"))) {
		fprintf(stderr, "Error: Cannot open %s\n\n", file_name);
		return -1;
	}

	status = fseek(file_handle, 0, SEEK_END);
	if (status != 0) {
		fprintf(stderr, "Error: Failed to seek %s\n", file_name);
		return -1;
	}

	size = ftell(file_handle);
	if (size == -1L) {
		fprintf(stderr, "Error: Failed to get file %s size\n", file_name);
		return -1;
	}
	fclose(file_handle);

	return size;
}

int evaluate_arg(const char *arg, parameters *p)
{
	int handled = 1;

	if(!strcmp(arg, "gap++")) {
		p->gap_more = 1;
	} else if(!strcmp(arg, "gap--")) {
		p->gap_less = 1;
	} else if(!strcmp(arg, "vmode")) {
		p->vmode = 1;
	} else if(!strcmp(arg, "trainer")) {
		p->trainer = 1;
	} else if(!strcmp(arg, "debug")) {
		debug = 1;
	} else {
		handled = 0;
	}
	return handled;
}

//Check that file has correct naming and that it actually exists
int is_cue(const char *file_name)
{
	FILE *file_handle;
	char *cue_loc;

	cue_loc = strstr(file_name, ".cue");
	if (cue_loc != NULL) {
		return 1;
	}

	cue_loc = strstr(file_name, ".CUE");
	if (cue_loc != NULL) {
		return 1;
	}

	file_handle = fopen(file_name, "Rb");
	if (file_handle) {
		fclose(file_handle);
		return 1;
	}

	return 0;
}

int convert_file_ending_to_vcd(const char *file_name)
{
	char *cue_loc;

	cue_loc = strstr(file_name, ".cue");
	if (cue_loc != NULL) {
		//FIXME: Is upper case ending necessary?
		strcpy(cue_loc, ".VCD");
		return 0;
	}

	cue_loc = strstr(file_name, ".CUE");
	if (cue_loc != NULL) {
		strcpy(cue_loc, ".VCD");
		return 0;
	}

	fprintf(stderr, "Error: Input argument was not a cue file\n");
	return -2;
}

int get_lead_out(unsigned char *hbuf, int b_size, int *sector_count)
{
	/* MSF is calculated from the .bin size so DO NOT APPLY gap++/gap-- ADJUSTMENTS IN THIS FUNCTION ! */

	// Formatted Lead-Out MM:SS:FF
	char LeadOut[7];
	int leadoutM; // Calculated Lead-Out MM:__:__
	int leadoutS; // Calculated Lead-Out __:SS:__
	int leadoutF; // Calculated Lead-Out __:__:FF

	*sector_count = (b_size / SECTORSIZE) + (150 * (pregap_count + postgap_count)) + 150; // Convert the b_size to sector count
	leadoutM = *sector_count / 4500;
	leadoutS = (*sector_count - leadoutM * 4500) / 75;
	leadoutF = *sector_count - leadoutM * 4500 - leadoutS * 75;
	if(debug != 0) {
		printf("Calculated Lead-Out MSF = %02d:%02d:%02d\n", leadoutM, leadoutS, leadoutF);
	}
	*sector_count = (b_size / SECTORSIZE) + (150 * (pregap_count + postgap_count));
	if(debug != 0) {
		printf("Calculated Sector Count = %08Xh (%i)\n", *sector_count, *sector_count);
	}

	// Additonally we can add a dbg printf of the sector count that's written in sector 16 for verification. Mmmm kinda waste of time
	/* Tired of math already. sprintf + redo what was done with the cue sheet MSFs */
	sprintf(&LeadOut[0], "%02d", leadoutM);
	sprintf(&LeadOut[2], "%02d", leadoutS);
	sprintf(&LeadOut[4], "%02d", leadoutF);

	/* Convert from ASCII symbol to number */
	hbuf[27] = ((LeadOut[0] - '0') * 16) + (LeadOut[1] - '0');
	hbuf[28] = ((LeadOut[2] - '0') * 16) + (LeadOut[3] - '0');
	hbuf[29] = ((LeadOut[4] - '0') * 16) + (LeadOut[5] - '0');
	if(debug != 0) {
		printf("Formatted Lead-Out MSF  = %02X:%02X:%02X\n\n", hbuf[27], hbuf[28], hbuf[29]);
	}

	return 0;
}

int main(int argc, char **argv)
{
	FILE *bin_file; //bin_file is used for opening the BIN that's attached to the cue.
	char *bin_path; // name/path of the BIN that is attached to the cue. Handled by the parser then altered if it doesn't contain the full path.
	int bin_size; // BIN (disc image) size

	size_t result;

	FILE *cue_file; //cue_file is used for opening the CUE
	char *cue_name = NULL; // Name of cue file
	char *cue_buf; // Buffer for the cue sheet
	char *cue_ptr; // Pointer to the Track 01 type in the cue. Used to set the sector size, the disc type or to reject the cue
	int cue_size; // Size of the cue sheet

	FILE *vcd_file;
	char *vcd_name = NULL;

	int binary_count = 0; // Number of "BINARY" occurrences in the cue
	int index0_count = 0; // Number of "INDEX 00" occurrences in the cue
	int index1_count = 0; // Number of "INDEX 01" occurrences in the cue
	int wave_count = 0; // Number of "WAVE" occurrences in the cue

	unsigned char *headerbuf; // Buffer for the POPS header
	unsigned char *outbuf; // File I/O cache
	int header_ptr = 20; // Integer that points to a location of the POPS header buffer. Decimal 20 is the start of the descriptor "A2"
	int i; // Tracker
	int m; // Calculated and formatted MM:__:__ of the current index
	int s; // Calculated and formatted __:SS:__ of the current index
	int f; // Calculated and formatted __:__:FF of the current index
	int noCDDA = 0; // 2013/04/22 - v1.2 : Is set to 1 if no CDDA track was found in the game dump, used by the NTSC patcher

	int gap_ptr = 0; // Indicates the location of the current INDEX 00 entry in the cue sheet

	int daTrack_ptr = 0; // Indicates the location of the pregap that's between the data track and the first audio track

	int track_count = 0; // Number of "TRACK " occurrences in the cue
	int fix_CDRWIN = 0; // Special CDRWIN pregap injection status
	int sector_count; // Calculated number of sectors

	parameters params;
	memset(&params, 0, sizeof(params));

	printf("\nBIN/CUE to IMAGE0.VCD conversion tool v2.0\n");
	printf("Last modified: %s\n\n", __DATE__);

	if(argc <= 1) {
		fprintf(stderr, "Error: No input file specified (cue sheet)\n\n");
		fprintf(stderr, "Usage :\n");
		fprintf(stderr, "%s input.cue <cmd_1> <cmd_2> <cmd_3> <output.vcd>\n\n", argv[0]);
		fprintf(stderr, "Commands are :\n");
		fprintf(stderr, "gap++ : Adds 2 seconds to all track indexes MSF\n");
		fprintf(stderr, "gap-- : Substracts 2 seconds to all track indexes MSF\n");
		fprintf(stderr, "vmode : Attempts to patch the video mode to NTSC and to fix the screen position\n");
		fprintf(stderr, "trainer : Enable cheats\n");
		fprintf(stderr, "debug : program behaves as if it was compiled on debug mode\n\n");
		fprintf(stderr, "Examples :\n");
		fprintf(stderr, "%s mygame.cue\n", argv[0]);
		fprintf(stderr, "%s mygame.cue IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "%s mygame.cue vmode\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap++\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap--\n", argv[0]);
		fprintf(stderr, "%s mygame.cue trainer\n", argv[0]);
		fprintf(stderr, "%s mygame.cue vmode IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap++ IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap-- IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "%s mygame.cue trainer IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap++ vmode\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap++ trainer\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap-- vmode\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap-- trainer\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap++ vmode IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap-- vmode IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap++ trainer IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap-- trainer IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap++ vmode trainer IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "%s mygame.cue gap-- vmode trainer IMAGE0.VCD\n", argv[0]);
		fprintf(stderr, "Commands and output file are optional.\n\n");
		return 0;
	}

	//Parse commands
	for (i = 1; i < argc; i++) {
		//Always assume that the first non command argument given is the input .cue file"
		if (!evaluate_arg(argv[i], &params) && (cue_name == NULL)) {
			cue_name = strdup(argv[i]);
		} else {
			vcd_name = strdup(argv[i]);
		}
	}

	if (is_cue(cue_name) == 0) {
		fprintf(stderr, "input .cue file: %s did not exist\n", cue_name);
		return 0;
	}

	if (vcd_name == NULL) {
		//Output file name was not defined. Assume it is the same as the input but with a .VCD ending
		vcd_name = strdup(argv[1]);
		if (vcd_name == NULL) {
			fprintf(stderr, "Error: Failed to copy destination string\n");
			return 0;
		}

		if (convert_file_ending_to_vcd(vcd_name)) {
			fprintf(stderr, "Error: Failed to change file ending to .VCD\n");
			return 0;
		}
	}

	if(params.gap_more == 1 && params.gap_less == 1) { // User is dumb
		fprintf(stderr, "Syntax Error : Conflicting gap++/gap-- arguments.\n\n");
		return 0;
	}

	if(debug != 0) {
		printf("CMD SWITCHES :\n");
		printf("gap_more   = %d\n", params.gap_more);
		printf("gap_less   = %d\n", params.gap_less);
		printf("vmode      = %d\n", params.vmode);
		printf("trainer    = %d\n\n", params.trainer);
	}


	cue_size = get_file_size(cue_name);
	if (cue_size < 0) {
		fprintf(stderr, "Failed to open cuefile %s, error %s\n", argv[1], strerror(errno));
		return 0;
	}

	cue_file = fopen(cue_name, "rb");
	if (cue_file == NULL) {
		fprintf(stderr, "Failed to open cuefile %s, error %s\n", argv[1], strerror(errno));
		return 0;
	}

	rewind(cue_file);
	cue_buf = malloc(cue_size * 2);
	if (cue_buf == NULL) {
		fprintf(stderr, "Failed to allocate memory for the cue buffer\n");
		return 0;
	}

	result = fread(cue_buf, cue_size, 1, cue_file);
	if (result != 1) {
		fprintf(stderr, "Failed to copy the cue to memory\n");
		free(cue_buf);
		return 0;
	}
	fclose(cue_file);

	cue_ptr = strstr(cue_buf, "INDEX 01 ");
	cue_ptr += 9;
	if((cue_ptr[0] != '0') || (cue_ptr[1] != '0')) {
		fprintf(stderr, "Error: The cue sheet is not valid\n\n");
		free(cue_buf);
		return 0;
	}

	cue_ptr = strstr(cue_buf, "FILE ");
	cue_ptr += 5; // Jump to the BINARY name/path starting with " (it's right after "FILE ")
	if(cue_ptr[0] != '"') {
		fprintf(stderr, "Error: The cue sheet is not valid\n\n");
		free(cue_buf);
		return 0;
	}

	for(i = 0; i < cue_size; i++) {
		if(cue_buf[i] == '"') {
			cue_buf[i] = '\0';
		}
	}
	cue_ptr++; // Jump to the BINARY name/path starting with "

	bin_path = malloc((strlen(cue_ptr) + strlen(cue_name)) * 2);
	if (bin_path == NULL) {
		fprintf(stderr, "Error: Failed to allocate memory for the bin_path string\n");
		free(cue_buf);
		return 0;
	}

	for(i = strlen(cue_ptr); i > 0; i--) { // Does the cue have the full BINARY path ?
		if((cue_ptr[i] == '\\') || (cue_ptr[i] == '/')) { // YES !
			strcpy(bin_path, cue_ptr);
			break;
		}
	}
	if(i == 0) { // If no..
		for(i = strlen(cue_name); i > 0; i--) { // Does the arg have the full cue path ?
		  if((cue_name[i] == '\\') || (cue_name[i] == '/')) {
		    break; // YES!
		  }
		}

		if(i == 0) {
		  // Having a filename without hierarchy is perfectly ok.
		  strcpy(bin_path, cue_ptr);
		} else { // Here we've got the full CUE path. We're gonna use it to make the BIN path.
			strcpy(bin_path, cue_name);
			/* Why should I use strrchr when I can do a n00ber thing ;D */
			for(i = strlen(bin_path); i > 0; i--) {
				if((bin_path[i] == '\\') || (bin_path[i] == '/')) {
					break;
				}
			}
			for(i = i+1; (unsigned long) i < strlen(bin_path); i++) {
				bin_path[i] = 0x00; // How kewl is dat ?
			}
			/* Me no liek strncat */
			i = strlen(bin_path);
			strcpy(bin_path + i, cue_ptr);
			i = strlen(bin_path);
			if(cue_name[0] == '"') {
				bin_path[i] = '"';
			} else {
				bin_path[i] = '\0';
			}
		}
	}

	if(debug != 0) {
		printf("CUE Path   = %s\n", cue_name);
		printf("BIN Path   = %s\n\n", bin_path);
	}

	headerbuf = malloc(HEADERSIZE * 2);
	if (headerbuf == NULL) {
		fprintf(stderr, "Error: Failed to allocate header buffer\n");
		return 0;
	}

	/*******************************************************************************************************
	********************************************************************************************************
	Below are the 3 descriptors (POPS header offset 0h to 1Eh) set up for PlayStation CD-ROM dumps
	Why I didn't use structure definitions ? Because I don't like it.
	--------------------------------------------------------------------------------------------------------
	Also note that headerbuf + 1032 to headerbuf + 1040 is the disc size in sectors :
	memcpy(headerbuf + 1032, &sector_count, 3); // Sector Count (LEHEX)
	memcpy(headerbuf + 1036, &sector_count, 3); // Sector Count (LEHEX)
	headerbuf[1032] = A
	headerbuf[1033] = B
	headerbuf[1034] = L
	headerbuf[1035] = NOP (else, the size is out of bounds)
	headerbuf[1036] = A
	headerbuf[1037] = B
	headerbuf[1038] = L
	headerbuf[1039] = NOP (else, the size is out of bounds)
	--------------------------------------------------------------------------------------------------------
	Example of a track entry (10 bytes) :
	headerbuf[30] = 0x41;	// Track Type = 41h for DATA, 01h for CDDA
	headerbuf[31] = 0x00;	// NULL
	headerbuf[32] = 0x01;	// Track Number (01h to 99h). We're at headerbuf + 32 in this example, so it's 01h for Track 01
	headerbuf[33] = 0x00;	// INDEX 00 MM
	headerbuf[34] = 0x00;	// INDEX 00 SS
	headerbuf[35] = 0x00;	// INDEX 00 FF
	headerbuf[36] = 0x00;	// NULL
	headerbuf[37] = 0x00;	// INDEX 01 MM
	headerbuf[38] = 0x02;	// INDEX 01 SS
	headerbuf[39] = 0x00;	// INDEX 01 FF
	========================================================================================================
	Alright, here are the 3 descriptors (A0, A1 and A2) :
	********************************************************************************************************
	*******************************************************************************************************/
	headerbuf[0] = 0x41;	// 1st track Type = 41h for DATA
//	headerbuf[1] = 0x00;	// NULL
	headerbuf[2] = 0xA0;	// Descriptor Identifier = A0h for DISC TYPE array
//	headerbuf[3] = 0x00;	// 1st track MM
//	headerbuf[4] = 0x00;	// 1st track SS
//	headerbuf[5] = 0x00;	// 1st track FF
//	headerbuf[6] = 0x00;	// NULL
	headerbuf[7] = 0x01;	// Track number of the 1st track = 1
	headerbuf[8] = 0x20;	// Disc Type = 20h for CD-XA001
//	headerbuf[9] = 0x00;	// NULL
//	headerbuf[10] = 0x41;	// Content Type = We'll put the last track type here later (01h = mixed with CD-DA, 41h = single DATA track)
//	headerbuf[11] = 0x00;	// NULL
	headerbuf[12] = 0xA1;	// Descriptor Identifier = A1h for CONTENT array
//	headerbuf[13] = 0x00;	// Content start MM
//	headerbuf[14] = 0x00;	// Content start SS
//	headerbuf[15] = 0x00;	// Content start FF
//	headerbuf[16] = 0x00;	// NULL
//	headerbuf[17] = 0x99;	// Track count
//	headerbuf[18] = 0x00;	// NULL
//	headerbuf[19] = 0x00;	// NULL
//	headerbuf[20] = 0x41	// 2013/05/16, v2.0 : Content Type = We'll put the last track type here later (01h = mixed with CD-DA, 41h = single DATA track)
//	headerbuf[21] = 0x00;	// NULL
	headerbuf[22] = 0xA2;	// Descriptor Identifier = A2h for LEAD-IN/LEAD-OUT array
//	headerbuf[23] = 0x00;	// Lead-In MM
//	headerbuf[24] = 0x00;	// Lead-In SS
//	headerbuf[25] = 0x00;	// Lead-In FF
//	headerbuf[26] = 0x00;	// NULL
//	headerbuf[27] = 0x79;	// Lead-Out MM
//	headerbuf[28] = 0x59;	// Lead-Out SS
//	headerbuf[29] = 0x74;	// Lead-Out FF
	/*******************************************************************************************************
	*******************************************************************************************************/

	for(i = 0; i < cue_size; i++) { // Since I've nulled some chars in the BIN handler, here I prelocate the TRACK 01 string so the track type substring can be found
		if(cue_buf[i] == 'T' && cue_buf[i+1] == 'R' && cue_buf[i+2] == 'A' && cue_buf[i+3] == 'C' && cue_buf[i+4] == 'K' && cue_buf[i+5] == ' ' && cue_buf[i+6] == '0' && cue_buf[i+7] == '1') {
			break;
		}
	}

	cue_ptr = strstr(cue_buf + i, "TRACK 01 MODE2/2352"); // Ought be
	if(cue_ptr != NULL) {
		if(debug != 0) {
			printf("Disc Type Check : Is MODE2/2352\n");
		}
	} else { // 2013/05/16, v2.0 : Not MODE2/2352, tell the user and terminate
		fprintf(stderr, "Error: Looks like your game dump is not MODE2/2352, or the cue is invalid.\n\n");
		free(cue_buf);
		free(bin_path);
		free(headerbuf);
		return 0;
	}

	//FIXME: Migrate to function
	for(i = 0; i < cue_size; i++) {
		/* Clean out some crap in the cue_buf */
		if(cue_buf[i] == ':') {
			cue_buf[i] = '\0';
		}
		//Carriage return
		if(cue_buf[i] == 0x0D) {
			cue_buf[i] = '\0';
		}
		//Line feed
		if(cue_buf[i] == 0x0A) {
			cue_buf[i] = '\0';
		}
		/* Count stuff in the cue */
		if(cue_buf[i] == 'T' && cue_buf[i+1] == 'R' && cue_buf[i+2] == 'A' && cue_buf[i+3] == 'C' && cue_buf[i+4] == 'K' && cue_buf[i+5] == ' ') {
			track_count++;
		}
		if(cue_buf[i] == 'I' && cue_buf[i+1] == 'N' && cue_buf[i+2] == 'D' && cue_buf[i+3] == 'E' && cue_buf[i+4] == 'X' && cue_buf[i+5] == ' ' && cue_buf[i+6] == '0' && cue_buf[i+7] == '1') {
			index1_count++;
		}
		if(cue_buf[i] == 'I' && cue_buf[i+1] == 'N' && cue_buf[i+2] == 'D' && cue_buf[i+3] == 'E' && cue_buf[i+4] == 'X' && cue_buf[i+5] == ' ' && cue_buf[i+6] == '0' && cue_buf[i+7] == '0') {
			index0_count++;
		}
		if(cue_buf[i] == 'B' && cue_buf[i+1] == 'I' && cue_buf[i+2] == 'N' && cue_buf[i+3] == 'A' && cue_buf[i+4] == 'R' && cue_buf[i+5] == 'Y') {
			binary_count++;
		}
		if(cue_buf[i] == 'W' && cue_buf[i+1] == 'A' && cue_buf[i+2] == 'V' && cue_buf[i+3] == 'E') {
			wave_count++;
		}
		if(cue_buf[i] == 'P' && cue_buf[i+1] == 'R' && cue_buf[i+2] == 'E' && cue_buf[i+3] == 'G' && cue_buf[i+4] == 'A' && cue_buf[i+5] == 'P') {
			pregap_count++;
		}
		if(cue_buf[i] == 'P' && cue_buf[i+1] == 'O' && cue_buf[i+2] == 'S' && cue_buf[i+3] == 'T' && cue_buf[i+4] == 'G' && cue_buf[i+5] == 'A' && cue_buf[i+6] == 'P') {
			postgap_count++;
		}
	}

	if(debug != 0) {
		printf("pregap_count    = %d\n", pregap_count);
		printf("postgap_count   = %d\n", postgap_count);
		printf("track_count     = %d\n", track_count);
		printf("index0_count    = %d\n", index0_count);
		printf("index1_count    = %d\n", index1_count);
		printf("binary_count    = %d\n\n", binary_count);
	}
	if(binary_count == 0) { // WTF ?
		fprintf(stderr, "Error: Unstandard cue sheet\n\n");
		free(cue_buf);
		free(bin_path);
		free(headerbuf);
		return 0;
	}
	if((track_count == 0) || (track_count != index1_count)) { // Huh ?
		fprintf(stderr, "Error: Cannot count tracks\n\n");
		free(cue_buf);
		free(bin_path);
		free(headerbuf);
		return 0;
	}
	if(binary_count != 1 || wave_count != 0) { // I urd u liek warez^^
		fprintf(stderr, "Error: Cue sheets of splitted dumps aren't supported\n\n");
		free(cue_buf);
		free(bin_path);
		free(headerbuf);
		return 0;
	}

	if(pregap_count == 1 && postgap_count == 0) { // Don't offer to fix the dump if more than 1 pregap was found or cue has postgaps
		printf("Warning : The input file seems to be a CDRWIN cue sheet\n");
		printf("          A pregap will be inserted in the output file...\n\n");
		fix_CDRWIN = 1; //... And turn the CDRWIN  fix on
	}

	for(i = 0; i < track_count; i++) {
		header_ptr += 10; // Put the pointer at the start of the correct track entry
		int cue_index_ptr;  // Indicates the location of the current INDEX 01 entry in the cue sheet

		for(cue_index_ptr = 0; cue_index_ptr < cue_size; cue_index_ptr++) {
			if(cue_buf[cue_index_ptr] == 'T' && cue_buf[cue_index_ptr+1] == 'R' && cue_buf[cue_index_ptr+2] == 'A' && cue_buf[cue_index_ptr+3] == 'C' && cue_buf[cue_index_ptr+4] == 'K' && cue_buf[cue_index_ptr+5] == ' ') {
				cue_buf[cue_index_ptr] = '\0';
				cue_buf[cue_index_ptr + 1] = '\0';
				cue_buf[cue_index_ptr + 2] = '\0';
				cue_buf[cue_index_ptr + 3] = '\0';
				cue_buf[cue_index_ptr + 4] = '\0';
				cue_buf[cue_index_ptr + 5] = '\0';

				/* Track Type : 41h = DATA Track / 01h CDDA Track */
				if(cue_buf[cue_index_ptr + 13] == '2' || cue_buf[cue_index_ptr + 13] == '1' || cue_buf[cue_index_ptr + 13] != 'O') {
					headerbuf[10] = 0x41; 			// Assume DATA Track
					headerbuf[20] = 0x41; 			// 2013/05/16, v2.0 : Assume DATA Track
					headerbuf[header_ptr] = 0x41; 	// Assume DATA Track
				}
				if(cue_buf[cue_index_ptr + 13] == 'O') {
					headerbuf[10] = 0x01; 			// Assume AUDIO Track
					headerbuf[20] = 0x01; 			// 2013/05/16, v2.0 : Assume AUDIO Track
					headerbuf[header_ptr] = 0x01; 	// Assume AUDIO Track
				}

				headerbuf[header_ptr + 2] = ((cue_buf[cue_index_ptr + 6] - 48) * 16) + (cue_buf[cue_index_ptr + 7] - 48);	// Track Number
				headerbuf[17] = ((cue_buf[cue_index_ptr + 6] - 48) * 16) + (cue_buf[cue_index_ptr + 7] - 48);				// Number of track entries
				break;
			}
		}

		for(cue_index_ptr = 0; cue_index_ptr < cue_size; cue_index_ptr++) {
			if(cue_buf[cue_index_ptr] == 'I' && cue_buf[cue_index_ptr+1] == 'N' && cue_buf[cue_index_ptr+2] == 'D' && cue_buf[cue_index_ptr+3] == 'E' && cue_buf[cue_index_ptr+4] == 'X' && cue_buf[cue_index_ptr+5] == ' ' && cue_buf[cue_index_ptr+6] == '0' && cue_buf[cue_index_ptr+7] == '0') {
				gap_ptr = cue_index_ptr; // Memoryze the location of the INDEX 00 entry
				cue_buf[cue_index_ptr] = '\0';
				cue_buf[cue_index_ptr + 1] = '\0';
				cue_buf[cue_index_ptr + 2] = '\0';
				cue_buf[cue_index_ptr + 3] = '\0';
				cue_buf[cue_index_ptr + 4] = '\0';
				cue_buf[cue_index_ptr + 5] = '\0';
				cue_buf[cue_index_ptr + 6] = '\0';
				cue_buf[cue_index_ptr + 7] = '\0';
			}
			if(cue_buf[cue_index_ptr] == 'I' && cue_buf[cue_index_ptr+1] == 'N' && cue_buf[cue_index_ptr+2] == 'D' && cue_buf[cue_index_ptr+3] == 'E' && cue_buf[cue_index_ptr+4] == 'X' && cue_buf[cue_index_ptr+5] == ' ' && cue_buf[cue_index_ptr+6] == '0' && cue_buf[cue_index_ptr+7] == '1') {
				cue_buf[cue_index_ptr] = '\0';
				cue_buf[cue_index_ptr + 1] = '\0';
				cue_buf[cue_index_ptr + 2] = '\0';
				cue_buf[cue_index_ptr + 3] = '\0';
				cue_buf[cue_index_ptr + 4] = '\0';
				cue_buf[cue_index_ptr + 5] = '\0';
				cue_buf[cue_index_ptr + 6] = '\0';
				cue_buf[cue_index_ptr + 7] = '\0';

				m = ((cue_buf[cue_index_ptr + 9] - 48) * 16) + (cue_buf[cue_index_ptr + 10] - 48);
				s = ((cue_buf[cue_index_ptr + 12] - 48) * 16) + (cue_buf[cue_index_ptr + 13] - 48);
				f = ((cue_buf[cue_index_ptr + 15] - 48) * 16) + (cue_buf[cue_index_ptr + 16] - 48);

				if(daTrack_ptr == 0 && headerbuf[10] == 0x01 && gap_ptr != 0) { // Targets the first AUDIO track INDEX 00 MSF
					daTrack_ptr = (((((cue_buf[gap_ptr + 9] - 48) * 10) + (cue_buf[gap_ptr + 10] - 48)) * 4500) + ((((cue_buf[gap_ptr + 12] - 48) * 10) + (cue_buf[gap_ptr + 13] - 48)) * 75) + (((cue_buf[gap_ptr + 15] - 48) * 10) + (cue_buf[gap_ptr + 16] - 48))) * SECTORSIZE;
					if(debug != 0) {
						printf("daTrack_ptr hit on TRACK %02d AUDIO INDEX 00 MSF minus 2 seconds !\n", i + 1);
						printf("Current daTrack_ptr = %d (%Xh)\n", daTrack_ptr, daTrack_ptr);
						printf("daTrack_ptr LBA     = %d (%Xh)\n\n", daTrack_ptr / SECTORSIZE, daTrack_ptr / SECTORSIZE);
					}
				} else if(daTrack_ptr == 0 && headerbuf[10] == 0x01 && gap_ptr == 0) { // Targets the first AUDIO track INDEX 01 MSF
					daTrack_ptr = (((((cue_buf[cue_index_ptr + 9] - 48) * 10) + (cue_buf[cue_index_ptr + 10] - 48)) * 4500) + ((((cue_buf[cue_index_ptr + 12] - 48) * 10) + (cue_buf[cue_index_ptr + 13] - 48)) * 75) + (((cue_buf[cue_index_ptr + 15] - 48) * 10) + (cue_buf[cue_index_ptr + 16] - 48))) * SECTORSIZE;
					if(debug != 0) {
						printf("daTrack_ptr hit on TRACK %02d AUDIO INDEX 01 MSF minus 2 seconds !\n", i + 1);
						printf("Current daTrack_ptr = %d (%Xh)\n", daTrack_ptr, daTrack_ptr);
						printf("daTrack_ptr LBA     = %d (%Xh)\n\n", daTrack_ptr / SECTORSIZE, daTrack_ptr / SECTORSIZE);
					}
				} else if(daTrack_ptr == 0 && headerbuf[10] == 0x41 && i == track_count - 1) { // Targets the EOF (if no audio track was found)
					noCDDA = 1; // 2013/05/16 - v2.0
					if(debug != 0) {
						printf("No CD-DA track found after TRACK %02d.\n", i + 1);
						printf("daTrack_ptr will be set to the EOF.\n\n"); // 2013/04/22 - v1.2 : Yeah, it WILL.
					}
				}

				/* Since not all cue sheet have INDEX 00 entries, we dupe the INDEX 01 MSF to the INDEX 00 MSF field... */
				headerbuf[header_ptr + 3] = m;		// Absolute MM (INDEX 00)
				headerbuf[header_ptr + 4] = s;		// Absolute SS (INDEX 00)
				headerbuf[header_ptr + 5] = f;		// Absolute FF (INDEX 00)

				headerbuf[header_ptr + 7] = m;		// Relative MM (INDEX 01)
				headerbuf[header_ptr + 8] = s;		// Relative SS (INDEX 01)
				headerbuf[header_ptr + 9] = f;		// Relative FF (INDEX 01)

				/* ... If INDEX 00 was found, then we register it's MSF to the INDEX 00 MSF field 	*/
				if(gap_ptr != 0) { // Pregap (INDEX 00) was found before
					m = ((cue_buf[gap_ptr + 9] - 48) * 16) + (cue_buf[gap_ptr + 10] - 48);
					s = ((cue_buf[gap_ptr + 12] - 48) * 16) + (cue_buf[gap_ptr + 13] - 48);
					f = ((cue_buf[gap_ptr + 15] - 48) * 16) + (cue_buf[gap_ptr + 16] - 48);
					headerbuf[header_ptr + 3] = m;		// Absolute MM (INDEX 00)
					headerbuf[header_ptr + 4] = s;		// Absolute SS (INDEX 00)
					headerbuf[header_ptr + 5] = f;		// Absolute FF (INDEX 00)
				}



				/* 2013/05/16 - v2.0, Enough already, let's fix this shit once and for all the NASTY WAY */

				/* INDEX 00 : UNCONDITIONAL + 2 SEC */
				if((i != 0) && (headerbuf[header_ptr + 4] == 0x08 || headerbuf[header_ptr + 4] == 0x09 || headerbuf[header_ptr + 4] == 0x18 || headerbuf[header_ptr + 4] == 0x19 || headerbuf[header_ptr + 4] == 0x28 || headerbuf[header_ptr + 4] == 0x29 || headerbuf[header_ptr + 4] == 0x38 || headerbuf[header_ptr + 4] == 0x39 || headerbuf[header_ptr + 4] == 0x48 || headerbuf[header_ptr + 4] == 0x49) && (headerbuf[header_ptr + 4] != 0x58 || headerbuf[header_ptr + 4] != 0x59)) {
					headerbuf[header_ptr + 4] += 8; // HEX -> DEC (+ 2 SEC)
				} else if((i != 0) && (headerbuf[header_ptr + 4] == 0x58 || headerbuf[header_ptr + 4] == 0x59)) {
					if(headerbuf[header_ptr + 4] == 0x58) headerbuf[header_ptr + 4] = 0x00; // HEX -> DEC (+ 2 SEC)
					if(headerbuf[header_ptr + 4] == 0x59) headerbuf[header_ptr + 4] = 0x01; // HEX -> DEC (+ 2 SEC)
					if(headerbuf[header_ptr + 3] == 0x09) headerbuf[header_ptr + 3] = 0x10; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x19) headerbuf[header_ptr + 3] = 0x20; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x29) headerbuf[header_ptr + 3] = 0x30; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x39) headerbuf[header_ptr + 3] = 0x40; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x49) headerbuf[header_ptr + 3] = 0x50; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x59) headerbuf[header_ptr + 3] = 0x60; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x69) headerbuf[header_ptr + 3] = 0x70; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x79) headerbuf[header_ptr + 3] = 0x80; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x89) headerbuf[header_ptr + 3] = 0x90; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] != 0x09 && headerbuf[header_ptr + 3] != 0x19 && headerbuf[header_ptr + 3] != 0x29 && headerbuf[header_ptr + 3] != 0x39 && headerbuf[header_ptr + 3] != 0x49 && headerbuf[header_ptr + 3] != 0x59 && headerbuf[header_ptr + 3] != 0x69 && headerbuf[header_ptr + 3] != 0x79 && headerbuf[header_ptr + 3] != 0x89) headerbuf[header_ptr + 3] += 1;
				} else if(i != 0) {
					headerbuf[header_ptr + 4] += 2; // + 2 SEC
				}


				/* INDEX 01 : UNCONDITIONAL + 2 SEC */
				if((i != 0) && (headerbuf[header_ptr + 8] == 0x08 || headerbuf[header_ptr + 8] == 0x09 || headerbuf[header_ptr + 8] == 0x18 || headerbuf[header_ptr + 8] == 0x19 || headerbuf[header_ptr + 8] == 0x28 || headerbuf[header_ptr + 8] == 0x29 || headerbuf[header_ptr + 8] == 0x38 || headerbuf[header_ptr + 8] == 0x39 || headerbuf[header_ptr + 8] == 0x48 || headerbuf[header_ptr + 8] == 0x49) && (headerbuf[header_ptr + 8] != 0x58 || headerbuf[header_ptr + 8] != 0x59)) {
					headerbuf[header_ptr + 8] += 8; // HEX -> DEC (+ 2 SEC)
				} else if((i != 0) && (headerbuf[header_ptr + 8] == 0x58 || headerbuf[header_ptr + 8] == 0x59)) {
					if(headerbuf[header_ptr + 8] == 0x58) headerbuf[header_ptr + 8] = 0x00; // HEX -> DEC (+ 2 SEC)
					if(headerbuf[header_ptr + 8] == 0x59) headerbuf[header_ptr + 8] = 0x01; // HEX -> DEC (+ 2 SEC)
					if(headerbuf[header_ptr + 7] == 0x09) headerbuf[header_ptr + 7] = 0x10; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x19) headerbuf[header_ptr + 7] = 0x20; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x29) headerbuf[header_ptr + 7] = 0x30; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x39) headerbuf[header_ptr + 7] = 0x40; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x49) headerbuf[header_ptr + 7] = 0x50; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x59) headerbuf[header_ptr + 7] = 0x60; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x69) headerbuf[header_ptr + 7] = 0x70; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x79) headerbuf[header_ptr + 7] = 0x80; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x89) headerbuf[header_ptr + 7] = 0x90; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] != 0x09 && headerbuf[header_ptr + 7] != 0x19 && headerbuf[header_ptr + 7] != 0x29 && headerbuf[header_ptr + 7] != 0x39 && headerbuf[header_ptr + 7] != 0x49 && headerbuf[header_ptr + 7] != 0x59 && headerbuf[header_ptr + 7] != 0x69 && headerbuf[header_ptr + 7] != 0x79 && headerbuf[header_ptr + 7] != 0x89) headerbuf[header_ptr + 7] += 1;
				} else { // Also for the 1st track
					headerbuf[header_ptr + 8] += 2; // + 2 SEC
				}

				/* 2013/05/16 - v2.0, End of the unconditional +2sec fix */



				/* 2013/05/16 - v2.0, And redo + 2 SEC for fixing CDRWIN dumps */

				if(fix_CDRWIN == 1) {
					/* INDEX 00 : + 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 4] == 0x08 || headerbuf[header_ptr + 4] == 0x09 || headerbuf[header_ptr + 4] == 0x18 || headerbuf[header_ptr + 4] == 0x19 || headerbuf[header_ptr + 4] == 0x28 || headerbuf[header_ptr + 4] == 0x29 || headerbuf[header_ptr + 4] == 0x38 || headerbuf[header_ptr + 4] == 0x39 || headerbuf[header_ptr + 4] == 0x48 || headerbuf[header_ptr + 4] == 0x49) && (headerbuf[header_ptr + 4] != 0x58 || headerbuf[header_ptr + 4] != 0x59)) {
						headerbuf[header_ptr + 4] += 8; // HEX -> DEC (+ 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 4] == 0x58 || headerbuf[header_ptr + 4] == 0x59)) {
						if(headerbuf[header_ptr + 4] == 0x58) headerbuf[header_ptr + 4] = 0x00; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 4] == 0x59) headerbuf[header_ptr + 4] = 0x01; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 3] == 0x09) headerbuf[header_ptr + 3] = 0x10; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x19) headerbuf[header_ptr + 3] = 0x20; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x29) headerbuf[header_ptr + 3] = 0x30; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x39) headerbuf[header_ptr + 3] = 0x40; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x49) headerbuf[header_ptr + 3] = 0x50; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x59) headerbuf[header_ptr + 3] = 0x60; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x69) headerbuf[header_ptr + 3] = 0x70; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x79) headerbuf[header_ptr + 3] = 0x80; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x89) headerbuf[header_ptr + 3] = 0x90; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] != 0x09 && headerbuf[header_ptr + 3] != 0x19 && headerbuf[header_ptr + 3] != 0x29 && headerbuf[header_ptr + 3] != 0x39 && headerbuf[header_ptr + 3] != 0x49 && headerbuf[header_ptr + 3] != 0x59 && headerbuf[header_ptr + 3] != 0x69 && headerbuf[header_ptr + 3] != 0x79 && headerbuf[header_ptr + 3] != 0x89) headerbuf[header_ptr + 3] += 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 4] += 2; // + 2 SEC
					}
					/* INDEX 01 : + 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 8] == 0x08 || headerbuf[header_ptr + 8] == 0x09 || headerbuf[header_ptr + 8] == 0x18 || headerbuf[header_ptr + 8] == 0x19 || headerbuf[header_ptr + 8] == 0x28 || headerbuf[header_ptr + 8] == 0x29 || headerbuf[header_ptr + 8] == 0x38 || headerbuf[header_ptr + 8] == 0x39 || headerbuf[header_ptr + 8] == 0x48 || headerbuf[header_ptr + 8] == 0x49) && (headerbuf[header_ptr + 8] != 0x58 || headerbuf[header_ptr + 8] != 0x59)) {
						headerbuf[header_ptr + 8] += 8; // HEX -> DEC (+ 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 8] == 0x58 || headerbuf[header_ptr + 8] == 0x59)) {
						if(headerbuf[header_ptr + 8] == 0x58) headerbuf[header_ptr + 8] = 0x00; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 8] == 0x59) headerbuf[header_ptr + 8] = 0x01; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 7] == 0x09) headerbuf[header_ptr + 7] = 0x10; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x19) headerbuf[header_ptr + 7] = 0x20; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x29) headerbuf[header_ptr + 7] = 0x30; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x39) headerbuf[header_ptr + 7] = 0x40; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x49) headerbuf[header_ptr + 7] = 0x50; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x59) headerbuf[header_ptr + 7] = 0x60; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x69) headerbuf[header_ptr + 7] = 0x70; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x79) headerbuf[header_ptr + 7] = 0x80; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x89) headerbuf[header_ptr + 7] = 0x90; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] != 0x09 && headerbuf[header_ptr + 7] != 0x19 && headerbuf[header_ptr + 7] != 0x29 && headerbuf[header_ptr + 7] != 0x39 && headerbuf[header_ptr + 7] != 0x49 && headerbuf[header_ptr + 7] != 0x59 && headerbuf[header_ptr + 7] != 0x69 && headerbuf[header_ptr + 7] != 0x79 && headerbuf[header_ptr + 7] != 0x89) headerbuf[header_ptr + 7] += 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 8] += 2; // + 2 SEC
					}
				}

				/* 2013/05/16 - v2.0, End of the CDRWIN +2sec fix */



				/* 2013/05/16 - v2.0, gap++ */

				if(params.gap_more == 1) {
					/* INDEX 00 : + 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 4] == 0x08 || headerbuf[header_ptr + 4] == 0x09 || headerbuf[header_ptr + 4] == 0x18 || headerbuf[header_ptr + 4] == 0x19 || headerbuf[header_ptr + 4] == 0x28 || headerbuf[header_ptr + 4] == 0x29 || headerbuf[header_ptr + 4] == 0x38 || headerbuf[header_ptr + 4] == 0x39 || headerbuf[header_ptr + 4] == 0x48 || headerbuf[header_ptr + 4] == 0x49) && (headerbuf[header_ptr + 4] != 0x58 || headerbuf[header_ptr + 4] != 0x59)) {
						headerbuf[header_ptr + 4] += 8; // HEX -> DEC (+ 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 4] == 0x58 || headerbuf[header_ptr + 4] == 0x59)) {
						if(headerbuf[header_ptr + 4] == 0x58) headerbuf[header_ptr + 4] = 0x00; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 4] == 0x59) headerbuf[header_ptr + 4] = 0x01; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 3] == 0x09) headerbuf[header_ptr + 3] = 0x10; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x19) headerbuf[header_ptr + 3] = 0x20; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x29) headerbuf[header_ptr + 3] = 0x30; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x39) headerbuf[header_ptr + 3] = 0x40; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x49) headerbuf[header_ptr + 3] = 0x50; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x59) headerbuf[header_ptr + 3] = 0x60; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x69) headerbuf[header_ptr + 3] = 0x70; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x79) headerbuf[header_ptr + 3] = 0x80; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x89) headerbuf[header_ptr + 3] = 0x90; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] != 0x09 && headerbuf[header_ptr + 3] != 0x19 && headerbuf[header_ptr + 3] != 0x29 && headerbuf[header_ptr + 3] != 0x39 && headerbuf[header_ptr + 3] != 0x49 && headerbuf[header_ptr + 3] != 0x59 && headerbuf[header_ptr + 3] != 0x69 && headerbuf[header_ptr + 3] != 0x79 && headerbuf[header_ptr + 3] != 0x89) headerbuf[header_ptr + 3] += 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 4] += 2; // + 2 SEC
					}
					/* INDEX 01 : + 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 8] == 0x08 || headerbuf[header_ptr + 8] == 0x09 || headerbuf[header_ptr + 8] == 0x18 || headerbuf[header_ptr + 8] == 0x19 || headerbuf[header_ptr + 8] == 0x28 || headerbuf[header_ptr + 8] == 0x29 || headerbuf[header_ptr + 8] == 0x38 || headerbuf[header_ptr + 8] == 0x39 || headerbuf[header_ptr + 8] == 0x48 || headerbuf[header_ptr + 8] == 0x49) && (headerbuf[header_ptr + 8] != 0x58 || headerbuf[header_ptr + 8] != 0x59)) {
						headerbuf[header_ptr + 8] += 8; // HEX -> DEC (+ 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 8] == 0x58 || headerbuf[header_ptr + 8] == 0x59)) {
						if(headerbuf[header_ptr + 8] == 0x58) headerbuf[header_ptr + 8] = 0x00; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 8] == 0x59) headerbuf[header_ptr + 8] = 0x01; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 7] == 0x09) headerbuf[header_ptr + 7] = 0x10; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x19) headerbuf[header_ptr + 7] = 0x20; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x29) headerbuf[header_ptr + 7] = 0x30; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x39) headerbuf[header_ptr + 7] = 0x40; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x49) headerbuf[header_ptr + 7] = 0x50; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x59) headerbuf[header_ptr + 7] = 0x60; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x69) headerbuf[header_ptr + 7] = 0x70; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x79) headerbuf[header_ptr + 7] = 0x80; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x89) headerbuf[header_ptr + 7] = 0x90; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] != 0x09 && headerbuf[header_ptr + 7] != 0x19 && headerbuf[header_ptr + 7] != 0x29 && headerbuf[header_ptr + 7] != 0x39 && headerbuf[header_ptr + 7] != 0x49 && headerbuf[header_ptr + 7] != 0x59 && headerbuf[header_ptr + 7] != 0x69 && headerbuf[header_ptr + 7] != 0x79 && headerbuf[header_ptr + 7] != 0x89) headerbuf[header_ptr + 7] += 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 8] += 2; // + 2 SEC
					}
				}

				/* 2013/05/16 - v2.0, End of the gap++ code */



				/* 2013/05/16 - v2.0, gap-- */

				if(params.gap_less == 1) {
					/* INDEX 00 : - 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 4] == 0x10 || headerbuf[header_ptr + 4] == 0x11 || headerbuf[header_ptr + 4] == 0x20 || headerbuf[header_ptr + 4] == 0x21 || headerbuf[header_ptr + 4] == 0x30 || headerbuf[header_ptr + 4] == 0x31 || headerbuf[header_ptr + 4] == 0x40 || headerbuf[header_ptr + 4] == 0x41 || headerbuf[header_ptr + 4] == 0x50 || headerbuf[header_ptr + 4] == 0x51) && (headerbuf[header_ptr + 4] != 0x00 || headerbuf[header_ptr + 4] != 0x01)) {
						headerbuf[header_ptr + 4] += 8; // HEX -> DEC (- 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 4] == 0x00 || headerbuf[header_ptr + 4] == 0x01)) {
						if(headerbuf[header_ptr + 4] == 0x00) headerbuf[header_ptr + 4] = 0x58; // HEX -> DEC (- 2 SEC)
						if(headerbuf[header_ptr + 4] == 0x01) headerbuf[header_ptr + 4] = 0x59; // HEX -> DEC (- 2 SEC)
						if(headerbuf[header_ptr + 3] == 0x09) headerbuf[header_ptr + 3] = 0x10; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x19) headerbuf[header_ptr + 3] = 0x20; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x29) headerbuf[header_ptr + 3] = 0x30; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x39) headerbuf[header_ptr + 3] = 0x40; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x49) headerbuf[header_ptr + 3] = 0x50; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x59) headerbuf[header_ptr + 3] = 0x60; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x69) headerbuf[header_ptr + 3] = 0x70; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x79) headerbuf[header_ptr + 3] = 0x80; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x89) headerbuf[header_ptr + 3] = 0x90; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] != 0x10 && headerbuf[header_ptr + 3] != 0x20 && headerbuf[header_ptr + 3] != 0x30 && headerbuf[header_ptr + 3] != 0x40 && headerbuf[header_ptr + 3] != 0x50 && headerbuf[header_ptr + 3] != 0x60 && headerbuf[header_ptr + 3] != 0x70 && headerbuf[header_ptr + 3] != 0x80 && headerbuf[header_ptr + 3] != 0x90) headerbuf[header_ptr + 3] -= 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 4] -= 2; // - 2 SEC
					}
					/* INDEX 01 : - 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 8] == 0x10 || headerbuf[header_ptr + 8] == 0x11 || headerbuf[header_ptr + 8] == 0x20 || headerbuf[header_ptr + 8] == 0x21 || headerbuf[header_ptr + 8] == 0x30 || headerbuf[header_ptr + 8] == 0x31 || headerbuf[header_ptr + 8] == 0x40 || headerbuf[header_ptr + 8] == 0x41 || headerbuf[header_ptr + 8] == 0x50 || headerbuf[header_ptr + 8] == 0x51) && (headerbuf[header_ptr + 8] != 0x00 || headerbuf[header_ptr + 8] != 0x01)) {
						headerbuf[header_ptr + 8] += 8; // HEX -> DEC (- 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 8] == 0x00 || headerbuf[header_ptr + 8] == 0x01)) {
						if(headerbuf[header_ptr + 8] == 0x00) headerbuf[header_ptr + 8] = 0x58; // HEX -> DEC (- 2 SEC)
						if(headerbuf[header_ptr + 8] == 0x01) headerbuf[header_ptr + 8] = 0x59; // HEX -> DEC (- 2 SEC)
						if(headerbuf[header_ptr + 7] == 0x10) headerbuf[header_ptr + 7] = 0x09; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x20) headerbuf[header_ptr + 7] = 0x19; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x30) headerbuf[header_ptr + 7] = 0x29; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x40) headerbuf[header_ptr + 7] = 0x39; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x50) headerbuf[header_ptr + 7] = 0x49; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x60) headerbuf[header_ptr + 7] = 0x59; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x70) headerbuf[header_ptr + 7] = 0x69; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x80) headerbuf[header_ptr + 7] = 0x79; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x90) headerbuf[header_ptr + 7] = 0x89; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] != 0x10 && headerbuf[header_ptr + 7] != 0x20 && headerbuf[header_ptr + 7] != 0x30 && headerbuf[header_ptr + 7] != 0x40 && headerbuf[header_ptr + 7] != 0x50 && headerbuf[header_ptr + 7] != 0x60 && headerbuf[header_ptr + 7] != 0x70 && headerbuf[header_ptr + 7] != 0x80 && headerbuf[header_ptr + 7] != 0x90) headerbuf[header_ptr + 7] -= 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 8] -= 2; // - 2 SEC
					}
				}

				/* 2013/05/16 - v2.0, End of the gap-- code */
				gap_ptr = 0; // Reset the integer that points to the last NULLed INDEX 00 entry of the cue sheet
				break;
			}
		}
	}
	free(cue_buf);

	bin_size = get_file_size(bin_path);
	if (bin_size < 0) {
		free(bin_path);
		free(headerbuf);
		return 0;
	}

	if(get_lead_out(headerbuf, bin_size, &sector_count) != 0) {
		free(bin_path);
		free(headerbuf);
		return 0;
	}

	if(noCDDA == 1) {
		daTrack_ptr = bin_size; // 2013/04/22 - v1.2 : Set it now. If no CDDA track was found in the game dump, the NTSC patcher will proceed in scanning the whole BIN.
	}
	if(noCDDA == 1 && debug != 0) {			// 2013/05/16 - v2.0 : dbg please
		printf("Current daTrack_ptr     = %d (%Xh)\n", daTrack_ptr, daTrack_ptr);
		printf("daTrack_ptr LBA         = %d (%Xh)\n\n", daTrack_ptr / SECTORSIZE, daTrack_ptr / SECTORSIZE);
	}

	if (sizeof(sector_count) != 4) {
		fprintf(stderr, "Error: sector_count variable is not 4 bytes. This will break the header generation.\n");
		return 0;
	}

	memcpy(headerbuf + 1032, &sector_count, 4); // Sector Count (LEHEX)
	memcpy(headerbuf + 1036, &sector_count, 4); // Sector Count (LEHEX)

	headerbuf[1024] = 0x6B;
	headerbuf[1025] = 0x48;
	headerbuf[1026] = 0x6E;
	headerbuf[1027] = 0x20;	// 2013/05/16 - v2.0 : CUE2POPS ver ident

	outbuf = malloc(HEADERSIZE);
	if (outbuf == NULL) {
		fprintf(stderr, "Failed to allocate output buffer\n");
		free(bin_path);
		free(headerbuf);
		return 0;
	}

	printf("Saving the virtual CD-ROM image. Please wait...\n");
	if(!(vcd_file = fopen(vcd_name, "wb"))) {
		fprintf(stderr, "Error: Cannot write to %s\n\n", vcd_name);
		free(bin_path);
		free(headerbuf);
		free(outbuf);
		return 0;
	}
	fwrite(headerbuf, 1, HEADERSIZE, vcd_file);
	fclose(vcd_file);
	free(headerbuf);

	if(!(vcd_file = fopen(vcd_name, "ab+"))) {
		fprintf(stderr, "Error: Cannot write to %s\n\n", vcd_name);
		free(bin_path);
		free(outbuf);
		return 0;
	}

	if(!(bin_file = fopen(bin_path, "rb"))) {
		fprintf(stderr, "Error: Cannot open %s\n\n", bin_path);
		free(bin_path);
		free(outbuf);
		return 0;
	}
	free(bin_path);

	for(i = 0; i < bin_size; i += HEADERSIZE) {
		if(fix_CDRWIN == 1 && (i + HEADERSIZE >= daTrack_ptr)) {
			char *padding;

			padding = malloc((150 * SECTORSIZE) * 2);
			if (padding == NULL) {
				fprintf(stderr, "Failed to allocate padding.\n");
				free(outbuf);
				return 0;
			}

			if(debug != 0) {
				printf("Padding the CDRWIN dump inside of the virtual CD-ROM image...");
			}
			fread(outbuf, HEADERSIZE - (i + HEADERSIZE - daTrack_ptr), 1, bin_file);
			fwrite(outbuf, HEADERSIZE - (i + HEADERSIZE - daTrack_ptr), 1, vcd_file);
			fwrite(padding, 150 * SECTORSIZE, 1, vcd_file);
			fread(outbuf, HEADERSIZE - (HEADERSIZE - (i + HEADERSIZE - daTrack_ptr)), 1, bin_file);
			fwrite(outbuf, HEADERSIZE - (HEADERSIZE - (i + HEADERSIZE - daTrack_ptr)), 1, vcd_file);
			fix_CDRWIN = 0;
			if(debug != 0) {
				printf(" Done.\n");
				printf("Continuing...\n");
			}
		} else {
			if(params.vmode == 1 && i == 0) {
				printf("----------------------------------------------------------------------------------\n");
				printf("NTSC Patcher is ON\n");
				printf("----------------------------------------------------------------------------------\n");
			}
			fread(outbuf, HEADERSIZE, 1, bin_file);
			if(i == 0) {
				game_identifier(outbuf, &params);
			}
			if(params.game_title >= 0 && params.game_has_cheats == 1 && params.trainer == 1 && i == 0) {
				printf("game_trainer is ON\n");
				printf("----------------------------------------------------------------------------------\n");
			}
			if(params.game_title >= 0 && params.game_trained == 0 && params.game_has_cheats == 1 && params.trainer == 1 && i <= daTrack_ptr) {
				game_trainer(outbuf, &params);
			}
			if(params.game_title >= 0 && params.game_fixed == 0 && params.fix_game == 1 && i <= daTrack_ptr) {
				game_fixer(outbuf, &params);
			}
			if(params.vmode == 1 && i <= daTrack_ptr) {
				NTSC_patcher(outbuf, i, &params);
			}
			if(i + HEADERSIZE >= bin_size) {
				fwrite(outbuf, HEADERSIZE - (i + HEADERSIZE - bin_size), 1, vcd_file);
			} else {
				fwrite(outbuf, HEADERSIZE, 1, vcd_file);
			}
		}
	}
	if(params.game_title >= 0 && params.fix_game == 1 && params.game_fixed == 0) {
		fprintf(stderr, "COULD NOT APPLY THE GAME FIXE(S) : No data to patch found\n");
		fprintf(stderr, "----------------------------------------------------------------------------------\n");
	}
	if(params.game_title >= 0 && params.game_has_cheats == 1 && params.game_trained == 0 && params.trainer == 1) {
		fprintf(stderr, "COULD NOT APPLY THE GAME CHEAT(S) : No data to patch found\n");
		fprintf(stderr, "----------------------------------------------------------------------------------\n");
	}

	free(outbuf);
	fclose(bin_file);
	fclose(vcd_file);

	printf("A POPS virtual CD-ROM image was saved to :\n");
	printf("%s\n\n", vcd_name);
	return 1;

}
/* EOSRC, oh mah dayum */