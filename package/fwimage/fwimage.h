
#define FIH_CARD_UNKNOWN 0
#define FIH_CARD_OTU4G   1
#define FIH_CARD_ATU10F  2
#define FIH_CARD_TTU4G   3
#define FIH_CARD_DCU     4
#define FIH_CARD_MXDH    5
#define FIH_CARD_M0DH0   5
#define FIH_CARD_M0CS0   6
#define FIH_CARD_MSCX    7
#define FIH_CARD_OPSU    8
#define FIH_CARD_OTU4C    9
#define FIH_CARD_OTU4GO    10
#define FIH_CARD_TTU4GO    11
#define FIH_CARD_MSUO_DCM    12
#define FIH_CARD_MSU_DCM    13
#define FIH_CARD_DCUE     14
#define FIH_CARD_M0AH1     15
#define FIH_CARD_ETU4G   16
#define FIH_CARD_OTU4H   17		//OTU4H
#define FIH_CARD_ROE3J   18     //ROE3J
#define FIH_CARD_OTU4X   19     //OTU4X
#define FIH_CARD_OTU4XO  20     //OTU4XO
#define FIH_CARD_LMUXX  21     //LMU1A/1B/2A/2B
#define FIH_CARD_EAG6L  26     //LMU1A/1B/2A/2B
#define FIH_CARD_MAX 22


#define FIH_MAGIC	0x48534e38	/* Image Magic Number "HSN8"		*/
#define FIH_NMLEN		32	/* Image Name Length		*/
#define FIH_VERSIONLEN		16	/* Version Length		*/

/*
 * all data in network byte order (aka natural aka bigendian)
 */

typedef struct fw_image_header {
	unsigned int	fih_magic;	/* Image Header Magic Number	*/
	unsigned int	fih_hcrc;	/* Image Header CRC Checksum	*/
	unsigned int	fih_time;	/* Image Creation Timestamp	*/
	unsigned int	fih_size;	/* Image Data Size		*/
	unsigned int	fih_card_type;	/* card type		*/
	unsigned int	fih_dcrc;	/* Image Data CRC Checksum	*/
	char		fih_ver[FIH_VERSIONLEN];	/* Image Version		*/
	char		fih_name[FIH_NMLEN];	/* Image Name		*/
	char		reserv[8];	/* reserv		*/
} fw_image_header_t;



