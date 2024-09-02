
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>		/* for host / network byte order conversions	*/
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


#ifndef	O_BINARY		
#define O_BINARY	0
#endif

#include "fwimage.h"

extern int errno;

#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

char *cmdname;

extern unsigned long crc32 (unsigned long crc, const char *buf, unsigned int len);

typedef struct table_entry {
	int	val;		/* as defined in image.h	*/
	char	*sname;		/* short (input) name		*/
	char	*lname;		/* long (output) name		*/
} table_entry_t;

table_entry_t card_name[] = {
	{	FIH_CARD_UNKNOWN ,		NULL,		"Unknown",	},
	{	FIH_CARD_OTU4G,		"otu4g",		"OTU4G",	},
	{	FIH_CARD_ATU10F,	"atu10f",		"ATU10F",	},
	{	FIH_CARD_TTU4G,		"ttu4g",		"TTU4G",	},
	{	FIH_CARD_DCU,		"dcu",			"DCU",		},
	{	FIH_CARD_MXDH,		"mxdh",			"MXDH",		},
	{	FIH_CARD_M0DH0,		"m0dh0",		"M0DH0",	},
	{	FIH_CARD_M0CS0,		"m0cs0",		"M0CS0",	},
	{	FIH_CARD_MSCX,		"mscx",			"MSCX",		},
	{	FIH_CARD_OPSU,		"opsu",			"OPSU",		},
	{	FIH_CARD_OTU4C,		"otu4c",		"OTU4C",	},
	{	FIH_CARD_OTU4GO,		"otu4go",		"OTU4GO",	},
	{	FIH_CARD_TTU4GO,		"ttu4go",		"TTU4GO",	},
	{	FIH_CARD_MSUO_DCM,		"msuodcm",		"MSUODCM",	},
	{	FIH_CARD_MSU_DCM,		"msudcm",		"MSUDCM",	},
	{	FIH_CARD_DCUE,		"dcue",			"DCUE",		},
	{	FIH_CARD_M0AH1,		"m0ah1",			"M0AH1",		},
	{	FIH_CARD_ETU4G,		"etu4g",			"ETU4G",		},
	{	FIH_CARD_OTU4H,		"otu4h",			"OTU4H",		},
	{	FIH_CARD_ROE3J,		"roe3j",			"ROE3J",		},
	{	FIH_CARD_OTU4X,		"otu4x",			"OTU4X",		},
	{	FIH_CARD_OTU4XO,		"otu4xo",			"OTU4XO",		},
	{   FIH_CARD_LMUXX,     "lmuxx",            "LMUXX",        },
	{   FIH_CARD_EAG6L,     "eag6l",            "EAG6L",        },
	{	-1,			"",		"",		},
};


static	void	copy_file (int, const char *, int);
static	void	usage	(void);
static	void	print_header (fw_image_header_t *);
static	void	print_type (fw_image_header_t *);
static	char	*put_table_entry (table_entry_t *, char *, int);
static	char	*put_cardname (int);
static	int	get_table_entry (table_entry_t *, char *, char *);
static	int	get_cardname(char *);


char	*versionstr;
char	*datafile;
char	*imagefile;
int dflag    = 0;
int eflag    = 0;
int lflag    = 0;
int vflag    = 0;


int opt_card   = FIH_CARD_OTU4G;


fw_image_header_t header;
fw_image_header_t *hdr = &header;

