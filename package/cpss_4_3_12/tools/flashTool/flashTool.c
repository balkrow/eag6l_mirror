#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <flashTool.h>
#include <mvSFlash.h>
#include <errno.h>
#include "slave_smi.h"
#include "imgHdr.h"
#include "fileOps.h"

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) printf("DEBUG: " __VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif


/* to simplify code Domain, bus,dev,func*/
#define DBDF pciDomain, pciBus, pciDev, pciFunc
#define DBDF_DECL        \
    uint32_t  pciDomain, \
    uint32_t  pciBus,    \
    uint32_t  pciDev,    \
    uint32_t  pciFunc

#define AC5P_MG_BASE_ADDRESS       0x3C200000
#define AC5P_SPI_BASE_ADDRESS      0x3D000000
#define AC5P_SSMI_XBAR_PORT_REMAP  0x3D013100
#define MG0_OFFSET_AC5P            0x3C200000
#define MG1_OFFSET_AC5P            0x3C300000
#define MG2_OFFSET_AC5P            0x3C400000
#define MG3_OFFSET_AC5P            0x3C500000
#define EXTERNAL_MG_OFFSET         0x00000000

#define INDEX_COUNT_UP 1
#define VERIFICATION_FILE_PATH "/tmp/temp_flash_read_file"
#define ETH_POLYNOM 0xedb88320   /* for crc calc*/
#define ATU_NUM        0         /* ATU #0 used by default.
                                    If CPSS driver running in parallel, must use ATU > 3 */
#define PARTITION_SIZE 0x200000  /* SPI flash partition size 2MB */
#define SI_OFFSET      0x40000   /* offset of super image in partition */

void *pp_space_ptr = NULL;
void *pp2_space_ptr = NULL;
void *dfx_space_ptr = NULL;
void *mg_space_ptr = NULL;

void *read_file_in_mem = NULL;

MV_SFLASH_INFO flash_info_instance;
MV_SFLASH_INFO *pflash_info_instance = &flash_info_instance;
bool verbose = 0;
bool smiFlag = 0;

/**
 * Function description:
 *   safe_open_fd opens fd according to path.
 *   creation of the file depends on 'create' bool input
 *   variable.
 *
 * @param path - full file path.
 * @param create - 1:create if doesn't exist. 0:do not create.
 *
 * @retval (-1) - Error.
 * @retval (positive value) - File descriptor number.
 */
static int safe_open_fd(char *path, bool create){
	int fd;

	if (create) {/*read command*/
		fd = open(path, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
	}
	else{
		fd = open(path, O_RDWR, S_IRUSR | S_IWUSR);
	}

	if (fd == (-1)) {
		printf("Opening %s failed! \n", path);
		perror("Error Description: ");
		return -1;
	}
	return fd;
}

int sysfs_pci_open(
	DBDF_DECL,
	const char *name,
	int     flags,
	int     *fd
)
{
	char fname[128];
	FILE *f;
	if (pciBus > 255 || pciDev > 31 || pciFunc > 7)
		return -1;

    sprintf(fname, "/sys/bus/pci/devices/%04x:%02x:%02x.%x/enable", DBDF);
	f = fopen(fname, "w");
	if (f == NULL)
	{
        printf("Error enabling PCI device at %s\n", fname);
		return -1;
	}

	fputs("1",f);
	fclose(f);

    sprintf(fname, "/sys/bus/pci/devices/%04x:%02x:%02x.%x/%s",
            DBDF, name);
	*fd = open(fname, flags);
	if (*fd < 0) {
		perror(fname);
		return -1;
	}
	return 0;
}

int sysfs_pci_map(
	DBDF_DECL,
	const char *res_name,
	int flags,
	int   *fd,
	void **vaddr
)
{
	int rc;
	struct stat st;

	rc = sysfs_pci_open(DBDF, res_name, flags, fd);
	if (rc != 0) {
		perror(res_name);
		return rc;
	}

	if (fstat(*fd, &st) < 0) {
		close(*fd);
		return -1;
	}
	*vaddr = mmap(NULL,
				st.st_size,
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				*fd,
				(off_t)0);
	if (MAP_FAILED == *vaddr) {
		perror("mmap");
		close(*fd);
		return -1;
	}
	return 0;
}

static uint32_t getPortId(uint32_t *offset)
{
    uint32_t port = 0xD;
    int reg = *offset;
    /* in case of address in one of the MG's, clear msb 3 nibbles,
       because address (0x000fffff) will map according to port to correct MG */
    *offset = *offset & 0x000fffff;
	switch( reg & 0xFFF00000) {
		case MG0_OFFSET_AC5P:
			port = 4; /*MG 0 */
			break;
		case MG1_OFFSET_AC5P:
			port = 5; /*MG 1 */
			break;
		case MG2_OFFSET_AC5P:
			port = 6; /*MG 2 */
			break;
		case MG3_OFFSET_AC5P:
			port = 7; /*MG 3 */
			break;
		case EXTERNAL_MG_OFFSET:
			port = 9; /* External MG */
			break;
		default:
			*offset = reg;
	}
    return port;
}

uint32_t read_register(uint32_t address)
{
	uint32_t ret_val = 0, port;
	MV_STATUS rc;

	if(!smiFlag)
	{
		*(volatile uint32_t *)(mg_space_ptr + 0x1314 + ATU_NUM * 0x200) = address & 0xfff00000;
		ret_val = *(volatile uint32_t *)(pp_space_ptr + (address & 0x000fffff));
		if (verbose) {
			printf("PCIE read: address:0x%08x(%p) value:0x%08x\n", address, (volatile uint32_t *)(pp_space_ptr + (address & 0x000fffff)), ret_val);
		}
	}
	else
	{
		port = getPortId(&address);
		rc   = slaveSmiRead(0 /* BusId */,0x6 /* phyAddr */, port, address, &ret_val);
		if (verbose) {
			if ( rc != MV_OK)
				printf("returned with error: %x\n",rc);
			else
				printf("SMI read: address:0x%08x value:0x%08x\n",address, ret_val);
		}
	}
	return  ret_val;
}


void write_register(uint32_t address, uint32_t value)
{
	uint32_t port;
	MV_STATUS rc;

	if(!smiFlag)
	{
		*(volatile uint32_t *)(mg_space_ptr + 0x1314 + ATU_NUM * 0x200) = address & 0xfff00000;
		*(volatile uint32_t *)(pp_space_ptr + (address & 0x000fffff)) = value;
		if (verbose) {
			printf("PCIE write: address:0x%08x(%p) value:0x%08x\n", address, (volatile uint32_t *)(pp_space_ptr + (address & 0x000fffff)), value);
		}
	}
	else
	{
		port = getPortId(&address);
		rc   = slaveSmiWrite(0 /* BusId */,0x6 /* phyAddr */, port, address, &value);
		if (verbose) {
			if ( rc != MV_OK)
				printf("returned with error: %x\n",rc);
			else
				printf("SMI write: address:0x%08x value:0x%08x\n",address, value);
		}
	}

}

static MV_STATUS erase_cmd(char interface, MV_U32 offset, MV_U32 size)
{
	MV_U32 num_of_sectors_to_erase = 0;
	MV_STATUS rc = MV_FAIL;
	int i;

	if ((offset + size) > mvSFlashSizeGet(interface))
	{
		printf("Error - ffset and size exceed flash size.\nflash_size:0x%x offset:0x%x byte_size:0x%x\n", mvSFlashSizeGet(interface), offset, size);
		return MV_FAIL;
	}

	num_of_sectors_to_erase = size / pflash_info_instance->sectorSize;
	if ((int)size - (int)(pflash_info_instance->sectorSize * num_of_sectors_to_erase) > 0)
		num_of_sectors_to_erase += 1;
	for (i = 0; i < num_of_sectors_to_erase; i++)
	{
		if ((rc = mvSFlashSectorErase(interface, (offset / pflash_info_instance->sectorSize) + i)) != MV_OK)
		{
			printf("mvSFlashSectorErase failed.\n offset:0x%x numOfSectors:%d current sector index = %d\n", offset, num_of_sectors_to_erase, ((offset / pflash_info_instance->sectorSize) + i));
			return rc;
		}
	}

	return MV_OK;
}

/********************************************************************
 * read_cmd - read from flash to buffer or to file
 *            optionally validate image / calculate crc
 *
 * Inputs:
 *          offset - offset in flash to read from (in bytes)
 *          buffer - pre-allocated buffer for storing read data
 *                   ignored if saving to file
 *          size - size to read
 *          validate_img - should image be validated as super-image
 *          filename - save as file, optional, can be NULL
 * Outputs:
 *          crc - optionl, if not NULL return crc32 of data
 *
 * Return value - MV_OK if successful, error code otherwise
 *******************************************************************/
static MV_STATUS read_cmd(
		char        interface,
		MV_U32      offset,
		uint8_t    *buffer,
		MV_U32      size,
		bool        validate_img,
		char       *filename,
		MV_U32     *crc)
{
	MV_STATUS rc = MV_OK;
	int fd = -1;

	if (buffer == NULL && filename == NULL)
	{
		printf("%s: wrong input, both buffer and filename are NULL\n", __func__);
		return MV_ERROR;
	}

	if ((offset + size > mvSFlashSizeGet(interface)))
	{
		printf("Error - offset and size exceed flash size.\nflash_size:0x%x offset:0x%x byte_size:0x%x\n", mvSFlashSizeGet(interface), offset, size);
		return MV_FAIL;
	}

	if (filename != NULL)
	{
		/* Output file name given, create file, map it */
		fd = safe_open_fd(filename, 1);
		if (fd == -1)
			return MV_ERROR;
		ftruncate(fd, size);
		buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (buffer == MAP_FAILED)
		{
			perror("mmap failed");
			close(fd);
			return MV_FAIL;
		}
		memset(buffer, 0, size);
	}

	/* read into buffer */
	rc = mvSFlashBlockRd(interface, offset, buffer, size);
	if (rc != MV_OK)
		printf("Flash block read failed.\n");

	if (validate_img)
		if (0 != validate_image(buffer))
		{
			printf("Image at offset 0x%x is not valid\n", offset);
			rc = MV_FAIL;
		}

	if (crc != NULL)
		*crc = check_crc(buffer, size);

	if (fd != -1)
		close(fd);
	return rc;
}

/********************************************************************
 * write_cmd - write to flash from buffer or from file
 *            optionally validate image
 *
 * Inputs:
 *          offset - offset in flash to write to (in bytes)
 *          buffer - buffer with data to write
 *                   ignored if write from file
 *          size - size to write, ignored if write from file
 *          validate_img - Only validate provided file / buffer
 *                         as super-image, do not write to flash
 *          filename - file to write, optional, can be NULL
 *
 * Return value - MV_OK if successful, error code otherwise
 *******************************************************************/
static MV_STATUS write_cmd(
		char        interface,
		MV_U32      offset,
		uint8_t    *buffer,
		MV_U32      size,
		bool        validate_img,
		char       *filename)
{

	struct stat file_stat;
	MV_STATUS rc = MV_OK;
	int fd = -1;
	MV_U32 crcWrite, crcRead;
	uint8_t *read_buffer = NULL;

	if (buffer == NULL && filename == NULL)
	{
		printf("%s: wrong input, both buffer and filename are NULL\n", __func__);
		return MV_ERROR;
	}

	if (filename != NULL)
	{
		/* Input file name given, open file, map it, check size */
		fd = safe_open_fd(filename, 0);
		if (fd == -1)
			return MV_ERROR;

		rc = fstat(fd, &file_stat);
		if (rc < 0)
		{
			perror("fstat failed");
			close(fd);
			return MV_FAIL;
		}
		size = file_stat.st_size;

		buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (buffer == MAP_FAILED)
		{
			perror("mmap failed");
			close(fd);
			return MV_FAIL;
		}
	}

	/* Validate super image if required */
	if (validate_img)
	{
		printf("Validating provided image...\n");
		if (0 != validate_image(buffer))
		{
			printf("failed\n");
			rc = MV_FAIL;
		}
		goto write_exit;
	}

	/* Check boudaries */
	if (offset + size > mvSFlashSizeGet(interface))
	{
		printf("Error - Invalid input.\nfile_size:(%ld) cannot fit into flash at offset:0x%x\n", file_stat.st_size, offset);
		rc = MV_FAIL;
		goto write_exit;
	}

	/* Calculate crc of data to be written */
	crcWrite = check_crc(buffer, size);

	/* erase rounds up size to block_size */
	rc = erase_cmd(interface, offset, size);
	if (rc != MV_OK)
	{
		printf("Erase failed\noffset:0x%x size:0x%x[B]\n", offset, size);
		goto write_exit;
	}

	/* Write to flash */
	rc = mvSFlashBlockWr(interface, offset, buffer, size);
	if (rc != MV_OK)
	{
		printf("Flash block write failed.\n");
		goto write_exit;
	}

	/* Read back from flash and compare crc */
	read_buffer = malloc(size);

	if (read_buffer == NULL)
	{
		printf("%s: Failed to allocate %d bytes\n", __func__, size);
		rc = MV_FAIL;
		goto write_exit;
	}

	rc = read_cmd(interface, offset, read_buffer, size, false, NULL, &crcRead);
	if (rc != MV_OK)
	{
		printf("READ failed\noffset:0x%x size:0x%x[B]\n", offset, size);
		goto write_exit;
	}
	if (crcRead != crcWrite)
	{
		printf("Error - read data differs from write data. \n");
		return MV_FAIL;
	}

write_exit:
	if (read_buffer)
		free(read_buffer);
	if (fd != -1)
	{
		munmap(buffer, size);
		close(fd);
	}
	return rc;
}

/********************************************************************
 * write_image - write super image from file to partition
 *               Verify integrity of image in other partition first
 *
 * Inputs:
 *          partition - partition number (0 - 1)
 *          filename - file to write
 *
 * Return value - MV_OK if successful, error code otherwise
 *******************************************************************/
static MV_STATUS write_image(char interface, int partition, char *filename)
{
	int size = PARTITION_SIZE - SI_OFFSET;
	uint32_t offset, other_offset;
	uint8_t *read_buffer;
	MV_STATUS rc = MV_OK;

	if (filename == NULL)
	{
		printf("%s: filename not provided\n", __func__);
		return MV_ERROR;
	}
	offset = partition * PARTITION_SIZE + SI_OFFSET;
	other_offset = ((partition + 1) % 2)  * PARTITION_SIZE + SI_OFFSET;

	printf("File: %s, partition %d\n", filename, partition);

	/* Check image in other partition */
	printf("Testing image on other partition:\n");
	read_buffer = malloc(size);
	if (read_buffer == NULL)
	{
		printf("%s: Failed to allocate %d bytes\n", __func__, size);
		return MV_ERROR;
	}
	rc = read_cmd(interface, other_offset, read_buffer, size, true, NULL, NULL);
	free(read_buffer);
	if (rc != MV_OK)
	{
		printf("Image at partition %d is corrupted, please replace it first\n", ((partition + 1) % 2));
		return rc;
	}
	printf("Pass.\nWriting image to partition %d\n", partition);

	/* Write image */
	rc = write_cmd(interface, offset, NULL, 0, false, filename);
	if (rc != MV_OK)
		printf("Failed to write Image at partition %d\n", partition);

	return rc;
}

static void print_usage(char *name)
{
	printf("Usage:\n");
	printf("       %s  [DBDF]/SMI [interface] <command> parameters\n", name);
	printf(" DBDF - DBDF - Domain Bus Device Function of PCIe device in DDDD:BB:DD.F, default 0000:01:00.0\n");
	printf(" SMI         - Interface configuration through SSMI\n");
	printf(" interface - number of SPI interface to use, default 0\n");
	printf(" Commands:\n");
	printf("       write <file>  <offset>         - write file to flash offset\n");
	printf("       read  <file>  <offset>  <size> - read size (byte)s from flash offset to file\n");
	printf("       erase <offset> <size>          - erase size (bytes) from flash offset \n");
	printf("       image <file>  <part>           - write image to flash partition [0, 1], 2 means both\n");
	printf("       -w <offset> <value>            - write PP register\n");
	printf("       -r <offset>                    - read PP register\n");
}
int main(int argc, char *argv[])
{
	int fd, byte_amount, partition;
	int i = 0;
	int rc;
	FILE *f;

	int mg_fd;

	unsigned long long start, end, flags;
	unsigned long long res2;

	char *filename, *ptr = NULL, shift = 0, interface = 0;
	unsigned int pciDomain, pciBus, pciDev, pciFunc;
	uint32_t offset = 0;
	uint32_t valid_input = 1, valueToWrite = 0;
	char *command_names[8] = {"read", "write", "erase", "image", "-r", "-w"};
	enum COMMANDS {READ, WRITE, ERASE, IMAGE, REG_R, REG_W, INVALID} command = INVALID;

	/* 
	 * Check input arguments:
	 * We expect, optionally, DBDF and interface, that's 2
	 * Max is name + DBDF + interface + longest command (4) = 7
	 * Min is name + shortest command (2) = 3
	 */
	if (argc > 7 || argc < 3)
	{
		print_usage(argv[0]);
		return 0;
	}

    /* scan DBDF, expect format DDDD:BB:DD.F */
    rc = sscanf(argv[1], "%04x:%02x:%02x.%01x%n", &pciDomain, &pciBus, &pciDev, &pciFunc, &i);

    if (rc == 4 && i == 12) /* 4 numbers read, total length is 12 characters */
        shift++;
    else
    {
        if ((!strcmp(argv[1], "SMI")))
        {
            smiFlag++;
            shift++;
        }
        else /* Default 0000:01:00.0 */
        {
            pciBus = 1;
            pciDomain = pciDev = pciFunc = 0;
        }
    }

	/* Check for interface number */
	interface = strtol(argv[1 + shift], &ptr, 0);
	if (ptr == argv[shift + 1] + strlen(argv[shift + 1]))
		shift++;

    if (!smiFlag)
		printf ("DBDF: %04x:%02x:%02x.%01x interface: %d\n", pciDomain, pciBus, pciDev, pciFunc, interface);
	else
	{
        if (MV_OK != slaveSmiInit(0 /* BusId */,0x6 /* phyAddr */, AC5P_SSMI_XBAR_PORT_REMAP))
        {
            printf("Failed to initialize SSMI\n");
            return -1;
        }
	}

	/* Shift argv[] back if DBDF and/or interface present, so command starts at argv[1] */
	if (shift > 0)
	{
		argc -= shift;
		for (i = 1; i < argc; i++)
			argv[i] = argv[i + shift];
	}

	/* Check if command is valid */
	for (i = 0; i < 6; i++)
		if (!strcmp(argv[1], command_names[i]))
			command = i;

	switch (command)
	{
	case READ:
		if (argc < 5) {valid_input = 0; break;}
		byte_amount = strtol(argv[4], NULL, 0);
		if (byte_amount == 0 && errno == EINVAL)
		{
			printf("%s\n", STR_TO_BYTES_ERR);
			return -1;
		}
	case WRITE:
		if (argc < 4) {valid_input = 0; break;}
		filename = argv[2];
		offset = strtol(argv[3], NULL, 0);
		break;
	case ERASE:
		if (argc < 4) {valid_input = 0; break;}
		offset = strtol(argv[2], NULL, 0);
		byte_amount = strtol(argv[3], NULL, 0);
		if (byte_amount == 0 && errno == EINVAL)
		{
			printf("%s\n", STR_TO_BYTES_ERR);
			return -1;
		}
		break;
	case IMAGE:
		if (argc < 4) {valid_input = 0; break;}
		filename = argv[2];
		partition = strtol(argv[3], NULL, 0);
		if ((partition == 0 && errno == EINVAL) | (partition > 2))
		{
			printf("Partition number (\"%s\") not in range\n", argv[3]);
			return -1;
		}
		break;
	case REG_R:
		if (argc < 3) {valid_input = 0; break;}
		offset = strtol(argv[2], NULL, 0);
		break;
	case REG_W:
		if (argc < 4) {valid_input = 0; break;}
		offset = strtol(argv[2], NULL, 0);
		valueToWrite = strtol(argv[3], NULL, 0);
		if (valueToWrite == 0 && errno == EINVAL)
		{
			printf("%s\n", STR_TO_BYTES_ERR);
			return -1;
		}
		break;
	default:
		valid_input = 0;
	}

	if ( !valid_input )
	{
		print_usage(argv[0]);
		return 0;
	}

	if(!smiFlag)
	{
		/*
		* Map switch device over the PCIe to access its registers
		* BAR 0 provides access to ATUs configuration.
		* BAR 2 provides access to switch registers through ATUs
		* We set ATU size to 1MB each. By default, BAR 2 size is 4MB
		* CPSS driver uses 4 ATUs, therefore consuming all 4MB
		*
		* If running in parallel to CPSS driver,
		*  BAR 2 must be set to 8MB and ATU > 3 must be used
		*  (see ATU_NUM macro)
		* BAR 2 size should be set before Linux starts,
		* preferably by u-boot cli commands
		*/
		rc = sysfs_pci_open(DBDF, "resource", O_RDONLY, &fd);
		if (rc != 0)
			return rc;

		f = fdopen(fd, "r");
		if (f == NULL)
			return -1;
		i = 0;
		while (!feof(f))
		{
			if (fscanf(f, "%lli %lli %lli", &start, &end, &flags) != 3)
				break;
			if (i == 2)
				res2 = start;
			i++;
		}
		fclose(f);

		// Config Aldrin PeX Window 0 to see Switching-Core
		rc = sysfs_pci_map(DBDF, "resource0", O_RDWR, &mg_fd, &mg_space_ptr);
		if (rc != 0)
			return rc;

		/* Init: create mappings for PP and XBAR */
		rc = sysfs_pci_map(DBDF, "resource2", O_RDWR | O_SYNC, &mg_fd, &pp_space_ptr);
		if (rc < 0)
			return -1;


		/* Sip6 Devices */

		/* add 1MB times number of window */
		res2 += ATU_NUM * 0x100000;
		pp_space_ptr += ATU_NUM * 0x100000;
		i = 0x1300 + 0x200 * ATU_NUM;

		/* configure inbound window into the PP MG0, size 1MB, at start of res2 address space */
		/* take this window because it not used by other resources */
		*(unsigned*)(mg_space_ptr + i + 0x04) = 0x80000000; /* enable the window */
		*(unsigned*)(mg_space_ptr + i + 0x08) = (unsigned long)res2 & 0xFFFFFFFF; /* the start of the window */
		*(unsigned*)(mg_space_ptr + i + 0x0c) = 0x0;
		*(unsigned*)(mg_space_ptr + i + 0x10) = ((unsigned long)(res2 & 0xFFFFFFFF)) | 0xfffff; /* the end of the window (window size is 0xfffff) */
		*(unsigned*)(mg_space_ptr + i + 0x14) = 0x3C200000; /* we can access sram_base address through this window */
		*(unsigned*)(mg_space_ptr + i + 0x1c) = 0x0;

		/********************************END PCIE WINDOW CONFIGURATIONS*************************************/
	}

	/*Init flash*/
	if (command < REG_R)
	{
		rc = mvSFlashInit(interface, pflash_info_instance, 300000000);
		if (rc != MV_OK)
		{
			printf("Error:%d\n", ERROR_UNINITILIZED_E);
			return 1;
		}

		printf("Found flash device:\nmanId:0x%x  devId:0x%x total_size:0x%x page_size:0x%x sectore_size:0x%x\n\n",
			   pflash_info_instance->manufacturerId, pflash_info_instance->deviceId, mvSFlashSizeGet(interface), pflash_info_instance->pageSize, pflash_info_instance->sectorSize);
	}

	switch (command)
	{
	case WRITE:
		rc = write_cmd(interface, offset, NULL, 0, false, filename);
		if (rc != MV_OK)
		{
			printf("Flash block write failed.\n");
			return -1;
		}
		printf("File %s written to flash offset: 0x%x successfully\n", filename, offset);
		break;

	case READ:
		rc = read_cmd(interface, offset, NULL, byte_amount, false, filename, NULL);
		if (rc != MV_OK)
		{
			printf("Read failed\n");
			return MV_FAIL;
		}
		printf("Successfully read 0x%x bytes @offset 0x%x to file %s\n", byte_amount, offset, filename);
		break;

	case ERASE:
		rc = erase_cmd(interface, offset, byte_amount);
		if (rc != MV_OK)
		{
			printf("Erase failed\noffset:0x%x byte_amount:0x%x[B]\n", offset, byte_amount);
			return MV_FAIL;
		}

		i = byte_amount / pflash_info_instance->sectorSize;
		if (byte_amount % pflash_info_instance->sectorSize)
			i++;
		byte_amount = i * pflash_info_instance->sectorSize;
		printf("Erased 0x%x[B] at 0x%x.\n", byte_amount, offset);
		break;

	case IMAGE:
		if (MV_OK != write_cmd(interface, offset, NULL, 0, true, filename))
		{
			printf("File %s is not a valid super image\n", filename);
			return -1;
		}

		if (partition < 2)
			return write_image(interface, partition, filename);

		/* Try programming partition 0 */
		printf("\nTrying partition 0:\n");
		rc = write_image(interface, 0, filename);
		if (rc == MV_OK)
		{
			/* Success, write also partition 1 */
			printf("\nNow writing to partition 1:\n");
			return write_image(interface, 1, filename);
		}

		/* Failed on 0, try partition 1 */
		printf("\n\nTrying partition 1:\n");
		rc = write_image(interface, 1, filename);
		if (rc == MV_OK)
		{
			/* Success, write also partition 0 */
			printf("\nNow writing to partition 0:\n");
			return write_image(interface, 0, filename);
		}

		printf("Failed to safe write image to both partitions\n"
			   "Either both existing images are corrupted, or flash programming failed\n"
			   "Try using command \"write\" with offset %d to initialize flash,\n"
			   "then try again\n", SI_OFFSET);
		return -1;
		break;

	case REG_W:
		/* PP write command */
		write_register(offset, valueToWrite);
		break;

	case REG_R:
		/* PP read command */
		printf("0x%08x: 0x%08x\n", offset, read_register(offset));
		break;
	default:
		break;
	}

	if(smiFlag)
		slaveSmiDestroy();

/****************************************************************/
	return 0;
}
