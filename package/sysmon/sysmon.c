
/* 
	Copyright ¨I 2014, by HFR,Inc. All Right Reserved.

	This Software of copyright is reserved by HFR.Inc. No all or part of this Software may be reproduced by  the 
	hard-copy format, electronically, or otherwise and transmitted to Ineligible Person. If not you should be subject 
	to civil suit and criminal case according to circumstances.

	본 파일의 저작권은 (주)에치에프알에 있습니다. (주)에치에프알의 명시적인 동의없이 본 파일의 전체 또는 일부를 
	하드카피 형식, 전자적 방식 또는 기타방식으로 재생산하거나 받아볼 수 있는 자격이 없는 사람에게 재배포하는 행위는
	저작권법을 위반하는 것이며 민사상 손해배상 그리고 해당하는 경우에는 형사상 처벌의 대상이 됩니다
*/

#include "sysmon.h" 
#include <getopt.h>
#include "log.h" 
#include "thread.h" 
#if 0  /*[#12] kieun07. 2024.04.30 */
#include "rdl_fsm.h"

extern RDL_FSM rdl_fsm_list[SV_TRANS_MAX];
extern RDL_INFO_LIST rdl_info_list;
#endif /* [#12] */

#define DEBUG

int32_t hdrv_fd;
struct thread_master *master;
const char *pid_file = PATH_SYSMON_PID;
char* progname;


#ifdef DEBUG
static void print_console(const char *fmt, ...)
{
	va_list args;
	char *p;
	p = malloc(1024);
	FILE * fp = NULL;
	va_start (args, fmt);
	vsprintf (p,  fmt, args);
	fp = fopen("/dev/console", "w");
	fprintf(fp, p);
	va_end (args);
	fclose(fp);

	free(p);
}
#else
static void print_console(const char *fmt, ...)
{

}
#endif

void sigint (int sig) {
	/* TODO. signal */
	exit(0);
}


int test_timer_func(struct thread *thread) {
	thread_add_timer (master, test_timer_func, NULL, 1);
	print_console("test...\n");	
	return 0;
}

#if 0  /*[#12] kieun07. 2024.04.30 */
SV_EVT get_rdl_evt(SV_ST state)
{
	SV_EVT evt;
	
	switch(state)	
	{
		case	RDL_ST_INIT:
			hw_val = FPGA_READ();
			if()
				evt = MCU_RDL_START;
			break;

		case	RDL_ST_START:
			hw_val = FPGA_READ();
			if()
				evt = MCU_RDL_WRITING_P1;
			break;

		case	RDL_WRITING_P1:
			hw_val = FPGA_READ();
			if()
				evt = MCU_RDL_WRITING_DONE_P1;
			else if()
				evt =MCU_RDL_WRIGTING_ERROR;
			break;

		case	RDL_READING_P1:
//check reading P1 fail file
			hw_val = FPGA_READ();
			if()
				evt = MCU_RDL_WRITING_P2;
			else if()
				evt = RDL_READING_ERROR;
			break;

		case	RDL_WRITING_P2:
			hw_val = FPGA_READ(_);
			if()
				evt = MCU_RDL_WRITING_DONE_P2;
			else if()
				evt = MCU_RDL_WRIGTING_ERROR;
			break;

		case	RDL_READING_P2:
//check reading P2 fail file
			hw_val = FPGA_READ();
			if()
				evt = RDL_READING_DONE_P2;
			else if()
				evt = RDL_READING_ERROR;
			break;

		case	RDL_WRITING_TOTAL:
			hw_val = FPGA_READ();
			if()
				evt = RDL_WRITING_DONE_TOTAL;
			else if()
				evt = RDL_WRITING_NOT_DONE;
			else if()
				evt = RDL_WRITING_ERROR_TOTAL;
			break;

		case	RDL_READING_TOTAL:
//check reading TOTAL fail file
			hw_val = FPGA_READ();
			if()
				evt = RDL_READING_DONE_TOTAL;
			else if()
				evt = RDL_READING_ERROR_TOTAL;
			break;

		case	RDL_IMG_ACTIVE_DONE:
			hw_val = FPGA_READ();
			if()
				evt = RDL_IMG_ACTIVE_SUCCESS;
			else if()
				evt = RDL_IMG_ACTIVE_FAIL;
			break;
		case	RDL_IMG_RUNNING_CHECK:
			hw_val = FPGA_READ();
			if()
				evt= RDL_IMG_RUNNING_SUCCESS;
			else if()
				evt = RDL_IMG_RUNNING_FAIL;
			break;
	}
	return evt;
}

