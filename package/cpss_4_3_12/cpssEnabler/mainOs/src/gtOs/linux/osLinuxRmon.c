#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE

#include <cpss/generic/version/cpssGenStream.h>
#include <gtOs/gtOsTask.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

#define gettid() ((pid_t)syscall(SYS_gettid))

pthread_mutex_t rmon_write_tlv_lock;

#define RMON_MAX_THREADS    32
#define RMON_MAX_NAME_LEN   64

#define RMON_TLV_TAG_SIZE   2
#define RMON_TLV_LEN_SIZE   4

static char CPSS_VERSION_FLAGS[512];
static char CPSS_VERSION_DATE[512];

typedef struct rmon_proc_pid_stat_ {
    uint32_t        pid;
    uint8_t         tcomm[RMON_MAX_NAME_LEN];
    uint8_t         state;
    uint32_t        ppid;
    uint32_t        pgrp;
    uint32_t        sid;
    uint32_t        tty_nr;
    uint32_t        tty_pgrp;
    uint32_t        flags;
    uint32_t        min_flt;
    uint64_t        cmin_flt;
    uint32_t        maj_flt;
    uint32_t        cmaj_flt;
    uint32_t        utime;
    uint32_t        stime;
    uint32_t        cutime;
    uint32_t        cstime;
    uint32_t        priority;
    uint32_t        nice;
    uint32_t        num_threads;
    uint32_t        it_real_value;
    uint32_t        start_time;
    uint32_t        vsize;
    uint32_t        rss;
    uint64_t        rsslim;
    uint32_t        start_code;
    uint32_t        end_code;
    uint32_t        start_stack;
    uint32_t        esp;
    uint32_t        eip;
    uint32_t        pending;
    uint32_t        blocked;
    uint32_t        sigign;
    uint32_t        sigcatch;
    uint32_t        res;
    uint32_t        exit_signal;
    uint32_t        task_cpu;
    uint32_t        rt_priority;
    uint32_t        policy;
    uint32_t        blkio_ticks;
    uint32_t        gtime;
    uint32_t        cgtime;
    uint32_t        start_data;
    uint32_t        end_data;
    uint32_t        start_brk;
    uint32_t        arg_start;
    uint32_t        arg_end;
    uint32_t        env_start;
    uint32_t        env_end;
    uint32_t        exit_code;
}RMON_PROC_PID_STAT;

typedef struct rmon_proc_pid_statm_ {
    uint32_t        size;
    uint32_t        resident;
    uint32_t        shared;
    uint32_t        trs;
    uint32_t        lrs;
    uint32_t        drs;
    uint32_t        dt;
}RMON_PROC_PID_STATM;

typedef struct rmon_proc_cpu_stat_ {
    uint32_t        user;
    uint32_t        nice;
    uint32_t        system;
    uint32_t        idle;
    uint32_t        iowait;
    uint32_t        irq;
    uint32_t        softirq;
}RMON_PROC_CPU_STAT;

typedef struct rmon_stat_time_ {
    uint64_t        cpu_epoch_start_sec;
    uint64_t        cpu_epoch_start_usec;
    uint64_t        cpu_epoch_end_sec;
    uint64_t        cpu_epoch_end_usec;
}RMON_STAT_TIME;

typedef struct rmon_stat_cpu_ {
    double          cpu_user_percent;
    double          cpu_nice_percent;
    double          cpu_syst_percent;
    double          cpu_idle_percent;
    double          cpu_iowait_percent;
    double          cpu_irq_percent;
    double          cpu_softirq_percent;
}RMON_STAT_CPU;

typedef struct rmon_stat_mem_ {
    double          mem_total_size;
    double          mem_code_size;
    double          mem_data_size;
    double          mem_shared_size;
}RMON_STAT_MEM;

typedef struct rmon_stat_pid_ {
    uint32_t        pid_num_threads;
    double          pid_cpu_percent[RMON_MAX_THREADS];
    uint32_t        pid_tid[RMON_MAX_THREADS];
    uint8_t         pid_name[RMON_MAX_THREADS][RMON_MAX_NAME_LEN];
}RMON_STAT_PID;

typedef struct rmon_stat_ {
    RMON_STAT_TIME  time;
    RMON_STAT_CPU   cpu;
    RMON_STAT_MEM   mem;
    RMON_STAT_PID   pid;
}RMON_STAT;

