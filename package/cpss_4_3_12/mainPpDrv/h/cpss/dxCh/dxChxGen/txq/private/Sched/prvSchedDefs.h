/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief Scheduler  Configuration Library Public Definitions.
 *
* @file prvSchedDefs.h
*
* $Revision: 2.0 $
 */

#ifndef   	SCHED_DEFS_H
#define   	SCHED_DEFS_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedOsRelatedDefinitions.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssEligFuncDefs.h>





#ifndef MIN
	#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif /* MIN */

#ifndef MAX
	#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif  /* MAX */

#define PER_NODE_SHAPING
/* #define VIRTUAL_LAYER */
#define USE_DEFAULT_CURVES



/*---------------- Constants definitions-------------*/



#define SCHED_MTU          _1K   /* bytes */



/** Infinite Profile */
#define SCHED_INF_SHP_PROFILE  0

#ifdef PER_NODE_SHAPING
	#define SCHED_DIRECT_NODE_SHAPING	(uint16_t)(-1) /* read only profile for node, set automatically if node shaping was configured directly */
#endif




/* TM INVALID constants */
/** 32 bit Invalid data indicator */
#define TM_INVAL         0xFFFFFFFF


/** 8 bit Invalid data indicator */
#define TM_INVAL_8_BIT        0xFF

/** 16 bit Invalid data indicator */
#define TM_INVAL_16_BIT        0xFFFF



/* Status constants */
/** Enable indicator */
#define TM_ENABLE  1
/** Disable indicator */
#define TM_DISABLE 0


/** Maximum Bandwidth (in Kbits/sec) */
#define TM_MAX_BW 100000000 /* 100GBit/sec */

/** Maximum Burst (in Kbytes) */
#define TM_MAX_BURST (100 * 1024)


/*---------------------- Enumerated Types---------------- */

/** scheduler levels */
enum schedLevel
{
    SCHED_Q_LEVEL = 0, /**< Queue Level */
    SCHED_A_LEVEL,     /**< A-nodes Level */
    SCHED_B_LEVEL,     /**< B-nodes Level */
    SCHED_C_LEVEL,     /**< C-nodes Level */
    SCHED_P_LEVEL      /**< Ports Level */
};

#define	PARENT_LEVEL(level) level+1
#define	CHILD_LEVEL(level) level-1

/** Port's physical bandwidth */

/** Token bucket usage */
enum prvSchedTokenBucketUsage {
    SCHED_MIN_TOKEN_BUCKET = 0, /**< Use Min token bucket */
    SCHED_MAX_TOKEN_BUCKET      /**< Use Max tiken bucket */
};



/*------------------- Global Parameters Data structures-----------------*/

/** tm lib init params */
struct PRV_CPSS_SCHED_LIB_INIT_PARAMS_STC {
    uint32_t schedLibMtu;            /**< tm MTU unit, if 0 then MTU is SCHED_MTU */
    uint32_t  schedLibPdqNum;
    /* ... more tm lib init params here */
};

/** Periodic Update Rate configuration params per level */
struct schedPerLevelPeriodicParams
{
    uint8_t per_state;  /** Periodic shaper update Enable/Disable */
    uint8_t shaper_dec;   /**< Shaper Decoupling Enable/Disable */
    uint32_t per_interval;      /**< interval in which a single level node
                                  * periodic update is performed (has HW reg)*/
};




/** Shared Shaper Profile configuration structure */
struct sched_shaping_profile_params {
    uint32_t cir_bw; /**< CIR BW in Kbits/sec */
    uint16_t cbs;    /**< CBS in kbytes */
    uint32_t eir_bw; /**< EIR BW in Kbits/sec */
    uint16_t ebs;    /**< EBS in kbytes */
};


/*----------------- Nodes Parameters Data Structures---------------*/
/* Note: only drop mode 0 is supported in current version.
 *  Drop profile reference fields are present only for queues and
 *  ports
*/

/** Queue Parameters Data Structure */
struct sched_queue_params {
    uint32_t shaping_profile_ref;  /**< Index of Shaping profile */
    uint16_t quantum;              /**< Queue DWRR Quantum in TM_NODE_QUANTUM_UNIT */
    uint8_t elig_prio_func_ptr;    /**< Eligible Priority Function pointer */
};


/** A-Node Parameters Data Structure */
struct sched_a_node_params {
    uint32_t shaping_profile_ref;  /**< Index of Shaping profile */
    uint16_t quantum;              /**< DWRR Quantum in TM_NODE_QUANTUM_UNIT */
    uint8_t dwrr_priority[8];      /**< DWRR Priority for Queue Scheduling */
    uint8_t elig_prio_func_ptr;    /**< Eligible Priority Function pointer */
    uint32_t num_of_children;      /**< Number of children nodes */
};


/** B-Node Parameters Data Structure */
struct sched_b_node_params {
    uint32_t shaping_profile_ref;  /**< Index of Shaping profile */
    uint16_t quantum;              /**< DWRR Quantum in TM_NODE_QUANTUM_UNIT */

    uint8_t dwrr_priority[8];      /**< DWRR Priority for A-Node Scheduling */
    uint8_t elig_prio_func_ptr;    /**< Eligible Priority Function pointer */
    uint16_t num_of_children;      /**< Number of children nodes */
};


