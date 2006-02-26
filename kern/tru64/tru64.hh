/*
 * Copyright (c) 2001-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __TRU64_HH__
#define __TRU64_HH__
#include "config/full_system.hh"

#if FULL_SYSTEM

class Tru64 {};

#else //!FULL_SYSTEM

#include <sys/types.h>
#include <sys/stat.h>
#if defined(__OpenBSD__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/statfs.h>
#endif

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>	// for memset()
#include <unistd.h>

#include "cpu/base.hh"
#include "sim/root.hh"
#include "sim/syscall_emul.hh"

using namespace std;

typedef struct stat global_stat;
typedef struct statfs global_statfs;
typedef struct dirent global_dirent;

///
/// This class encapsulates the types, structures, constants,
/// functions, and syscall-number mappings specific to the Alpha Tru64
/// syscall interface.
///
class Tru64 {

  public:

    //@{
    /// Basic Tru64 types.
    typedef uint64_t size_t;
    typedef uint64_t off_t;
    typedef uint16_t nlink_t;
    typedef int32_t  dev_t;
    typedef uint32_t uid_t;
    typedef uint32_t gid_t;
    typedef uint32_t time_t;
    typedef uint32_t mode_t;
    typedef uint32_t ino_t;
    typedef struct { int val[2]; } quad;
    typedef quad fsid_t;
    //@}

    //@{
    /// open(2) flag values.
    static const int TGT_O_RDONLY	= 00000000;
    static const int TGT_O_WRONLY	= 00000001;
    static const int TGT_O_RDWR	 	= 00000002;
    static const int TGT_O_NONBLOCK 	= 00000004;
    static const int TGT_O_APPEND	= 00000010;
    static const int TGT_O_CREAT	= 00001000;
    static const int TGT_O_TRUNC	= 00002000;
    static const int TGT_O_EXCL	 	= 00004000;
    static const int TGT_O_NOCTTY	= 00010000;
    static const int TGT_O_SYNC	 	= 00040000;
    static const int TGT_O_DRD	 	= 00100000;
    static const int TGT_O_DIRECTIO  	= 00200000;
    static const int TGT_O_CACHE	= 00400000;
    static const int TGT_O_DSYNC	= 02000000;
    static const int TGT_O_RSYNC	= 04000000;
    //@}

    /// This table maps the target open() flags to the corresponding
    /// host open() flags.
    static OpenFlagTransTable openFlagTable[];

    /// Number of entries in openFlagTable[].
    static const int NUM_OPEN_FLAGS;

    /// Stat buffer.  Note that Tru64 v5.0+ use a new "F64" stat
    /// structure, and a new set of syscall numbers for stat calls.
    /// On some hosts (notably Linux) define st_atime, st_mtime, and
    /// st_ctime as macros, so we append an X to get around this.
    struct F64_stat {
        dev_t	st_dev;			//!< st_dev
        int32_t	st_retired1;		//!< st_retired1
        mode_t	st_mode;		//!< st_mode
        nlink_t	st_nlink;		//!< st_nlink
        uint16_t st_nlink_reserved;	//!< st_nlink_reserved
        uid_t	st_uid;			//!< st_uid
        gid_t	st_gid;			//!< st_gid
        dev_t	st_rdev;		//!< st_rdev
        dev_t	st_ldev;		//!< st_ldev
        off_t	st_size;		//!< st_size
        time_t	st_retired2;		//!< st_retired2
        int32_t	st_uatime;		//!< st_uatime
        time_t	st_retired3;		//!< st_retired3
        int32_t	st_umtime;		//!< st_umtime
        time_t	st_retired4;		//!< st_retired4
        int32_t	st_uctime;		//!< st_uctime
        int32_t	st_retired5;		//!< st_retired5
        int32_t	st_retired6;		//!< st_retired6
        uint32_t	st_flags;	//!< st_flags
        uint32_t	st_gen;		//!< st_gen
        uint64_t	st_spare[4];	//!< st_spare[4]
        ino_t	st_ino;			//!< st_ino
        int32_t	st_ino_reserved;	//!< st_ino_reserved
        time_t	st_atimeX;		//!< st_atime
        int32_t	st_atime_reserved;	//!< st_atime_reserved
        time_t	st_mtimeX;		//!< st_mtime
        int32_t	st_mtime_reserved;	//!< st_mtime_reserved
        time_t	st_ctimeX;		//!< st_ctime
        int32_t	st_ctime_reserved;	//!< st_ctime_reserved
        uint64_t	st_blksize;	//!< st_blksize
        uint64_t	st_blocks;	//!< st_blocks
    };


    /// Old Tru64 v4.x stat struct.
    /// Tru64 maintains backwards compatibility with v4.x by
    /// implementing another set of stat functions using the old
    /// structure definition and binding them to the old syscall
    /// numbers.
    struct pre_F64_stat {
        dev_t   st_dev;
        ino_t   st_ino;
        mode_t  st_mode;
        nlink_t st_nlink;
        uid_t   st_uid;
        gid_t   st_gid;
        dev_t   st_rdev;
        off_t   st_size;
        time_t  st_atimeX;
        int32_t st_uatime;
        time_t  st_mtimeX;
        int32_t st_umtime;
        time_t  st_ctimeX;
        int32_t st_uctime;
        uint32_t st_blksize;
        int32_t st_blocks;
        uint32_t st_flags;
        uint32_t st_gen;
    };

    /// For statfs().
    struct F64_statfs {
        int16_t   f_type;
        int16_t   f_flags;
        int32_t     f_retired1;
        int32_t     f_retired2;
        int32_t     f_retired3;
        int32_t     f_retired4;
        int32_t     f_retired5;
        int32_t     f_retired6;
        int32_t     f_retired7;
        fsid_t	f_fsid;
        int32_t     f_spare[9];
        char    f_retired8[90];
        char    f_retired9[90];
        uint64_t dummy[10]; // was union mount_info mount_info;
        uint64_t  f_flags2;
        int64_t    f_spare2[14];
        int64_t    f_fsize;
        int64_t    f_bsize;
        int64_t    f_blocks;
        int64_t    f_bfree;
        int64_t    f_bavail;
        int64_t    f_files;
        int64_t    f_ffree;
        char    f_mntonname[1024];
        char    f_mntfromname[1024];
    };

    /// For old Tru64 v4.x statfs()
    struct pre_F64_statfs {
        int16_t   f_type;
        int16_t   f_flags;
        int32_t     f_fsize;
        int32_t     f_bsize;
        int32_t     f_blocks;
        int32_t     f_bfree;
        int32_t     f_bavail;
        int32_t     f_files;
        int32_t     f_ffree;
        fsid_t  f_fsid;
        int32_t     f_spare[9];
        char    f_mntonname[90];
        char    f_mntfromname[90];
        uint64_t dummy[10]; // was union mount_info mount_info;
    };

    /// For getdirentries().
    struct dirent
    {
        ino_t d_ino;		//!< file number of entry
        uint16_t d_reclen;	//!< length of this record
        uint16_t d_namlen;	//!< length of string in d_name
        char d_name[256];	//!< dummy name length
    };


    /// Length of strings in struct utsname (plus 1 for null char).
    static const int _SYS_NMLN = 32;

    /// Interface struct for uname().
    struct utsname {
        char sysname[_SYS_NMLN];        //!< System name.
        char nodename[_SYS_NMLN];       //!< Node name.
        char release[_SYS_NMLN];        //!< OS release.
        char version[_SYS_NMLN];        //!< OS version.
        char machine[_SYS_NMLN];        //!< Machine type.
    };

    //@{
    /// ioctl() command codes.
    static const unsigned TIOCGETP   = 0x40067408;
    static const unsigned TIOCSETP   = 0x80067409;
    static const unsigned TIOCSETN   = 0x8006740a;
    static const unsigned TIOCSETC   = 0x80067411;
    static const unsigned TIOCGETC   = 0x40067412;
    static const unsigned FIONREAD   = 0x4004667f;
    static const unsigned TIOCISATTY = 0x2000745e;
    // TIOCGETS not defined in tru64, so I made up a number
    static const unsigned TIOCGETS   = 0x40000000;
    static const unsigned TIOCGETA   = 0x402c7413;
    //@}

    /// Resource enumeration for getrlimit().
    enum rlimit_resources {
        TGT_RLIMIT_CPU = 0,
        TGT_RLIMIT_FSIZE = 1,
        TGT_RLIMIT_DATA = 2,
        TGT_RLIMIT_STACK = 3,
        TGT_RLIMIT_CORE = 4,
        TGT_RLIMIT_RSS = 5,
        TGT_RLIMIT_NOFILE = 6,
        TGT_RLIMIT_AS = 7,
        TGT_RLIMIT_VMEM = 7
    };

    /// Limit struct for getrlimit/setrlimit.
    struct rlimit {
        uint64_t  rlim_cur;	//!< soft limit
        uint64_t  rlim_max;	//!< hard limit
    };


    /// For mmap().
    static const unsigned TGT_MAP_ANONYMOUS = 0x10;


    //@{
    /// For getsysinfo().
    static const unsigned GSI_PLATFORM_NAME = 103; //!< platform name as string
    static const unsigned GSI_CPU_INFO = 59;	//!< CPU information
    static const unsigned GSI_PROC_TYPE = 60;	//!< get proc_type
    static const unsigned GSI_MAX_CPU = 30;   //!< max # cpu's on this machine
    static const unsigned GSI_CPUS_IN_BOX = 55;	//!< number of CPUs in system
    static const unsigned GSI_PHYSMEM = 19;	//!< Physical memory in KB
    static const unsigned GSI_CLK_TCK = 42;	//!< clock freq in Hz
    //@}

    /// For getsysinfo() GSI_CPU_INFO option.
    struct cpu_info {
        uint32_t     current_cpu;	//!< current_cpu
        uint32_t     cpus_in_box;	//!< cpus_in_box
        uint32_t     cpu_type;		//!< cpu_type
        uint32_t     ncpus;		//!< ncpus
        uint64_t     cpus_present;	//!< cpus_present
        uint64_t     cpus_running;	//!< cpus_running
        uint64_t     cpu_binding;	//!< cpu_binding
        uint64_t     cpu_ex_binding;	//!< cpu_ex_binding
        uint32_t     mhz;		//!< mhz
        uint32_t     unused[3];		//!< future expansion
    };

    //@{
    /// For setsysinfo().
    static const unsigned SSI_IEEE_FP_CONTROL = 14; //!< ieee_set_fp_control()
    //@}

    /// For gettimeofday.
    struct timeval {
        uint32_t tv_sec;	//!< seconds
        uint32_t tv_usec;	//!< microseconds
    };

    //@{
    /// For getrusage().
    static const int TGT_RUSAGE_THREAD = 1;
    static const int TGT_RUSAGE_SELF = 0;
    static const int TGT_RUSAGE_CHILDREN = -1;
    //@}

    /// For getrusage().
    struct rusage {
        struct timeval ru_utime;	//!< user time used
        struct timeval ru_stime;	//!< system time used
        uint64_t ru_maxrss;		//!< ru_maxrss
        uint64_t ru_ixrss;		//!< integral shared memory size
        uint64_t ru_idrss;		//!< integral unshared data "
        uint64_t ru_isrss;		//!< integral unshared stack "
        uint64_t ru_minflt;		//!< page reclaims - total vmfaults
        uint64_t ru_majflt;		//!< page faults
        uint64_t ru_nswap;		//!< swaps
        uint64_t ru_inblock;		//!< block input operations
        uint64_t ru_oublock;		//!< block output operations
        uint64_t ru_msgsnd;		//!< messages sent
        uint64_t ru_msgrcv;		//!< messages received
        uint64_t ru_nsignals;		//!< signals received
        uint64_t ru_nvcsw;		//!< voluntary context switches
        uint64_t ru_nivcsw;		//!< involuntary "
    };

    /// For sigreturn().
    struct sigcontext {
        int64_t sc_onstack;		//!< sigstack state to restore
        int64_t sc_mask;		//!< signal mask to restore
        int64_t sc_pc;			//!< pc at time of signal
        int64_t sc_ps;			//!< psl to retore
        int64_t sc_regs[32];		//!< processor regs 0 to 31
        int64_t sc_ownedfp;		//!< fp has been used
        int64_t sc_fpregs[32];		//!< fp regs 0 to 31
        uint64_t sc_fpcr;		//!< floating point control reg
        uint64_t sc_fp_control;		//!< software fpcr
        int64_t sc_reserved1;		//!< reserved for kernel
        uint32_t sc_kreserved1;		//!< reserved for kernel
        uint32_t sc_kreserved2;		//!< reserved for kernel
        size_t  sc_ssize;		//!< stack size
        caddr_t sc_sbase;		//!< stack start
        uint64_t sc_traparg_a0;		//!< a0 argument to trap on exc
        uint64_t sc_traparg_a1;		//!< a1 argument to trap on exc
        uint64_t sc_traparg_a2;		//!< a2 argument to trap on exc
        uint64_t sc_fp_trap_pc;		//!< imprecise pc
        uint64_t sc_fp_trigger_sum;	//!< Exception summary at trigg
        uint64_t sc_fp_trigger_inst; 	//!< Instruction at trigger pc
    };


    /// For table().
    static const int TBL_SYSINFO = 12;

    /// For table().
    struct tbl_sysinfo {
        uint64_t si_user;	//!< User time
        uint64_t si_nice;	//!< Nice time
        uint64_t si_sys;	//!< System time
        uint64_t si_idle;	//!< Idle time
        uint64_t si_hz;		//!< hz
        uint64_t si_phz;	//!< phz
        uint64_t si_boottime;	//!< Boot time in seconds
        uint64_t wait;		//!< Wait time
        uint32_t  si_max_procs;	//!< rpb->rpb_numprocs
        uint32_t  pad;		//!< padding
    };


    /// For stack_create.
    struct vm_stack {
        // was void *
        Addr	address;	//!< address hint
        size_t	rsize;		//!< red zone size
        size_t	ysize;		//!< yellow zone size
        size_t	gsize;		//!< green zone size
        size_t	swap;		//!< amount of swap to reserve
        size_t	incr;		//!< growth increment
        uint64_t	align;		//!< address alignment
        uint64_t	flags;		//!< MAP_FIXED etc.
        // was struct memalloc_attr *
        Addr	attr;		//!< allocation policy
        uint64_t reserved;	//!< reserved
    };

    /// Return values for nxm calls.
    enum {
        KERN_NOT_RECEIVER = 7,
        KERN_NOT_IN_SET = 12
    };

    /// For nxm_task_init.
    static const int NXM_TASK_INIT_VP = 2;	//!< initial thread is VP

    /// Task attribute structure.
    struct nxm_task_attr {
        int64_t nxm_callback;	//!< nxm_callback
        unsigned int nxm_version;	//!< nxm_version
        unsigned short nxm_uniq_offset;	//!< nxm_uniq_offset
        unsigned short flags;	//!< flags
        int nxm_quantum;	//!< nxm_quantum
        int pad1;		//!< pad1
        int64_t pad2;		//!< pad2
    };

    /// Signal set.
    typedef uint64_t   sigset_t;

    /// Thread state shared between user & kernel.
    struct ushared_state {
        sigset_t        sigmask;        //!< thread signal mask
        sigset_t        sig;            //!< thread pending mask
        // struct nxm_pth_state *
        Addr pth_id; //!< out-of-line state
        int             flags;          //!< shared flags
#define US_SIGSTACK     0x1             // thread called sigaltstack
#define US_ONSTACK      0x2             // thread is running on altstack
#define US_PROFILE      0x4             // thread called profil
#define US_SYSCALL      0x8             // thread in syscall
#define US_TRAP         0x10            // thread has trapped
#define US_YELLOW       0x20            // thread has mellowed yellow
#define US_YZONE        0x40            // thread has zoned out
#define US_FP_OWNED     0x80            // thread used floating point

        int             cancel_state;   //!< thread's cancelation state
#define US_CANCEL         0x1           // cancel pending
#define US_NOCANCEL       0X2           // synch cancel disabled
#define US_SYS_NOCANCEL   0x4           // syscall cancel disabled
#define US_ASYNC_NOCANCEL 0x8           // asynch cancel disabled
#define US_CANCEL_BITS  (US_NOCANCEL|US_SYS_NOCANCEL|US_ASYNC_NOCANCEL)
#define US_CANCEL_MASK  (US_CANCEL|US_NOCANCEL|US_SYS_NOCANCEL| \
                         US_ASYNC_NOCANCEL)

        // These are semi-shared. They are always visible to
        // the kernel but are never context-switched by the library.

        int             nxm_ssig;       //!< scheduler's synchronous signals
        int             reserved1;	//!< reserved1
        int64_t            nxm_active;     //!< scheduler active
        int64_t            reserved2;	//!< reserved2
    };

    struct nxm_sched_state {
        struct          ushared_state nxm_u;    //!< state own by user thread
        unsigned int    nxm_bits;               //!< scheduler state / slot
        int             nxm_quantum;            //!< quantum count-down value
        int             nxm_set_quantum;        //!< quantum reset value
        int             nxm_sysevent;           //!< syscall state
        // struct nxm_upcall *
        Addr	    nxm_uc_ret; //!< stack ptr of null thread
        // void *
        Addr nxm_tid;               //!< scheduler's thread id
        int64_t            nxm_va;                 //!< page fault address
        // struct nxm_pth_state *
        Addr nxm_pthid; //!< id of null thread
        uint64_t   nxm_bound_pcs_count;    //!< bound PCS thread count
        int64_t            pad[2];	   //!< pad
    };

    /// nxm_shared.
    struct nxm_shared {
        int64_t nxm_callback;              //!< address of upcall routine
        unsigned int nxm_version;       //!< version number
        unsigned short nxm_uniq_offset; //!< correction factor for TEB
        unsigned short pad1;		//!< pad1
        int64_t space[2];                  //!< future growth
        struct nxm_sched_state nxm_ss[1]; //!< array of shared areas
    };

    /// nxm_slot_state_t.
    enum nxm_slot_state_t {
        NXM_SLOT_AVAIL,
        NXM_SLOT_BOUND,
        NXM_SLOT_UNBOUND,
        NXM_SLOT_EMPTY
    };

    /// nxm_config_info
    struct nxm_config_info {
        int nxm_nslots_per_rad;         //!< max number of VP slots per RAD
        int nxm_nrads;                  //!< max number of RADs
        // nxm_slot_state_t *
        Addr nxm_slot_state; //!< per-VP slot state
        // struct nxm_shared *
        Addr nxm_rad[1];  //!< per-RAD shared areas
    };

    /// For nxm_thread_create.
    enum nxm_thread_type {
        NXM_TYPE_SCS	= 0,
        NXM_TYPE_VP		= 1,
        NXM_TYPE_MANAGER	= 2
    };

    /// Thread attributes.
    struct nxm_thread_attr {
        int version;	//!< version
        int type;	//!< type
        int cancel_flags;	//!< cancel_flags
        int priority;	//!< priority
        int policy;	//!< policy
        int signal_type;	//!< signal_type
        // void *
        Addr pthid;	//!< pthid
        sigset_t sigmask;	//!< sigmask
        /// Initial register values.
        struct {
            uint64_t pc;	//!< pc
            uint64_t sp;	//!< sp
            uint64_t a0;	//!< a0
        } registers;
        uint64_t pad2[2];	//!< pad2
    };

    /// Helper function to convert a host stat buffer to a target stat
    /// buffer.  Also copies the target buffer out to the simulated
    /// memory space.  Used by stat(), fstat(), and lstat().
    template <class T>
    static void
    copyOutStatBuf(FunctionalMemory *mem, Addr addr, global_stat *host)
    {
        TypedBufferArg<T> tgt(addr);

        tgt->st_dev = htog(host->st_dev);
        tgt->st_ino = htog(host->st_ino);
        tgt->st_mode = htog(host->st_mode);
        tgt->st_nlink = htog(host->st_nlink);
        tgt->st_uid = htog(host->st_uid);
        tgt->st_gid = htog(host->st_gid);
        tgt->st_rdev = htog(host->st_rdev);
        tgt->st_size = htog(host->st_size);
        tgt->st_atimeX = htog(host->st_atime);
        tgt->st_mtimeX = htog(host->st_mtime);
        tgt->st_ctimeX = htog(host->st_ctime);
        tgt->st_blksize = htog(host->st_blksize);
        tgt->st_blocks = htog(host->st_blocks);

        tgt.copyOut(mem);
    }

    /// Helper function to convert a host statfs buffer to a target statfs
    /// buffer.  Also copies the target buffer out to the simulated
    /// memory space.  Used by statfs() and fstatfs().
    template <class T>
    static void
    copyOutStatfsBuf(FunctionalMemory *mem, Addr addr, global_statfs *host)
    {
        TypedBufferArg<T> tgt(addr);

#if defined(__OpenBSD__) || defined(__APPLE__) || defined(__FreeBSD__)
        tgt->f_type = 0;
#else
        tgt->f_type = htog(host->f_type);
#endif
        tgt->f_bsize = htog(host->f_bsize);
        tgt->f_blocks = htog(host->f_blocks);
        tgt->f_bfree = htog(host->f_bfree);
        tgt->f_bavail = htog(host->f_bavail);
        tgt->f_files = htog(host->f_files);
        tgt->f_ffree = htog(host->f_ffree);

        // Is this as string normally?
        memcpy(&tgt->f_fsid, &host->f_fsid, sizeof(host->f_fsid));

        tgt.copyOut(mem);
    }

    class F64 {
      public:
        static void copyOutStatBuf(FunctionalMemory *mem, Addr addr,
                                   global_stat *host)
        {
            Tru64::copyOutStatBuf<Tru64::F64_stat>(mem, addr, host);
        }

        static void copyOutStatfsBuf(FunctionalMemory *mem, Addr addr,
                                     global_statfs *host)
        {
            Tru64::copyOutStatfsBuf<Tru64::F64_statfs>(mem, addr, host);
        }
    };

    class PreF64 {
      public:
        static void copyOutStatBuf(FunctionalMemory *mem, Addr addr,
                                   global_stat *host)
        {
            Tru64::copyOutStatBuf<Tru64::pre_F64_stat>(mem, addr, host);
        }

        static void copyOutStatfsBuf(FunctionalMemory *mem, Addr addr,
                                     global_statfs *host)
        {
            Tru64::copyOutStatfsBuf<Tru64::pre_F64_statfs>(mem, addr, host);
        }
    };

    /// Helper function to convert a host stat buffer to an old pre-F64
    /// (4.x) target stat buffer.  Also copies the target buffer out to
    /// the simulated memory space.  Used by pre_F64_stat(),
    /// pre_F64_fstat(), and pre_F64_lstat().
    static void
    copyOutPreF64StatBuf(FunctionalMemory *mem, Addr addr, struct stat *host)
    {
        TypedBufferArg<Tru64::pre_F64_stat> tgt(addr);

        tgt->st_dev = htog(host->st_dev);
        tgt->st_ino = htog(host->st_ino);
        tgt->st_mode = htog(host->st_mode);
        tgt->st_nlink = htog(host->st_nlink);
        tgt->st_uid = htog(host->st_uid);
        tgt->st_gid = htog(host->st_gid);
        tgt->st_rdev = htog(host->st_rdev);
        tgt->st_size = htog(host->st_size);
        tgt->st_atimeX = htog(host->st_atime);
        tgt->st_mtimeX = htog(host->st_mtime);
        tgt->st_ctimeX = htog(host->st_ctime);
        tgt->st_blksize = htog(host->st_blksize);
        tgt->st_blocks = htog(host->st_blocks);

        tgt.copyOut(mem);
    }


    /// The target system's hostname.
    static const char *hostname;


    /// Target getdirentries() handler.
    static SyscallReturn
    getdirentriesFunc(SyscallDesc *desc, int callnum, Process *process,
                      ExecContext *xc)
    {
#ifdef __CYGWIN__
        panic("getdirent not implemented on cygwin!");
#else
        int fd = process->sim_fd(xc->getSyscallArg(0));
        Addr tgt_buf = xc->getSyscallArg(1);
        int tgt_nbytes = xc->getSyscallArg(2);
        Addr tgt_basep = xc->getSyscallArg(3);

        char * const host_buf = new char[tgt_nbytes];

        // just pass basep through uninterpreted.
        TypedBufferArg<int64_t> basep(tgt_basep);
        basep.copyIn(xc->mem);
        long host_basep = (off_t)htog((int64_t)*basep);
        int host_result = getdirentries(fd, host_buf, tgt_nbytes, &host_basep);

        // check for error
        if (host_result < 0) {
            delete [] host_buf;
            return -errno;
        }

        // no error: copy results back to target space
        Addr tgt_buf_ptr = tgt_buf;
        char *host_buf_ptr = host_buf;
        char *host_buf_end = host_buf + host_result;
        while (host_buf_ptr < host_buf_end) {
            global_dirent *host_dp = (global_dirent *)host_buf_ptr;
            int namelen = strlen(host_dp->d_name);

            // Actual size includes padded string rounded up for alignment.
            // Subtract 256 for dummy char array in Tru64::dirent definition.
            // Add 1 to namelen for terminating null char.
            int tgt_bufsize = sizeof(Tru64::dirent) - 256 + roundUp(namelen+1, 8);
            TypedBufferArg<Tru64::dirent> tgt_dp(tgt_buf_ptr, tgt_bufsize);
            tgt_dp->d_ino = host_dp->d_ino;
            tgt_dp->d_reclen = tgt_bufsize;
            tgt_dp->d_namlen = namelen;
            strcpy(tgt_dp->d_name, host_dp->d_name);
            tgt_dp.copyOut(xc->mem);

            tgt_buf_ptr += tgt_bufsize;
            host_buf_ptr += host_dp->d_reclen;
        }

        delete [] host_buf;

        *basep = htog((int64_t)host_basep);
        basep.copyOut(xc->mem);

        return tgt_buf_ptr - tgt_buf;
#endif
    }

    /// Target sigreturn() handler.
    static SyscallReturn
    sigreturnFunc(SyscallDesc *desc, int callnum, Process *process,
                  ExecContext *xc)
    {
        using TheISA::RegFile;
        RegFile *regs = &xc->regs;
        TypedBufferArg<Tru64::sigcontext> sc(xc->getSyscallArg(0));

        sc.copyIn(xc->mem);

        // Restore state from sigcontext structure.
        // Note that we'll advance PC <- NPC before the end of the cycle,
        // so we need to restore the desired PC into NPC.
        // The current regs->pc will get clobbered.
        regs->npc = htog(sc->sc_pc);

        for (int i = 0; i < 31; ++i) {
            regs->intRegFile[i] = htog(sc->sc_regs[i]);
            regs->floatRegFile.q[i] = htog(sc->sc_fpregs[i]);
        }

        regs->miscRegs.fpcr = htog(sc->sc_fpcr);

        return 0;
    }

    /// Target table() handler.
    static SyscallReturn
    tableFunc(SyscallDesc *desc, int callnum, Process *process,
              ExecContext *xc)
    {
        int id = xc->getSyscallArg(0);		// table ID
        int index = xc->getSyscallArg(1);	// index into table
        // arg 2 is buffer pointer; type depends on table ID
        int nel = xc->getSyscallArg(3);		// number of elements
        int lel = xc->getSyscallArg(4);		// expected element size

        switch (id) {
          case Tru64::TBL_SYSINFO: {
              if (index != 0 || nel != 1 || lel != sizeof(Tru64::tbl_sysinfo))
                  return -EINVAL;
              TypedBufferArg<Tru64::tbl_sysinfo> elp(xc->getSyscallArg(2));

              const int clk_hz = one_million;
              elp->si_user = htog(curTick / (Clock::Frequency / clk_hz));
              elp->si_nice = htog(0);
              elp->si_sys = htog(0);
              elp->si_idle = htog(0);
              elp->wait = htog(0);
              elp->si_hz = htog(clk_hz);
              elp->si_phz = htog(clk_hz);
              elp->si_boottime = htog(seconds_since_epoch); // seconds since epoch?
              elp->si_max_procs = htog(process->numCpus());
              elp.copyOut(xc->mem);
              return 0;
          }

          default:
            cerr << "table(): id " << id << " unknown." << endl;
            return -EINVAL;
        }
    }

    //
    // Mach syscalls -- identified by negated syscall numbers
    //

    /// Create a stack region for a thread.
    static SyscallReturn
    stack_createFunc(SyscallDesc *desc, int callnum, Process *process,
                     ExecContext *xc)
    {
        TypedBufferArg<Tru64::vm_stack> argp(xc->getSyscallArg(0));

        argp.copyIn(xc->mem);

        // if the user chose an address, just let them have it.  Otherwise
        // pick one for them.
        if (htog(argp->address) == 0) {
            argp->address = htog(process->next_thread_stack_base);
            int stack_size = (htog(argp->rsize) + htog(argp->ysize) +
                    htog(argp->gsize));
            process->next_thread_stack_base -= stack_size;
            argp.copyOut(xc->mem);
        }

        return 0;
    }

    /// NXM library version stamp.
    static
    const int NXM_LIB_VERSION = 301003;

    /// This call sets up the interface between the user and kernel
    /// schedulers by creating a shared-memory region.  The shared memory
    /// region has several structs, some global, some per-RAD, some per-VP.
    static SyscallReturn
    nxm_task_initFunc(SyscallDesc *desc, int callnum, Process *process,
                      ExecContext *xc)
    {
        TypedBufferArg<Tru64::nxm_task_attr> attrp(xc->getSyscallArg(0));
        TypedBufferArg<Addr> configptr_ptr(xc->getSyscallArg(1));

        attrp.copyIn(xc->mem);

        if (gtoh(attrp->nxm_version) != NXM_LIB_VERSION) {
            cerr << "nxm_task_init: thread library version mismatch! "
                 << "got " << attrp->nxm_version
                 << ", expected " << NXM_LIB_VERSION << endl;
            abort();
        }

        if (gtoh(attrp->flags) != Tru64::NXM_TASK_INIT_VP) {
            cerr << "nxm_task_init: bad flag value " << attrp->flags
                 << " (expected " << Tru64::NXM_TASK_INIT_VP << ")" << endl;
            abort();
        }

        const Addr base_addr = 0x12000; // was 0x3f0000000LL;
        Addr cur_addr = base_addr; // next addresses to use
        // first comes the config_info struct
        Addr config_addr = cur_addr;
        cur_addr += sizeof(Tru64::nxm_config_info);
        // next comes the per-cpu state vector
        Addr slot_state_addr = cur_addr;
        int slot_state_size =
            process->numCpus() * sizeof(Tru64::nxm_slot_state_t);
        cur_addr += slot_state_size;
        // now the per-RAD state struct (we only support one RAD)
        cur_addr = 0x14000;	// bump up addr for alignment
        Addr rad_state_addr = cur_addr;
        int rad_state_size =
            (sizeof(Tru64::nxm_shared)
             + (process->numCpus()-1) * sizeof(Tru64::nxm_sched_state));
        cur_addr += rad_state_size;

        // now initialize a config_info struct and copy it out to user space
        TypedBufferArg<Tru64::nxm_config_info> config(config_addr);

        config->nxm_nslots_per_rad = htog(process->numCpus());
        config->nxm_nrads = htog(1);	// only one RAD in our system!
        config->nxm_slot_state = htog(slot_state_addr);
        config->nxm_rad[0] = htog(rad_state_addr);

        config.copyOut(xc->mem);

        // initialize the slot_state array and copy it out
        TypedBufferArg<Tru64::nxm_slot_state_t> slot_state(slot_state_addr,
                                                           slot_state_size);
        for (int i = 0; i < process->numCpus(); ++i) {
            // CPU 0 is bound to the calling process; all others are available
            // XXX this code should have an endian conversion, but I don't think
            // it works anyway
            slot_state[i] =
                (i == 0) ? Tru64::NXM_SLOT_BOUND : Tru64::NXM_SLOT_AVAIL;
        }

        slot_state.copyOut(xc->mem);

        // same for the per-RAD "shared" struct.  Note that we need to
        // allocate extra bytes for the per-VP array which is embedded at
        // the end.
        TypedBufferArg<Tru64::nxm_shared> rad_state(rad_state_addr,
                                                    rad_state_size);

        rad_state->nxm_callback = attrp->nxm_callback;
        rad_state->nxm_version = attrp->nxm_version;
        rad_state->nxm_uniq_offset = attrp->nxm_uniq_offset;
        for (int i = 0; i < process->numCpus(); ++i) {
            Tru64::nxm_sched_state *ssp = &rad_state->nxm_ss[i];
            ssp->nxm_u.sigmask = htog(0);
            ssp->nxm_u.sig = htog(0);
            ssp->nxm_u.flags = htog(0);
            ssp->nxm_u.cancel_state = htog(0);
            ssp->nxm_u.nxm_ssig = 0;
            ssp->nxm_bits = htog(0);
            ssp->nxm_quantum = attrp->nxm_quantum;
            ssp->nxm_set_quantum = attrp->nxm_quantum;
            ssp->nxm_sysevent = htog(0);

            if (i == 0) {
                uint64_t uniq = xc->regs.miscRegs.uniq;
                ssp->nxm_u.pth_id = htog(uniq + gtoh(attrp->nxm_uniq_offset));
                ssp->nxm_u.nxm_active = htog(uniq | 1);
            }
            else {
                ssp->nxm_u.pth_id = htog(0);
                ssp->nxm_u.nxm_active = htog(0);
            }
        }

        rad_state.copyOut(xc->mem);

        //
        // copy pointer to shared config area out to user
        //
        *configptr_ptr = htog(config_addr);
        configptr_ptr.copyOut(xc->mem);

        // Register this as a valid address range with the process
        process->nxm_start = base_addr;
        process->nxm_end = cur_addr;

        return 0;
    }

    /// Initialize execution context.
    static void
    init_exec_context(ExecContext *ec,
                      Tru64::nxm_thread_attr *attrp, uint64_t uniq_val)
    {
        memset(&ec->regs, 0, sizeof(ec->regs));

        ec->regs.intRegFile[TheISA::ArgumentReg0] = gtoh(attrp->registers.a0);
        ec->regs.intRegFile[27/*t12*/] = gtoh(attrp->registers.pc);
        ec->regs.intRegFile[TheISA::StackPointerReg] = gtoh(attrp->registers.sp);
        ec->regs.miscRegs.uniq = uniq_val;

        ec->regs.pc = gtoh(attrp->registers.pc);
        ec->regs.npc = gtoh(attrp->registers.pc) + sizeof(TheISA::MachInst);

        ec->activate();
    }

    /// Create thread.
    static SyscallReturn
    nxm_thread_createFunc(SyscallDesc *desc, int callnum, Process *process,
                          ExecContext *xc)
    {
        TypedBufferArg<Tru64::nxm_thread_attr> attrp(xc->getSyscallArg(0));
        TypedBufferArg<uint64_t> kidp(xc->getSyscallArg(1));
        int thread_index = xc->getSyscallArg(2);

        // get attribute args
        attrp.copyIn(xc->mem);

        if (gtoh(attrp->version) != NXM_LIB_VERSION) {
            cerr << "nxm_thread_create: thread library version mismatch! "
                 << "got " << attrp->version
                 << ", expected " << NXM_LIB_VERSION << endl;
            abort();
        }

        if (thread_index < 0 | thread_index > process->numCpus()) {
            cerr << "nxm_thread_create: bad thread index " << thread_index
                 << endl;
            abort();
        }

        // On a real machine, the per-RAD shared structure is in
        // shared memory, so both the user and kernel can get at it.
        // We don't have that luxury, so we just copy it in and then
        // back out again.
        int rad_state_size =
            (sizeof(Tru64::nxm_shared) +
             (process->numCpus()-1) * sizeof(Tru64::nxm_sched_state));

        TypedBufferArg<Tru64::nxm_shared> rad_state(0x14000,
                                                    rad_state_size);
        rad_state.copyIn(xc->mem);

        uint64_t uniq_val = gtoh(attrp->pthid) - gtoh(rad_state->nxm_uniq_offset);

        if (gtoh(attrp->type) == Tru64::NXM_TYPE_MANAGER) {
            // DEC pthreads seems to always create one of these (in
            // addition to N application threads), but we don't use it,
            // so don't bother creating it.

            // This is supposed to be a port number.  Make something up.
            *kidp = htog(99);
            kidp.copyOut(xc->mem);

            return 0;
        } else if (gtoh(attrp->type) == Tru64::NXM_TYPE_VP) {
            // A real "virtual processor" kernel thread.  Need to fork
            // this thread on another CPU.
            Tru64::nxm_sched_state *ssp = &rad_state->nxm_ss[thread_index];

            if (gtoh(ssp->nxm_u.nxm_active) != 0)
                return (int) Tru64::KERN_NOT_RECEIVER;

            ssp->nxm_u.pth_id = attrp->pthid;
            ssp->nxm_u.nxm_active = htog(uniq_val | 1);

            rad_state.copyOut(xc->mem);

            Addr slot_state_addr = 0x12000 + sizeof(Tru64::nxm_config_info);
            int slot_state_size =
                process->numCpus() * sizeof(Tru64::nxm_slot_state_t);

            TypedBufferArg<Tru64::nxm_slot_state_t>
                slot_state(slot_state_addr,
                           slot_state_size);

            slot_state.copyIn(xc->mem);

            if (slot_state[thread_index] != Tru64::NXM_SLOT_AVAIL) {
                cerr << "nxm_thread_createFunc: requested VP slot "
                     << thread_index << " not available!" << endl;
                fatal("");
            }

            // XXX This should have an endian conversion but I think this code
            // doesn't work anyway
            slot_state[thread_index] = Tru64::NXM_SLOT_BOUND;

            slot_state.copyOut(xc->mem);

            // Find a free simulator execution context.
            for (int i = 0; i < process->numCpus(); ++i) {
                ExecContext *xc = process->execContexts[i];

                if (xc->status() == ExecContext::Unallocated) {
                    // inactive context... grab it
                    init_exec_context(xc, attrp, uniq_val);

                    // This is supposed to be a port number, but we'll try
                    // and get away with just sticking the thread index
                    // here.
                    *kidp = htog(thread_index);
                    kidp.copyOut(xc->mem);

                    return 0;
                }
            }

            // fell out of loop... no available inactive context
            cerr << "nxm_thread_create: no idle contexts available." << endl;
            abort();
        } else {
            cerr << "nxm_thread_create: can't handle thread type "
                 << attrp->type << endl;
            abort();
        }

        return 0;
    }

    /// Thread idle call (like yield()).
    static SyscallReturn
    nxm_idleFunc(SyscallDesc *desc, int callnum, Process *process,
                 ExecContext *xc)
    {
        return 0;
    }

    /// Block thread.
    static SyscallReturn
    nxm_thread_blockFunc(SyscallDesc *desc, int callnum, Process *process,
                         ExecContext *xc)
    {
        uint64_t tid = xc->getSyscallArg(0);
        uint64_t secs = xc->getSyscallArg(1);
        uint64_t flags = xc->getSyscallArg(2);
        uint64_t action = xc->getSyscallArg(3);
        uint64_t usecs = xc->getSyscallArg(4);

        cout << xc->cpu->name() << ": nxm_thread_block " << tid << " " << secs
             << " " << flags << " " << action << " " << usecs << endl;

        return 0;
    }

    /// block.
    static SyscallReturn
    nxm_blockFunc(SyscallDesc *desc, int callnum, Process *process,
                  ExecContext *xc)
    {
        Addr uaddr = xc->getSyscallArg(0);
        uint64_t val = xc->getSyscallArg(1);
        uint64_t secs = xc->getSyscallArg(2);
        uint64_t usecs = xc->getSyscallArg(3);
        uint64_t flags = xc->getSyscallArg(4);

        BaseCPU *cpu = xc->cpu;

        cout << cpu->name() << ": nxm_block "
             << hex << uaddr << dec << " " << val
             << " " << secs << " " << usecs
             << " " << flags << endl;

        return 0;
    }

    /// Unblock thread.
    static SyscallReturn
    nxm_unblockFunc(SyscallDesc *desc, int callnum, Process *process,
                    ExecContext *xc)
    {
        Addr uaddr = xc->getSyscallArg(0);

        cout << xc->cpu->name() << ": nxm_unblock "
             << hex << uaddr << dec << endl;

        return 0;
    }

    /// Switch thread priority.
    static SyscallReturn
    swtch_priFunc(SyscallDesc *desc, int callnum, Process *process,
                  ExecContext *xc)
    {
        // Attempts to switch to another runnable thread (if there is
        // one).  Returns false if there are no other threads to run
        // (i.e., the thread can reasonably spin-wait) or true if there
        // are other threads.
        //
        // Since we assume at most one "kernel" thread per CPU, it's
        // always safe to return false here.
        return 0; //false;
    }


    /// Activate exec context waiting on a channel.  Just activate one
    /// by default.
    static int
    activate_waiting_context(Addr uaddr, Process *process,
                             bool activate_all = false)
    {
        int num_activated = 0;

        list<Process::WaitRec>::iterator i = process->waitList.begin();
        list<Process::WaitRec>::iterator end = process->waitList.end();

        while (i != end && (num_activated == 0 || activate_all)) {
            if (i->waitChan == uaddr) {
                // found waiting process: make it active
                ExecContext *newCtx = i->waitingContext;
                assert(newCtx->status() == ExecContext::Suspended);
                newCtx->activate();

                // get rid of this record
                i = process->waitList.erase(i);

                ++num_activated;
            } else {
                ++i;
            }
        }

        return num_activated;
    }

    /// M5 hacked-up lock acquire.
    static void
    m5_lock_mutex(Addr uaddr, Process *process, ExecContext *xc)
    {
        TypedBufferArg<uint64_t> lockp(uaddr);

        lockp.copyIn(xc->mem);

        if (gtoh(*lockp) == 0) {
            // lock is free: grab it
            *lockp = htog(1);
            lockp.copyOut(xc->mem);
        } else {
            // lock is busy: disable until free
            process->waitList.push_back(Process::WaitRec(uaddr, xc));
            xc->suspend();
        }
    }

    /// M5 unlock call.
    static void
    m5_unlock_mutex(Addr uaddr, Process *process, ExecContext *xc)
    {
        TypedBufferArg<uint64_t> lockp(uaddr);

        lockp.copyIn(xc->mem);
        assert(*lockp != 0);

        // Check for a process waiting on the lock.
        int num_waiting = activate_waiting_context(uaddr, process);

        // clear lock field if no waiting context is taking over the lock
        if (num_waiting == 0) {
            *lockp = 0;
            lockp.copyOut(xc->mem);
        }
    }

    /// Lock acquire syscall handler.
    static SyscallReturn
    m5_mutex_lockFunc(SyscallDesc *desc, int callnum, Process *process,
                      ExecContext *xc)
    {
        Addr uaddr = xc->getSyscallArg(0);

        m5_lock_mutex(uaddr, process, xc);

        // Return 0 since we will always return to the user with the lock
        // acquired.  We will just keep the context inactive until that is
        // true.
        return 0;
    }

    /// Try lock (non-blocking).
    static SyscallReturn
    m5_mutex_trylockFunc(SyscallDesc *desc, int callnum, Process *process,
                         ExecContext *xc)
    {
        Addr uaddr = xc->getSyscallArg(0);
        TypedBufferArg<uint64_t> lockp(uaddr);

        lockp.copyIn(xc->mem);

        if (gtoh(*lockp) == 0) {
            // lock is free: grab it
            *lockp = htog(1);
            lockp.copyOut(xc->mem);
            return 0;
        } else {
            return 1;
        }
    }

    /// Unlock syscall handler.
    static SyscallReturn
    m5_mutex_unlockFunc(SyscallDesc *desc, int callnum, Process *process,
                        ExecContext *xc)
    {
        Addr uaddr = xc->getSyscallArg(0);

        m5_unlock_mutex(uaddr, process, xc);

        return 0;
    }

    /// Signal ocndition.
    static SyscallReturn
    m5_cond_signalFunc(SyscallDesc *desc, int callnum, Process *process,
                       ExecContext *xc)
    {
        Addr cond_addr = xc->getSyscallArg(0);

        // Wake up one process waiting on the condition variable.
        activate_waiting_context(cond_addr, process);

        return 0;
    }

    /// Wake up all processes waiting on the condition variable.
    static SyscallReturn
    m5_cond_broadcastFunc(SyscallDesc *desc, int callnum, Process *process,
                          ExecContext *xc)
    {
        Addr cond_addr = xc->getSyscallArg(0);

        activate_waiting_context(cond_addr, process, true);

        return 0;
    }

    /// Wait on a condition.
    static SyscallReturn
    m5_cond_waitFunc(SyscallDesc *desc, int callnum, Process *process,
                     ExecContext *xc)
    {
        Addr cond_addr = xc->getSyscallArg(0);
        Addr lock_addr = xc->getSyscallArg(1);
        TypedBufferArg<uint64_t> condp(cond_addr);
        TypedBufferArg<uint64_t> lockp(lock_addr);

        // user is supposed to acquire lock before entering
        lockp.copyIn(xc->mem);
        assert(gtoh(*lockp) != 0);

        m5_unlock_mutex(lock_addr, process, xc);

        process->waitList.push_back(Process::WaitRec(cond_addr, xc));
        xc->suspend();

        return 0;
    }

    /// Thread exit.
    static SyscallReturn
    m5_thread_exitFunc(SyscallDesc *desc, int callnum, Process *process,
                       ExecContext *xc)
    {
        assert(xc->status() == ExecContext::Active);
        xc->deallocate();

        return 0;
    }

    /// Indirect syscall invocation (call #0).
    static SyscallReturn
    indirectSyscallFunc(SyscallDesc *desc, int callnum, Process *process,
                        ExecContext *xc)
    {
        int new_callnum = xc->getSyscallArg(0);
        LiveProcess *lp = dynamic_cast<LiveProcess*>(process);
        assert(lp);

        for (int i = 0; i < 5; ++i)
            xc->setSyscallArg(i, xc->getSyscallArg(i+1));


        SyscallDesc *new_desc = lp->getDesc(new_callnum);
        if (desc == NULL)
            fatal("Syscall %d out of range", callnum);

        new_desc->doSyscall(new_callnum, process, xc);

        return 0;
    }

};  // class Tru64


#endif // FULL_SYSTEM

#endif // __TRU64_HH__