enum rmon_tlv_tags {
    RMON_TLV_TAG_RMON_VERSION,
    RMON_TLV_TAG_RMON_DATE_TIME,
    RMON_TLV_TAG_RMON_EPOCH_TIME,
    RMON_TLV_TAG_RMON_OS,
    RMON_TLV_TAG_CPSS_VERSION_STREAM,
    RMON_TLV_TAG_CPSS_VERSION_DATE,
    RMON_TLV_TAG_CPSS_VERSION_FLAGS,
    RMON_TLV_TAG_RMON_TIME_STAT,
    RMON_TLV_TAG_RMON_CPU_STAT,
    RMON_TLV_TAG_RMON_MEM_STAT,
    RMON_TLV_TAG_RMON_PROCESS_STAT,
    RMON_TLV_TAG_RMON_ENTRY_CHECKPOINT,
    RMON_TLV_TAG_RMON_EXIT_CHECKPOINT,
    RMON_TLV_TAG_RMON_CHECKPOINT_CALLER,
    RMON_TLV_TAG_RMON_CHECKPOINT_CALLEE,
    RMON_TLV_TAG_RMON_CHECKPOINT_FILE,
};

static uint32_t rmon_read_proc_pid_task_pid(uint32_t ppid, uint32_t task_pid[])
{
    char            dirname[128] = "";
    struct dirent   *list = NULL;
    DIR             *dir = NULL;
    uint32_t        count = 0;

    sprintf(dirname, "/proc/%d/task", ppid);

    dir = opendir(dirname);

    if(dir == NULL) {
        fprintf(stderr, "Failed to open %s, Exiting...\n", dirname);
        return 0;
    }

    do {
        list = readdir(dir);
        if(list) {
            if(strcmp(list->d_name, ".") && strcmp(list->d_name, "..")) {
                if(count < RMON_MAX_THREADS) {
                    task_pid[count++] = atoi(list->d_name);
                } else {
                    fprintf(stderr, "Detected more than %d threads. Update RMON_MAX_THREADS macro to increase thread limit\n", RMON_MAX_THREADS);
                    break;
                }
            }
        } else {
            closedir(dir);
        }
    } while(list);

    return count;
}

