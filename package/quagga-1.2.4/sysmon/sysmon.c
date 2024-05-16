
/* 
	Copyright ¨I 2014, by HFR,Inc. All Right Reserved.

	This Software of copyright is reserved by HFR.Inc. No all or part of this Software may be reproduced by  the 
	hard-copy format, electronically, or otherwise and transmitted to Ineligible Person. If not you should be subject 
	to civil suit and criminal case according to circumstances.

	본 파일의 저작권은 (주)에치에프알에 있습니다. (주)에치에프알의 명시적인 동의없이 본 파일의 전체 또는 일부를 
	하드카피 형식, 전자적 방식 또는 기타방식으로 재생산하거나 받아볼 수 있는 자격이 없는 사람에게 재배포하는 행위는
	저작권법을 위반하는 것이며 민사상 손해배상 그리고 해당하는 경우에는 형사상 처벌의 대상이 됩니다
*/
#include "zebra.h"
#include "sysmon.h" 
#include <getopt.h>
#include "log.h" 
#include "thread.h" 

#undef DEBUG

int32_t hdrv_fd;
struct thread_master *master;
//const char *pid_file = PATH_SYSMON_PID;
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
#ifdef DEBUG
	print_console("test...\n");	
#endif
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

	sysmon_thread_init();
	
}