/** C-Node Parameters Data Structure */
struct sched_c_node_params {
    uint32_t shaping_profile_ref;  /**< Index of Shaping profile */
    uint16_t quantum;              /**< DWRR Quantum in TM_NODE_QUANTUM_UNIT */

    uint8_t dwrr_priority[8];      /**< DWRR Priority for B-Node Scheduling */

    uint8_t elig_prio_func_ptr;    /**< Eligible Priority Function pointer */
    uint16_t num_of_children;      /**< Number of children nodes */
};


/** Port Parameters Data Structure */
struct sched_port_params {
    uint32_t cir_bw;               /**< CIR BW in Kbits/sec */
    uint32_t eir_bw;               /**< EIR BW in Kbits/sec */
    uint32_t cbs;                  /**< CBS in bytes */
    uint32_t ebs;                  /**< EBS in bytes */
    uint16_t quantum[8];           /**< DWRR Quantum for each instance in TM_PORT_QUANTUM_UNIT */

    uint8_t dwrr_priority[8];      /**< DWRR Priority for C-Node Scheduling */
    uint8_t elig_prio_func_ptr;    /**< Eligible Priority Function pointer */
    uint16_t num_of_children;      /**< Number of children nodes */
};


/** Port status data structure */
struct sched_port_status {
    uint32_t max_bucket_level;  /**< Maximal Shaper Bucket level */
    uint32_t min_bucket_level;  /**< Minimal Shaper Bucket level */
    uint32_t deficit[8];        /**< DWRR Deficit per instance */
};


/** Node status data structure */
struct schedNodeStatus {
    uint32_t max_bucket_level;  /**< Maximal Shaper Bucket level */
    uint32_t min_bucket_level;  /**< Minimal Shaper Bucket level */
    uint32_t min_bucket_sign;  /**< Minimal Shaper Bucket sign 0 -pos 1 - neg */
    uint32_t deficit;           /**< DWRR Deficit */
};



/** TM Blocks Error Information */
struct schedErrorInfo {
    uint16_t error_counter; /**< TM Block Error counter */
    uint16_t exception_counter; /**< TM Block Exception Counter */
};






#ifdef VIRTUAL_LAYER_TO_REMOVE_IN_FUTURE
#else
	/** Reshuffling index/range change structure */
	struct sched_tree_change {
		uint32_t index; /**< Index of changed parent node */
		uint32_t old_index;   /**< Old index/range */
		uint32_t new_index;   /**< New index/range */
		struct sched_tree_change *next; /**< Pointer to the next change */
	};
#endif

/** Eligible Priority Function Data structures */
struct sched_elig_prio_func_out {
	uint8_t max_tb;             /**< Use Max Token Bucket   */
    uint8_t min_tb;             /**< Use Min Token Bucket   */
    uint8_t prop_prio;          /**< Propagated priority    */
    uint8_t sched_prio;         /**< Scheduling priority    */
    uint8_t elig;               /**< Eligibility            */
};

/** Eligible Priority Function storage */
union sched_elig_prio_func{
	struct sched_elig_prio_func_out queue_elig_prio_func[4];		/**< Eligible Priority Functions for queues   */
	struct sched_elig_prio_func_out node_elig_prio_func[8][4];		/**< Eligible Priority Functions for intermediate nodes   */
};




struct sched_quantum_limits /* applicable for port or nodes  quantum limits */
{
    uint32_t minQuantum;	/* minimal possible quantum value in bytes */
    uint32_t maxQuantum;	/* maximal possible quantum value in bytes */
    uint32_t resolution;	/* quantum granularity in bytes*/
};


typedef void * PRV_CPSS_SCHED_HANDLE;


struct PRV_CPSS_SCHED_TREE_MAX_SIZES_STC
{
    /* TM tree structure parameters */
    uint16_t    numOfQueues;
    uint16_t    numOfAnodes;
    uint16_t    numOfBnodes;
    uint16_t    numOfCnodes;
    uint16_t    numOfPorts;

};

struct PRV_CPSS_SCHED_PORT_TO_Q_CREATE_OPTIONS_STC
{
    /*if GT_TRUE then queue number is scpecified by the caller,otherwise first free queue number is given*/
    GT_BOOL explicitQueueStart;
    /*relevant if explicitQueueStart is equal GT_TRUE.Queue number requested by the caller*/
    GT_U32  queueStart;
    /*if equal GT_TRUE then A node is added to already existing P->C->B branch*/
    GT_BOOL addToExistingBnode;
    GT_BOOL addToExistingCnode;
    /*if equal GT_TRUE then A node index is  is scpecified by the caller,otherwise first free a node index is given*/
    GT_BOOL explicitAnodeInd;
    /*relevant if explicitAnodeInd is equal GT_TRUE.A node indexrequested by the caller*/
    GT_U32  aNodeInd;
    GT_BOOL explicitBnodeInd;
    GT_U32  bNodeInd;
};

typedef struct PRV_CPSS_SCHED_PORT_TO_Q_CREATE_OPTIONS_STC PRV_CPSS_SCHED_PORT_TO_Q_CREATE_OPTIONS_STC;

#endif   /* TM_DEFS_H */