static GT_STATUS rmon_read_proc_pid_stat(uint32_t ppid, uint32_t pid, RMON_PROC_PID_STAT *proc_pid_stat)
{
    FILE        *procfd = NULL;
    char        file[128] = "";
    int         rc = 0;

    sprintf(file, "/proc/%d/task/%d/stat", ppid, pid);

    procfd = fopen(file, "r");

    if(procfd == NULL) {
        fprintf(stderr, "Failed to open %s, Exiting...\n", file);
        return GT_NOT_FOUND;
    }

    /* process id */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->pid);
    if(rc < 0) return GT_FAIL;
    /* filename of the executable */
    rc = fscanf(procfd,  "%s ", proc_pid_stat->tcomm);
    if(rc < 0) return GT_FAIL;
    /* state (R is running, S is sleeping, D is sleeping in an uninterruptible wait, Z is zombie, T is traced or stopped) */
    rc = fscanf(procfd,  "%c ", &proc_pid_stat->state);
    if(rc < 0) return GT_FAIL;
    /* process id of the parent process */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->ppid);
    if(rc < 0) return GT_FAIL;
    /* pgrp of the process */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->pgrp);
    if(rc < 0) return GT_FAIL;
    /* session id */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->sid);
    if(rc < 0) return GT_FAIL;
    /* tty the process uses */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->tty_nr);
    if(rc < 0) return GT_FAIL;
    /* pgrp of the tty */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->tty_pgrp);
    if(rc < 0) return GT_FAIL;
    /* task flags */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->flags);
    if(rc < 0) return GT_FAIL;
    /* number of minor faults */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->min_flt);
    if(rc < 0) return GT_FAIL;
    /* number of minor faults with child’s */
    rc = fscanf(procfd,  "%lu ", &proc_pid_stat->cmin_flt);
    if(rc < 0) return GT_FAIL;
    /* number of major faults */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->maj_flt);
    if(rc < 0) return GT_FAIL;
    /* number of major faults with child’s */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->cmaj_flt);
    if(rc < 0) return GT_FAIL;
    /* user mode jiffies */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->utime);
    if(rc < 0) return GT_FAIL;
    /* kernel mode jiffies */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->stime);
    if(rc < 0) return GT_FAIL;
    /* user mode jiffies with child’s */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->cutime);
    if(rc < 0) return GT_FAIL;
    /* kernel mode jiffies with child’s */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->cstime);
    if(rc < 0) return GT_FAIL;
    /* priority level */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->priority);
    if(rc < 0) return GT_FAIL;
    /* nice level */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->nice);
    if(rc < 0) return GT_FAIL;
    /* number of threads */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->num_threads);
    if(rc < 0) return GT_FAIL;
    /* obsolete, always 0 */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->it_real_value);
    if(rc < 0) return GT_FAIL;
    /* time the process started after system boot */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->start_time);
    if(rc < 0) return GT_FAIL;
    /* virtual memory size */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->vsize);
    if(rc < 0) return GT_FAIL;
    /* resident set memory size */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->rss);
    if(rc < 0) return GT_FAIL;
    /* current limit in bytes on the rss */
    rc = fscanf(procfd,  "%lu ", &proc_pid_stat->rsslim);
    if(rc < 0) return GT_FAIL;
    /* address above which program text can run */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->start_code);
    if(rc < 0) return GT_FAIL;
    /* address below which program text can run */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->end_code);
    if(rc < 0) return GT_FAIL;
    /* address of the start of the main process stack */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->start_stack);
    if(rc < 0) return GT_FAIL;
    /* current value of ESP */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->esp);
    if(rc < 0) return GT_FAIL;
    /* current value of EIP */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->eip);
    if(rc < 0) return GT_FAIL;
    /* bitmap of pending signals */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->pending);
    if(rc < 0) return GT_FAIL;
    /* bitmap of blocked signals */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->blocked);
    if(rc < 0) return GT_FAIL;
    /* bitmap of ignored signals */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->sigign);
    if(rc < 0) return GT_FAIL;
    /* bitmap of caught signals */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->sigcatch);
    if(rc < 0) return GT_FAIL;
    /* reserved */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->res);
    if(rc < 0) return GT_FAIL;
    /* reserved */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->res);
    if(rc < 0) return GT_FAIL;
    /* reserved */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->res);
    if(rc < 0) return GT_FAIL;
    /* signal to send to parent thread on exit */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->exit_signal);
    if(rc < 0) return GT_FAIL;
    /* which CPU the task is scheduled on */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->task_cpu);
    if(rc < 0) return GT_FAIL;
    /* realtime priority */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->rt_priority);
    if(rc < 0) return GT_FAIL;
    /* scheduling policy (man sched_setscheduler) */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->policy);
    if(rc < 0) return GT_FAIL;
    /* time spent waiting for block IO */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->blkio_ticks);
    if(rc < 0) return GT_FAIL;
    /* guest time of the task in jiffies */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->gtime);
    if(rc < 0) return GT_FAIL;
    /* guest time of the task children in jiffies */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->cgtime);
    if(rc < 0) return GT_FAIL;
    /* address above which program data+bss is placed */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->start_data);
    if(rc < 0) return GT_FAIL;
    /* address below which program data+bss is placed */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->end_data);
    if(rc < 0) return GT_FAIL;
    /* address above which program heap can be expanded with brk() */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->start_brk);
    if(rc < 0) return GT_FAIL;
    /* address above which program command line is placed */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->arg_start);
    if(rc < 0) return GT_FAIL;
    /* address below which program command line is placed */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->arg_end);
    if(rc < 0) return GT_FAIL;
    /* address above which program environment is placed */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->env_start);
    if(rc < 0) return GT_FAIL;
    /* address below which program environment is placed */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->env_end);
    if(rc < 0) return GT_FAIL;
    /* the thread’s exit_code in the form reported by the waitpid system call */
    rc = fscanf(procfd,  "%u ", &proc_pid_stat->exit_code);
    if(rc < 0) return GT_FAIL;

    fclose(procfd);

    return GT_OK;
}

