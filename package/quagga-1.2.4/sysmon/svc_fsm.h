typedef enum {
	SVC_EVT_NONE,
	SVC_EVT_INIT,
	SVC_EVT_IPC_COM_SUCCESS,
	SVC_EVT_IPC_COM_WAIT,
	SVC_EVT_IPC_COM_FAIL,
	SVC_EVT_PLL_LOCK,
	SVC_EVT_PLL_UNLOCK,
	SVC_EVT_DPRAM_ACCESS_SUCCESS,
	SVC_EVT_DPRAM_ACCESS_FAIL,
	SVC_EVT_FPGA_ACCESS_SUCCESS,
	SVC_EVT_FPGA_ACCESS_FAIL, /* 10 */
	SVC_EVT_CPLD_ACCESS_SUCCESS,
	SVC_EVT_CPLD_ACCESS_FAIL,
	SVC_EVT_SDK_INIT_SUCCESS,
	SVC_EVT_SDK_INIT_FAIL,
	SVC_EVT_GET_INVEN_SUCCESS,
	SVC_EVT_GET_INVEN_FAIL,
	SVC_EVT_PORT_LINK_DOWN,
	SVC_EVT_PORT_LINK_UP,
	SVC_EVT_INIT_DONE,
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	SVC_EVT_APPDEMO_SHUTDOWN, /* 20 */
#endif 
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
	SVC_EVT_SDK_INIT_WAIT,
	SVC_EVT_INIT_FAIL,
#endif
	SVC_EVT_MAX
} SVC_EVT;

typedef enum {
	SVC_ST_INIT,
	SVC_ST_INIT_FAIL,
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	SVC_ST_APPDEMO_SHUTDOWN,
#endif
	SVC_ST_DPRAM_CHK,
	SVC_ST_FPGA_CHK,
	SVC_ST_CPLD_CHK, /*5*/
	SVC_ST_SDK_INIT,
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
	SVC_ST_SDK_INIT_CHK,
#endif
	SVC_ST_GET_INVEN,
	SVC_ST_INIT_DONE,
	/*
	SVC_ST_TERM,
	*/
	SVC_ST_MAX,
} SVC_ST; 

typedef SVC_EVT (*ST_CB_FUNC) (SVC_ST state);

typedef struct {
	ST_CB_FUNC svc_init;
	ST_CB_FUNC svc_init_fail;
	ST_CB_FUNC svc_dpram_chk;
	ST_CB_FUNC svc_fpga_chk;
	ST_CB_FUNC svc_cpld_chk;
	ST_CB_FUNC svc_sdk_init;
	ST_CB_FUNC svc_sdk_init_done;
	ST_CB_FUNC svc_get_inven;
	ST_CB_FUNC svc_init_done;
} SVC_ST_CALLBACKS;

typedef struct svc_mng_fsm {
	SVC_ST state;
	SVC_EVT evt;
	ST_CB_FUNC cb[SVC_ST_MAX];
} SVC_FSM;