int main (int argc, char **argv)
{
	int ifd,i;
	uint32_t checksum;
	uint32_t addr;
	uint32_t ep;
	struct stat sbuf;
	unsigned char *ptr;
	char *name = "";

	cmdname = *argv;

	addr = ep = 0;

	while (--argc > 0 && **++argv == '-') {
		while (*++*argv) {
			switch (**argv) {
				case 'l':
					lflag = 1;
					break;
				case 'C':
					if ((--argc <= 0) ||
							(opt_card = get_cardname(*++argv)) < 0)
						usage ();
					goto NXTARG;
				case 'd':
					if (--argc <= 0)
						usage ();
					datafile = *++argv;
					dflag = 1;
					goto NXTARG;
				case 'n':
					if (--argc <= 0)
						usage ();
					name = *++argv;
					goto NXTARG;
				case 'v':
					if (--argc <= 0)
						usage ();
					versionstr = *++argv;
					vflag = 1;
					goto NXTARG;	
					break;
				default:
					usage ();
			}
		}
NXTARG:		;
	}

	if ((argc != 1) || ((lflag ^ dflag) == 0))
		usage();


	imagefile = *argv;

	if (lflag) {
		ifd = open(imagefile, O_RDONLY|O_BINARY);
	} else {
		ifd = open(imagefile, O_RDWR|O_CREAT|O_TRUNC|O_BINARY, 0666);
	}

	if (ifd < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
				cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	//printf( "header size  %x \n",sizeof(fw_image_header_t));
	if (lflag) {
		int len;
		char *data;
		/*
		 * list header information of existing image
		 */
		if (fstat(ifd, &sbuf) < 0) {
			fprintf (stderr, "%s: Can't stat %s: %s\n",
					cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		if ((unsigned)sbuf.st_size < sizeof(fw_image_header_t)) {
			fprintf (stderr,
					"%s: Bad size: \"%s\" is no valid image\n",
					cmdname, imagefile);
			exit (EXIT_FAILURE);
		}

		ptr = (unsigned char *)mmap(0, sbuf.st_size,
				PROT_READ, MAP_SHARED, ifd, 0);
		if ((caddr_t)ptr == (caddr_t)-1) {
			fprintf (stderr, "%s: Can't read %s: %s\n",
					cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		/*
		 * create copy of header so that we can blank out the
		 * checksum field for checking - this can't be done
		 * on the PROT_READ mapped data.
		 */
		memcpy (hdr, ptr, sizeof(fw_image_header_t));

		if (ntohl(hdr->fih_magic) != FIH_MAGIC) {
			fprintf (stderr,
					"%s: Bad Magic Number: \"%s\" is no valid image\n",
					cmdname, imagefile);
			exit (EXIT_FAILURE);
		}

		data = (char *)hdr;
		len  = sizeof(fw_image_header_t);

		checksum = ntohl(hdr->fih_hcrc);
		hdr->fih_hcrc = htonl(0);	/* clear for re-calculation */

		if (crc32 (0, data, len) != checksum) {
			fprintf (stderr,
					"%s: ERROR: \"%s\" has bad header checksum!\n",
					cmdname, imagefile);
			exit (EXIT_FAILURE);
		}

		data = (char *)(ptr + sizeof(fw_image_header_t));
		len  = sbuf.st_size - sizeof(fw_image_header_t) ;

		if (crc32 (0, data, len) != ntohl(hdr->fih_dcrc)) {
			fprintf (stderr,
					"%s: ERROR: \"%s\" has corrupted data!\n",
					cmdname, imagefile);
			exit (EXIT_FAILURE);
		}

		/* for multi-file images we need the data part, too */
		print_header ((fw_image_header_t *)ptr);

		(void) munmap((void *)ptr, sbuf.st_size);
		(void) close (ifd);

		exit (EXIT_SUCCESS);
	}

	/*
	 * Must be -w then:
	 *
	 * write dummy header, to be fixed later
	 */
	memset (hdr, 0, sizeof(fw_image_header_t));

	if (write(ifd, hdr, sizeof(fw_image_header_t)) != sizeof(fw_image_header_t)) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
				cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}


	copy_file (ifd, datafile, 0);


	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && !defined(__sun__) && !defined(__FreeBSD__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif

	if (fstat(ifd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
				cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	ptr = (unsigned char *)mmap(0, sbuf.st_size,
			PROT_READ|PROT_WRITE, MAP_SHARED, ifd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		fprintf (stderr, "%s: Can't map %s: %s\n",
				cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	hdr = (fw_image_header_t *)ptr;

	checksum = crc32 (0,
			(const char *)(ptr + sizeof(fw_image_header_t)),
			sbuf.st_size - sizeof(fw_image_header_t)
			);

	/* Build new header */
	hdr->fih_magic = htonl(FIH_MAGIC);
	hdr->fih_time  = htonl(sbuf.st_mtime);
	hdr->fih_size  = htonl(sbuf.st_size - sizeof(fw_image_header_t));
	hdr->fih_dcrc  = htonl(checksum);
	hdr->fih_card_type = htonl(opt_card);
	for(i = 0 ; i < 8 ; i++ )
		hdr->reserv[i] = i & 0xff ;

	strncpy((char *)hdr->fih_name, name, FIH_NMLEN);
	if(vflag)
	{
		strncpy((char *)hdr->fih_ver, versionstr, FIH_VERSIONLEN);
	}
	else
	{
		strncpy((char *)hdr->fih_ver, "vx.x.x", FIH_VERSIONLEN);
	}

	checksum = crc32(0,(const char *)hdr,sizeof(fw_image_header_t));

	hdr->fih_hcrc = htonl(checksum);

	print_header (hdr);

	(void) munmap((void *)ptr, sbuf.st_size);

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && !defined(__sun__) && !defined(__FreeBSD__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif

	if (close(ifd)) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
				cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	exit (EXIT_SUCCESS);
}

	static void
copy_file (int ifd, const char *datafile, int pad)
{
	int dfd;
	struct stat sbuf;
	unsigned char *ptr;
	int tail;
	int zero = 0;
	int offset = 0;
	int size;



	if ((dfd = open(datafile, O_RDONLY|O_BINARY)) < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
				cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
				cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	ptr = (unsigned char *)mmap(0, sbuf.st_size,
			PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		fprintf (stderr, "%s: Can't read %s: %s\n",
				cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}


	size = sbuf.st_size - offset;
	if (write(ifd, ptr + offset, size) != size) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
				cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (pad && ((tail = size % 4) != 0)) {

		if (write(ifd, (char *)&zero, 4-tail) != 4-tail) {
			fprintf (stderr, "%s: Write error on %s: %s\n",
					cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}
	}

	(void) munmap((void *)ptr, sbuf.st_size);
	(void) close (dfd);
}

	void
usage ()
{
	fprintf (stderr, "Usage: %s -l image\n"
			"          -l ==> list image header information\n"
			"       %s -C cardname -v x.x.x"
			" -n name -d data_file image\n"
			"          -C ==> set cardname [out4g,ttu4g,atu10f,dcu,mxdh,mscx..]\n"
			"          -v ==> version string eg v0.0.1\n"
			"          -n ==> name string \n"
			"          -d datafile \n",
			cmdname, cmdname);
	exit (EXIT_FAILURE);
}


char *format_time_str(char *pszBuf, size_t nLen, 
		char *pszFormat, time_t nTime)
{
	struct tm *tm = NULL;

	if (NULL == pszBuf || NULL == pszFormat)
	{
		return NULL;
	}

	tm = localtime(&nTime);
	strftime(pszBuf, nLen, pszFormat, tm);
	return pszBuf;
}


	static void
print_header (fw_image_header_t *hdr)
{
	time_t timestamp;
	uint32_t size;
	char createtime[32];

	timestamp = (time_t)ntohl(hdr->fih_time);
	size = ntohl(hdr->fih_size);

	printf ("Image Name:   %.*s\n", FIH_NMLEN, hdr->fih_name);
	format_time_str(createtime, 32,"%Y-%m-%d %H:%M:%S", timestamp);
	printf ("Created:      %s(%x)\n", createtime,timestamp);
	printf ("Card Type:   "); print_type(hdr);
	printf ("Version:      %.*s\n",FIH_VERSIONLEN, hdr->fih_ver);
	printf ("Data Size:    %d Bytes = %.2f kB = %.2f MB\n",
			size, (double)size / 1.024e3, (double)size / 1.048576e6 );

}


	static void
print_type (fw_image_header_t *hdr)
{
	printf (" %s\n",		put_cardname (htonl(hdr->fih_card_type)));
}

static char *put_cardname (int card)
{
	return (put_table_entry(card_name, "Unknown Card", card));
}




static char *put_table_entry (table_entry_t *table, char *msg, int type)
{
	for (; table->val>=0; ++table) {
		if (table->val == type)
			return (table->lname);
	}
	return (msg);
}

static int get_cardname(char *name)
{
	return (get_table_entry(card_name, "Card", name));
}



static int get_table_entry (table_entry_t *table, char *msg, char *name)
{
	table_entry_t *t;
	int first = 1;

	for (t=table; t->val>=0; ++t) {
		if (t->sname && strcasecmp(t->sname, name)==0)
			return (t->val);
	}
	fprintf (stderr, "\nInvalid %s Type - valid names are", msg);
	for (t=table; t->val>=0; ++t) {
		if (t->sname == NULL)
			continue;
		fprintf (stderr, "%c %s", (first) ? ':' : ',', t->sname);
		first = 0;
	}
	fprintf (stderr, "\n");
	return (-1);
}