static GT_STATUS rmon_read_proc_pid_statm(uint32_t ppid, uint32_t pid, RMON_PROC_PID_STATM *proc_pid_statm)
{
    FILE        *procfd = NULL;
    char        file[128] = "";
    int         rc = 0;

    sprintf(file, "/proc/%d/task/%d/statm", ppid, pid);

    procfd = fopen(file, "r");

    if(procfd == NULL) {
        fprintf(stderr, "Failed to open %s, Exiting...\n", file);
        return GT_NOT_FOUND;
    }

    /*
     * total program size (pages)
     * A task's currently used share of available physical memory.
     */
    rc = fscanf(procfd,  "%u ", &proc_pid_statm->size);
    if(rc < 0) return GT_FAIL;
    /*
     * size of memory portions (pages)
     * The non-swapped physical memory a task has used.
     * RES = CODE + DATA
     */
    rc = fscanf(procfd,  "%u ", &proc_pid_statm->resident);
    if(rc < 0) return GT_FAIL;
    /*
     * number of pages that are shared
     * The amount of shared memory used by a task. It simply reflects memory that could be potentially shared with other processes.
     */
    rc = fscanf(procfd,  "%u ", &proc_pid_statm->shared);
    if(rc < 0) return GT_FAIL;
    /*
     * number of pages that are ‘code’
     * The amount of physical memory devoted to executable code, also known as the 'text resident set' size or TRS
     */
    rc = fscanf(procfd,  "%u ", &proc_pid_statm->trs);
    if(rc < 0) return GT_FAIL;
    /*
     * number of pages of library
     */
    rc = fscanf(procfd,  "%u ", &proc_pid_statm->lrs);
    if(rc < 0) return GT_FAIL;
    /*
     * number of pages of data/stack
     * The amount of physical memory devoted to other than executable code, also known as the 'data resident set' size or DRS.
     */
    rc = fscanf(procfd,  "%u ", &proc_pid_statm->drs);
    if(rc < 0) return GT_FAIL;
    /*
     * number of dirty pages
     * The number of pages that have been modified since they were last written to disk.
     * Dirty pages must be written to disk before the corresponding physical memory location can be used for some other virtual page.
     */
    rc = fscanf(procfd,  "%u ", &proc_pid_statm->dt);
    if(rc < 0) return GT_FAIL;

    fclose(procfd);
    return GT_OK;
}

static GT_STATUS rmon_read_proc_cpu_stat(RMON_PROC_CPU_STAT *proc_cpu_stat)
{
    FILE        *proc_stat_fd = NULL;
    int         rc = 0;

    proc_stat_fd = fopen("/proc/stat", "r");

    if(proc_stat_fd == NULL) {
        fprintf(stderr, "Failed to open /proc/stat, Exiting...\n");
        return GT_NOT_FOUND;
    }

    rc = fscanf(proc_stat_fd,  "cpu %u %u %u %u %u %u %u", &proc_cpu_stat->user,
        &proc_cpu_stat->nice, &proc_cpu_stat->system, &proc_cpu_stat->idle, &proc_cpu_stat->iowait,
        &proc_cpu_stat->irq, &proc_cpu_stat->softirq);

    if(rc < 0) return GT_FAIL;

    fclose(proc_stat_fd);
    return GT_OK;
}

static uint32_t rmon_write_tlv(uint32_t ppid, uint16_t tag, uint32_t length, void *value)
{
    FILE        *rmon_stat_file = NULL;
    char        file[RMON_MAX_NAME_LEN] = "";
    uint32_t    rc = 0;

    pthread_mutex_lock(&rmon_write_tlv_lock);

    sprintf(file, "/%d_rmon_stat_file.dat", ppid);

    rmon_stat_file = fopen(file, "ab+");

    if(rmon_stat_file) {
        rc = fwrite(&tag, sizeof(tag), 1, rmon_stat_file);

        rc = fwrite(&length, sizeof(length), 1, rmon_stat_file);

        rc = fwrite(value, length, 1, rmon_stat_file);

        fclose(rmon_stat_file);
    }

    pthread_mutex_unlock(&rmon_write_tlv_lock);

    return rc;
}

