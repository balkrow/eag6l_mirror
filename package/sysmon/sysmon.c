
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
#include "log.h" 
#include "thread.h" 

struct thread_master *master;

#if 0
static void print_console(const char *fmt, ...)
    {

    }
#else
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

int main(int argc, char **argv)
{

	struct thread thread;

	init_rlimit();
#if 0
	zlog_default = openzlog ("logsaved", ZLOG_NOLOG, ZLOG_LOGSAVED,
			LOG_CONS|LOG_NDELAY|LOG_PID, LOG_DAEMON);
#endif

	signal_init ();

	master = thread_master_create ();
	sysmon_thread_init();
#if 0	
	zlog_info("logsaved start.");
#endif
#if 0
	while (thread_fetch (master, &thread))
		thread_call (&thread);
#endif

	thread_main (master);

	//close_unix();
	exit(0);

}
