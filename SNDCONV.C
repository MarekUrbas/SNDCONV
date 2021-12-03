#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>
#include <io.h>

void		main(int argc,char **argv);
void		getSoundInfo(char *name,int type);
void		WriteWaveHeader(FILE *fh);

enum filetypes { RAW, GSS, VOC, WAV, SND, VMD };

#define NUMTYPES 6
char		*typelist[] = {
												"RAW",
												"GSS",
												"VOC",
												"WAV",
												"SND",
												"VMD"

											};
int			skipsize[] =  {
												0x00,
												0x40,
												0x20,
												0x2C,
												0x00,
												0x00
											};

typedef struct
{
	long		length;
	long		samprate;
} SOUND;

SOUND		sinfo;
long		gSampleRate;


void		main(int argc,char **argv)
{
	FILE		*infh,*outfh;
	int			xcon,i,skip,intype,outtype;
	unsigned read;
	char		inname[13],outname[13],t[13],t1[13],inext[5],outext[5];
	char		bffr[50000U];

	if (argc < 3 || argc > 4) 	// Means 3 or 4 parameters accepted
	{
		printf("Usage: %s (fromname.ext) (toname.ext) [samplerate]\n",argv[0]);
		printf("       samplerate is 11000 if not entered.\n");
		printf("Valid input file types: .WAV a Microsoft WAVE file.\n");
		printf("                       .VOC a Soundblaster RAW .VOC file.\n");
		printf("                       .GSS ??\n");
		printf("                       .SND A raw file.\n");
		printf("                       .RAW A raw file.\n");
		printf("                       .VMD A raw file.\n");
		printf("Valid output types are .WAV and .RAW.  Other file\n");
		printf("formats to be added at a later date.\n");
		exit(1);
	}

	strcpy(t,argv[1]);
	strcpy(inname,argv[1]);
	strcpy(t1,strtok(t,"."));
	strcpy(inext,strtok(NULL,"."));
	strupr(inext);
	for (i=0,intype=0; i<NUMTYPES; i++)
	{
		if (!strcmp(typelist[i],inext))
		{
			intype = i;
			break;
		}
	}

	strcpy(t,argv[2]);
	strcpy(outname,argv[2]);
	strcpy(t1,strtok(t,"."));
	strcpy(outext,strtok(NULL,"."));
	strupr(outext);
	for (i=0,outtype=0; i<NUMTYPES; i++)
	{
		if (!strcmp(typelist[i],outext))
		{
			outtype = i;
			break;
		}
	}

	if (access(inname,0) == -1)
	{
		printf("Error: File not found: %s\n",inname);
		exit(1);
	}

	gSampleRate = 11000L;
	if (argc == 4)
	{
		gSampleRate = atol(argv[2]);
	}

	getSoundInfo(inname,intype);


	infh = fopen(inname,"rb");
	outfh = fopen(outname,"wb");

	skip = skipsize[intype];
	fread(bffr,1,skip,infh);			// Skip header stuff from input format

	switch (outtype)
	{
		case RAW:
			break;
		case GSS:
			break;
		case VOC:
			break;
		case WAV:
			WriteWaveHeader(outfh);
	}

	xcon = 0;
	do
	{
		read = fread(bffr,1,50000U,infh);
		if (read)
			fwrite(bffr,1,read,outfh);
		else
			xcon = 1;
	} while (!xcon);
}


void		getSoundInfo(char *name,int type)
{
	FILE		*fh;
	struct stat statbuf;

	stat(name,&statbuf);

	switch (type)
	{
		case RAW:
		case SND:
		case VMD:
			sinfo.length = statbuf.st_size;
			sinfo.samprate = gSampleRate;
			break;
		case GSS:
			sinfo.length = statbuf.st_size - (long)skipsize[type];
			sinfo.samprate = gSampleRate;
			break;
		case VOC:
			sinfo.length = statbuf.st_size - (long)(skipsize[type]+1);
			sinfo.samprate = gSampleRate;
			break;
		case WAV:
			fh = fopen(name,"rb");
			fseek(fh,24L,SEEK_SET);					// Offset to Sample rate
			fread(&sinfo.samprate,4,1,fh);	// Read Sample rate
			fseek(fh,40L,SEEK_SET);					// Offset to Sample length
			fread(&sinfo.length,4,1,fh);		// Read Sample length
			fclose(fh);
			break;
	}
}


void		WriteWaveHeader(FILE *fh)
{
	long		bl;
	int			bi;

	fwrite("RIFF",4,1,fh);			// Write "RIFF"
	bl = sinfo.length + 36L;
	fwrite(&bl,4,1,fh);					// Write Size of file with header
	fwrite("WAVE",4,1,fh);			// Write "WAVE"
	fwrite("fmt ",4,1,fh);			// Write "fmt "
	bl = 16L;
	fwrite(&bl,4,1,fh);					// Size of previous header (fixed)
	bi = 1;
	fwrite(&bi,2,1,fh);					// formatTag
	fwrite(&bi,2,1,fh);					// nChannels
	bl = sinfo.samprate;
	fwrite(&bl,4,1,fh);					// nSamplesPerSec
	fwrite(&bl,4,1,fh); 				// nAvgBytesPerSec
	fwrite(&bi,2,1,fh);					// nBlockAlign (always 1?)
	bi = 8;
	fwrite(&bi,2,1,fh);					// nBitsPerSample (8 or 16 I assume)
	fwrite("data",4,1,fh);			// Write "data"
	bl = sinfo.length;
	fwrite(&bl,4,1,fh);					// True length of sample data
}