static GT_VOID rmon_create_stat_file(uint32_t ppid)
{
    struct timeval tv;
    char    date[RMON_MAX_NAME_LEN];
    struct  tm *tm;
    time_t  t;

    t = time(NULL);
    tm = localtime(&t);
    strftime(date, sizeof(date), "%T %F", tm);

    rmon_write_tlv(ppid, RMON_TLV_TAG_RMON_VERSION, sizeof("0.1"), "0.1");
    rmon_write_tlv(ppid, RMON_TLV_TAG_RMON_DATE_TIME, strlen(date), date);

    gettimeofday(&tv, NULL);
    memset(date, 0, sizeof(date));
    sprintf(date, "%u.%u", tv.tv_sec,  tv.tv_usec);

    rmon_write_tlv(ppid, RMON_TLV_TAG_RMON_EPOCH_TIME, strlen(date), date);
    rmon_write_tlv(ppid, RMON_TLV_TAG_RMON_OS, strlen("UNIX"), "UNIX");
    rmon_write_tlv(ppid, RMON_TLV_TAG_CPSS_VERSION_STREAM, strlen(CPSS_STREAM_NAME_CNS), CPSS_STREAM_NAME_CNS);
    rmon_write_tlv(ppid, RMON_TLV_TAG_CPSS_VERSION_DATE, strlen(CPSS_VERSION_DATE), (void *)CPSS_VERSION_DATE);
    rmon_write_tlv(ppid, RMON_TLV_TAG_CPSS_VERSION_FLAGS, strlen(CPSS_VERSION_FLAGS), (void *)CPSS_VERSION_FLAGS);
}

static GT_VOID rmon_update_stat(uint32_t ppid, RMON_STAT *rmon_stat, RMON_STAT *rmon_stat_prev)
{
    void            *data = NULL;
    uint32_t        length = 0;
    uint32_t        index = 0;

    rmon_write_tlv(ppid, RMON_TLV_TAG_RMON_TIME_STAT, sizeof(RMON_STAT_TIME), &rmon_stat->time);

    if(memcmp(&rmon_stat_prev->cpu, &rmon_stat->cpu, sizeof(RMON_STAT_CPU))) {
            rmon_write_tlv(ppid, RMON_TLV_TAG_RMON_CPU_STAT, sizeof(RMON_STAT_CPU), &rmon_stat->cpu);
    }

    if(memcmp(&rmon_stat_prev->mem, &rmon_stat->mem, sizeof(RMON_STAT_MEM))) {
        rmon_write_tlv(ppid, RMON_TLV_TAG_RMON_MEM_STAT, sizeof(RMON_STAT_MEM), &rmon_stat->mem);
    }

    if(memcmp(&rmon_stat_prev->pid, &rmon_stat->pid, sizeof(RMON_STAT_PID))) {
        length = 0;
        data = (void *)malloc(sizeof(RMON_STAT_PID));
        if(data) {
            memset(data, 0, sizeof(RMON_STAT_PID));

            memcpy(data+length, &rmon_stat->pid.pid_num_threads, sizeof(rmon_stat->pid.pid_num_threads));
            length += sizeof(rmon_stat->pid.pid_num_threads);

            for(index = 0; index < rmon_stat->pid.pid_num_threads; index++) {
                memcpy(data+length, &rmon_stat->pid.pid_cpu_percent[index], sizeof(rmon_stat->pid.pid_cpu_percent[0]));
                length += sizeof(rmon_stat->pid.pid_cpu_percent[0]);

                memcpy(data+length, &rmon_stat->pid.pid_tid[index], sizeof(rmon_stat->pid.pid_tid[0]));
                length += sizeof(rmon_stat->pid.pid_tid[0]);

                memcpy(data+length, &rmon_stat->pid.pid_name[index], RMON_MAX_NAME_LEN);
                length += RMON_MAX_NAME_LEN;
            }

            rmon_write_tlv(ppid, RMON_TLV_TAG_RMON_PROCESS_STAT, length, data);

            free(data);
        } else {
            fprintf(stderr, "Failed to malloc memory for RMON_STAT_PID\n");
        }
    }
}

