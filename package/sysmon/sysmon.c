
/* 
	Copyright ��I 2014, by HFR,Inc. All Right Reserved.

	This Software of copyright is reserved by HFR.Inc. No all or part of this Software may be reproduced by  the 
	hard-copy format, electronically, or otherwise and transmitted to Ineligible Person. If not you should be subject 
	to civil suit and criminal case according to circumstances.

	�� ������ ���۱��� (��)��ġ�����˿� �ֽ��ϴ�. (��)��ġ�������� ������� ���Ǿ��� �� ������ ��ü �Ǵ� �Ϻθ� 
	�ϵ�ī�� ����, ������ ��� �Ǵ� ��Ÿ������� ������ϰų� �޾ƺ� �� �ִ� �ڰ��� ���� ������� ������ϴ� ������
	���۱ǹ��� �����ϴ� ���̸� �λ�� ���ع�� �׸��� �ش��ϴ� ��쿡�� ����� ó���� ����� �˴ϴ�
*/

#include "sysmon.h" 
#include <getopt.h>
#include "log.h" 
#include "thread.h" 

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


/* Allocate new sys structure and set default value. */
void
sysmon_thread_init (void)
{
	//TODO
	thread_add_timer (master, test_timer_func, NULL, 1);
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