SV_ST update_rdl_fsm()
{
	SV_ST st = sv_info_list.st;
	SV_EVT evt;
	uint8_t offset;
	uint8_t n, ret = 0;
	
	if(st == RDL_ST_TERM)
		st = RDL_ST_INIT;

	while (st != RDL_ST_TERM)
	{
		evt = get_rdl_evt(st);
		
		for(n = 0; n < RDL_TRANS_MAX; n++)
		{
			if(rdl_fsm_list.state == st && rdl_fsm_list.evt == evt)
			{
				st = rdl_fsm_list.func();
				break;
			}
		}
		if(st == rdl_info_list.st)
			break;
	}
next_turn :
	return st;

}
int rdl_fsm_func(struct thread *thread)
{
	update_rdl_fsm();

	thread_add_timer_msec (master, (int)rdl_fsm_func, NULL, 100);
	return 0;
}
#endif /* [#12] */

/* Allocate new sys structure and set default value. */
void
sysmon_thread_init (void)
{
	//TODO
	thread_add_timer (master, test_timer_func, NULL, 1);
#if 1  /*[#12] kieun07. 2024.04.30 */
	thread_add_timer (master, rdl_fsm_func, NULL, 1);
#endif /* [#12] */
}


int init_rlimit(void)
{
	
	struct rlimit limit;

	getrlimit(RLIMIT_CORE, &limit);

	limit.rlim_cur = RLIM_INFINITY;
	limit.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &limit);

	getrlimit(RLIMIT_CORE, &limit);
	
	return 0;
}

int hdriv_open(void) {

	hdrv_fd = open("/dev/hdrv",O_RDWR);
	if ( hdrv_fd < 0 ) {
		//zlog_warn("hdriver open err\n");
		print_console("hdriver open err\n");
		return -1;
	}
	return 0;
}

void sysmon_init(void) {

	if(hdriv_open() == -1)
		print_console("sysmon init failure\n");
	
}

struct option longopts[] = {
	{ "daemon",        no_argument,       NULL, 'd'},
	{ "config_file",   required_argument, NULL, 'f'},
	{ "pid_file",      required_argument, NULL, 'i'},
	{ "help",          no_argument,       NULL, 'h'},
	{ 0 }
};

static void usage(int status)
{
	if (status != 0)
		fprintf (stderr, "Try `%s --help' for more information.\n", progname);
	else {    
		printf ("Usage : %s [OPTION...]\n\
			Daemon which manages EAG6L.\n\n\
			-d, --daemon         Run in daemon mode\n\
			-f, --config_file    Set configuration file name\n\
			-i, --pid_file       Set process identifier file name\n\
			-h, --help           Display this help and exit\n\
			\n\
			Report bugs to %s\n", progname, SYSMON_BUG_ADDRESS);
	}

	exit (status);
}

int main(int argc, char **argv)
{
	int daemon_mode = 0;
	char *config_file = NULL, *p = NULL;
	struct thread thread;

	umask(0027);
 
	progname = ((p = strrchr(argv[0], '/')) ? ++p : argv[0]);

	/* this while just reads the options */                       
	while (1) {
		int opt;

		opt = getopt_long (argc, argv, "df:i:h", longopts, 0);

		if (opt == EOF)
			break;

		switch (opt) {
		case 0:
			break;
		case 'd':
			daemon_mode = 1;
			break;
		case 'f':
			config_file = optarg;
			break;
		case 'i':
			pid_file = optarg;
			break;
		case 'h':
			usage (0);
			break;
		default:
			usage (1);
			break;
		}
	}

	init_rlimit();
#if 0
	zlog_default = openzlog ("logsaved", ZLOG_NOLOG, ZLOG_LOGSAVED,
				 LOG_CONS|LOG_NDELAY|LOG_PID, LOG_DAEMON);
#endif

	signal_init();
	sysmon_init();

	master = thread_master_create ();
	sysmon_thread_init();

	/* Change to the daemon program. */
	if (daemon_mode) {
		if (daemon(0, 0)) {
			//zlog_warn("failed to daemonize");
			print_console("failed to daemonize\n");
		}
	}

	/* Process ID file creation. */
	pid_output(pid_file);

	thread_main (master);

	exit(0);
}