static GT_VOID rmon_update_checkpoint(
    GT_U32          event,
    const GT_CHAR   *caller,
    const GT_CHAR   *callee,
    const GT_CHAR   *file,
    GT_U32          line
)
{
    GT_VOID         *data = NULL;
    uint32_t        length = 0;
    uint32_t        size = 0;
    GT_U32          ppid;
    GT_U32          tid;
    GT_U16          tag;
    GT_U32          len;
    struct timeval  tv;

    gettimeofday(&tv, NULL);

    ppid = getpid();
    tid = gettid();

    /* TID + EPOC Sec + EPOC uSec */
    size = sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t);
    /* Caller TLV */
    size += RMON_TLV_TAG_SIZE + RMON_TLV_LEN_SIZE + strlen(caller);
    /* Callee TLV */
    size += RMON_TLV_TAG_SIZE + RMON_TLV_LEN_SIZE + strlen(callee);
    /* File TLV */
    size += RMON_TLV_TAG_SIZE + RMON_TLV_LEN_SIZE + strlen(file);

    length = 0;
    data = malloc(size);
    if(data) {
        memset(data, 0, size);

        /* TID */
        memcpy(data + length, &tid, sizeof(tid));
        length = sizeof(tid);

        /* EPOC Sec */
        memcpy(data + length, &tv.tv_sec, sizeof(uint64_t));
        length += sizeof(uint64_t);

        /* EPOC uSec */
        memcpy(data + length, &tv.tv_usec, sizeof(uint64_t));
        length += sizeof(uint64_t);

        /* Line */
        memcpy(data + length, &line, sizeof(line));
        length += sizeof(line);

        /* Caller TLV */
        tag = RMON_TLV_TAG_RMON_CHECKPOINT_CALLER;
        len = strlen(caller);

        memcpy(data + length, &tag, sizeof(tag));
        length += sizeof(tag);

        memcpy(data + length, &len, sizeof(len));
        length += sizeof(len);

        memcpy(data + length, caller, len);
        length += len;

        /* Callee TLV */
        tag = RMON_TLV_TAG_RMON_CHECKPOINT_CALLEE;
        len = strlen(callee);

        memcpy(data + length, &tag, sizeof(tag));
        length += sizeof(tag);

        memcpy(data + length, &len, sizeof(len));
        length += sizeof(len);

        memcpy(data + length, callee, len);
        length += len;

        /* File TLV */
        tag = RMON_TLV_TAG_RMON_CHECKPOINT_CALLEE;
        len = strlen(file);

        memcpy(data + length, &tag, sizeof(tag));
        length += sizeof(tag);

        memcpy(data + length, &len, sizeof(len));
        length += sizeof(len);

        memcpy(data + length, file, len);
        length += len;

        rmon_write_tlv(ppid, event, length, data);
    }
}

