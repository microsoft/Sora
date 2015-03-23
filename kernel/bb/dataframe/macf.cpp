#pragma warning(disable:4530)

#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include <sora.h>
#include "dot11_pkt.h"
#include "crc32.h"


char pnfile [255];
char pmfile [255];
int  dumpDataSize = 0;

void usage () {
	printf ( "macframe -f payload_file -o mac_file\n" );
}


bool ParseCmdLine ( int argc, const char* argv[] ) {
	int i=1;
	while ( i< argc ) {
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 'f' ) 
		{
			if ( ++i < argc ) {
				strncpy ( pnfile, argv[i++], 255 );
			} else {
				return false;
			}
		} else 
		if ( argv[i][0] == '-' 
			 && argv[i][1] == 'o' ) 
		{
			if ( ++i < argc ) {
				strncpy ( pmfile, argv[i++], 255 );
			} else {
				return false;
			}
		} else
		if (argv[i][0] == '-' 
			 && argv[i][1] == 'd' ) 
		{
			if ( ++i < argc ) {
				dumpDataSize = atoi (argv[i++]);
			} else {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}

char framebuf[1024*1024];
MAC_ADDRESS bssid = {1,2,3,4,5,6};
MAC_ADDRESS dest  = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
MAC_ADDRESS src   = { 5, 5, 5, 5, 5, 5};

int __cdecl main(int argc, const char *argv[])
{
	if ( ParseCmdLine (argc, argv) == false )
	{
		usage();
		return 0;
	}

	DOT11RFC1042ENCAP * wheader = (DOT11RFC1042ENCAP *)framebuf;

	// Three address format with ap mode
	(*wheader).MacHeader.Address1 = bssid;
	(*wheader).MacHeader.Address2 = src;
	(*wheader).MacHeader.Address3 = dest;
	(*wheader).MacHeader.FrameControl.Type    = FRAME_DATA;
	(*wheader).MacHeader.FrameControl.Subtype = SUBT_DATA;
	(*wheader).MacHeader.FrameControl.ToDS  = 1;	// to DS
	(*wheader).MacHeader.FrameControl.Retry = 1;
	(*wheader).MacHeader.DurationID = 0; 

	// Kun: SequenceControl includes fragment seq
	//
	(*wheader).MacHeader.SequenceControl.usValue = 400;
	(*wheader).SNAP.DSAP = 0xAA;
	(*wheader).SNAP.SSAP = 0xAA;
	(*wheader).SNAP.Control = 0x03;
	(*wheader).Type = 0x800;	

	int nRead = 0;

	if ( dumpDataSize == 0 ) {
		FILE * fin = fopen ( pnfile, "rb" );
		if ( fin == NULL ) {
			printf ( "Payload file is not found!\n" );
			return -1;
		}
	
		fseek (fin, 0, SEEK_END );
		int len = ftell (fin);
		fseek ( fin, 0, SEEK_SET );
		nRead = fread ( &framebuf[sizeof(DOT11RFC1042ENCAP)], 1, len, fin );
		fclose ( fin );
		printf ( "Payload read - length %d\n", nRead );
	} else {
		memset ( &framebuf[sizeof(DOT11RFC1042ENCAP)], 0, dumpDataSize );
		nRead = dumpDataSize;
		printf ( "Dummy data payload - length %d\n", nRead );
	}

	ULONG * FCS = (ULONG * ) &framebuf[nRead + sizeof(DOT11RFC1042ENCAP)];
	*FCS =  CalcCRC32 ((PUCHAR) framebuf, nRead + sizeof(DOT11RFC1042ENCAP));

	FILE * fout = fopen ( pmfile, "wb" );
	if ( fout == NULL ) {
		printf ( "Fail to open output file!\n" );
		return -1;
	}
	fwrite ( framebuf, 1, nRead + sizeof(DOT11RFC1042ENCAP) + 4, fout );
	fclose ( fout );
	
    return 0;
}
