
/* 
	Copyright ��I 2014, by HFR,Inc. All Right Reserved.

	This Software of copyright is reserved by HFR.Inc. No all or part of this Software may be reproduced by  the 
	hard-copy format, electronically, or otherwise and transmitted to Ineligible Person. If not you should be subject 
	to civil suit and criminal case according to circumstances.

	�� ������ ���۱��� (��)��ġ�����˿� �ֽ��ϴ�. (��)��ġ�������� ������� ���Ǿ��� �� ������ ��ü �Ǵ� �Ϻθ� 
	�ϵ�ī�� ����, ������ ��� �Ǵ� ��Ÿ������� ������ϰų� �޾ƺ� �� �ִ� �ڰ��� ���� ������� ������ϴ� ������
	���۱ǹ��� �����ϴ� ���̸� �λ�� ���ع�� �׸��� �ش��ϴ� ��쿡�� ����� ó���� ����� �˴ϴ�
*/
#include "zebra.h"
#include "sysmon.h" 
#include "sys_fifo.h" 
#include <getopt.h>
#include "log.h" 
#include "thread.h" 

#define DEBUG

int32_t hdrv_fd;
struct thread_master *master;
//const char *pid_file = PATH_SYSMON_PID;
char* progname;


#ifdef DEBUG
void print_console(const char *fmt, ...)
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
void print_console(const char *fmt, ...)
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

#if 0//PWY_FIXME
int sfp_timer_func(struct thread *thread)
{
extern void update_sfp(void);

    thread_add_timer (master, (int)sfp_timer_func, NULL, 5);
    update_sfp();
    return 0;
}
#endif //PWY_FIXME

/* Allocate new sys structure and set default value. */
void sysmon_thread_init (void)
{
	//TODO
#if 0//PWY_FIXME
	thread_add_timer (master, test_timer_func, NULL, 1);
#endif //PWY_FIXME
#if 0//PWY_FIXME cuased a thread crash after 10 sec.
	thread_add_timer (master, sfp_timer_func, NULL, 10);
#endif //PWY_FIXME
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
	sysmon_master_fifo_init ();
	
}