unsigned __TASKCONV rmon_main(GT_VOID *ppid)
{
    RMON_PROC_PID_STAT  proc_pid_stat_1[RMON_MAX_THREADS];
    RMON_PROC_PID_STAT  proc_pid_stat_2[RMON_MAX_THREADS];

    uint32_t            task_pid[RMON_MAX_THREADS];

    double              cpu_softirq_time_delta;
    double              cpu_iowait_time_delta;
    double              cpu_total_time_delta;
    double              cpu_user_time_delta;
    double              cpu_nice_time_delta;
    double              cpu_syst_time_delta;
    double              cpu_idle_time_delta;
    double              cpu_irq_time_delta;
    double              cpu_total_time_1;
    double              cpu_total_time_2;
    double              pid_total_time_1;
    double              pid_total_time_2;

    RMON_PROC_CPU_STAT   proc_cpu_stat_1;
    RMON_PROC_CPU_STAT   proc_cpu_stat_2;
    RMON_PROC_PID_STATM  proc_pid_statm;

    uint32_t            task_count_1;
    uint32_t            task_count_2;
    uint32_t            task_index;
    uint32_t            task_ppid;

    RMON_STAT           rmon_stat_prev;
    RMON_STAT           rmon_stat;

    struct timeval      start_tv;
    struct timeval      end_tv;

    GT_STATUS           rc;

    memset(&rmon_stat_prev, 0, sizeof(rmon_stat_prev));
    memset(&rmon_stat, 0, sizeof(rmon_stat));

    task_ppid = (uint32_t)((GT_UINTPTR)ppid);

    while(1) {
STARTRMON:
        gettimeofday(&start_tv, NULL);
        task_count_1 = rmon_read_proc_pid_task_pid(task_ppid, task_pid);
        if(!task_count_1)
            goto STARTRMON;

        rc = rmon_read_proc_cpu_stat(&proc_cpu_stat_1);
        if(rc != GT_OK)
            goto STARTRMON;

        rc = rmon_read_proc_pid_statm(task_ppid, task_ppid, &proc_pid_statm);
        if(rc != GT_OK)
            goto STARTRMON;

        for(task_index = 0; task_index < task_count_1; task_index++) {
            rc = rmon_read_proc_pid_stat(task_ppid, task_pid[task_index], &proc_pid_stat_1[task_index]);
            if(rc != GT_OK)
                goto STARTRMON;
        }

        usleep(100 * 1000);

        task_count_2 = rmon_read_proc_pid_task_pid(task_ppid, task_pid);
        if(!task_count_2)
            goto STARTRMON;

        if(task_count_2 != task_count_1)
            goto STARTRMON;

        rc = rmon_read_proc_pid_statm(task_ppid, task_ppid, &proc_pid_statm);
        if(rc != GT_OK)
            goto STARTRMON;

        for(task_index = 0; task_index < task_count_2; task_index++) {
            rc = rmon_read_proc_pid_stat(task_ppid, task_pid[task_index], &proc_pid_stat_2[task_index]);
            if(rc != GT_OK)
                goto STARTRMON;
        }

        rc = rmon_read_proc_cpu_stat(&proc_cpu_stat_2);
        if(rc != GT_OK)
            goto STARTRMON;

        gettimeofday(&end_tv, NULL);

        rmon_stat.time.cpu_epoch_start_sec = start_tv.tv_sec;
        rmon_stat.time.cpu_epoch_start_usec = start_tv.tv_usec;
        rmon_stat.time.cpu_epoch_end_sec = end_tv.tv_sec;
        rmon_stat.time.cpu_epoch_end_usec = end_tv.tv_usec;

        cpu_total_time_1 = proc_cpu_stat_1.user + proc_cpu_stat_1.nice + proc_cpu_stat_1.system + proc_cpu_stat_1.idle + proc_cpu_stat_1.iowait + proc_cpu_stat_1.irq + proc_cpu_stat_1.softirq;
        cpu_total_time_2 = proc_cpu_stat_2.user + proc_cpu_stat_2.nice + proc_cpu_stat_2.system + proc_cpu_stat_2.idle + proc_cpu_stat_2.iowait + proc_cpu_stat_2.irq + proc_cpu_stat_2.softirq;

        cpu_total_time_delta = cpu_total_time_2 - cpu_total_time_1;

        cpu_user_time_delta = proc_cpu_stat_2.user - proc_cpu_stat_1.user;
        cpu_nice_time_delta = proc_cpu_stat_2.nice - proc_cpu_stat_1.nice;
        cpu_syst_time_delta = proc_cpu_stat_2.system - proc_cpu_stat_1.system;
        cpu_idle_time_delta = proc_cpu_stat_2.idle - proc_cpu_stat_1.idle;
        cpu_iowait_time_delta = proc_cpu_stat_2.iowait - proc_cpu_stat_1.iowait;
        cpu_irq_time_delta = proc_cpu_stat_2.irq - proc_cpu_stat_1.irq;
        cpu_softirq_time_delta = proc_cpu_stat_2.softirq - proc_cpu_stat_1.softirq;

        rmon_stat.cpu.cpu_user_percent = (cpu_user_time_delta / cpu_total_time_delta) * 100;
        rmon_stat.cpu.cpu_nice_percent = (cpu_nice_time_delta / cpu_total_time_delta) * 100;
        rmon_stat.cpu.cpu_syst_percent = (cpu_syst_time_delta / cpu_total_time_delta) * 100;
        rmon_stat.cpu.cpu_idle_percent = (cpu_idle_time_delta / cpu_total_time_delta) * 100;
        rmon_stat.cpu.cpu_iowait_percent = (cpu_iowait_time_delta / cpu_total_time_delta) * 100;
        rmon_stat.cpu.cpu_irq_percent = (cpu_irq_time_delta / cpu_total_time_delta) * 100;
        rmon_stat.cpu.cpu_softirq_percent = (cpu_softirq_time_delta / cpu_total_time_delta) * 100;

        rmon_stat.mem.mem_total_size = (double)(proc_pid_statm.size*4096);
        rmon_stat.mem.mem_code_size = (double)(proc_pid_statm.trs*4096);
        rmon_stat.mem.mem_data_size = (double)(proc_pid_statm.drs*4096);
        rmon_stat.mem.mem_shared_size = (double)(proc_pid_statm.shared*4096);

        rmon_stat.pid.pid_num_threads = proc_pid_stat_1[0].num_threads;

        for(task_index = 0; task_index < task_count_2; task_index++) {
            pid_total_time_1 = proc_pid_stat_1[task_index].utime + proc_pid_stat_1[task_index].stime + proc_pid_stat_1[task_index].cutime + proc_pid_stat_1[task_index].cstime;
            pid_total_time_2 = proc_pid_stat_2[task_index].utime + proc_pid_stat_2[task_index].stime + proc_pid_stat_2[task_index].cutime + proc_pid_stat_2[task_index].cstime;

    #if 0
            /*
             * IRIX MODE - Treat 100% as full utilization of one CPU. top(1) calls this Irix mode.
             */
            rmon_stat.pid[task_index].pid_cpu_percent = (((pid_total_time_2 - pid_total_time_1) / ((cpu_total_time_2 - cpu_total_time_1) / 12))) * 100;
    #else
            /*
             * SOLARIS MODE - Treat 100% as full utilization of all CPUs. top(1) calls this Solaris mode.
             */
             rmon_stat.pid.pid_cpu_percent[task_index] = (pid_total_time_2 - pid_total_time_1) / ((cpu_total_time_2 - cpu_total_time_1)) * 100;
    #endif

            rmon_stat.pid.pid_tid[task_index] = proc_pid_stat_2[task_index].pid;
            strncpy((char *)rmon_stat.pid.pid_name[task_index], (char *)proc_pid_stat_2[task_index].tcomm, RMON_MAX_NAME_LEN);
        }

        rmon_update_stat(task_ppid, &rmon_stat, &rmon_stat_prev);

        memcpy(&rmon_stat_prev, &rmon_stat, sizeof(rmon_stat));
    }

    return 0;
}

/**
* @internal osRmonCreateEntryCheckpoint function
* @endinternal
*
* @brief   This function stores entry checkpoint data in rmon stat file
*
* @param[in] caller                - Caller to checkpoint
* @param[in] callee                - Callee to checkpoint
* @param[in] file                  - File to checkpoint
* @param[in] line                  - Line to checkpoint
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS osRmonCreateEntryCheckpoint(
    const GT_CHAR   *caller,
    const GT_CHAR   *callee,
    const GT_CHAR   *file,
    GT_U32          line
)
{

    rmon_update_checkpoint(RMON_TLV_TAG_RMON_ENTRY_CHECKPOINT, caller, callee, file, line);
    return GT_OK;
}

/**
* @internal osRmonCreateExitCheckpoint function
* @endinternal
*
* @brief   This function stores exit checkpoint data in rmon stat file
*
* @param[in] caller                - Caller to checkpoint
* @param[in] callee                - Callee to checkpoint
* @param[in] file                  - File to checkpoint
* @param[in] line                  - Line to checkpoint
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS osRmonCreateExitCheckpoint(
    const GT_CHAR   *caller,
    const GT_CHAR   *callee,
    const GT_CHAR   *file,
    GT_U32          line
)
{

    rmon_update_checkpoint(RMON_TLV_TAG_RMON_EXIT_CHECKPOINT, caller, callee, file, line);
    return GT_OK;
}

/**
* @internal osTaskRmonStart function
* @endinternal
*
* @brief   Start resource monitoring thread
*
* @param[in] date                  - CPSS build date
* @param[in] flags                 - CPSS build flags
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS osTaskRmonStart(const char *date, const char *flags)
{
    GT_STATUS               rc = GT_OK;
    GT_TASK                 rmonTaskId;
    GT_U32                  ppid;

    ppid = getpid();

    strncpy(CPSS_VERSION_DATE, date, strlen(date));
    strncpy(CPSS_VERSION_FLAGS, flags, strlen(flags));

    rmon_create_stat_file(ppid);

    if (pthread_mutex_init(&rmon_write_tlv_lock, NULL) != 0) {
        fprintf(stderr, "rmon_write_tlv_lock mutex init has failed\n");
        return GT_FAIL;
    }

    rc = osTaskCreate("rmon",                           /* Task Name      */
                        210,                            /* Task Priority  */
                        0x8000,                         /* Stack Size     */
                        rmon_main,                      /* Starting Point */
                        (GT_VOID*)((GT_UINTPTR)ppid),   /* Arguments list */
                        &rmonTaskId);                   /* task ID        */

    return rc;
}