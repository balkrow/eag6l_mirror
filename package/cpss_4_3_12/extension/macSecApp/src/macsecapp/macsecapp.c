/**

   QuickSec for MACsec example application.

   File: macsecapp.c

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#include "macsecapp_platform.h"
#include "dot1xpae.h"
#include "eapauth.h"
#include "macseclmi.h"
#include "macsec_list.h"
#include "sshincludes.h"
#undef snprintf
#undef vsnprintf
#include "ssheloop.h"
#include "sshfsm.h"
#include "sshtimeouts.h"
#include "sshfileio.h"
#include "sshgetopt.h"
#include "sshradius.h"
#include "x509.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "macsec_dataplane.h"

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x)
#endif

typedef struct {
  unsigned char dot1x[(DOT1X_MESSAGE_LAST + 7) >> 3];
} Debug;

/*
 * EAP key provider data.
 */
typedef struct {
  /* externalkey provider type */
  char *type;
  /* externalkey provider initialization info */
  char *init;
  /* externalkey provider handle */
  char *provider;
  /* parent EAP configuration */
  struct EGlobalS *eap;
  /* list link */
  MacsecListLink link;
} EProvider;

/*
 * EAP certificate authority data.
 */
typedef struct {
  /* file name for diagnostic purposes */
  char *file;
  /* DER encoding of CA certificate */
  unsigned char *derbuf;
  unsigned derlen;
  /* parent EAP configuration */
  struct EGlobalS *eap;
  /* certificate manager handle */
  struct SshCMCertificateRec *certificate;
  /* list link */
  MacsecListLink link;
} ECa;

/*
 * EAP CRL data.
 */
typedef struct {
  /* file name for diagnostic purposes */
  char *file;
  /* DER encoding of CRL */
  unsigned char *derbuf;
  unsigned derlen;
  /* certficate manager handle */
  struct SshCMCrlRec *crl;
  /* parent EAP configuration */
  struct EGlobalS *eap;
  /* list link */
  MacsecListLink link;
} ECrl;

/*
 * EAP RADIUS server data.
 */
typedef struct {
  /* address string passed directly to lib */
  char *address;
  /* secret */
  unsigned char secret_buf[256];
  unsigned secret_len;
  /* port string passed directly to lib, or NULL */
  char *port;
  /* accounting port string passed directly to lib, or NULL */
  char *aport;
  /* parent EAP configuration */
  struct EGlobalS *eap;
  /* list link */
  MacsecListLink link;
} ERserver;

/*
 * Per-network EAP data.
 */
typedef struct {
  /* UTF-8 network name */
  Dot1xNid nid;
  /* EAP identity */
  char *identity;
  /* keystore handle */
  struct EapKeystoreS *keystore;
  /* protocol network handle */
  struct EapNetworkS *network;
  /* parent EAP configuration */
  struct EGlobalS *eap;
  /* list link */
  MacsecListLink link;
} ENetwork;

/*
 * Global EAP configuration.
 */
typedef struct EGlobalS {
  /* key providers */
  MacsecListLink providers;
  /* certificate authorities */
  MacsecListLink cas;
  /* CRLs */
  MacsecListLink crls;
  /* RADIUS servers */
  MacsecListLink rservers;
  /* default EAP network */
  ENetwork default_network;
  /* EAP networks */
  MacsecListLink networks;
  /* keystore handle */
  struct EapKeystoreS *keystore;
  /* RADIUS servers changed flag */
  unsigned rservers_changed : 1;
  /* RADIUS library configuration handle */
  struct SshRadiusClientServerInfoRec *radius_servers;
} EGlobal;

/*
 * Network parameter validity flags.
 */
typedef struct {
  unsigned char use_eap;
  unsigned char unauth_allowed;
  unsigned char unsecured_allowed;
  unsigned char kmd;
} NetworkHas;

/*
 * Network configuration data.
 */
typedef struct MaNetworkS {
  /* UTF-8 network name */
  Dot1xNid nid;
  /* configuration parameters */
  Dot1xNetworkCtrl ctrl;
  /* parameter validity flags */
  NetworkHas has;
  /* protocol data */
  Dot1xNetwork *dot1x;
  Dot1xNetworkCtrl initial_ctrl;
  /* parent interface */
  struct MaInterfaceS *interface;
  /* list link */
  MacsecListLink link;
} Network;

/*
 * Pre-shared key configuration data.
 */
typedef struct MaPskS {
  /* CAK name */
  Dot1xCkn ckn;
  /* CAK value */
  MacsecKey cak;
  /* pairwise/group */
  Dot1xCakType type;
  /* NID */
  Dot1xNid nid;
  /* authorization data */
  MacsecAuthData auth_data;
  /* protocol data */
  Dot1xCak *dot1x;
  /* parent interface */
  struct MaInterfaceS *interface;
  /* list link */
  MacsecListLink link;
} Psk;

/*
 * Per-interface EAP data.
 */
typedef struct {
  /* EAP context data */
  EapContext context;
  /* EAP context initialized flag */
  int init;
} EapIntf;

/*
 * MACsec SecY parameter validity flags.
 */
typedef struct {
  unsigned char validate_frames;
  unsigned char replay_protect;
  unsigned char replay_window;
  unsigned char protect_frames;
  unsigned char always_include_sci;
  unsigned char use_es;
  unsigned char use_scb;
  unsigned char current_cipher_suite;
  unsigned char confidentiality_offset;
  unsigned char admin_point_to_point_mac;
} SecyHas;

/*
 * MACsec interface data.
 */
typedef struct {
  /* configuration parameters */
  MacsecSecyCtrl secy_ctrl;
  /* parameter validity flags */
  SecyHas secy_has;
  /* protocol data */
  MacsecIntfId intf;
  MacsecSecyCtrl initial_secy_ctrl;
  /* protocol data initialized flag */
  int init;
  /* waitable file handle for uncontrolled port receive */
  int fd;
  /* fd registered flag */
  int fd_register;
  /* flags */
  unsigned flags;
} MInterface;

/*
 * Dot1x PAE parameter validity flags.
 */
typedef struct {
  unsigned char eapol_group;
  unsigned char virtual_ports_enable;
  unsigned char announcer_enable;
  unsigned char listener_enable;
  unsigned char mka_enable;
  unsigned char bounded_delay;
  unsigned char actor_priority;
  unsigned char cipher_suite;
  unsigned char confidentiality_offset;
  unsigned char logon;
  unsigned char selected_nid;
  unsigned char cak_caching_enable;
  unsigned char group_ca_enable;
  unsigned char long_eap_caks;
  unsigned char supplicant_enable;
  unsigned char authenticator_enable;
  unsigned char held_period;
  unsigned char quiet_period;
  unsigned char reauth_enabled;
  unsigned char reauth_period;
  unsigned char retry_max;
  unsigned char logoff_enable;
  unsigned char macsec_protect;
  unsigned char macsec_validate;
  unsigned char macsec_replay_protect;
  unsigned char transmit_delay;
  unsigned char retire_delay;
  unsigned char suspend_for;
  unsigned char suspend_on_request;
  unsigned char suspended_while;
} PaeHas;

/*
 * Dot1x interface data.
 */
typedef struct {
  /* configuration parameters */
  Dot1xPaeCtrl pae_ctrl;
  /* parameter validity flags */
  PaeHas pae_has;
  /* protocol interface data */
  Dot1xIntf intf;
  Dot1xPaeCtrl initial_pae_ctrl;
  /* protocol default network data */
  Network default_network;
  /* protocol network data */
  MacsecListLink networks;
  /* protocol pre-shared key data */
  MacsecListLink psks;
  /* protocol data initialized flag */
  int init;
} DInterface;

/*
 * Interface configuration data.
 */
typedef struct MaInterfaceS {
  /* interface name */
  char *name;
  /* EAP data */
  EapIntf eap;
  /* 802.1AE data */
  MInterface macsec;
  /* 802.1X data */
  DInterface dot1x;
  /* debug configuration */
  Debug debug;
  /* timer object for protocol timing */
  SshTimeoutStruct tick_timeout;
  /* timer object to break EAP callback recursion */
  SshTimeoutStruct eap_timeout;
  /* parent configuration */
  struct MaConfigS *config;
  /* list link */
  MacsecListLink link;
} Interface;

/*
 * Configuration data.
 */
typedef struct MaConfigS {
  EGlobal eap;
  MacsecListLink interfaces;
  char *quicksec_debug;
} Config;

/*
 * Configuration file reading state.
 */
#define TOKEN_SIZE 256
typedef struct {
  char *file;
  unsigned char *inbuf;
  unsigned inlen;
  unsigned inpos;
  char valuebuf[TOKEN_SIZE];
  unsigned valueline;
  char tokenbuf[TOKEN_SIZE];
  unsigned tokenpos;
  unsigned tokenline;
  enum {
    TOKEN_NONE,
    TOKEN_UNQUOTED,
    TOKEN_QUOTED,
    TOKEN_PUNCT,
    TOKEN_END
  } tokentype;
  unsigned char charbuf[4];
  unsigned charlen;
  unsigned charline;
  int charval;
  int byte;
} Reader;

struct CAKCacheEntry
{
  int ifindex;
  Dot1xCak cak;
};

#define CAK_CACHE_SIZE 128
static struct CAKCacheEntry cak_cache[CAK_CACHE_SIZE];

/*
 * String to integer mapping.
 */
typedef struct {
  const char *s;
  int i;
} StringInt;

/*
 * Buffer for printable output.
 */
typedef char PrintBuffer[65];

/*
 * Print command line usage.
 */
static void usage(void);
static void usage_debug(void);

/*
 * Set debug flags from a command line argument.
 */
static void set_debug(const char *s);

/*
 * Handler for terminal input.
 */
static void terminal_callback(unsigned events, void *context);

/*
 * Dummy timeout handler.
 */
static void handle_timeout(void *context);

/*
 * Handle events for the event loop.
 */
static SSH_FSM_STEP(handle_events);
static void handle_quit(void);
static void handle_configure(void);
static void handle_print(void);
static void handle_dtoggle(void);
static void handle_terminal(void);
static void handle_hint(void);
static void handle_help(void);

static void
cak_cache_callback(Dot1xCakCacheOp op, const Dot1xCak *cak);

static void
cak_cache_add_cached(void);

/*
 * EAP signalling function and another one called through timeout.
 */
static void eap_wakeup(void *arg);
static void eap_wakeup2(void *context);

/*
 * Uncontrolled port I/O callback.
 */
static void macsec_io(unsigned int events, void *context);

/*
 * Interface timer callback.
 */
static void tick(void *context);

/*
 * Configuration reader.
 */
static Config *input_config(const char *file);
static Config *read_config(Reader *r);
static int read_version(Reader *r);
static int read_unique_eap(Reader *r, unsigned char *has, EGlobal *e);
static int read_provider(Reader *r, EGlobal *e);
static int read_ca(Reader *r, EGlobal *e);
static int read_crl(Reader *r, EGlobal *e);
static int read_rserver(Reader *r, EGlobal *e);
static int read_default_eap_network(Reader *r, unsigned char *has, EGlobal *e);
static int read_eap_network(Reader *r, EGlobal *e);
static void read_eap_network_sub(Reader *r, ENetwork *en);
static int read_interface(Reader *r, Config *c);
static int read_default_network(Reader *r, unsigned char *has, Interface *i);
static int read_network(Reader *r, Interface *i);
static void read_network_sub(Reader *r, Network *n);
static int read_psk(Reader *r, Interface *i);
static int read_unique_debug(Reader *r, unsigned char *has, Debug *d);
static int read_unique_choice(
  Reader *r, const char *keyword, unsigned char *has,
  int *n, const StringInt *map);
static int read_unique_bool(
  Reader *r, const char *keyword, unsigned char *has, int *n);
static int read_unique_key(
  Reader *r, const char *keyword, unsigned char *has, MacsecKey *key);
static int read_unique_unsigned(
  Reader *r, const char *keyword, unsigned char *has, unsigned *u,
  unsigned min, unsigned max);
static int read_unique_uint64(
  Reader *r, const char *keyword, unsigned char *has, uint64_t *u64,
  uint64_t min, uint64_t max);
static int read_unique_nid(
  Reader *r, const char *keyword, unsigned char *has, Dot1xNid *nid);
static int read_unique_kmd(
  Reader *r, const char *keyword, unsigned char *has, Dot1xKmd *kmd);
static int read_unique_ad(
  Reader *r, const char *keyword, unsigned char *has, MacsecAuthData *ad);
static int read_unique_hex(
  Reader *r, const char *keyword, unsigned char *has,
  unsigned char *buf, unsigned *len, unsigned minlen, unsigned maxlen);
static int read_unique_utf8(
  Reader *r, const char *keyword, unsigned char *has,
  unsigned char *buf, unsigned *len, unsigned max);
static int read_unique_string(
  Reader *r, const char *keyword, unsigned char *has, char **s);
static int read_unique_keyword_and_value(
  Reader *r, const char *keyword, unsigned char *has);
static int read_keyword_and_value(Reader *r, const char *keyword);
static int read_unique_keyword(
  Reader *r, unsigned char *has, const char *keyword);
static void read_required_ckn(Reader *r, Dot1xCkn *ckn);
static void read_required_nid(Reader *r, Dot1xNid *nid);
static void read_required_hex(
  Reader *r, unsigned char *buf, unsigned *len,
  unsigned minlen, unsigned maxlen);
static void read_required_utf8(
  Reader *r, unsigned char *buf, unsigned *len, unsigned max);
static void read_required_string(Reader *r, char **s);
static void read_required_value(Reader *r);
static void read_required_punct(Reader *r, int punct);
static int read_keyword(Reader *r, const char *keyword);
static int read_value(Reader *r);
static int read_punct(Reader *r, int punct);
static int read_end(Reader *r);
static void read_token(Reader *r);
static int read_token_unquoted(Reader *r);
static int read_token_quoted(Reader *r);
static int read_token_punct(Reader *r);
static int read_token_end(Reader *r);
static int read_comment(Reader *r);
static int read_white(Reader *r);
static void read_char(Reader *r);
static int read_bom(Reader *r);
static void read_byte(Reader *r);
static void clear_token(Reader *r);
static void grow_token(Reader *r);
#define read_fail_value(r, ...) \
  read_fail((r), (r)->valueline, __VA_ARGS__)
#define read_fail_token(r, ...) \
  read_fail((r), (r)->tokenline, __VA_ARGS__)
#define read_fail_char(r, ...) \
  read_fail((r), (r)->charline, __VA_ARGS__)
static void read_fail_expected(Reader *r, const char *fmt, ...)
     __attribute__ ((format (printf, 2, 3), noreturn));
static void read_fail_unexpected(Reader *r);
static void read_fail(Reader *r, unsigned line, const char *fmt, ...)
     __attribute__ ((format (printf, 3, 4), noreturn));
static int charhex(int c);
static int charflags(int c);

/*
 * Configuration manipulation.
 */
static Config *create_config(void);
static void destroy_config(Config *c);
static void deactivate_config(Config *c);
static void update_config(Config *old, const Config *new);
static void update_providers(EGlobal *old, const EGlobal *new);
static void update_cas(EGlobal *old, const EGlobal *new);
static void update_crls(EGlobal *old, const EGlobal *new);
static void update_rservers(EGlobal *old, const EGlobal *new);
static void update_eap_networks(EGlobal *old, const EGlobal *new);
static void update_interfaces(Config *old, const Config *new);

/*
 * Key provider object manipulation.
 */
static int insert_provider(EGlobal *e, EProvider *p);
static void remove_provider(EGlobal *e, EProvider *p);
static void foreach_provider(EGlobal *e, void (*func)(EProvider *p));
static EProvider *next_provider(const EGlobal *e, const EProvider *prev);
static int compare_provider(const EProvider *p1, const EProvider *p2);
static int activate_provider(EProvider *p, const EProvider *p0);
static void deactivate_provider(EProvider *p);
static int reconfigure_provider(EProvider *old, const EProvider *new);
static EProvider *create_provider(EGlobal *eap);
static void destroy_provider(EProvider *p);

/*
 * CA object manipulation.
 */
static int insert_ca(EGlobal *e, ECa *c);
static void remove_ca(EGlobal *e, ECa *c);
static void foreach_ca(EGlobal *e, void (*func)(ECa *c));
static ECa *next_ca(const EGlobal *e, const ECa *prev);
static int compare_ca(const ECa *c1, const ECa *c2);
static int activate_ca(ECa *c, const ECa *c0);
static void deactivate_ca(ECa *c);
static int reconfigure_ca(ECa *old, const ECa *new);
static ECa *create_ca(EGlobal *eap);
static void destroy_ca(ECa *c);

/*
 * CRL object manipulation.
 */
static int insert_crl(EGlobal *e, ECrl *c);
static void remove_crl(EGlobal *e, ECrl *c);
static void foreach_crl(EGlobal *e, void (*func)(ECrl *c));
static ECrl *next_crl(const EGlobal *e, const ECrl *prev);
static int compare_crl(const ECrl *c1, const ECrl *c2);
static int activate_crl(ECrl *c, const ECrl *c0);
static void deactivate_crl(ECrl *c);
static int reconfigure_crl(ECrl *old, const ECrl *new);
static ECrl *create_crl(EGlobal *eap);
static void destroy_crl(ECrl *c);

/*
 * RADIUS server object manipulation.
 */
static int insert_rserver(EGlobal *e, ERserver *r);
static void remove_rserver(EGlobal *e, ERserver *r);
static void foreach_rserver(EGlobal *e, void (*func)(ERserver *r));
static ERserver *next_rserver(const EGlobal *e, const ERserver *prev);
static int compare_rserver(const ERserver *r1, const ERserver *r2);
static int activate_rserver(ERserver *r, const ERserver *r0);
static void deactivate_rserver(ERserver *r);
static int reconfigure_rserver(ERserver *old, const ERserver *new);
static ERserver *create_rserver(EGlobal *eap);
static void destroy_rserver(ERserver *r);

/*
 * EAP network object manipulation.
 */
static int insert_eap_network(EGlobal *e, ENetwork *en);
static void remove_eap_network(EGlobal *e, ENetwork *en);
static void foreach_eap_network(EGlobal *e, void (*func)(ENetwork *en));
static ENetwork *next_eap_network(const EGlobal *e, const ENetwork *prev);
static int compare_eap_network(const ENetwork *en1, const ENetwork *en2);
static int activate_eap_network(ENetwork *en, const ENetwork *en0);
static void deactivate_eap_network(ENetwork *en);
static int reconfigure_eap_network(ENetwork *old, const ENetwork *new);
static ENetwork *create_eap_network(EGlobal *eap);
static void destroy_eap_network(ENetwork *en);

/*
 * Interface object manipulation.
 */
static int insert_interface(Config *c, Interface *i);
static void remove_interface(Config *c, Interface *i);
static void foreach_interface(Config *c, void (*func)(Interface *i));
static Interface *next_interface(const Config *c, const Interface *prev);
static int compare_interface(const Interface *i1, const Interface *i2);
static int activate_interface(Interface *i, const Interface *i0);
static void deactivate_interface(Interface *i);
static int reconfigure_interface(Interface *old, const Interface *new);
static Interface *create_interface(Config *config);
static void destroy_interface(Interface *i);
static void update_networks(Interface *old, const Interface *new);
static void update_psks(Interface *old, const Interface *new);
static void set_debug_suppress(Interface *i);

/*
 * Network object manipulation.
 */
static int insert_network(Interface *i, Network *n);
static void remove_network(Interface *i, Network *n);
static void foreach_network(Interface *i, void (*func)(Network *n));
static Network *next_network(const Interface *i, const Network *prev);
static int compare_network(const Network *n1, const Network *n2);
static int activate_network(Network *n, const Network *n0);
static void deactivate_network(Network *n);
static int reconfigure_network(Network *old, const Network *new);
static Network *create_network(Interface *interface);
static void destroy_network(Network *n);

/*
 * Pre-shared key object manipulation.
 */
static int insert_psk(Interface *i, Psk *p);
static void remove_psk(Interface *i, Psk *p);
static void foreach_psk(Interface *i, void (*func)(Psk *p));
static Psk *next_psk(const Interface *i, const Psk *prev);
static int compare_psk(const Psk *p1, const Psk *p2);
static int activate_psk(Psk *p, const Psk *p0);
static void deactivate_psk(Psk *p);
static int reconfigure_psk(Psk *old, const Psk *new);
static Psk *create_psk(Interface *interface);
static void destroy_psk(Psk *p);

/*
 * Update a list of configuration objects and associated protocol
 * configuration to match a new list.
 */
static void
update_list(
  void *oldparent, const void *newparent,
  void *nextfunc, void *cmpfunc,
  void *removefunc, void *insertfunc,
  void *activatefunc, void *deactivatefunc, void *reconfigurefunc,
  void *createfunc, void *destroyfunc);

/*
 * Insert a new object into an ordered list. Return 1 if successful, 0
 * if a matching object (with respect to the strcmp-like comparison
 * function 'compare') already exists in the list.
 */
static int
insert_unique(MacsecListLink *list, unsigned offset, void *cmpfunc, void *obj);

/*
 * Apply changes to protocol variables.
 */

static void set_secy_ctrl(
  MacsecIntfId i, const MacsecSecyCtrl *def,
  const SecyHas *has, const MacsecSecyCtrl *new);
static void set_pae_ctrl(
  Dot1xIntf *i, const Dot1xPaeCtrl *def,
  const PaeHas *has, const Dot1xPaeCtrl *new);
static void set_default_network_ctrl(
  Dot1xIntf *i, const Dot1xNetworkCtrl *def,
  const NetworkHas *has, const Dot1xNetworkCtrl *new);
static void set_network_ctrl(
  Dot1xNetwork *n, const Dot1xNetworkCtrl *def,
  const NetworkHas *has, const Dot1xNetworkCtrl *new);
static void
set_network_ctrl_sub(
  Dot1xNetworkCtrl *c, const Dot1xNetworkCtrl *def,
  const NetworkHas *has, const Dot1xNetworkCtrl *new);

/*
 * Printable output.
 */
static void print_provider(EProvider *p);
static void print_ca(ECa *c);
static void print_crl(ECrl *c);
static void print_rserver(ERserver *r);
static void print_eap_network(ENetwork *en);
static void print_interface(Interface *i);
static void print_interface_dot1x(unsigned i, Dot1xIntf *intf);
static void print_pae_ctrl(unsigned i, Dot1xPaeCtrl *c);
static void print_pae_stat(unsigned i, Dot1xPaeStat *s);
static void print_network_ctrl(unsigned i, Dot1xNetworkCtrl *c);
static void print_network_stat(unsigned i, Dot1xNetworkStat *s);
static void print_cak_stat(unsigned i, Dot1xCakStat *s);
static void print_xport_stat(unsigned i, const Dot1xPortStat *s);
static void print_interface_macsec(unsigned i, MacsecIntfId intf);
static void print_macsec_capabilities(unsigned i, const MacsecCapabilities *c);
static void print_port_stat(unsigned i, const MacsecPortStat *s);
static void print_secy_ctrl(unsigned i, const MacsecSecyCtrl *c);
static void print_secy_stat(unsigned i, const MacsecSecyStat *s);
static void print_transmit_sc_stat(unsigned i, const MacsecTransmitScStat *s);
static void print_transmit_sa_stat(unsigned i, const MacsecTransmitSaStat *s);
static void print_receive_sc_stat(unsigned i, const MacsecReceiveScStat *s);
static void print_receive_sa_stat(unsigned i, const MacsecReceiveSaStat *s);
static void print_cipher_suites(unsigned i, const MacsecCipherCapability *ccv);
static void print(unsigned indent, const char *fmt, ...)
     __attribute__ ((format (printf, 2, 3)));
static const char *format_sysuptime(PrintBuffer *pb, uint32_t t);
static const char *format_sci(PrintBuffer *pb, const MacsecSci *s);
static const char *format_address(PrintBuffer *pb, const MacsecAddress *a);
static const char *format_ckn(PrintBuffer *pb, const Dot1xCkn *ckn);
static const char *format_nid(PrintBuffer *pb, const Dot1xNid *nid);
static const char *format_kmd(PrintBuffer *pb, const Dot1xKmd *kmd);
static const char *format_ad(PrintBuffer *pb, const MacsecAuthData *ad);
static const char *format_hex(
PrintBuffer *pb, const unsigned char *buf, unsigned len);
static const char *format_utf8(
PrintBuffer *pb, const unsigned char *buf, unsigned len);
static const char *str_bool(const unsigned b);
static const char *str_choice(const StringInt *map, const unsigned c);

/*
 * Low-level utilities.
 */
static int get_char(const char **p);
static int strtoint(const StringInt *map, const char *s);
static const char *inttostr(const StringInt *map, int i);
static int buftoint(const StringInt *map, const char *buf, unsigned len);
static void bufcpy(
  unsigned char *buf1, unsigned *len1,
  const unsigned char *buf2, unsigned len2);
static int bufcmp(
  const unsigned char *buf1, unsigned len1,
  const unsigned char *buf2, unsigned len2);
static int bufstrcmp(const char *buf, unsigned len, const char *s);
static void set_bit(unsigned char *tab, int n);
static int get_bit(unsigned char *tab, int n);

/*
 * QuickSec debug callback.
 */
static void debug_callback(const char *message, void *context);

/*
 * Nonlocal failure and error message utilities.
 */
#define failed() setjmp(*failpush())
#define notfailed() failpop()
static void failf(const char *fmt, ...)
     __attribute__ ((format (printf, 1, 2), noreturn));
static void fail(void)
     __attribute__ ((noreturn));
static jmp_buf *failpush(void);
static jmp_buf *failpop(void);
static void fatal(const char *fmt, ...)
     __attribute__ ((format (printf, 1, 2), noreturn));

/*
 * Misc variables.
 */
static const char *program;
static int terminal;
static const char *configfile = "macsec.txt";
static const char *outputfile;
static FILE *outputf;
static Config *config;

/*
 * Debug message configuration.
 */
static Debug default_debug;
static Debug debug;
static const char *default_quicksec_debug = "3";
static const char *quicksec_debug;
static int debug_suppress;
static int suspended;
static int resume;

/*
 * Event loop related variables.
 */
static SshFSMStruct fsm;
static SshFSMThreadStruct thread;
static SshFSMConditionStruct condition;
static SshTimeoutStruct timeout;
static int terminal_registered;
static int event_quit;
static int event_configure;
static int event_print;
static int event_dtoggle;
static int event_terminal;
static int event_hint;
static int event_help;
static int event_suspend;

static int first_configure;

/*
 * Configuration file character properties for character codes less
 * than 256.
 */
#define CHAR_WHITE 1
#define CHAR_PUNCT 2
#define CHAR_CONTROL 4
static const int charflagv[256] = {
  4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int charhexv[256] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static const char hexcharv[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

/*
 * Nonlocal failure jump contexts and current nesting level.
 */
#define MAX_FAILLEVELS 16
static jmp_buf failenv[MAX_FAILLEVELS];
static int faillevel;

/*
 * String to int mappings.
 */

static const StringInt eapol_group[] = {
  {"bridge", DOT1X_EAPOL_GROUP_BRIDGE},
  {"pae", DOT1X_EAPOL_GROUP_PAE},
  {"lldp", DOT1X_EAPOL_GROUP_LLDP},
  {NULL, 0}
};

static const StringInt use_eap[] = {
  {"never", DOT1X_USE_EAP_NEVER},
  {"immediate", DOT1X_USE_EAP_IMMEDIATE},
  {"mka-fail", DOT1X_USE_EAP_MKA_FAIL},
  {NULL, 0}
};

static const StringInt unauth_allowed[] = {
  {"never", DOT1X_UNAUTH_ALLOWED_NEVER},
  {"immediate", DOT1X_UNAUTH_ALLOWED_IMMEDIATE},
  {"auth-fail", DOT1X_UNAUTH_ALLOWED_AUTH_FAIL},
  {NULL, 0}
};

static const StringInt unsecured_allowed[] = {
  {"never", DOT1X_UNSECURED_ALLOWED_NEVER},
  {"immediate", DOT1X_UNSECURED_ALLOWED_IMMEDIATE},
  {"mka-fail", DOT1X_UNSECURED_ALLOWED_MKA_FAIL},
  {"mka-server", DOT1X_UNSECURED_ALLOWED_MKA_SERVER},
  {NULL, 0}
};

static const StringInt access_status[] = {
  {"no-access", DOT1X_ACCESS_STATUS_NO_ACCESS},
  {"remedial-access", DOT1X_ACCESS_STATUS_REMEDIAL_ACCESS},
  {"restricted-access", DOT1X_ACCESS_STATUS_RESTRICTED_ACCESS},
  {"expected-access", DOT1X_ACCESS_STATUS_EXPECTED_ACCESS},
  {NULL, 0}
};

static const StringInt unauthenticated_access[] = {
  {"no-access", DOT1X_UNAUTHENTICATED_ACCESS_NO_ACCESS},
  {"fallback-access", DOT1X_UNAUTHENTICATED_ACCESS_FALLBACK_ACCESS},
  {"limited-access", DOT1X_UNAUTHENTICATED_ACCESS_LIMITED_ACCESS},
  {"open-access", DOT1X_UNAUTHENTICATED_ACCESS_OPEN_ACCESS},
  {NULL, 0}
};

static const StringInt cipher_suite[] = {
  {"gcm-aes-128", MACSEC_CIPHER_SUITE_GCM_AES_128},
  {"gcm-aes-256", MACSEC_CIPHER_SUITE_GCM_AES_256},
  {"gcm-aes-xpn-128", MACSEC_CIPHER_SUITE_GCM_AES_XPN_128},
  {"gcm-aes-xpn-256", MACSEC_CIPHER_SUITE_GCM_AES_XPN_256},
  {NULL, 0}
};

static const StringInt confidentiality_offset[] = {
  {"not-used", MACSEC_CONFIDENTIALITY_NOT_USED},
  {"0", MACSEC_CONFIDENTIALITY_OFFSET_0},
  {"30", MACSEC_CONFIDENTIALITY_OFFSET_30},
  {"50", MACSEC_CONFIDENTIALITY_OFFSET_50},
  {NULL, 0}
};

static const StringInt admin_point_to_point_mac[] = {
  {"force-true", MACSEC_ADMIN_POINT_TO_POINT_MAC_FORCE_TRUE},
  {"force-false", MACSEC_ADMIN_POINT_TO_POINT_MAC_FORCE_FALSE},
  {"auto", MACSEC_ADMIN_POINT_TO_POINT_MAC_AUTO},
  {NULL, 0}
};

static const StringInt macsec_capability[] = {
  {"not implemented", MACSEC_CAPABILITY_NOT_IMPLEMENTED},
  {"integrity only", MACSEC_CAPABILITY_INTEGRITY},
  {"integrity and confidentiality", MACSEC_CAPABILITY_CONFIDENTIALITY},
  {"integrity and confidentiality with offsets",
   MACSEC_CAPABILITY_CONFIDENTIALITY_WITH_OFFSET},
  {NULL, 0}
};

static const StringInt validate_frames[] = {
  {"disabled", MACSEC_VALIDATE_FRAMES_DISABLED},
  {"check", MACSEC_VALIDATE_FRAMES_CHECK},
  {"strict", MACSEC_VALIDATE_FRAMES_STRICT},
  {NULL, 0}
};

static const StringInt boolean[] = {
  {"false", 0},
  {"true", 1},
  {NULL, 0}
};

static const StringInt psk_type[] = {
  {"pairwise", DOT1X_CAK_PAIRWISE},
  {"group", DOT1X_CAK_GROUP},
  {NULL, 0}
};

static const StringInt cak_type[] = {
  {"pre-shared pairwise", DOT1X_CAK_PAIRWISE},
  {"pre-shared group", DOT1X_CAK_GROUP},
  {"EAP-based pairwise", DOT1X_CAK_EAP},
  {"distributed group", DOT1X_CAK_DISTRIBUTED},
  {NULL, 0}
};

static const StringInt dot1x_message[] = {
  {"error", DOT1X_MESSAGE_ERROR},
  {"discard", DOT1X_MESSAGE_DISCARD},
  {"port", DOT1X_MESSAGE_PORT},
  {"network", DOT1X_MESSAGE_NETWORK},
  {"peer", DOT1X_MESSAGE_PEER},
  {"cak", DOT1X_MESSAGE_CAK},
  {"sak", DOT1X_MESSAGE_SAK},
  {"key-server", DOT1X_MESSAGE_KEY_SERVER},
  {"eapol", DOT1X_MESSAGE_EAPOL},
  {"eap", DOT1X_MESSAGE_EAP},
  {"mka", DOT1X_MESSAGE_MKA},
  {"announcement", DOT1X_MESSAGE_ANNOUNCEMENT},
  {"packet-detail", DOT1X_MESSAGE_PACKET_DETAIL},
  {"pacp", DOT1X_MESSAGE_PACP},
  {"mka-actor", DOT1X_MESSAGE_MKA_ACTOR},
  {"mka-peer", DOT1X_MESSAGE_MKA_PEER},
  {"cp", DOT1X_MESSAGE_CP},
  {"control", DOT1X_MESSAGE_CONTROL},
  {"logon-pacp", DOT1X_MESSAGE_LOGON_PACP},
  {"logon-mka", DOT1X_MESSAGE_LOGON_MKA},
  {"logon-cp", DOT1X_MESSAGE_LOGON_CP},
  {"pacp-eapol", DOT1X_MESSAGE_PACP_EAPOL},
  {"pacp-eap", DOT1X_MESSAGE_PACP_EAP},
  {"mka-cp", DOT1X_MESSAGE_MKA_CP},
  {"mka-secy", DOT1X_MESSAGE_MKA_SECY},
  {"cp-secy", DOT1X_MESSAGE_CP_SECY},
  {"cp-client", DOT1X_MESSAGE_CP_CLIENT},
  {NULL, 0}
};

/*
 * Command line long options.
 */
static const SshLongOptionStruct longopts[] = {
  {"help", SSH_GETOPT_LONG_NO_ARGUMENT, NULL, 'h'},
  {"file", SSH_GETOPT_LONG_REQUIRED_ARGUMENT, NULL, 'f'},
  {"output", SSH_GETOPT_LONG_REQUIRED_ARGUMENT, NULL, 'o'},
  {"debug", SSH_GETOPT_LONG_REQUIRED_ARGUMENT, NULL, 'd'},
  {"sshdebug", SSH_GETOPT_LONG_REQUIRED_ARGUMENT, NULL, 'D'},
  {NULL, 0, NULL, 0},
};

#define CAK_CACHE_FILE "./cak_cache.bin"

/*
 * Entry.
 */
int
ma_main(int argc, char *argv[])
{
  SshGetOptDataStruct getopt;
  int opt;

  if ((program = strrchr(argv[0], '/')))
    program++;
  else
    program = argv[0];

  set_bit(default_debug.dot1x, DOT1X_MESSAGE_ERROR);
  set_bit(default_debug.dot1x, DOT1X_MESSAGE_PORT);
  set_bit(default_debug.dot1x, DOT1X_MESSAGE_PEER);
  set_bit(default_debug.dot1x, DOT1X_MESSAGE_KEY_SERVER);
  memcpy(&debug, &default_debug, sizeof debug);

  ssh_debug_register_callbacks(
    debug_callback, debug_callback, debug_callback, NULL);

  quicksec_debug = default_quicksec_debug;
  ssh_debug_set_level_string(quicksec_debug);

  ssh_getopt_init_data(&getopt);

  while (1)
    {
      opt = ssh_getopt_long(
        argc, argv, "?hf:o:d:D:", longopts, NULL, &getopt);
      if (opt == -1)
        break;

      switch (opt)
        {
        case '?':
        case 'h':
          usage();
          return 0;

        case 'f':
          configfile = getopt.arg;
          break;

        case 'o':
          outputfile = getopt.arg;
          break;

        case 'd':
          if (!strcmp(getopt.arg, "?"))
            {
              usage_debug();
              return 0;
            }
          set_debug(getopt.arg);
          break;

        case 'D':
          quicksec_debug = getopt.arg;
          ssh_debug_set_level_string(quicksec_debug);
          break;

        default:
          usage();
          return 1;
        }
    }

  if (getopt.ind != argc)
    {
      usage();
      return 1;
    }

  if (!ssh_x509_library_initialize_framework(NULL))
    {
      ma_errorf("cannot initialize X509 library");
      return 1;
    }
  if (!ssh_x509_library_register_functions(
        SSH_X509_PKIX_CERT, ssh_x509_cert_decode_asn1, NULL))
    {
      ma_errorf("cannot register X509 decoder");
      ssh_x509_library_uninitialize();
      return 1;
    }
#if 0
  if (!macsec_test())
    {
      ma_errorf("MACsec selftest failed");
      return 1;
    }
#endif
  if (!dot1x_test())
    {
      ma_errorf("802.1X selftest failed");
      return 1;
    }

    {
      FILE *f = fopen(CAK_CACHE_FILE, "r");

      if (f)
        {
          if (fread(cak_cache, sizeof cak_cache, 1, f) != 1)
            {
              ma_errorf("Error reading CAK cache");
            }

          fclose(f);
          errno = 0;
          ma_errorf("Loaded CAK cache.");
          ma_errorf("Resuming from suspension");
        }
      else if (errno != ENOENT)
        {
          ma_errorf("Error opening CAK cache for reading");
        }

      unlink(CAK_CACHE_FILE);
    }

#if 0 /* NOT_REQUIRED */
  if (macsec_open() != 1)
  {
    ma_errorf("MACsec data plane open failed");
    return 1;
  }
#endif

  ssh_event_loop_initialize();

  /* add dummy timeout to keep event loop alive even without terminal */
  ssh_register_timeout(&timeout, 1, 0, handle_timeout, NULL);

  terminal = ma_open_terminal();

  if (terminal >= 0)
    {
      if (!ssh_io_register_fd(terminal, terminal_callback, NULL))
        ma_errorf("cannot register terminal to event loop");
      else
        {
          ssh_io_set_fd_request(terminal, SSH_IO_READ);
          terminal_registered = 1;
        }
    }

  ssh_fsm_init(&fsm, NULL);
  ssh_fsm_condition_init(&fsm, &condition);

  ssh_fsm_thread_init(&fsm, &thread, handle_events, NULL, NULL, NULL);

  event_hint = 1;
  event_configure = 1;
  first_configure = 1;

  ssh_event_loop_run();

  ssh_fsm_condition_uninit(&condition);
  ssh_fsm_uninit(&fsm);
  ssh_event_loop_uninitialize();

#if 0 /* NOT_REQUIRED */
  macsec_close();
#endif

  if (terminal >= 0)
    ma_close_terminal(terminal);

  if (suspended)
    {
      FILE *f = fopen(CAK_CACHE_FILE, "w");

      if (f)
        {
          if (fwrite(cak_cache, sizeof cak_cache, 1, f) != 1)
            {
              ma_errorf("Error writing CAK cache");
            }

          fclose(f);
          errno = 0;
          ma_errorf("Saved CAK cache");
        }
      else
        {
          ma_errorf("Error opening CAK cache for writing");
        }

      ma_errorf("Suspended.");
    }

  ssh_x509_library_uninitialize();
  ssh_util_uninit();
  return 0;
}

void
ma_quit(void)
{
  if (!suspended)
    {
      event_quit = 1;
      ssh_fsm_condition_signal(&fsm, &condition);
    }
}

void
ma_print(void)
{
  if (!suspended)
    {
      event_print = 1;
      ssh_fsm_condition_signal(&fsm, &condition);
    }
}

void
ma_configure(void)
{
  if (!suspended)
    {
      event_configure = 1;
      ssh_fsm_condition_signal(&fsm, &condition);
    }
}

void
ma_suspend(void)
{
  event_suspend = 1;
  ssh_fsm_condition_signal(&fsm, &condition);
}

void
ma_resume(void)
{
    resume = 1;
}

static void
usage()
{
  ma_errorf("usage: %s [options]", program);
  ma_errorf("options:");
  ma_errorf("  -h         this help message");
  ma_errorf("  -f file    configuration file (default macsec.txt)");
  ma_errorf("  -o file    output file for the print command");
  ma_errorf("  -d categs  comma-separated debug categories (-d ? for list)");
  ma_errorf("  -D string  QuickSec debug string");
}

static void
usage_debug()
{
  ma_errorf("debug categories:");
  ma_errorf("  error                    error events");
  ma_errorf("  discard                  PDU discards");
  ma_errorf("  port                     port status events");
  ma_errorf("  network                  announced network events");
  ma_errorf("  peer                     MACsec peer events");
  ma_errorf("  cak                      CAK events");
  ma_errorf("  sak                      SAK events");
  ma_errorf("  key-server               MKA key server events");
  ma_errorf("  eapol                    EAPOL packets");
  ma_errorf("  eap                      EAP packets and start/logoff events");
  ma_errorf("  mka                      MKA packets");
  ma_errorf("  announcement             announcement packets");
  ma_errorf("  packet-detail            enable full packet decodes");
  ma_errorf("  pacp                     PACP state changes and other events");
  ma_errorf("  mka-actor                MKA actor events");
  ma_errorf("  mka-peer                 MKA peer events");
  ma_errorf("  cp                       CP state changes");
  ma_errorf("  control                  control variable changes");
  ma_errorf("  logon-pacp               Logon/PACP communication");
  ma_errorf("  logon-mka                Logon/MKA communication");
  ma_errorf("  logon-cp                 Logon/CP communication");
  ma_errorf("  pacp-eapol               PACP/EAPOL/communication");
  ma_errorf("  pacp-eap                 PACP/EAP communication");
  ma_errorf("  mka-cp                   MKA/CP communication");
  ma_errorf("  mka-secy                 MKA/SecY communication");
  ma_errorf("  cp-secy                  CP/SecY communication");
  ma_errorf("  cp-client                CP/client communication");
  ma_errorf("  no-defaults              turn off default messages");
  ma_errorf("  everything               turn on all messages");
}

static void
set_debug(const char *s)
{
  const char *ptr;
  int len, i, defaults, everything;
  const StringInt *si;

  defaults = 1;
  everything = 0;
  ptr = s;
  while (*ptr)
    {
      if ((len = strcspn(ptr, ",")) > 0)
        {
          if ((i = buftoint(dot1x_message, ptr, len)) >= 0)
            set_bit(debug.dot1x, i);
          else if (!bufstrcmp(ptr, len, "no-defaults"))
            defaults = 0;
          else if (!bufstrcmp(ptr, len, "everything"))
            everything = 1;
          else
            ma_errorf("%.*s: invalid debug flag", len, ptr);
          ptr += len;
        }
      if (*ptr == ',')
        ptr++;
    }

  if (defaults)
    {
      for (i = 0; i < sizeof default_debug.dot1x; i++)
        debug.dot1x[i] |= default_debug.dot1x[i];
    }

  if (everything)
    {
      for (si = dot1x_message; si->s; si++)
        {
          if (si->i == DOT1X_MESSAGE_PACKET_DETAIL)
            continue;
          set_bit(debug.dot1x, si->i);
        }
    }
}

static void
terminal_callback(unsigned events, void *context)
{
  (void)events;
  (void)context;

  event_terminal = 1;
  ssh_fsm_condition_signal(&fsm, &condition);
}

static void
handle_timeout(void *context)
{
  (void)context;
  ssh_register_timeout(&timeout, 1, 0, handle_timeout, NULL);
}

static
SSH_FSM_STEP(handle_events)
{
  (void)fsm;
  (void)fsm_context;
  (void)thread_context;

  while (1)
    {
      if (event_quit)
        {
          event_quit = 0;
          suspended = 0;
          handle_quit();
          if (terminal_registered)
            {
              ssh_io_unregister_fd(terminal, FALSE);
              terminal_registered = 0;
            }
          ssh_cancel_timeout(&timeout);
          return SSH_FSM_FINISH;
        }
      else if (event_suspend)
        {
          event_suspend = 0;
          handle_quit();
          if (terminal_registered)
          {
            ssh_io_unregister_fd(terminal, FALSE);
            terminal_registered = 0;
          }
          ssh_cancel_timeout(&timeout);
        }
      else if (event_configure)
        {
          event_configure = 0;
          handle_configure();
          if (config && first_configure)
            {
              first_configure = 0;
              cak_cache_add_cached();
            }
        }
      else if (event_print)
        {
          event_print = 0;
          handle_print();
        }
      else if (event_dtoggle)
        {
          event_dtoggle = 0;
          handle_dtoggle();
        }
      else if (event_terminal)
        {
          event_terminal = 0;
          handle_terminal();
        }
      else if (event_hint)
        {
          event_hint = 0;
          handle_hint();
        }
      else if (event_help)
        {
          event_help = 0;
          handle_help();
        }
      else
        {
          SSH_FSM_CONDITION_WAIT(&condition);
        }
    }
}

static void
handle_quit(void)
{
  if (config)
    {
      deactivate_config(config);
      destroy_config(config);
      config = NULL;
    }
}

static void
handle_configure(void)
{
  Config *c = NULL;

  if (!(c = input_config(configfile)))
    {
      ma_errorf("configuration failed");
      goto end;
    }

  if (!config && !(config = create_config()))
    {
      ma_errorf("configuration creation failed");
      goto end;
    }

  update_config(config, c);
  ma_errorf("configuration complete");

 end:
  if (c)
    destroy_config(c);
}

static void
handle_print(void)
{
  MacsecCapabilities mc;

  if (!config)
    {
      ma_errorf("no configuration");
      return;
    }

  if (outputfile && !(outputf = fopen(outputfile, "w")))
    {
      ma_errorf("fopen");
      return;
    }

  foreach_provider(&config->eap, print_provider);
  foreach_ca(&config->eap, print_ca);
  foreach_crl(&config->eap, print_crl);
  foreach_rserver(&config->eap, print_rserver);
  print_eap_network(&config->eap.default_network);
  foreach_eap_network(&config->eap, print_eap_network);

  macsec_get_capabilities(&mc);
  print(0, "802.1AE capabilities");
  print_macsec_capabilities(1, &mc);
  foreach_interface(config, print_interface);

  if (outputf)
    {
      fclose(outputf);
      outputf = NULL;

      ma_errorf("printed to %s", outputfile);
    }
}

static void
handle_dtoggle(void)
{
  debug_suppress ^= 1;
  if (config)
    foreach_interface(config, set_debug_suppress);

  if (debug_suppress)
    ma_errorf("debug messages suppressed");
  else
    ma_errorf("debug messages allowed");
}

static void
handle_terminal(void)
{
  int chr;

  if ((chr = ma_read_terminal(terminal)) < 0)
    return;

  chr |= 0x20;

  if (chr == 'q')
    event_quit = 1;
  else if (chr == 'h' || chr == '?')
    event_help = 1;
  else if (chr == 'c')
    event_configure = 1;
  else if (chr == 'p')
    event_print = 1;
  else if (chr == 'd')
    event_dtoggle = 1;
}

static void
handle_hint(void)
{
  ma_errorf("press h or ? to display help");
}

static void
handle_help(void)
{
  ma_errorf("keyboard commands:");
  ma_errorf("  h  this message");
  ma_errorf("  q  quit");
  ma_errorf("  p  print configuration and status");
  ma_errorf("  c  reread configuration file");
  ma_errorf("  d  toggle suppressing of debug output");
}

static void
eap_wakeup(void *arg)
{
  Interface *i = arg;

  ssh_cancel_timeout(&i->eap_timeout);
  ssh_register_timeout(&i->eap_timeout, 0, 0, eap_wakeup2, i);
}

static void
eap_wakeup2(void *context)
{
  Interface *i = context;
  Dot1xEvents dot1x_events;

  memset(&dot1x_events, 0, sizeof dot1x_events);
  dot1x_events.eap_state = 1;
  dot1x_handle_events(&i->dot1x.intf, &dot1x_events);
}

static void
macsec_io(unsigned int events, void *context)
{
  Interface *i = context;
  Dot1xEvents dot1x_events;

  if (!(events & SSH_IO_READ))
    return;

  memset(&dot1x_events, 0, sizeof dot1x_events);
  dot1x_events.uncontrolled_port_receive = 1;

  dot1x_events.common_port_state = 1;
  dot1x_handle_events(&i->dot1x.intf, &dot1x_events);
}

static int interfaces_suspended_count;
static int interfaces_total_count;
static int suspension_time;

static void
check_suspend_interface(Interface *i)
{
    Dot1xPaeStat stat;

    if (i->dot1x.pae_ctrl.suspend_for > 0)
      {
        dot1x_get_pae_stat(&i->dot1x.intf, &stat);

        if (stat.suspended_while > 0)
          {
            interfaces_suspended_count++;
            if (suspension_time > stat.suspended_while)
              suspension_time = stat.suspended_while;
          }
      }

    interfaces_total_count++;
}

static void
tick(void *context)
{
  Interface *i = context;
  Dot1xEvents dot1x_events;

  ssh_register_timeout(&i->tick_timeout, 0, 500000, tick, i);

  memset(&dot1x_events, 0, sizeof dot1x_events);
  dot1x_events.time_tick = 1;
  dot1x_handle_events(&i->dot1x.intf, &dot1x_events);

  interfaces_suspended_count = 0;
  interfaces_total_count = 0;
  suspension_time = INT_MAX;

  foreach_interface(config, check_suspend_interface);

  if (interfaces_suspended_count == interfaces_total_count)
    {
      suspended = suspension_time;
      ma_suspend();
    }
}

static void
update_config(Config *old, const Config *new)
{
  if (new->quicksec_debug)
    ssh_debug_set_level_string(new->quicksec_debug);
  else
    ssh_debug_set_level_string(quicksec_debug);
  update_providers(&old->eap, &new->eap);
  update_cas(&old->eap, &new->eap);
  update_crls(&old->eap, &new->eap);
  update_rservers(&old->eap, &new->eap);
  update_eap_networks(&old->eap, &new->eap);
  update_interfaces(old, new);
}

static void
update_providers(EGlobal *old, const EGlobal *new)
{
  update_list(
    old, new,
    next_provider, compare_provider,
    remove_provider, insert_provider,
    activate_provider, deactivate_provider, reconfigure_provider,
    create_provider, destroy_provider);
}

static void
update_cas(EGlobal *old, const EGlobal *new)
{
  update_list(
    old, new,
    next_ca, compare_ca,
    remove_ca, insert_ca,
    activate_ca, deactivate_ca, reconfigure_ca,
    create_ca, destroy_ca);
}

static void
update_crls(EGlobal *old, const EGlobal *new)
{
  update_list(
    old, new,
    next_crl, compare_crl,
    remove_crl, insert_crl,
    activate_crl, deactivate_crl, reconfigure_crl,
    create_crl, destroy_crl);

  if (!next_crl(old, NULL))
    eap_set_crl_check_disable(old->keystore, 1);
  else
    eap_set_crl_check_disable(old->keystore, 0);
}

static void
update_rservers(EGlobal *old, const EGlobal *new)
{
  ERserver *r;

  update_list(
    old, new,
    next_rserver, compare_rserver,
    remove_rserver, insert_rserver,
    activate_rserver, deactivate_rserver, reconfigure_rserver,
    create_rserver, destroy_rserver);

  if (old->rservers_changed)
    {
      old->rservers_changed = 0;
      if (old->radius_servers)
        ssh_radius_client_server_info_destroy(old->radius_servers);
      if (!(old->radius_servers = ssh_radius_client_server_info_create()))
        {
          ma_errorf("cannot create RADIUS configuration");
          return;
        }
      r = NULL;
      while ((r = next_rserver(old, r)))
        {
          if (!ssh_radius_client_server_info_add_server(
                old->radius_servers, (unsigned char *)r->address,
                (unsigned char *)r->port, (unsigned char *)r->aport,
                r->secret_buf, r->secret_len))
            ma_errorf("cannot add RADIUS server %s", r->address);
        }
    }
}

static void
update_eap_networks(EGlobal *old, const EGlobal *new)
{
  reconfigure_eap_network(&old->default_network, &new->default_network);

  update_list(
    old, new,
    next_eap_network, compare_eap_network,
    remove_eap_network, insert_eap_network,
    activate_eap_network, deactivate_eap_network,
    reconfigure_eap_network,
    create_eap_network, destroy_eap_network);
}

static void
update_interfaces(Config *old, const Config *new)
{
  update_list(
    old, new,
    next_interface, compare_interface,
    remove_interface, insert_interface,
    activate_interface, deactivate_interface, reconfigure_interface,
    create_interface, destroy_interface);
}

static Config *
input_config(const char *file)
{
  volatile Reader rbuf = {.inbuf = NULL, .file = NULL};
  Reader *r = (void *)&rbuf;
  size_t size;
  Config *c;

  if (failed())
    {
      c = NULL;
      goto end;
    }

  memset(r, 0, sizeof *r);

  if (!(r->file = ssh_strdup(file)))
    failf("out of memory");

  if (!ssh_read_file_with_limit(file, UINT_MAX, &r->inbuf, &size))
    failf("file read failed: %s", file);
  r->inlen = size;

  r->charline = 1;
  read_byte(r);
  (void)read_bom(r);
  read_token(r);

  c = read_config(r);

  notfailed();

 end:
  if (r->inbuf)
    ssh_free(r->inbuf);
  if (r->file)
    ssh_free(r->file);
  return c;
}

static Config *
read_config(Reader *r)
{
  Config *c;
  unsigned char has_eap = 0, has_quicksec_debug = 0;

  if (!(c = create_config()))
    fail();

  if (failed())
    {
      destroy_config(c);
      fail();
    }

  if (!read_version(r))
    read_fail_token(r, "missing version information");

  while (!read_end(r))
    {
      if (!read_unique_eap(r, &has_eap, &c->eap) &&
          !read_interface(r, c) &&
          !read_unique_string(
            r, "quicksec-debug", &has_quicksec_debug, &c->quicksec_debug))
        read_fail_unexpected(r);
    }

  notfailed();
  return c;
}

static int
read_version(Reader *r)
{
  if (!read_keyword_and_value(r, "version"))
    return 0;

  if (strcmp(r->valuebuf, "2.0"))
    read_fail_value(
      r, "%s: unsupported configuration version", r->valuebuf);

  return 1;
}

static int
read_unique_eap(Reader *r, unsigned char *has, EGlobal *e)
{
  unsigned char has_default_network = 0;

  if (!read_unique_keyword(r, has, "eap"))
    return 0;

  read_required_punct(r, '{');

  while (!read_punct(r, '}'))
    if (!read_provider(r, e) &&
        !read_ca(r, e) &&
        !read_crl(r, e) &&
        !read_rserver(r, e) &&
        !read_default_eap_network(r, &has_default_network, e) &&
        !read_eap_network(r, e))
      read_fail_unexpected(r);

  return 1;
}

static int
read_provider(Reader *r, EGlobal *e)
{
  unsigned char has_init = 0;
  EProvider *p;
  unsigned line = r->tokenline;

  if (!read_keyword(r, "key-provider"))
    return 0;

  if (!(p = create_provider(e)))
    fail();

  if (failed())
    {
      destroy_provider(p);
      fail();
    }

  read_required_string(r, &p->type);
  read_required_punct(r, '{');

  while (!read_punct(r, '}'))
    {
      if (!read_unique_string(r, "init", &has_init, &p->init))
        read_fail_unexpected(r);
    }

  if (!p->type)
    read_fail(r, line, "missing key provider type");

  if (!insert_provider(e, p))
    read_fail(r, line, "duplicate provider");

  notfailed();
  return 1;
}

static int
read_ca(Reader *r, EGlobal *e)
{
  ECa *c;
  size_t size;
  unsigned line = r->tokenline;

  if (!read_keyword(r, "certificate-authority"))
    return 0;

  if (!(c = create_ca(e)))
    fail();

  if (failed())
    {
      destroy_ca(c);
      fail();
    }

  read_required_string(r, &c->file);
  read_required_punct(r, ';');

  if (!ssh_read_gen_file_with_limit(c->file, UINT_MAX, &c->derbuf, &size))
    read_fail_value(r, "file read failed: %s", c->file);
  c->derlen = (unsigned)size;

  if (!insert_ca(e, c))
    read_fail(r, line, "duplicate CA (by file contents)");

  notfailed();
  return 1;
}

static int
read_crl(Reader *r, EGlobal *e)
{
  ECrl *c;
  size_t size;
  unsigned line = r->tokenline;

  if (!read_keyword(r, "certificate-revocation-list"))
    return 0;

  if (!(c = create_crl(e)))
    fail();

  if (failed())
    {
      destroy_crl(c);
      fail();
    }

  read_required_string(r, &c->file);
  read_required_punct(r, ';');

  if (!ssh_read_gen_file_with_limit(c->file, UINT_MAX, &c->derbuf, &size))
    read_fail_value(r, "file read failed: %s", c->file);
  c->derlen = (unsigned)size;

  if (!insert_crl(e, c))
    read_fail(r, line, "duplicate CRL (by file contents)");

  notfailed();
  return 1;
}

static int
read_rserver(Reader *r, EGlobal *e)
{
  ERserver *s;
  unsigned line = r->tokenline;
  unsigned char has_secret = 0, has_port = 0, has_aport = 0;

  if (!read_keyword(r, "radius-server"))
    return 0;

  if (!(s = create_rserver(e)))
    fail();

  if (failed())
    {
      destroy_rserver(s);
      fail();
    }

  read_required_string(r, &s->address);

  read_required_punct(r, '{');

  while (!read_punct(r, '}'))
    {
      if (read_unique_hex(
            r, "secret", &has_secret,
            s->secret_buf, &s->secret_len, 1, sizeof s->secret_buf))
        ;
      else if (read_unique_string(
                 r, "port", &has_port, &s->port))
        ;
      else if (read_unique_string(
                 r, "accounting-port", &has_aport, &s->aport))
        ;
      else
        read_fail_unexpected(r);
    }

  if (!has_secret)
    read_fail(r, line, "missing RADIUS secret");

  if (!insert_rserver(e, s))
    read_fail(r, line, "duplicate RADIUS server");

  notfailed();
  return 1;
}

static int
read_default_eap_network(Reader *r, unsigned char *has, EGlobal *e)
{
  ENetwork *en;

  if (!read_unique_keyword(r, has, "default-network"))
    return 0;

  en = &e->default_network;
  en->eap = e;

  if (failed())
    {
      if (en->identity)
        ssh_free(en->identity);
      memset(en, 0, sizeof *en);
      fail();
    }

  read_eap_network_sub(r, en);

  notfailed();
  return 1;
}

static int
read_eap_network(Reader *r, EGlobal *e)
{
  ENetwork *en;
  unsigned line0;

  if (!read_keyword(r, "network"))
    return 0;

  if (!(en = create_eap_network(e)))
    fail();

  if (failed())
    {
      destroy_eap_network(en);
      fail();
    }

  line0 = r->tokenline;
  read_required_nid(r, &en->nid);

  read_eap_network_sub(r, en);

  if (!insert_eap_network(e, en))
    read_fail(r, line0, "duplicate EAP network");

  notfailed();
  return 1;
}

static void
read_eap_network_sub(Reader *r, ENetwork *en)
{
  unsigned char has_identity = 0;

  if (read_punct(r, ';'))
    return;

  read_required_punct(r, '{');

  while (!read_punct(r, '}'))
    {
      if (!read_unique_string(r, "identity", &has_identity, &en->identity))
        read_fail_unexpected(r);
    }
}

static int
read_interface(Reader *r, Config *c)
{
  Interface *i = NULL;
  MacsecSecyCtrl *sc;
  SecyHas *sh;
  Dot1xPaeCtrl *pc;
  PaeHas *ph;
  uint64_t u64;
  unsigned line0;
  int t;
  unsigned char has_default_network = 0;
  unsigned char has_debug = 0;

  if (!read_keyword(r, "interface"))
    return 0;

  if (!(i = create_interface(c)))
    fail();

  if (failed())
    {
      destroy_interface(i);
      fail();
    }

  sc = &i->macsec.secy_ctrl;
  sh = &i->macsec.secy_has;
  pc = &i->dot1x.pae_ctrl;
  ph = &i->dot1x.pae_has;

  line0 = r->tokenline;
  read_required_string(r, &i->name);

  if (read_punct(r, ';'))
    goto end;

  if (!read_punct(r, '{'))
    read_fail_expected(r, "';' or '{'");

  while (!read_punct(r, '}'))
    {
      if (read_unique_choice(
                 r, "eapol-group", &ph->eapol_group, &t, eapol_group))
        pc->eapol_group = t;
      else if (read_unique_bool(
                 r, "virtual-ports-enable", &ph->virtual_ports_enable, &t))
        pc->virtual_ports_enable = t;
      else if (read_unique_bool(
                 r, "announcer-enable", &ph->announcer_enable, &t))
        pc->announcer_enable = t;
      else if (read_unique_bool(
                 r, "listener-enable", &ph->listener_enable, &t))
        pc->listener_enable = t;
      else if (read_unique_bool(
                 r, "mka-enable", &ph->mka_enable, &t))
        pc->mka_enable = t;
      else if (read_unique_bool(
                 r, "bounded-delay", &ph->bounded_delay, &t))
        pc->bounded_delay = t;
      else if (read_unique_unsigned(
                 r, "actor-priority", &ph->actor_priority,
                 &pc->actor_priority, 0, 255))
        ;
      else if (read_unique_choice(
                 r, "cipher-suite", &ph->cipher_suite, &t, cipher_suite))
        pc->cipher_suite = t;
      else if (read_unique_choice(
                 r, "confidentiality-offset", &ph->confidentiality_offset,
                 &t, confidentiality_offset))
        pc->confidentiality_offset = t;
      else if (read_unique_bool(
                 r, "logon", &ph->logon, &t))
        pc->logon = t;
      else if (read_unique_nid(
                 r, "selected-network", &ph->selected_nid, &pc->selected_nid))
        ;
      else if (read_unique_bool(
                 r, "cak-caching-enable", &ph->cak_caching_enable, &t))
        pc->cak_caching_enable = t;
      else if (read_unique_bool(
                 r, "group-ca-enable", &ph->group_ca_enable, &t))
        pc->group_ca_enable = t;
      else if (read_unique_bool(
                 r, "long-eap-caks", &ph->long_eap_caks, &t))
        pc->long_eap_caks = t;
      else if (read_unique_bool(
                 r, "supplicant-enable", &ph->supplicant_enable, &t))
        pc->supplicant_enable = t;
      else if (read_unique_bool(
                 r, "authenticator-enable", &ph->authenticator_enable, &t))
        pc->authenticator_enable = t;
      else if (read_unique_unsigned(
                 r, "held-period", &ph->held_period, &pc->held_period, 0, 600))
        ;
      else if (read_unique_unsigned(
                 r, "quiet-period", &ph->quiet_period,
                 &pc->quiet_period, 0, 600))
        ;
      else if (read_unique_bool(
                 r, "reauth-enabled", &ph->reauth_enabled, &t))
        pc->reauth_enabled = t;
      else if (read_unique_unsigned(
                 r, "reauth-period", &ph->reauth_period,
                 &pc->reauth_period, 10, 86400))
        ;
      else if (read_unique_unsigned(
                 r, "retry-max", &ph->retry_max, &pc->retry_max, 1, 10))
        ;
      else if (read_unique_bool(
                 r, "logoff-enable", &ph->logoff_enable, &t))
        pc->logoff_enable = t;
      else if (read_unique_bool(
                 r, "macsec-protect", &ph->macsec_protect, &t))
        pc->macsec_protect = t;
      else if (read_unique_choice(
                 r, "macsec-validate", &ph->macsec_validate,
                 &t, validate_frames))
        pc->macsec_validate = t;
      else if (read_unique_bool(
                 r, "macsec-replay-protect", &ph->macsec_replay_protect, &t))
        pc->macsec_replay_protect = t;
      else if (read_unique_unsigned(
                 r, "transmit-delay", &ph->transmit_delay,
                 &pc->transmit_delay, 1, 10))
        ;
      else if (read_unique_unsigned(
                 r, "retire-delay", &ph->retire_delay,
                 &pc->retire_delay, 1, 10))
        ;
      else if (read_unique_uint64(
                 r, "replay-window", &sh->replay_window, &u64,
                 0, (uint64_t)(uint32_t)-1))
        sc->replay_window = (uint32_t)u64;
      else if (read_unique_bool(
                 r, "always-include-sci", &sh->always_include_sci, &t))
        sc->always_include_sci = t;
      else if (read_unique_bool(r, "use-es", &sh->use_es, &t))
        sc->use_es = t;
      else if (read_unique_bool(r, "use-scb", &sh->use_scb, &t))
        sc->use_scb = t;
      else if (read_unique_choice(
                 r, "admin-point-to-point-mac", &sh->admin_point_to_point_mac,
                 &t, admin_point_to_point_mac))
        sc->admin_point_to_point_mac = t;
      else if (read_unique_debug(r, &has_debug, &i->debug))
        ;
      else if (read_default_network(r, &has_default_network, i))
        ;
      else if (read_network(r, i))
        ;
      else if (read_psk(r, i))
        ;
      else if (read_unique_unsigned(
                       r, "suspend-for", &ph->suspend_for,
                       &pc->suspend_for, 0, 120))
          ;
      else if (read_unique_bool(
                       r, "suspend-on-request", &ph->suspend_on_request, &t))
          pc->suspend_on_request = t;
      else if (read_unique_unsigned(
                       r, "suspended-while", &ph->suspended_while,
                       &pc->suspended_while, 0, 120))
          ;
      else
        read_fail_unexpected(r);
    }

  if (!has_debug)
    memcpy(&i->debug, &debug, sizeof i->debug);

 end:
  if (!insert_interface(c, i))
    read_fail(r, line0, "duplicate interface");

  notfailed();
  return 1;
}

static int
read_default_network(Reader *r, unsigned char *has, Interface *i)
{
  Network *n;

  if (!read_unique_keyword(r, has, "default-network"))
    return 0;

  n = &i->dot1x.default_network;

  if (failed())
    {
      memset(n, 0, sizeof *n);
      fail();
    }

  read_network_sub(r, n);

  notfailed();
  return 1;
}

static int
read_network(Reader *r, Interface *i)
{
  Network *n;
  unsigned line0;

  if (!read_keyword(r, "network"))
    return 0;

  if (!(n = create_network(i)))
    fail();

  if (failed())
    {
      destroy_network(n);
      fail();
    }

  line0 = r->tokenline;
  read_required_nid(r, &n->nid);

  read_network_sub(r, n);

  if (!insert_network(i, n))
    read_fail(r, line0, "duplicate network");

  notfailed();
  return 1;
}

static void
read_network_sub(Reader *r, Network *n)
{
  int t;

  if (read_punct(r, ';'))
    return;

  read_required_punct(r, '{');

  while (!read_punct(r, '}'))
    {
      if (read_unique_choice(r, "use-eap", &n->has.use_eap, &t, use_eap))
        n->ctrl.use_eap = t;
      else if (read_unique_choice(
                 r, "unauth-allowed", &n->has.unauth_allowed,
                 &t, unauth_allowed))
        n->ctrl.unauth_allowed = t;
      else if (read_unique_choice(
                 r, "unsecured-allowed", &n->has.unsecured_allowed,
                 &t, unsecured_allowed))
        n->ctrl.unsecured_allowed = t;
      else if (read_unique_kmd(r, "kmd", &n->has.kmd, &n->ctrl.kmd))
        ;
      else
        read_fail_unexpected(r);
    }
}

static int
read_psk(Reader *r, Interface *i)
{
  Psk *p;
  unsigned line0;
  unsigned char has_cak = 0, has_type = 0;
  unsigned char has_network = 0, has_auth_data = 0;
  int t;

  if (!read_keyword(r, "preshared-key"))
    return 0;

  if (!(p = create_psk(i)))
    fail();

  if (failed())
    {
      destroy_psk(p);
      fail();
    }

  line0 = r->tokenline;
  read_required_ckn(r, &p->ckn);

  read_required_punct(r, '{');

  while (!read_punct(r, '}'))
    {
      if (read_unique_key(r, "value", &has_cak, &p->cak))
        ;
      else if (read_unique_choice(r, "type", &has_type, &t, psk_type))
        p->type = t;
      else if (read_unique_nid(r, "network", &has_network, &p->nid))
        ;
      else if (read_unique_ad(
                 r, "authorization-data", &has_auth_data, &p->auth_data))
        ;
      else
        read_fail_unexpected(r);
    }
  if (!has_cak)
    read_fail(r, line0, "missing CAK value");
  if (!has_type)
    read_fail(r, line0, "missing CAK type");

  if (!insert_psk(i, p))
    read_fail(r, line0, "duplicate PSK");

  notfailed();
  return 1;
}

static int
read_unique_debug(Reader *r, unsigned char *has, Debug *d)
{
  const StringInt *si;
  int i, defaults = 1, everything = 0;

  if (!read_unique_keyword(r, has, "debug"))
    return 0;

  /* enable any messages specified on command line */
  memcpy(&d->dot1x, &debug.dot1x, sizeof d->dot1x);

  while (!read_punct(r, ';'))
    {
      if (!read_value(r))
        read_fail_unexpected(r);
      if ((i = strtoint(dot1x_message, r->valuebuf)) >= 0)
        set_bit(d->dot1x, i);
      else if (!strcmp(r->valuebuf, "no-defaults"))
        defaults = 0;
      else if (!strcmp(r->valuebuf, "everything"))
        everything = 1;
      else
        read_fail_value(r, "%s: invalid debug flag", r->valuebuf);
    }

  if (defaults)
    {
      for (i = 0; i < sizeof default_debug.dot1x; i++)
        d->dot1x[i] |= default_debug.dot1x[i];
    }

  if (everything)
    {
      for (si = dot1x_message; si->s; si++)
        {
          if (si->i == DOT1X_MESSAGE_PACKET_DETAIL)
            continue;
          set_bit(d->dot1x, si->i);
        }
    }

  return 1;
}

static int
read_unique_choice(Reader *r, const char *keyword, unsigned char *has, int *n,
  const StringInt *map)
{
  int i;

  if (!read_unique_keyword_and_value(r, keyword, has))
    return 0;

  if ((i = strtoint(map, r->valuebuf)) < 0)
    read_fail_value(r, "%s: invalid value for %s", r->valuebuf, keyword);

  *n = i;
  return 1;
}

static int
read_unique_bool(Reader *r, const char *keyword, unsigned char *has, int *n)
{
  int i;

  if (!read_unique_keyword_and_value(r, keyword, has))
    return 0;

  if ((i = strtoint(boolean, r->valuebuf)) < 0)
    read_fail_value(r, "%s: invalid value for %s", r->valuebuf, keyword);

  *n = i;
  return 1;
}

static int
read_unique_key(
  Reader *r, const char *keyword, unsigned char *has, MacsecKey *key)
{
  if (!read_unique_hex(
        r, keyword, has, key->buf, &key->len, 0, sizeof key->buf))
    return 0;

  if (key->len != 16 && key->len != 32)
    read_fail_value(r, "invalid key length");

  return 1;
}

static int
read_unique_unsigned(
  Reader *r, const char *keyword, unsigned char *has, unsigned *u,
  unsigned min, unsigned max)
{
  char *end;
  unsigned long ul;

  if (!read_unique_keyword_and_value(r, keyword, has))
    return 0;

  ul = strtoul(r->valuebuf, &end, 10);

  if (r->valuebuf[0] == '\0' || *end || ul < min || ul > max)
    {
      errno = 0;
      read_fail_value(r, "%s: invalid value for %s", r->valuebuf, keyword);
    }

  *u = (unsigned)ul;
  return 1;
}

static int
read_unique_uint64(
  Reader *r, const char *keyword, unsigned char *has, uint64_t *u64,
  uint64_t min, uint64_t max)
{
  char *end;
  unsigned long long ull;

  if (!read_unique_keyword_and_value(r, keyword, has))
    return 0;

  ull = strtoull(r->valuebuf, &end, 10);

  if (r->valuebuf[0] == '\0' || *end || ull < min || ull > max)
    {
      errno = 0;
      read_fail_value(r, "%s: invalid value for %s", r->valuebuf, keyword);
    }

  *u64 = (uint64_t)ull;
  return 1;
}

static int
read_unique_nid(
  Reader *r, const char *keyword, unsigned char *has, Dot1xNid *nid)
{
  return read_unique_utf8(
    r, keyword, has, nid->buf, &nid->len, sizeof nid->buf);
}

static int
read_unique_kmd(
  Reader *r, const char *keyword, unsigned char *has, Dot1xKmd *kmd)
{
  return read_unique_utf8(
    r, keyword, has, kmd->buf, &kmd->len, sizeof kmd->buf);
}

static int
read_unique_ad(
  Reader *r, const char *keyword, unsigned char *has, MacsecAuthData *ad)
{
  return read_unique_hex(
    r, keyword, has, ad->buf, &ad->len, 0, sizeof ad->buf);
}

static int
read_unique_hex(
  Reader *r, const char *keyword, unsigned char *has,
  unsigned char *buf, unsigned *len, unsigned minlen, unsigned maxlen)
{
  if (!read_unique_keyword(r, has, keyword))
    return 0;

  read_required_hex(r, buf, len, minlen, maxlen);
  read_required_punct(r, ';');
  return 1;
}

static int
read_unique_utf8(
  Reader *r, const char *keyword, unsigned char *has,
  unsigned char *buf, unsigned *len, unsigned max)
{
  if (!read_unique_keyword(r, has, keyword))
    return 0;

  read_required_utf8(r, buf, len, max);
  read_required_punct(r, ';');
  return 1;
}

static int
read_unique_string(
  Reader *r, const char *keyword, unsigned char *has, char **s)
{
  if (!read_unique_keyword(r, has, keyword))
    return 0;

  read_required_string(r, s);
  read_required_punct(r, ';');
  return 1;
}

static int
read_unique_keyword_and_value(
  Reader *r, const char *keyword, unsigned char *has)
{
  if (!read_unique_keyword(r, has, keyword))
    return 0;

  read_required_value(r);
  read_required_punct(r, ';');
  return 1;
}

static int
read_keyword_and_value(Reader *r, const char *keyword)
{
  if (!read_keyword(r, keyword))
    return 0;

  read_required_value(r);
  read_required_punct(r, ';');
  return 1;
}

static int
read_unique_keyword(Reader *r, unsigned char *has, const char *keyword)
{
  unsigned line = r->tokenline;

  if (!read_keyword(r, keyword))
    return 0;

  if (*has)
    read_fail(r, line, "duplicate %s", keyword);
  *has = 1;

  return 1;
}

static void
read_required_ckn(Reader *r, Dot1xCkn *ckn)
{
  read_required_hex(r, ckn->buf, &ckn->len, 1, sizeof ckn->buf);
}

static void
read_required_nid(Reader *r, Dot1xNid *nid)
{
  read_required_utf8(r, nid->buf, &nid->len, sizeof nid->buf);
}

static void
read_required_hex(
  Reader *r, unsigned char *buf, unsigned *len,
  unsigned minlen, unsigned maxlen)
{
  const char *p;
  unsigned pos = 0;
  int c1, c2, n1, n2;

  read_required_value(r);

  p = r->valuebuf;
  while (1)
    {
      if ((c1 = get_char(&p)) < 0)
        read_fail_value(r, "invalid UTF-8 character");
      if (!c1)
        break;

      if ((c2 = get_char(&p)) < 0)
        read_fail_value(r, "invalid UTF-8 character");
      if (!c2)
        read_fail_value(r, "premature end of hex string");

      if ((n1 = charhex(c1)) < 0 || (n2 = charhex(c2)) < 0)
        read_fail_value(r, "invalid character in hex string");

      if (pos >= maxlen)
        read_fail_value(r, "hex string too long");

      buf[pos++] = ((unsigned char)n1 << 4) | (unsigned char)n2;
    }

  if (pos < minlen)
    read_fail_value(r, "hex string too short");

  *len = pos;
}

static void
read_required_utf8(
  Reader *r, unsigned char *buf, unsigned *len, unsigned max)
{
  int n;

  read_required_value(r);

  if ((n = strlen(r->valuebuf)) > max)
    read_fail_value(r, "UTF-8 string too large");

  bufcpy(buf, len, (void *)r->valuebuf, n);
}

static void
read_required_string(Reader *r, char **s)
{
  read_required_value(r);
  if (!(*s = ssh_strdup(r->valuebuf)))
    read_fail_value(r, "out of memory");
}

static void
read_required_value(Reader *r)
{
  if (!read_value(r))
    read_fail_expected(r, "value");
}

static void
read_required_punct(Reader *r, int punct)
{
  if (!read_punct(r, punct))
    read_fail_expected(r, "'%c'", punct);
}

static int
read_keyword(Reader *r, const char *keyword)
{
  if (r->tokentype != TOKEN_UNQUOTED)
    return 0;

  if (strcmp(r->tokenbuf, keyword))
    return 0;

  read_token(r);
  return 1;
}

static int
read_value(Reader *r)
{
  int n;

  if (r->tokentype != TOKEN_QUOTED &&
      r->tokentype != TOKEN_UNQUOTED)
    return 0;

  n = (int)strlen(r->tokenbuf);

  if (n >= sizeof r->valuebuf)
    read_fail_token(r, "value too large");

  r->valueline = r->tokenline;
  strcpy(r->valuebuf, r->tokenbuf);
  read_token(r);
  return 1;
}

static int
read_punct(Reader *r, int punct)
{
  if (r->tokentype != TOKEN_PUNCT)
    return 0;

  if ((int)(unsigned char)r->tokenbuf[0] != punct)
    return 0;

  read_token(r);
  return 1;
}

static int
read_end(Reader *r)
{
  if (r->tokentype != TOKEN_END)
    return 0;
  return 1;
}

static void
read_token(Reader *r)
{
  while (read_white(r) || read_comment(r))
    ;
  r->tokenline = r->charline;
  if (!read_token_unquoted(r) &&
      !read_token_quoted(r) &&
      !read_token_punct(r) &&
      !read_token_end(r))
    read_fail_char(r, "bad character %d", r->charval);
}

static int
read_token_unquoted(Reader *r)
{
  if (r->charval == EOF ||
      (charflags(r->charval) & (CHAR_WHITE | CHAR_PUNCT)))
     return 0;

  clear_token(r);

  while (1)
    {
      grow_token(r);
      read_char(r);
      if (r->charval == EOF ||
          (charflags(r->charval) & (CHAR_WHITE | CHAR_PUNCT)))
        break;
    }

  r->tokentype = TOKEN_UNQUOTED;
  return 1;
}

static int
read_token_quoted(Reader *r)
{
  int escape = 0;

  if (r->charval != '"')
    return 0;

  read_char(r);

  clear_token(r);

  while (1)
    {
      if (r->charval == EOF)
        read_fail_token(r, "unterminated quoted value");

      if (!escape && r->charval == '"')
        {
          read_char(r);
          break;
        }

      if (!escape && r->charval == '\\')
        {
          escape = 1;
          read_char(r);
          continue;
        }

      if (escape)
        escape = 0;

      grow_token(r);
      read_char(r);
    }
  r->tokentype = TOKEN_QUOTED;
  return 1;
}

static int
read_token_punct(Reader *r)
{
  if (r->charval != ';' && r->charval != '{' && r->charval != '}')
    return 0;

  clear_token(r);
  grow_token(r);
  read_char(r);

  r->tokentype = TOKEN_PUNCT;
  return 1;
}

static int
read_token_end(Reader *r)
{
  if (r->charval != EOF)
    return 0;

  clear_token(r);
  r->tokentype = TOKEN_END;
  return 1;
}

static int
read_comment(Reader *r)
{
  if (r->charval != '#')
    return 0;

  do
    read_char(r);
  while (r->charval != '\n' && r->charval != EOF);

  return 1;

}

static int
read_white(Reader *r)
{
  if (!(charflags(r->charval) & CHAR_WHITE))
    return 0;

  do
    read_char(r);
  while ((charflags(r->charval) & CHAR_WHITE));

  return 1;
}

static void
read_char(Reader *r)
{
  int i, n;

  if (r->charval == EOF)
    read_fail_char(r, "premature end of file");

  if (r->byte == EOF)
    {
      r->charval = EOF;
      return;
    }

  r->charlen = 0;

  if ((r->byte & 0x80) == 0x00)
    {
      r->charval = r->byte & 0x7f;
      n = 0;
    }
  else if ((r->byte & 0xe0) == 0xc0)
    {
      r->charval = r->byte & 0x1f;
      n = 1;
    }
  else if ((r->byte & 0xf0) == 0xe0)
    {
      r->charval = r->byte & 0x0f;
      n = 2;
    }
  else if ((r->byte & 0xf8) == 0xf0)
    {
      r->charval = r->byte & 0x07;
      n = 3;
    }
  else
    {
      read_fail_char(r, "bad UTF-8 byte");
    }

  r->charbuf[r->charlen++] = r->byte;
  read_byte(r);

  for (i = 0; i < n; i++)
    {
      if (r->byte == EOF)
        read_fail_char(r, "premature end of file");
      if ((r->byte & 0xc0) != 0x80)
        read_fail_char(r, "bad UTF-8 byte");
      r->charval <<= 6;
      r->charval |= r->byte & 0x3f;
      r->charbuf[r->charlen++] = r->byte;
      read_byte(r);
    }

  if ((charflags(r->charval) & (CHAR_CONTROL | CHAR_WHITE)) ==
      CHAR_CONTROL)
    read_fail_char(r, "illegal character %d", r->charval);

  if (r->charval == '\n')
    r->charline++;
}

static int
read_bom(Reader *r)
{
  if (r->byte != 0xef)
    return 0;

  read_byte(r);
  if (r->byte != 0xbb)
    read_fail_char(r, "incomplete UTF-8 byte order mark");
  read_byte(r);
  if (r->byte != 0xbf)
    read_fail_char(r, "incomplete UTF-8 byte order mark");
  read_byte(r);
  return 1;
}

static void
read_byte(Reader *r)
{
  if (r->byte == EOF)
    read_fail_char(r, "premature end of file");

  if (r->inpos >= r->inlen)
    {
      r->byte = EOF;
      return;
    }

  r->byte = r->inbuf[r->inpos++];
}

static void
clear_token(Reader *r)
{
  /* fill with zeros to ensure null-termination */
  memset(r->tokenbuf, 0, sizeof r->tokenbuf);
  r->tokenpos = 0;
}

static void
grow_token(Reader *r)
{
  /* do not overwrite last null in the buffer */
  if (r->tokenpos + r->charlen >= sizeof r->tokenbuf)
    read_fail_char(r, "token too large");

  memcpy(r->tokenbuf + r->tokenpos, r->charbuf, r->charlen);
  r->tokenpos += r->charlen;
}

static void
read_fail_expected(Reader *r, const char *fmt, ...)
{
  va_list ap;
  char buf[64];

  va_start(ap, fmt);
  vsnprintf(buf, sizeof buf, fmt, ap);
  buf[sizeof buf - 1] = '\0';
  va_end(ap);

  switch (r->tokentype)
    {
    default:
    case TOKEN_NONE:
    case TOKEN_UNQUOTED:
    case TOKEN_QUOTED:
    case TOKEN_PUNCT:
      read_fail_token(r, "expected %s, got '%s'", buf, r->tokenbuf);
    case TOKEN_END:
      read_fail_token(r, "expected %s, got end of file", buf);
    }
}

static void
read_fail_unexpected(Reader *r)
{
  switch (r->tokentype)
    {
    default:
    case TOKEN_NONE:
    case TOKEN_UNQUOTED:
    case TOKEN_QUOTED:
      read_fail_token(r, "unexpected keyword or value '%s'", r->tokenbuf);
    case TOKEN_PUNCT:
      read_fail_token(r, "unexpected punctuation '%s'", r->tokenbuf);
    case TOKEN_END:
      read_fail_token(r, "unexpected end of file");
    }
}

static void
read_fail(Reader *r, unsigned line, const char *fmt, ...)
{
  va_list ap;

  ma_errorf_begin("%s:%d: ", r->file, line);
  va_start(ap, fmt);
  ma_verrorf_end(fmt, ap);
  va_end(ap);
  fail();
}

static int
charhex(int c)
{
  if (c >= 0 && c < 0x100)
    return charhexv[c];
  else
    return -1;
}

static int
charflags(int c)
{
  if (c >= 0 && c < 0x100)
    return charflagv[c];
  else
    return 0;
}

static Config *
create_config(void)
{
  Config *c = NULL;

  if (!(c = ssh_calloc(1, sizeof *c)))
    {
      ma_errorf("out of memory");
      goto fail;

    }

  macsec_list_init(&c->interfaces);
  macsec_list_init(&c->eap.providers);
  macsec_list_init(&c->eap.cas);
  macsec_list_init(&c->eap.crls);
  macsec_list_init(&c->eap.rservers);
  c->eap.default_network.eap = &c->eap;
  macsec_list_init(&c->eap.networks);

  if (!(c->eap.keystore = eap_create_keystore()))
    {
      ma_errorf("cannot create EAP keystore");
      goto fail;
    }
  return c;

 fail:
  destroy_config(c);
  return NULL;
}

static void
destroy_config(Config *c)
{
  if (!c)
    return;

  foreach_interface(c, destroy_interface);
  foreach_eap_network(&c->eap, destroy_eap_network);
  if (c->eap.default_network.identity)
    ssh_free(c->eap.default_network.identity);
  foreach_rserver(&c->eap, destroy_rserver);
  foreach_crl(&c->eap, destroy_crl);
  foreach_ca(&c->eap, destroy_ca);
  foreach_provider(&c->eap, destroy_provider);
  if (c->eap.radius_servers)
    ssh_radius_client_server_info_destroy(c->eap.radius_servers);
  if (c->eap.keystore)
    eap_destroy_keystore(c->eap.keystore);
  if (c->quicksec_debug)
    ssh_free(c->quicksec_debug);
  ssh_free(c);
}

static void
deactivate_config(Config *c)
{
  foreach_interface(c, deactivate_interface);
  foreach_eap_network(&c->eap, deactivate_eap_network);
  deactivate_eap_network(&c->eap.default_network);
  foreach_rserver(&c->eap, deactivate_rserver);
  foreach_crl(&c->eap, deactivate_crl);
  foreach_ca(&c->eap, deactivate_ca);
  foreach_provider(&c->eap, deactivate_provider);
}


static int
insert_provider(EGlobal *e, EProvider *p)
{
  if (!insert_unique(
        &e->providers, offsetof(EProvider, link), compare_provider, p))
    return 0;

  p->eap = e;
  return 1;
}

static void
remove_provider(EGlobal *e, EProvider *p)
{
  (void)e;
  p->eap = NULL;
  macsec_list_remove(&p->link);
}

static void
foreach_provider(EGlobal *e, void (*func)(EProvider *p))
{
  macsec_list_foreach(&e->providers, offsetof(EProvider, link), func);
}

static EProvider *
next_provider(const EGlobal *e, const EProvider *prev)
{
  return macsec_list_next(&e->providers, offsetof(EProvider, link), prev);
}

static int
compare_provider(const EProvider *p1, const EProvider *p2)
{
  return strcmp(p1->type, p2->type);
}

static int
activate_provider(EProvider *p, const EProvider *p0)
{
  if (!(p->type = ssh_strdup(p0->type)) ||
      (p0->init && !(p->init = ssh_strdup(p0->init))))
    {
      ma_errorf("out of memory");
      return 0;
    }

  if (!(p->provider = eap_add_provider(p->eap->keystore, p->type, p->init)))
    {
      ma_errorf("cannot add EAP key provider");
      return 0;
    }

  return 1;
}

static void
deactivate_provider(EProvider *p)
{
  if (p->provider)
    {
      eap_remove_provider(p->eap->keystore, p->provider);
      p->provider = NULL;
    }
}

static int
reconfigure_provider(EProvider *old, const EProvider *new)
{
  if ((!old->init && new->init) ||
      (old->init && !new->init) ||
      (old->init && new->init && strcmp(old->init, new->init)))
    {
      if (old->provider)
        {
          eap_remove_provider(old->eap->keystore, old->provider);
          old->provider = NULL;
        }
      if (old->init)
        {
          ssh_free(old->init);
          old->init = NULL;
        }

      if (new->init && !(old->init = ssh_strdup(new->init)))
        {
          ma_errorf("out of memory");
          return 0;
        }

      if (!(old->provider = eap_add_provider(
              old->eap->keystore, old->type, old->init)))
        {
          ma_errorf("cannot add EAP key provider");
          return 0;
        }
    }

  return 1;
}

static EProvider *
create_provider(EGlobal *eap)
{
  EProvider *p;

  if (!(p = ssh_calloc(1, sizeof *p)))
    {
      ma_errorf("out of memory");
      return NULL;
    }

  p->eap = eap;
  return p;
}

static void
destroy_provider(EProvider *p)
{
  if (p->init)
    ssh_free(p->init);
  if (p->type)
    ssh_free(p->type);
  ssh_free(p);
}

static int
insert_ca(EGlobal *e, ECa *c)
{
  if (!insert_unique(&e->cas, offsetof(ECa, link), compare_ca, c))
    return 0;

  c->eap = e;
  return 1;
}

static void
remove_ca(EGlobal *e, ECa *c)
{
  (void)e;
  c->eap = NULL;
  macsec_list_remove(&c->link);
}

static void
foreach_ca(EGlobal *e, void (*func)(ECa *c))
{
  macsec_list_foreach(&e->cas, offsetof(ECa, link), func);
}

static ECa *
next_ca(const EGlobal *e, const ECa *prev)
{
  return macsec_list_next(&e->cas, offsetof(ECa, link), prev);
}

static int
compare_ca(const ECa *c1, const ECa *c2)
{
  return bufcmp(c1->derbuf, c1->derlen, c2->derbuf, c2->derlen);
}

static int
activate_ca(ECa *c, const ECa *c0)
{
  if (!(c->derbuf = ssh_malloc(c0->derlen)))
    {
      ma_errorf("out of memory");
      return 0;
    }

  bufcpy(c->derbuf, &c->derlen, c0->derbuf, c0->derlen);

  if (!(c->certificate = eap_add_ca(c->eap->keystore, c->derbuf, c->derlen)))
    {
      ma_errorf("cannot add EAP CA");
      return 0;
    }

  return 1;
}

static void
deactivate_ca(ECa *c)
{
  if (c->certificate)
    {
      eap_remove_ca(c->eap->keystore, c->certificate);
      c->certificate = NULL;
    }
}

static int
reconfigure_ca(ECa *old, const ECa *new)
{
  if (!old->file || strcmp(old->file, new->file))
    {
      if (old->file)
        {
          ssh_free(old->file);
          old->file = NULL;
        }
      if (!(old->file = ssh_strdup(new->file)))
        {
          ma_errorf("out of memory");
          return 0;
        }
    }
  return 1;
}

static ECa *
create_ca(EGlobal *eap)
{
  ECa *c;

  if (!(c = ssh_calloc(1, sizeof *c)))
    {
      ma_errorf("out of memory");
      return NULL;
    }

  c->eap = eap;
  return c;
}

static void
destroy_ca(ECa *c)
{
  if (c->derbuf)
    ssh_free(c->derbuf);
  if (c->file)
    ssh_free(c->file);
  ssh_free(c);
}

static int
insert_crl(EGlobal *e, ECrl *c)
{
  if (!insert_unique(&e->crls, offsetof(ECrl, link), compare_crl, c))
    return 0;

  c->eap = e;
  return 1;
}

static void
remove_crl(EGlobal *e, ECrl *c)
{
  (void)e;
  c->eap = NULL;
  macsec_list_remove(&c->link);
}

static void
foreach_crl(EGlobal *e, void (*func)(ECrl *c))
{
  macsec_list_foreach(&e->crls, offsetof(ECrl, link), func);
}

static ECrl *
next_crl(const EGlobal *e, const ECrl *prev)
{
  return macsec_list_next(&e->crls, offsetof(ECrl, link), prev);
}

static int
compare_crl(const ECrl *c1, const ECrl *c2)
{
  return bufcmp(c1->derbuf, c1->derlen, c2->derbuf, c2->derlen);
}

static int
activate_crl(ECrl *c, const ECrl *c0)
{
  if (!(c->derbuf = ssh_malloc(c0->derlen)))
    {
      ma_errorf("out of memory");
      return 0;
    }

  bufcpy(c->derbuf, &c->derlen, c0->derbuf, c0->derlen);

  if (!(c->crl = eap_add_crl(c->eap->keystore, c->derbuf, c->derlen)))
    {
      ma_errorf("cannot add EAP CRL");
      return 0;
    }

  return 1;
}

static void
deactivate_crl(ECrl *c)
{
  if (c->crl)
    {
      eap_remove_crl(c->eap->keystore, c->crl);
      c->crl = NULL;
    }
}

static int
reconfigure_crl(ECrl *old, const ECrl *new)
{
  if (!old->file || strcmp(old->file, new->file))
    {
      if (old->file)
        {
          ssh_free(old->file);
          old->file = NULL;
        }
      if (!(old->file = ssh_strdup(new->file)))
        {
          ma_errorf("out of memory");
          return 0;
        }
    }
  return 1;
}

static ECrl *
create_crl(EGlobal *eap)
{
  ECrl *c;

  if (!(c = ssh_calloc(1, sizeof *c)))
    {
      ma_errorf("out of memory");
      return NULL;
    }

  c->eap = eap;
  return c;
}

static void
destroy_crl(ECrl *c)
{
  if (c->derbuf)
    ssh_free(c->derbuf);
  if (c->file)
    ssh_free(c->file);
  ssh_free(c);
}

static int
insert_rserver(EGlobal *e, ERserver *r)
{
  if (!insert_unique(
        &e->rservers, offsetof(ERserver, link), compare_rserver, r))
    return 0;
  return 1;
}

static void
remove_rserver(EGlobal *e, ERserver *r)
{
  (void)e;
  macsec_list_remove(&r->link);
}

static void
foreach_rserver(EGlobal *e, void (*func)(ERserver *r))
{
  macsec_list_foreach(&e->rservers, offsetof(ERserver, link), func);
}

static ERserver *
next_rserver(const EGlobal *e, const ERserver *prev)
{
  return macsec_list_next(&e->rservers, offsetof(ERserver, link), prev);
}

static int
compare_rserver(const ERserver *r1, const ERserver *r2)
{
  return strcmp(r1->address, r2->address);
}

static int
activate_rserver(ERserver *r, const ERserver *r0)
{
  if (!(r->address = ssh_strdup(r0->address)) ||
      (r0->port && !(r->port = ssh_strdup(r0->port))) ||
      (r0->aport && !(r->aport = ssh_strdup(r0->aport))))
    {
      ma_errorf("out of memory");
      return 0;
    }

  bufcpy(r->secret_buf, &r->secret_len, r0->secret_buf, r0->secret_len);

  r->eap->rservers_changed = 1;
  return 1;
}

static void
deactivate_rserver(ERserver *r)
{
  r->eap->rservers_changed = 1;
}

static int
reconfigure_rserver(ERserver *old, const ERserver *new)
{
  if (old->secret_len != new->secret_len ||
      memcmp(old->secret_buf, new->secret_buf, old->secret_len) ||
      (!old->port && new->port) || (old->port && !new->port) ||
      (old->port && new->port && strcmp(old->port, new->port)) ||
      (!old->aport && new->aport) || (old->aport && !new->aport) ||
      (old->aport && new->aport && strcmp(old->aport, new->aport)))
    {
      if (old->port)
        {
          ssh_free(old->port);
          old->port = NULL;
        }
      if (old->aport)
        {
          ssh_free(old->aport);
          old->aport = NULL;
        }

      if ((new->port && !(old->port = ssh_strdup(new->port))) ||
          (new->aport && !(old->aport = ssh_strdup(new->aport))))
        {
          ma_errorf("out of memory");
          return 0;
        }

      bufcpy(
        old->secret_buf, &old->secret_len, new->secret_buf, new->secret_len);

      old->eap->rservers_changed = 1;
    }

  return 1;
}

static ERserver *
create_rserver(EGlobal *eap)
{
  ERserver *r;

  if (!(r = ssh_calloc(1, sizeof *r)))
    {
      ma_errorf("out of memory");
      return  NULL;
    }

  r->eap = eap;
  return r;
}

static void
destroy_rserver(ERserver *r)
{
  if (r->aport)
    ssh_free(r->aport);
  if (r->port)
    ssh_free(r->port);
  if (r->address)
    ssh_free(r->address);
  memset(r->secret_buf, 0, sizeof r->secret_buf);
  ssh_free(r);
}

static int
insert_eap_network(EGlobal *e, ENetwork *en)
{
  if (!insert_unique(
        &e->networks, offsetof(ENetwork, link), compare_eap_network, en))
    return 0;
  return 1;
}

static void
remove_eap_network(EGlobal *e, ENetwork *en)
{
  (void)e;
  macsec_list_remove(&en->link);
}

static void
foreach_eap_network(EGlobal *e, void (*func)(ENetwork *en))
{
  macsec_list_foreach(&e->networks, offsetof(ENetwork, link), func);
}

static ENetwork *
next_eap_network(const EGlobal *e, const ENetwork *prev)
{
  return macsec_list_next(&e->networks, offsetof(ENetwork, link), prev);
}

static int
compare_eap_network(const ENetwork *en1, const ENetwork *en2)
{
  return bufcmp(en1->nid.buf, en1->nid.len, en2->nid.buf, en2->nid.len);
}

static int
activate_eap_network(ENetwork *en, const ENetwork *en0)
{
  bufcpy(en->nid.buf, &en->nid.len, en0->nid.buf, en0->nid.len);

  if (en0->identity && !(en->identity = ssh_strdup(en0->identity)))
    {
      ma_errorf("out of memory");
      return 0;
    }

  if (en->identity &&
      !(en->network = eap_add_network(
          en->eap->keystore, &en->nid, en->identity)))
      {
        ma_errorf("cannot add EAP network");
        return 0;
      }

  return 1;
}

static void
deactivate_eap_network(ENetwork *en)
{
  if (en->network)
    {
      eap_remove_network(en->network);
      en->network = NULL;
    }
}

static int
reconfigure_eap_network(ENetwork *old, const ENetwork *new)
{
  if ((!old->identity && new->identity) ||
      (old->identity && !new->identity) ||
      (old->identity && strcmp(old->identity, new->identity)))
    {
      if (old->network)
        {
          eap_remove_network(old->network);
          old->network = NULL;
        }
      if (old->identity)
        {
          ssh_free(old->identity);
          old->identity = NULL;
        }

      if (new->identity)
        {
          if (!(old->identity = ssh_strdup(new->identity)))
            {
              ma_errorf("out of memory");
              return 0;
            }
          if (!(old->network = eap_add_network(
                  old->eap->keystore, &old->nid, old->identity)))
            {
              ma_errorf("cannot add EAP network");
              return 0;
            }
        }
    }
  return 1;
}

static ENetwork *
create_eap_network(EGlobal *eap)
{
  ENetwork *en;

  if (!(en = ssh_calloc(1, sizeof *en)))
    {
      ma_errorf("out of memory");
      return NULL;
    }

  en->eap = eap;
  return en;
}

static void
destroy_eap_network(ENetwork *en)
{
  if (en->identity)
    ssh_free(en->identity);
  ssh_free(en);
}

static int
insert_interface(Config *c, Interface *i)
{
  if (!insert_unique(
        &c->interfaces, offsetof(Interface, link), compare_interface, i))
    return 0;

  i->config = c;
  return 1;
}

static void
remove_interface(Config *c, Interface *i)
{
  (void)c;
  macsec_list_remove(&i->link);
}

static void
foreach_interface(Config *c, void (*func)(Interface *i))
{
  macsec_list_foreach(&c->interfaces, offsetof(Interface, link), func);
}

static Interface *
next_interface(const Config *c, const Interface *prev)
{
  return macsec_list_next(&c->interfaces, offsetof(Interface, link), prev);
}

static int
compare_interface(const Interface *i1, const Interface *i2)
{
  return strcmp(i1->name, i2->name);
}

static int
activate_interface(Interface *i, const Interface *i0)
{
  EapIntf *e = &i->eap;
  MInterface *m = &i->macsec;
  DInterface *x = &i->dot1x;
  EapConfig ec;
  MacsecConfig mc;

  if (!(i->name = ssh_strdup(i0->name)))
    {
      ma_errorf("out of memory");
      return 0;
    }

  ec.keystore = config->eap.keystore;
  ec.radius_servers = config->eap.radius_servers;
  ec.wakeup_func = eap_wakeup;
  ec.wakeup_arg = i;

  if (!eap_init_context(&e->context, &ec))
    {
      ma_errorf("cannot create EAP context");
      goto fail;
    }
  e->init = 1;

  strncpy(mc.ifname, i->name, sizeof mc.ifname);
  mc.ifname[sizeof mc.ifname - 1] = '\0';
  mc.fdescp = &m->fd;
  m->intf = macsec_init_intf(&mc);
  if (!m->intf)
    {
      ma_errorf("cannot init MACsec interface");
      goto fail;
    }
  m->init = 1;
  macsec_get_secy_ctrl(m->intf, 1, &m->initial_secy_ctrl);

  dot1x_init_intf(&x->intf, &e->context, m->intf, mc.ifname);
  dot1x_set_cak_cache_callback(&x->intf, cak_cache_callback);

  x->init = 1;
  dot1x_get_pae_ctrl(&x->intf, &x->initial_pae_ctrl);
  dot1x_get_default_network_ctrl(&x->intf, &x->default_network.initial_ctrl);

  if (!ssh_io_register_fd(m->fd, macsec_io, i))
    {
      ma_errorf("cannot register uncontrolled port file handle");
      goto fail;
    }
  m->fd_register = 1;
  ssh_io_set_fd_request(m->fd, SSH_IO_READ);

  ssh_register_timeout(&i->tick_timeout, 0, 500000, tick, i);

  return 1;

 fail:
  deactivate_interface(i);
  return 0;
}

void
deactivate_interface(Interface *i)
{
  ssh_cancel_timeout(&i->eap_timeout);
  ssh_cancel_timeout(&i->tick_timeout);

  if (i->macsec.fd_register)
    {
      ssh_io_unregister_fd(i->macsec.fd, TRUE);
      i->macsec.fd_register = 0;
    }
  if (i->dot1x.init)
    {
      dot1x_uninit_intf(&i->dot1x.intf);
      i->dot1x.init = 0;
    }
  if (i->macsec.init)
    {
      if (!suspended)
        macsec_uninit_intf(i->macsec.intf);
      close(i->macsec.fd);
      i->macsec.init = 0;
    }
  if (i->eap.init)
    {
      eap_uninit_context(&i->eap.context);
      i->eap.init = 0;
    }
}

static int
reconfigure_interface(Interface *old, const Interface *new)
{
  MInterface *om = &old->macsec;
  const MInterface *nm = &new->macsec;
  DInterface *ox = &old->dot1x;
  const DInterface *nx = &new->dot1x;
  Debug *od = &old->debug;
  const Debug *nd = &new->debug;
  int i;

  if (memcmp(&om->secy_ctrl, &nm->secy_ctrl, sizeof om->secy_ctrl) ||
      memcmp(&om->secy_has, &nm->secy_has, sizeof om->secy_has))
    {
      om->secy_ctrl = nm->secy_ctrl;
      om->secy_has = nm->secy_has;
      set_secy_ctrl(
        om->intf, &om->initial_secy_ctrl, &om->secy_has, &om->secy_ctrl);
    }

  if (memcmp(&od->dot1x, &nd->dot1x, sizeof od->dot1x))
    {
      memcpy(&od->dot1x, &nd->dot1x, sizeof od->dot1x);
      for (i = 0; i < DOT1X_MESSAGE_LAST; i++)
        {
          if (get_bit(od->dot1x, i))
            dot1x_enable_message(&ox->intf, i);
          else
            dot1x_disable_message(&ox->intf, i);
        }
    }

  if (memcmp(&ox->pae_ctrl, &nx->pae_ctrl, sizeof ox->pae_ctrl) ||
      memcmp(&ox->pae_has, &nx->pae_has, sizeof ox->pae_has))
    {
      ox->pae_ctrl = nx->pae_ctrl;
      ox->pae_has = nx->pae_has;
      set_pae_ctrl(
        &ox->intf, &ox->initial_pae_ctrl, &ox->pae_has, &ox->pae_ctrl);
    }

  if (memcmp(
        &ox->default_network.ctrl, &nx->default_network.ctrl,
        sizeof ox->default_network.ctrl) ||
      memcmp(
        &ox->default_network.has, &nx->default_network.has,
        sizeof ox->default_network.has))
    {
      ox->default_network.ctrl = nx->default_network.ctrl;
      ox->default_network.has = nx->default_network.has;
      set_default_network_ctrl(
        &ox->intf, &ox->default_network.initial_ctrl,
        &ox->default_network.has, &ox->default_network.ctrl);
    }

  update_networks(old, new);
  update_psks(old, new);

  return 1;
}

static Interface *
create_interface(Config *config)
{
  Interface *i;

  if (!(i = ssh_calloc(1, sizeof *i)))
    {
      ma_errorf("out of memory");
      return NULL;
    }
  i->config = config;
  macsec_list_init(&i->dot1x.networks);
  macsec_list_init(&i->dot1x.psks);
  return i;
}

static void
destroy_interface(Interface *i)
{
  foreach_psk(i, destroy_psk);
  foreach_network(i, destroy_network);
  if (i->name)
    ssh_free(i->name);
  ssh_free(i);
}

static void
update_networks(Interface *old, const Interface *new)
{
  update_list(
    old, new,
    next_network, compare_network,
    remove_network, insert_network,
    activate_network, deactivate_network, reconfigure_network,
    create_network, destroy_network);
}

static void
update_psks(Interface *old, const Interface *new)
{
  update_list(
    old, new,
    next_psk, compare_psk,
    remove_psk, insert_psk,
    activate_psk, deactivate_psk, reconfigure_psk,
    create_psk, destroy_psk);
}

static void
set_debug_suppress(Interface *i)
{
  if (debug_suppress)
    i->macsec.flags ^= MACSEC_CONFIG_FLAGS_SUPPRESS_MESSAGES;
  else
    i->macsec.flags &= ~(unsigned)MACSEC_CONFIG_FLAGS_SUPPRESS_MESSAGES;

  macsec_set_intf_flags(i->macsec.intf, i->macsec.flags);
}

static int
insert_network(Interface *i, Network *n)
{
  if (!insert_unique(
        &i->dot1x.networks, offsetof(Network, link), compare_network, n))
    return 0;

  n->interface = i;
  return 1;
}

static void
remove_network(Interface *i, Network *n)
{
  (void)i;
  macsec_list_remove(&n->link);
  n->interface = NULL;
}

static void
foreach_network(Interface *i, void (*func)(Network *n))
{
  macsec_list_foreach(&i->dot1x.networks, offsetof(Network, link), func);
}

static Network *
next_network(const Interface *i, const Network *prev)
{
  return macsec_list_next(&i->dot1x.networks, offsetof(Network, link), prev);
}

static int
compare_network(const Network *n1, const Network *n2)
{
  return bufcmp(n1->nid.buf, n1->nid.len, n2->nid.buf, n2->nid.len);
}

static int
activate_network(Network *n, const Network *n0)
{
  Interface *i = n->interface;

  bufcpy(n->nid.buf, &n->nid.len, n0->nid.buf, n0->nid.len);

  if (!(n->dot1x = dot1x_create_network(&i->dot1x.intf, &n->nid)))
    {
      ma_errorf("cannot create network");
      return 0;
    }

  dot1x_get_network_ctrl(n->dot1x, &n->initial_ctrl);
  return 1;
}

static void
deactivate_network(Network *n)
{
  if (n->dot1x)
    {
      dot1x_destroy_network(n->dot1x);
      n->dot1x = NULL;
    }
}

static int
reconfigure_network(Network *old, const Network *new)
{
  if (memcmp(&old->ctrl, &new->ctrl, sizeof old->ctrl) ||
      memcmp(&old->has, &new->has, sizeof old->has))
    {
      old->ctrl = new->ctrl;
      old->has = new->has;
      set_network_ctrl(
        old->dot1x, &old->initial_ctrl, &old->has, &old->ctrl);
    }
  return 1;
}

static Network *
create_network(Interface *interface)
{
  Network *n;

  if (!(n = ssh_calloc(1, sizeof *n)))
    {
      ma_errorf("out of memory");
      return NULL;
    }
  n->interface = interface;
  return n;
}

static void
destroy_network(Network *n)
{
  ssh_free(n);
}

static int
insert_psk(Interface *i, Psk *p)
{
  if (!insert_unique(
        &i->dot1x.psks, offsetof(Psk, link), compare_psk, p))
    return 0;

  p->interface = i;
  return 1;
}

static void
remove_psk(Interface *i, Psk *p)
{
  (void)i;
  macsec_list_remove(&p->link);
  p->interface = NULL;
}

static void
foreach_psk(Interface *i, void (*func)(Psk *p))
{
  macsec_list_foreach(&i->dot1x.psks, offsetof(Psk, link), func);
}

static Psk *
next_psk(const Interface *i, const Psk *prev)
{
  return macsec_list_next(&i->dot1x.psks, offsetof(Psk, link), prev);
}

static int
compare_psk(const Psk *p1, const Psk *p2)
{
  return bufcmp(p1->ckn.buf, p1->ckn.len, p2->ckn.buf, p2->ckn.len);
}

/* example callback for pre-shared CAK activated signal */
static void
cak_activated_callback(Dot1xCak* cak)
{
  /* here information that certain CAK has been activated (received or
   * sent keys) could be used for example to remove older CAKs.
   * CAK is signaled activated after it has sent or received distributed
   * CAK (in case of pairwise CAK) or SAK.
   */
}

#include "dot1xpae_util.h"

static void
cak_cache_insert(const Dot1xCak *cak)
{
  Dot1xFormatBuffer fb;
  int i = 0;

  while (i < CAK_CACHE_SIZE &&
         memcmp(&cak_cache[i].cak.ckn, &cak->ckn, sizeof cak->ckn) != 0)
    i++;

  if (i < CAK_CACHE_SIZE)
    {
      ma_errorf("CAK Cache DELETE index %i intf %d CKN %s",
                i, cak->intf->local_ifindex, dot1x_format_ckn(&fb, &cak->ckn));
    }
  else
    {
      i = 0;
      while (i < CAK_CACHE_SIZE && cak_cache[i].ifindex)
        i++;
    }

  if (i < CAK_CACHE_SIZE)
    {
      struct CAKCacheEntry *entry = &cak_cache[i];

      entry->ifindex = cak->intf->local_ifindex;
      entry->cak = *cak;

      ma_errorf("partner %s", dot1x_format_address(&fb, &cak->partner));

      ma_errorf("CAK Cache INSERT index %i intf %d CKN %s",
        i, entry->ifindex, dot1x_format_ckn(&fb, &entry->cak.ckn));
    }
}

static void
cak_cache_delete(const Dot1xCkn *ckn)
{
  Dot1xFormatBuffer fb;
  int i = 0;

  while (i < CAK_CACHE_SIZE &&
         memcmp(&cak_cache[i].cak.ckn, ckn, sizeof *ckn) != 0)
    i++;

  if (i < CAK_CACHE_SIZE)
    {
      struct CAKCacheEntry *entry = &cak_cache[i];

      ma_errorf("CAK Cache DELETE index %i intf %d CKN %s",
        i, entry->ifindex, dot1x_format_ckn(&fb, &entry->cak.ckn));
      entry->ifindex = 0;
    }
}

static void
cak_cache_callback(Dot1xCakCacheOp op, const Dot1xCak *cak)
{
  if (cak->type == DOT1X_CAK_GROUP || cak->type == DOT1X_CAK_PAIRWISE)
    {
      /* Don't cache pre-shared keys. */
      return;
    }

  if (op == DOT1X_CAK_CACHE_INSERT)
    {
      cak_cache_insert(cak);
    }

  if (op == DOT1X_CAK_CACHE_DELETE)
    {
      cak_cache_delete(&cak->ckn);
    }
}

static void
cak_cache_add_cached(void)
{
  int i;

  for (i = 0; i < CAK_CACHE_SIZE; i++)
    {
      struct CAKCacheEntry *entry = &cak_cache[i];
      Interface *interface = NULL;
      Dot1xIntf *intf = NULL;

      if (entry->ifindex == 0)
        continue;

      while ((interface = next_interface(config, interface)))
        {
          if (interface->dot1x.intf.local_ifindex == entry->ifindex)
            {
              intf = &interface->dot1x.intf;
              break;
            }
        }

      if (intf)
        {
          Dot1xFormatBuffer fb;

          ma_errorf("CAK Cache ADD index %i intf %d CKN %s",
            i, entry->ifindex, dot1x_format_ckn(&fb, &entry->cak.ckn));

          dot1x_insert_cak_cache(intf, &entry->cak);
        }
      else
        {
          cak_cache_delete(&entry->cak.ckn);
        }
    }
}

static int
activate_psk(Psk *p, const Psk *p0)
{
  Interface *i = p->interface;

  bufcpy(p->ckn.buf, &p->ckn.len, p0->ckn.buf, p0->ckn.len);
  bufcpy(p->cak.buf, &p->cak.len, p0->cak.buf, p0->cak.len);
  p->type = p0->type;
  bufcpy(p->nid.buf, &p->nid.len, p0->nid.buf, p0->nid.len);
  bufcpy(
    p->auth_data.buf, &p->auth_data.len, p0->auth_data.buf, p0->auth_data.len);

  if (!(p->dot1x = dot1x_create_cak(
          &i->dot1x.intf, &p->ckn, &p->cak, p->type, &p->auth_data, &p->nid, 
          cak_activated_callback)))
    {
      ma_errorf("cannot create pre-shared CAK");
      return 0;
    }

  return 1;
}

static void
deactivate_psk(Psk *p)
{
  if (p->dot1x)
    {
      dot1x_destroy_cak(p->dot1x);
      p->dot1x = NULL;
    }
}

static int
reconfigure_psk(Psk *old, const Psk *new)
{
  if (bufcmp(old->cak.buf, old->cak.len, new->cak.buf, new->cak.len) ||
      old->type != new->type ||
      bufcmp(old->nid.buf, old->nid.len, new->nid.buf, new->nid.len) ||
      bufcmp(
        old->auth_data.buf, old->auth_data.len,
        new->auth_data.buf, new->auth_data.len) ||
      bufcmp(old->nid.buf, old->nid.len, new->nid.buf, new->nid.len))
    {
      bufcpy(old->cak.buf, &old->cak.len, new->cak.buf, new->cak.len);
      old->type = new->type;
      bufcpy(old->nid.buf, &old->nid.len, new->nid.buf, new->nid.len);
      bufcpy(
        old->auth_data.buf, &old->auth_data.len,
        new->auth_data.buf, new->auth_data.len);

      if (old->dot1x)
        dot1x_destroy_cak(old->dot1x);

      if (!(old->dot1x = dot1x_create_cak(
              &old->interface->dot1x.intf, &old->ckn, &old->cak, old->type,
              &old->auth_data, &old->nid, cak_activated_callback)))
        {
          ma_errorf("cannot recreate pre-shared CAK");
          return 0;
        }
    }
  return 1;
}

static Psk *
create_psk(Interface *interface)
{
  Psk *p;

  if (!(p = ssh_calloc(1, sizeof *p)))
    {
      ma_errorf("out of memory");
      return NULL;
    }
  p->interface = interface;
  return p;
}

static void
destroy_psk(Psk *p)
{
  ssh_free(p);
}

static void
update_list(
  void *oldparent, const void *newparent,
  void *nextfunc, void *cmpfunc,
  void *removefunc, void *insertfunc,
  void *activatefunc, void *deactivatefunc, void *reconfigurefunc,
  void *createfunc, void *destroyfunc)
{
  void *(*next)(const void *parent, const void *prev) = nextfunc;
  int (*cmp)(const void *e1, const void *e2) = cmpfunc;
  int (*insert)(void *parent, void *e) = insertfunc;
  void (*remove)(void *parent, void *e) = removefunc;
  int (*activate)(void *e, const void *e0) = activatefunc;
  void (*deactivate)(void *e) = deactivatefunc;
  int (*reconfigure)(void *o, const void *n) = reconfigurefunc;
  void *(*create)(void *parent) = createfunc;
  void (*destroy)(void *e) = destroyfunc;
  void *o, *n, *onext, *onew;
  int diff;

  o = next(oldparent, NULL);
  n = next(newparent, NULL);

  while (1)
    {
      if (o && n)
        diff = cmp(o, n);
      else
        diff = 0;

      if (diff < 0 || (o && !n))
        {
          onext = next(oldparent, o);
          deactivate(o);
          remove(oldparent, o);
          destroy(o);
          o = onext;
        }
      else if (diff > 0 || (!o && n))
        {
          if ((onew = create(oldparent)))
            {
              if (activate(onew, n))
                {
                  if (reconfigure(onew, n))
                    {
                      insert(oldparent, onew);
                    }
                  else
                    {
                      deactivate(onew);
                      destroy(onew);
                    }
                }
              else
                {
                  destroy(onew);
                }
            }
          n = next(newparent, n);
        }
      else if (o && n)
        {
          onext = next(oldparent, o);
          if (!reconfigure(o, n))
            {
              deactivate(o);
              remove(oldparent, o);
              destroy(o);
            }
          o = onext;
          n = next(newparent, n);
        }
      else
        {
          break;
        }
    }
}

static int
insert_unique(MacsecListLink *list, unsigned offset, void *cmpfunc, void *obj)
{
  int (*cmp)(const void *e1, const void *e2) = cmpfunc;
  MacsecListLink *link = (void *)((unsigned char *)obj + offset);
  MacsecListLink *l;
  void *o = NULL;
  int diff;

  while ((o = macsec_list_next(list, offset, o)))
    {
      diff = cmp(o, obj);
      if (diff > 0)
        {
          l = (void *)((unsigned char *)o + offset);
          macsec_list_insbefore(l, link);
          return 1;
        }
      if (diff == 0)
        return 0;
    }

  macsec_list_append(list, link);
  return 1;
}

static void
set_secy_ctrl(
  MacsecIntfId i, const MacsecSecyCtrl *def,
  const SecyHas *has, const MacsecSecyCtrl *new)
{
  MacsecSecyCtrl c;

  c = *def;

  if (has->validate_frames)
    c.validate_frames = new->validate_frames;
  if (has->replay_protect)
    c.replay_protect = new->replay_protect;
  if (has->replay_window)
    c.replay_window = new->replay_window;
  if (has->protect_frames)
    c.protect_frames = new->protect_frames;
  if (has->always_include_sci)
    c.always_include_sci = new->always_include_sci;
  if (has->use_es)
    c.use_es = new->use_es;
  if (has->use_scb)
    c.use_scb = new->use_scb;
  if (has->current_cipher_suite)
    c.current_cipher_suite = new->current_cipher_suite;
  if (has->confidentiality_offset)
    c.confidentiality_offset = new->confidentiality_offset;
  if (has->admin_point_to_point_mac)
    c.admin_point_to_point_mac = new->admin_point_to_point_mac;

  macsec_set_secy_ctrl(i, 1, &c);
}

static void
set_pae_ctrl(
  Dot1xIntf *i, const Dot1xPaeCtrl *def,
  const PaeHas *has, const Dot1xPaeCtrl *new)
{
  Dot1xPaeCtrl c;

  c = *def;

  if (has->eapol_group)
    c.eapol_group = new->eapol_group;
  if (has->virtual_ports_enable)
    c.virtual_ports_enable = new->virtual_ports_enable;
  if (has->announcer_enable)
    c.announcer_enable = new->announcer_enable;
  if (has->listener_enable)
    c.listener_enable = new->listener_enable;
  if (has->mka_enable)
    c.mka_enable = new->mka_enable;
  if (has->bounded_delay)
    c.bounded_delay = new->bounded_delay;
  if (has->actor_priority)
    c.actor_priority = new->actor_priority;
  if (has->cipher_suite)
    c.cipher_suite = new->cipher_suite;
  if (has->confidentiality_offset)
    c.confidentiality_offset = new->confidentiality_offset;
  if (has->logon)
    c.logon = new->logon;
  if (has->selected_nid)
    c.selected_nid = new->selected_nid;
  if (has->cak_caching_enable)
    c.cak_caching_enable = new->cak_caching_enable;
  if (has->group_ca_enable)
    c.group_ca_enable = new->group_ca_enable;
  if (has->long_eap_caks)
    c.long_eap_caks = new->long_eap_caks;
  if (has->supplicant_enable)
    c.supplicant_enable = new->supplicant_enable;
  if (has->authenticator_enable)
    c.authenticator_enable = new->authenticator_enable;
  if (has->held_period)
    c.held_period = new->held_period;
  if (has->quiet_period)
    c.quiet_period = new->quiet_period;
  if (has->reauth_enabled)
    c.reauth_enabled = new->reauth_enabled;
  if (has->reauth_period)
    c.reauth_period = new->reauth_period;
  if (has->retry_max)
    c.retry_max = new->retry_max;
  if (has->logoff_enable)
    c.logoff_enable = new->logoff_enable;
  if (has->macsec_protect)
    c.macsec_protect = new->macsec_protect;
  if (has->macsec_validate)
    c.macsec_validate = new->macsec_validate;
  if (has->macsec_replay_protect)
    c.macsec_replay_protect = new->macsec_replay_protect;
  if (has->transmit_delay)
    c.transmit_delay = new->transmit_delay;
  if (has->retire_delay)
    c.retire_delay = new->retire_delay;
  if (has->suspend_for)
      c.suspend_for = new->suspend_for;
  if (has->suspend_on_request)
      c.suspend_on_request = new->suspend_on_request;
  if (has->suspended_while)
      c.suspended_while = new->suspended_while;

  dot1x_set_pae_ctrl(i, &c);
}

static void
set_default_network_ctrl(
  Dot1xIntf *i, const Dot1xNetworkCtrl *def,
  const NetworkHas *has, const Dot1xNetworkCtrl *new)
{
  Dot1xNetworkCtrl c;

  set_network_ctrl_sub(&c, def, has, new);
  dot1x_set_default_network_ctrl(i, &c);
}

static void
set_network_ctrl(
  Dot1xNetwork *n, const Dot1xNetworkCtrl *def,
  const NetworkHas *has, const Dot1xNetworkCtrl *new)
{
  Dot1xNetworkCtrl c;

  set_network_ctrl_sub(&c, def, has, new);
  dot1x_set_network_ctrl(n, &c);
}

static void
set_network_ctrl_sub(
  Dot1xNetworkCtrl *c, const Dot1xNetworkCtrl *def,
  const NetworkHas *has, const Dot1xNetworkCtrl *new)
{
  *c = *def;

  if (has->use_eap)
    c->use_eap = new->use_eap;
  if (has->unauth_allowed)
    c->unauth_allowed = new->unauth_allowed;
  if (has->unsecured_allowed)
    c->unsecured_allowed = new->unsecured_allowed;
  if (has->kmd)
    c->kmd = new->kmd;
}

static void
print_provider(EProvider *p)
{
  print(0, "key-provider %s", p->type);
  if (p->init)
    print(1, "init: %s", p->init);
}

static void
print_ca(ECa *c)
{
  print(0, "certificate-authority %s", c->file);
}

static void
print_crl(ECrl *c)
{
  print(0, "certificate-revocation-list %s", c->file);
}

static void
print_rserver(ERserver *r)
{
  print(0, "radius-server %s", r->address);
  if (r->port)
    print(1, "port: %s", r->port);
  if (r->aport)
    print(1, "accounting port: %s", r->aport);
}

static void
print_eap_network(ENetwork *en)
{
  PrintBuffer pb;

  if (en->nid.len <= 0)
    print(0, "EAP default network");
  else
    print(0, "EAP network %s", format_nid(&pb, &en->nid));

  if (en->identity)
    print(1, "identity: %s", en->identity);
}

static void
print_interface(Interface *i)
{
  print(0, "interface %s", i->name);

  if (i->dot1x.init)
    print_interface_dot1x(1, &i->dot1x.intf);

  if (i->macsec.init)
    print_interface_macsec(1, i->macsec.intf);
}

static void
print_interface_dot1x(unsigned i, Dot1xIntf *intf)
{
  Dot1xPaeCtrl paectrl;
  Dot1xPaeStat paestat;
  Dot1xNetworkCtrl netwctrl;
  Dot1xNetworkStat netwstat;
  Dot1xCakStat cakstat;
  Dot1xPortStat portstat;
  Dot1xNetwork *n;
  Dot1xNid nid;
  MacsecAuthData ad;
  Dot1xCak *c;
  Dot1xCkn ckn;
  Dot1xCakType caktype;
  Dot1xPort *p;
  unsigned portid;
  MacsecAddress address;
  PrintBuffer pb;

  print(i, "802.1X information");
  dot1x_get_pae_ctrl(intf, &paectrl);
  print_pae_ctrl(i + 1, &paectrl);
  dot1x_get_pae_stat(intf, &paestat);
  print_pae_stat(i + 1, &paestat);

  print(i + 1, "default network");
  dot1x_get_default_network_ctrl(intf, &netwctrl);
  print_network_ctrl(i + 2, &netwctrl);
  dot1x_get_default_network_stat(intf, &netwstat);
  print_network_stat(i + 2, &netwstat);

  n = NULL;
  while ((n = dot1x_next_network(intf, n)))
    {
      dot1x_get_network_attributes(n, &nid);
      print(i + 1, "network %s", format_nid(&pb, &nid));
      dot1x_get_network_ctrl(n, &netwctrl);
      print_network_ctrl(i + 2, &netwctrl);
      dot1x_get_network_stat(n, &netwstat);
      print_network_stat(i + 2, &netwstat);
    }

  c = NULL;
  while ((c = dot1x_next_cak(intf, c)))
    {
      dot1x_get_cak_attributes(c, &ckn, &caktype, &ad, &nid);
      print(i + 1, "CAK %s", format_ckn(&pb, &ckn));
      print(i + 2, "type: %s", str_choice(cak_type, caktype));
      print(i + 2, "authorization data: %s", format_ad(&pb, &ad));
      print(i + 2, "network: %s", format_nid(&pb, &nid));
      dot1x_get_cak_stat(c, &cakstat);
      print_cak_stat(i + 2, &cakstat);
    }

  print(i + 1, "real port");
  dot1x_get_real_port_stat(intf, &portstat);
  print_xport_stat(i + 2, &portstat);

  p = NULL;
  while ((p = dot1x_next_virtual_port(intf, p)))
    {
      dot1x_get_virtual_port_attributes(p, &portid, &address);
      print(i + 1, "port %u (peer %s)", portid, format_address(&pb, &address));
      dot1x_get_virtual_port_stat(p, &portstat);
      print_xport_stat(i + 2, &portstat);
    }
}

static void
print_pae_ctrl(unsigned i, Dot1xPaeCtrl *c)
{
  PrintBuffer pb;

  print(i, "EAPOL group: %s", str_choice(eapol_group, c->eapol_group));
  print(i, "virtualPortsEnable: %s", str_bool(c->virtual_ports_enable));
  print(i, "announcer.enable: %s", str_bool(c->announcer_enable));
  print(i, "listener.enable: %s", str_bool(c->listener_enable));
  print(i, "mka.enable: %s", str_bool(c->mka_enable));
  print(i, "bounded delay: %s", str_bool(c->bounded_delay));
  print(i, "actorPriority: %u", c->actor_priority);
  print(i, "cipherSuite: %s", str_choice(cipher_suite, c->cipher_suite));
  print(
    i, "confidentialityOffset: %s",
    str_choice(confidentiality_offset, c->confidentiality_offset));
  print(i, "logon: %s", str_bool(c->logon));
  print(i, "selectedNID: %s", format_nid(&pb, &c->selected_nid));
  print(i, "CAK caching enable: %s", str_bool(c->cak_caching_enable));
  print(i, "group CA enable: %s", str_bool(c->group_ca_enable));
  print(i, "long EAP CAKs: %s", str_bool(c->long_eap_caks));
  print(i, "supplicant.enable: %s", str_bool(c->supplicant_enable));
  print(i, "authenticator.enable: %s", str_bool(c->authenticator_enable));
  print(i, "heldPeriod: %u", c->held_period);
  print(i, "quietPeriod: %u", c->quiet_period);
  print(i, "reAuthEnabled: %s", str_bool(c->reauth_enabled));
  print(i, "reAuthPeriod: %u", c->reauth_period);
  print(i, "retryMax: %u", c->retry_max);
  print(i, "logoff enable: %s", str_bool(c->logoff_enable));
  print(i, "macsecProtect: %s", str_bool(c->macsec_protect));
  print(
    i, "macsecValidate: %s", str_choice(validate_frames, c->macsec_validate));
  print(
    i, "macsecReplayProtect: %s", str_bool(c->macsec_replay_protect));
  print(i, "transmitDelay: %u", c->transmit_delay);
  print(i, "retireDelay: %u", c->retire_delay);
  print(i, "suspendFor: %u", c->suspend_for);
  print(i, "suspendOnRequest: %s", str_bool(c->suspend_on_request));
  print(i, "suspendedWhile: %u", c->suspended_while);
}

static void
print_pae_stat(unsigned i, Dot1xPaeStat *s)
{
  PrintBuffer pb;

  print(
    i, "connectedNID: %s", format_nid(&pb, &s->connected_nid));
  print(
    i, "requestedNID: %s", format_nid(&pb, &s->requested_nid));
  print(
    i, "suppEntersAuthenticating: %lu",
    (unsigned long)s->supp_enters_authenticating);
  print(
    i, "suppAuthTimeoutsWhileAuthenticating: %lu",
    (unsigned long)s->supp_auth_timeouts_while_authenticating);
  print(
    i, "suppEapLogoffWhileAuthenticating: %lu",
    (unsigned long)s->supp_eap_logoff_while_authenticating);
  print(
    i, "suppAuthFailWhileAuthenticating: %lu",
    (unsigned long)s->supp_auth_fail_while_authenticating);
  print(
    i, "suppAuthSuccessesWhileAuthenticating: %lu",
    (unsigned long)s->supp_auth_successes_while_authenticating);
  print(
    i, "suppAuthFailWhileAuthenticated: %lu",
    (unsigned long)s->supp_auth_fail_while_authenticated);
  print(
    i, "suppAuthEapLogoffWhileAuthenticated: %lu",
    (unsigned long)s->supp_auth_eap_logoff_while_authenticated);
  print(
    i, "authEntersAuthenticating: %lu",
    (unsigned long)s->auth_enters_authenticating);
  print(
    i, "authAuthTimeoutsWhileAuthenticating: %lu",
    (unsigned long)s->auth_auth_timeouts_while_authenticating);
  print(
    i, "authAuthEapStartsWhileAuthenticating: %lu",
    (unsigned long)s->auth_auth_eap_starts_while_authenticating);
  print(
    i, "authAuthEapLogoffWhileAuthenticating: %lu",
    (unsigned long)s->auth_auth_eap_logoff_while_authenticating);
  print(
    i, "authAuthSuccessesWhileAuthenticating: %lu",
    (unsigned long)s->auth_auth_successes_while_authenticating);
  print(
    i, "authAuthFailWhileAuthenticating: %lu",
    (unsigned long)s->auth_auth_fail_while_authenticating);
  print(
    i, "authAuthReauthsWhileAuthenticating: %lu",
    (unsigned long)s->auth_auth_reauths_while_authenticated);
  print(
    i, "authAuthEapStartsWhileAuthenticated: %lu",
    (unsigned long)s->auth_auth_eap_starts_while_authenticated);
  print(
    i, "authAuthEapLogoffWhileAuthenticated: %lu",
    (unsigned long)s->auth_auth_eap_logoff_while_authenticated);
  print(
    i, "eapolStartFramesRx: %lu",
    (unsigned long)s->eapol_start_frames_rx);
  print(
    i, "eapolStartFramesTx: %lu",
    (unsigned long)s->eapol_start_frames_tx);
  print(
    i, "eapolLogoffFramesRx: %lu",
    (unsigned long)s->eapol_logoff_frames_rx);
  print(
    i, "eapolLogoffFramesTx: %lu",
    (unsigned long)s->eapol_logoff_frames_tx);
  print(
    i, "eapolSuppEapFramesRx: %lu",
    (unsigned long)s->eapol_supp_eap_frames_rx);
  print(
    i, "eapolAuthEapFramesRx: %lu",
    (unsigned long)s->eapol_auth_eap_frames_rx);
  print(
    i, "eapolSuppEapFramesTx: %lu",
    (unsigned long)s->eapol_supp_eap_frames_tx);
  print(
    i, "eapolAuthEapFramesTx: %lu",
    (unsigned long)s->eapol_auth_eap_frames_tx);
  print(
    i, "eapolMKNoCKN: %lu",
    (unsigned long)s->eapol_mk_no_ckn_rx);
  print(
    i, "eapolMKInvalidRx: %lu",
    (unsigned long)s->eapol_mk_invalid_rx);
  print(
    i, "eapolMKAFramesTx: %lu",
    (unsigned long)s->eapol_mka_frames_tx);
  print(
    i, "eapolAnnouncementsRx: %lu",
    (unsigned long)s->eapol_announcements_rx);
  print(
    i, "eapolAnnouncementsTx: %lu",
    (unsigned long)s->eapol_announcements_tx);
  print(
    i, "eapolAnnoucementReqsRx: %lu",
    (unsigned long)s->eapol_announcement_reqs_rx);
  print(
    i, "eapolAnnouncementReqsTx: %lu",
    (unsigned long)s->eapol_announcement_reqs_tx);
  print(
    i, "invalidEapolFrames: %lu",
    (unsigned long)s->invalid_eapol_frames);
  print(
    i, "eapLengthErrorFrames: %lu",
    (unsigned long)s->eap_length_error_frames);
  print(
    i, "eapolPortUnavailable: %lu",
    (unsigned long)s->eapol_port_unavailable);
  print(
    i, "lastEapolFrameVersion: %lu",
    (unsigned long)s->last_eapol_frame_version);
  print(
    i, "lastEapolFrameSource: %s",
    format_address(&pb, &s->last_eapol_frame_source));
}

static void
print_network_ctrl(unsigned i, Dot1xNetworkCtrl *c)
{
  PrintBuffer pb;

  print(i, "useEap: %s", str_choice(use_eap, c->use_eap));
  print(
    i, "unauthAllowed: %s",
    str_choice(unauth_allowed, c->unauth_allowed));
  print(
    i, "unsecuredAllowed: %s",
    str_choice(unsecured_allowed, c->unsecured_allowed));
  print(i, "KMD: %s", format_kmd(&pb, &c->kmd));
}

static void
print_network_stat(unsigned i, Dot1xNetworkStat *s)
{
  Dot1xAccessInformation *ai = &s->access_information;
  PrintBuffer pb;

  if (s->access_information_valid)
    {
      print(
        i, "accessStatus: %s",
        str_choice(access_status, ai->access_status));
      print(
        i, "unauthenticatedAccess: %s",
        str_choice(unauthenticated_access, ai->unauthenticated_access));

      print(i, "accessCapabilities");
      print(
        i + 1, "EAP: %s", str_bool(ai->access_capabilities.eap));
      print(
        i + 1, "EAP + MKA: %s",
        str_bool(ai->access_capabilities.eap_mka));
      print(
        i + 1, "EAP + MKA + MACsec: %s",
        str_bool(ai->access_capabilities.eap_mka_macsec));
      print(
        i + 1, "higher layer (WebAuth): %s",
        str_bool(ai->access_capabilities.higher_layer));
      print(
        i + 1, "higher layer fallback (WebAuth): %s",
        str_bool(ai->access_capabilities.higher_layer_fallback));
      print(
        i + 1, "vendor specific: %s",
        str_bool(ai->access_capabilities.vendor_specific));

      print(
        i, "accessRequested: %s", str_bool(ai->access_requested));
      print(
        i, "virtualPortAccess: %s", str_bool(ai->virtual_port_access));
      print(
        i, "groupAccess: %s", str_bool(ai->group_access));
    }

  if (s->cipher_suites_valid)
    {
      print(i, "cipher suites");
      print_cipher_suites(i + 1, s->cipher_suites);
    }

  if (s->kmd_valid)
    print(i, "KMD: %s", format_kmd(&pb, &s->kmd));
}

static void
print_cak_stat(unsigned i, Dot1xCakStat *s)
{
  PrintBuffer pb;
  int k;

  print(i, "KMD: %s", format_kmd(&pb, &s->kmd));
  print(i, "lifetime: %u", s->lifetime);
  print(i, "enabled: %s", str_bool(s->enabled));
  print(i, "principal: %s", str_bool(s->principal));
  print(i, "live peer count: %u", s->live_peer_num);
  for (k = 0; k < s->live_peer_num; k++)
    print(i + 1, "live peer %s", format_sci(&pb, &s->live_peer_tab[k]));
  print(i, "potential peer count: %u", s->potential_peer_count);
}

static void
print_xport_stat(unsigned i, const Dot1xPortStat *s)
{
  PrintBuffer pb;

  print(i, "MKA active: %s", str_bool(s->mka_active));
  print(i, "MKA failed: %s", str_bool(s->mka_failed));
  print(i, "MKA authenticated: %s", str_bool(s->mka_authenticated));
  print(i, "MKA secured: %s", str_bool(s->mka_secured));
  print(i, "key server SCI: %s", format_sci(&pb, &s->key_server_sci));
  print(i, "key server priority: %u", s->key_server_priority);
  print(i, "KN: %lu", (unsigned long)s->kn);
  print(i, "AN: %u", s->an);
}

static void
print_interface_macsec(unsigned i, MacsecIntfId intf)
{
  unsigned ifindex, portid, peer_portid, an;
  MacsecAddress address, peer_address;
  MacsecPortStat portstat;
  MacsecSecyCtrl secyctrl;
  MacsecSecyStat secystat;
  MacsecTransmitScStat tscstat;
  MacsecTransmitSaStat tsastat;
  MacsecReceiveScStat rscstat;
  MacsecReceiveSaStat rsastat;
  MacsecPeerId peer;
  PrintBuffer pb;

  print(i, "802.1AE information");

  macsec_get_intf_attributes(intf, &ifindex, &address);
  print(i + 1, "ifindex: %u", ifindex);
  print(i + 1, "address: %s", format_address(&pb, &address));

  print(i + 1, "common port");
  macsec_get_common_stat(intf, &portstat);
  print_port_stat(i + 2, &portstat);

  print(i + 1, "uncontrolled port");
  macsec_get_uncontrolled_stat(intf, &portstat);
  print_port_stat(i + 2, &portstat);

  print(i + 1, "controlled port 1 (real port)");
  macsec_get_controlled_stat(intf, 1, &portstat);
  print_port_stat(i + 2, &portstat);
  macsec_get_secy_ctrl(intf, 1, &secyctrl);
  print_secy_ctrl(i + 2, &secyctrl);
  macsec_get_secy_stat(intf, 1, &secystat);
  print_secy_stat(i + 2, &secystat);
  macsec_get_transmit_sc_stat(intf, 1, &tscstat);
  print_transmit_sc_stat(i + 2, &tscstat);
  for (an = 0; an < 4; an++)
    {
      print(i + 2, "SA %d", an);
      macsec_get_transmit_sa_stat(intf, 1, an, &tsastat);
      print_transmit_sa_stat(i + 3, &tsastat);
    }

  peer = 0;
  while ((peer = macsec_next_peer(intf, peer)))
    {
      macsec_get_peer_attributes(
        peer, &portid, &peer_address, &peer_portid);

      if (portid > 1)
        {
          print(i + 1, "controlled port %u (virtual port)", portid);
          macsec_get_controlled_stat(intf, 1, &portstat);
          print_port_stat(i + 2, &portstat);
          macsec_get_secy_ctrl(intf, portid, &secyctrl);
          print_secy_ctrl(i + 2, &secyctrl);
          macsec_get_secy_stat(intf, portid, &secystat);
          print_secy_stat(i + 2, &secystat);
          macsec_get_transmit_sc_stat(intf, portid, &tscstat);
          print_transmit_sc_stat(i + 2, &tscstat);
          for (an = 0; an < 4; an++)
            {
              print(i + 2, "SA %d", an);
              macsec_get_transmit_sa_stat(intf, portid, an, &tsastat);
              print_transmit_sa_stat(6, &tsastat);
            }
        }

      print(i + 1, "peer %s", format_address(&pb, &peer_address));
      print(i + 2, "local portid: %u", portid);
      print(i + 2, "remote portid: %u", peer_portid);

      macsec_get_receive_sc_stat(peer, &rscstat);
      print_receive_sc_stat(i + 2, &rscstat);

      for (an = 0; an < 4; an++)
        {
          print(i + 2, "SA %d", an);
          macsec_get_receive_sa_stat(peer, an, &rsastat);
          print_receive_sa_stat(i + 3, &rsastat);
        }
    }
}

static void
print_macsec_capabilities(unsigned i, const MacsecCapabilities *c)
{
  print(i, "maxPeerSCs: %u", c->max_peer_scs);
  print(i, "maxReceiveKeys: %u", c->max_receive_keys);
  print(i, "maxTransmitKeys: %u", c->max_transmit_keys);
  print(i, "cipher suites");
  print_cipher_suites(i + 1, c->cipher_suites);
}

static void
print_port_stat(unsigned i, const MacsecPortStat *s)
{
  print(i, "ifInOctets: %llu", (unsigned long long)s->if_in_octets);
  print(i, "ifInUcastPkts: %llu", (unsigned long long)s->if_in_ucast_pkts);
  print(
    i, "ifInMulticastPkts: %llu", (unsigned long long)s->if_in_multicast_pkts);
  print(
    i, "ifInBroadcastPkts: %llu", (unsigned long long)s->if_in_broadcast_pkts);
  print(i, "ifInDiscards: %llu", (unsigned long long)s->if_in_discards);
  print(i, "ifInErrors: %llu", (unsigned long long)s->if_in_errors);
  print(i, "ifOutOctets: %llu", (unsigned long long)s->if_out_octets);
  print(
    i, "ifOutUcastPkts: %llu", (unsigned long long)s->if_out_ucast_pkts);
  print(
    i, "ifOutMulticastPkts: %llu",
    (unsigned long long)s->if_out_multicast_pkts);
  print(
    i, "ifOutBroadcastPkts: %llu",
    (unsigned long long)s->if_out_broadcast_pkts);
  print(i, "ifOutErrors: %llu", (unsigned long long)s->if_out_errors);
  print(i, "MAC_Enabled: %s", str_bool(s->mac_enabled));
  print(i, "MAC_Operational: %s", str_bool(s->mac_operational));
  print(
    i, "operPointToPointMAC: %s", str_bool(s->oper_point_to_point_mac));
}

static void
print_secy_ctrl(unsigned i, const MacsecSecyCtrl *c)
{
  print(
    i, "validateFrames: %s",
    str_choice(validate_frames, c->validate_frames));
  print(i, "replayProtect: %s", str_bool(c->replay_protect));
  print(i, "replayWindow: %lu", (unsigned long)c->replay_window);
  print(i, "protectFrames: %s", str_bool(c->protect_frames));
  print(i, "alwaysIncludeSci: %s", str_bool(c->always_include_sci));
  print(i, "useES: %s", str_bool(c->use_es));
  print(i, "useSCB: %s", str_bool(c->use_scb));
  print(
    i, "currentCipherSuite: %s",
    str_choice(cipher_suite, c->current_cipher_suite));
  print(
    i, "currentConfidentialityOffset: %s",
    str_choice(confidentiality_offset, c->confidentiality_offset));
  print(
    i, "adminPointToPointMAC: %s",
    str_choice(admin_point_to_point_mac, c->admin_point_to_point_mac));
}

static void
print_secy_stat(unsigned i, const MacsecSecyStat *s)
{
  print(i, "inPktsUntagged: %llu", (unsigned long long)s->in_pkts_untagged);
  print(i, "inPktsNoTag: %llu", (unsigned long long)s->in_pkts_no_tag);
  print(i, "inPktsBadTag: %llu", (unsigned long long)s->in_pkts_bad_tag);
  print(
    i, "inPktsUnknownSCI: %llu", (unsigned long long)s->in_pkts_unknown_sci);
  print(i, "inPktsNoSCI: %llu", (unsigned long long)s->in_pkts_no_sci);
  print(i, "inPktsOverrun: %llu", (unsigned long long)s->in_pkts_overrun);
  print(
    i, "inOctetsValidated: %llu", (unsigned long long)s->in_octets_validated);
  print(
    i, "inOctetsDecrypted: %llu", (unsigned long long)s->in_octets_decrypted);
  print(
    i, "outPktsUntagged: %llu", (unsigned long long)s->out_pkts_untagged);
  print(
    i, "outPktsTooLong: %llu", (unsigned long long)s->out_pkts_too_long);
  print(
    i, "outOctetsProtected: %llu",
    (unsigned long long)s->out_octets_protected);
  print(
    i, "outOctetsEncrypted: %llu",
    (unsigned long long)s->out_octets_encrypted);
}

static void
print_transmit_sc_stat(unsigned i, const MacsecTransmitScStat *s)
{
  PrintBuffer pb;

  print(i, "SCI: %s", format_sci(&pb, &s->sci));
  print(i, "transmitting: %s", str_bool(s->transmitting));
  print(i, "encodingSA: %u", s->encoding_sa);
  print(i, "encipheringSA: %u", s->enciphering_sa);
  print(i, "createdTime: %s", format_sysuptime(&pb, s->created_time));
  print(i, "startedTime: %s", format_sysuptime(&pb, s->started_time));
  print(i, "stoppedTime: %s", format_sysuptime(&pb, s->stopped_time));
  print(
    i, "outPktsProtected: %llu", (unsigned long long)s->out_pkts_protected);
  print(
    i, "outPktsEncrypted: %llu", (unsigned long long)s->out_pkts_encrypted);
}

static void
print_transmit_sa_stat(unsigned i, const MacsecTransmitSaStat *s)
{
  PrintBuffer pb;

  print(i, "inUse: %s", str_bool(s->in_use));
  print(i, "nextPn: %llu", (unsigned long long)s->next_pn);
  print(i, "createdTime: %s", format_sysuptime(&pb, s->created_time));
  print(i, "startedTime: %s", format_sysuptime(&pb, s->started_time));
  print(i, "stoppedTime: %s", format_sysuptime(&pb, s->stopped_time));
  print(
    i, "outPktsProtected: %llu", (unsigned long long)s->out_pkts_protected);
  print(
    i, "outPktsEncrypted: %llu", (unsigned long long)s->out_pkts_encrypted);
}

static void
print_receive_sc_stat(unsigned i, const MacsecReceiveScStat *s)
{
  PrintBuffer pb;

  print(i, "SCI: %s", format_sci(&pb, &s->sci));
  print(i, "receiving: %s", str_bool(s->receiving));
  print(i, "createdTime: %s", format_sysuptime(&pb, s->created_time));
  print(i, "startedTime: %s", format_sysuptime(&pb, s->started_time));
  print(i, "stoppedTime: %s", format_sysuptime(&pb, s->stopped_time));
  print(i, "inPktsUnchecked: %llu", (unsigned long long)s->in_pkts_unchecked);
  print(i, "inPktsDelayed: %llu", (unsigned long long)s->in_pkts_delayed);
  print(i, "inPktsLate: %llu", (unsigned long long)s->in_pkts_late);
  print(i, "inPktsOK: %llu", (unsigned long long)s->in_pkts_ok);
  print(i, "inPktsInvalid: %llu", (unsigned long long)s->in_pkts_invalid);
  print(i, "inPktsNotValid: %llu", (unsigned long long)s->in_pkts_not_valid);
  print(
    i, "inPktsNotUsingSA: %llu", (unsigned long long)s->in_pkts_not_using_sa);
  print(
    i, "inPktsUnusedSA: %llu", (unsigned long long)s->in_pkts_unused_sa);
}

static void
print_receive_sa_stat(unsigned i, const MacsecReceiveSaStat *s)
{
  PrintBuffer pb;

  print(i, "inUse: %s", str_bool(s->in_use));
  print(i, "nextPn: %llu", (unsigned long long)s->next_pn);
  print(i, "lowestPn: %llu", (unsigned long long)s->lowest_pn);
  print(i, "createdTime: %s", format_sysuptime(&pb, s->created_time));
  print(i, "startedTime: %s", format_sysuptime(&pb, s->started_time));
  print(i, "stoppedTime: %s", format_sysuptime(&pb, s->stopped_time));
  print(i, "inPktsOK: %llu", (unsigned long long)s->in_pkts_ok);
  print(i, "inPktsInvalid: %llu", (unsigned long long)s->in_pkts_invalid);
  print(
    i, "inPktsNotValid: %llu", (unsigned long long)s->in_pkts_not_valid);
  print(
    i, "inPktsNotUsingSA: %llu", (unsigned long long)s->in_pkts_not_using_sa);
  print(
    i, "inPktsUnusedSA: %llu", (unsigned long long)s->in_pkts_unused_sa);
}

static void
print_cipher_suites(unsigned i, const MacsecCipherCapability *ccv)
{
  int k;

  for (k = 0; k < MACSEC_CIPHER_SUITE_COUNT; k++)
    {
      print(i, "%s", str_choice(cipher_suite, k));
      print(i + 1, "implemented: %s", str_bool(ccv[k].implemented));
      print(
        i, "capability: %s",
        str_choice(macsec_capability, ccv[k].capability));
    }
}

static void
print(unsigned indent, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);

  if (outputf)
    {
      fprintf(outputf, "%*s", 4 * indent, "");
      vfprintf(outputf, fmt, ap);
      fputc('\n', outputf);
    }
  else
    {
      ma_errorf_begin("%*s", 4 * indent, "");
      ma_verrorf_end(fmt, ap);
    }

  va_end(ap);
}

static const char *
format_sysuptime(PrintBuffer *pb, uint32_t t)
{
  uint32_t d, h, m, s, ms;

  s = t / 100;
  ms = (t - s * 100) * 10;

  d = s / 86400;
  s -= d * 86400;
  h = s / 3600;
  s -= h * 3600;
  m = s / 60;
  s -= m * 60;

  snprintf(
    *pb, sizeof *pb, "%lud%02lu:%02lu:%02lu.%03lu",
    (unsigned long)d, (unsigned long)h, (unsigned long)m,
    (unsigned long)s, (unsigned long)ms);
  (*pb)[sizeof *pb - 1] = '\0';
  return *pb;
}

static const char *
format_sci(PrintBuffer *pb, const MacsecSci *s)
{
  PrintBuffer pb2;
  const char *a;

  a = format_address(&pb2, &s->address);
  snprintf(*pb, sizeof *pb, "%s/%u", a, s->portid);
  (*pb)[sizeof *pb - 1] = '\0';
  return *pb;
}

static const char *
format_address(PrintBuffer *pb, const MacsecAddress *a)
{
  snprintf(
    *pb, sizeof *pb, "%02x:%02x:%02x:%02x:%02x:%02x",
    (unsigned)a->buf[0], (unsigned)a->buf[1], (unsigned)a->buf[2],
    (unsigned)a->buf[3], (unsigned)a->buf[4], (unsigned)a->buf[5]);
  (*pb)[sizeof *pb - 1] = '\0';
  return *pb;
}

static const char *
format_ckn(PrintBuffer *pb, const Dot1xCkn *ckn)
{
  return format_hex(pb, ckn->buf, ckn->len);
}

static const char *
format_nid(PrintBuffer *pb, const Dot1xNid *nid)
{
  if (nid->len <= 0)
    return "<no NID>";

  return format_utf8(pb, nid->buf, nid->len);
}

static const char *
format_kmd(PrintBuffer *pb, const Dot1xKmd *kmd)
{
  if (kmd->len <= 0)
    return "<no KMD>";

  return format_utf8(pb, kmd->buf, kmd->len);
}

static const char *
format_ad(PrintBuffer *pb, const MacsecAuthData *ad)
{
  if (ad->len <= 0)
    return "<no authData>";

  return format_hex(pb, ad->buf, ad->len);
}

static const char *
format_hex(PrintBuffer *pb, const unsigned char *buf, unsigned len)
{
  unsigned pi, pos;
  char c1, c2;

  pi = 0;
  for (pos = 0; pos < len; pos++)
    {
      c1 = hexcharv[(buf[pos] & 0xf0) >> 4];
      c2 = hexcharv[buf[pos] & 0x0f];

      if (pi + 2 >= sizeof *pb)
        goto end;

      (*pb)[pi++] = c1;
      (*pb)[pi++] = c2;
    }
 end:
  (*pb)[pi] = '\0';
  return *pb;
}

static const char *
format_utf8(PrintBuffer *pb, const unsigned char *buf, unsigned len)
{
  int pi, pos, n, i;
  unsigned char byte;
  unsigned u;
  char c;

  pi = 0;
  pos = 0;
  while (pos < len)
    {
      /* decode first byte of a character, set n to # of remaining bytes */
      byte = (unsigned char)buf[pos++];
      if ((byte & 0x80) == 0x00)
        {
          u = byte & 0x7f;
          n = 0;
        }
      else if ((byte & 0xe0) == 0xc0)
        {
          u = byte & 0x1f;
          n = 1;
        }
      else if ((byte & 0xf0) == 0xe0)
        {
          u = byte & 0x0f;
          n = 2;
        }
      else if ((byte & 0xf8) == 0xf0)
        {
          u = byte & 0x07;
          n = 3;
        }
      else
        {
          goto end; /* bad UTF-8 first byte */
        }

      /* decode remaining bytes of a character */
      for (i = 0; i < n; i++)
        {
          if (pos >= len)
            goto end; /* truncated UTF-8 character */
          byte = (unsigned char)buf[pos++];
          if ((byte & 0xc0) != 0x80)
            goto end; /* bad UTF-8 byte */
          u <<= 6;
          u |= byte & 0x3f;
        }

      if (u >= 0x20 && u < 0x7f)
        c = (char)u;
      else
        c = '_';

      if (pi + 1 >= sizeof *pb)
        goto end;

      (*pb)[pi++] = c;
    }

 end:
  (*pb)[pi] = '\0';
  return *pb;
}

static const char *
str_bool(const unsigned b)
{
  return str_choice(boolean, b);
}

static const char *
str_choice(const StringInt *map, const unsigned c)
{
  const char *s;

  if (!(s = inttostr(map, c)))
    return "<invalid>";

  return s;
}

static int
get_char(const char **p)
{
  const char *ptr = *p;
  unsigned char byte;
  int value, n, i;

  byte = (unsigned char)*ptr;

  if (!byte)
    return 0;

  if ((byte & 0x80) == 0x00)
    {
      value = byte & 0x7f;
      n = 0;
    }
  else if ((byte & 0xe0) == 0xc0)
    {
      value = byte & 0x1f;
      n = 1;
    }
  else if ((byte & 0xf0) == 0xe0)
    {
      value = byte & 0x0f;
      n = 2;
    }
  else if ((byte & 0xf8) == 0xf0)
    {
      value = byte & 0x07;
      n = 3;
    }
  else
    {
      return -1;
    }

  ptr++;

  for (i = 0; i < n; i++)
    {
      byte = (unsigned char)*ptr;

      if (!byte)
        return -1;

      if ((byte & 0xc0) != 0x80)
        return 1;

      value <<= 6;
      value |= byte & 0x3f;

      ptr++;
    }

  *p = ptr;
  return value;
}

static int
strtoint(const StringInt *map, const char *s)
{
  const StringInt *si;

  for (si = map; si->s; si++)
    if (!strcmp(si->s, s))
      return si->i;

  return -1;
}

static const char *
inttostr(const StringInt *map, int i)
{
  const StringInt *si;

  for (si = map; si->s; si++)
    if (si->i == i)
      return si->s;

  return NULL;
}

static int
buftoint(const StringInt *map, const char *buf, unsigned len)
{
  const StringInt *si;

  for (si = map; si->s; si++)
    if (strlen(si->s) == len && !strncmp(si->s, buf, len))
      return si->i;

  return -1;
}

static void
bufcpy(
  unsigned char *buf1, unsigned *len1,
  const unsigned char *buf2, unsigned len2)
{
  memcpy(buf1, buf2, len2);
  *len1 = len2;
}

static int
bufcmp(
  const unsigned char *buf1, unsigned len1,
  const unsigned char *buf2, unsigned len2)
{
  int lendiff, minlen, n;

  if ((lendiff = len1 - len2) < 0)
    minlen = len1;
  else
    minlen = len2;

  if ((n = memcmp(buf1, buf2, minlen)))
    return n;
  else
    return lendiff;
}

static int
bufstrcmp(const char *buf, unsigned len, const char *s)
{
  int l, lendiff, minlen, n;

  l = strlen(s);
  if ((lendiff = len - l) < 0)
    minlen = len;
  else
    minlen = l;

  if ((n = memcmp(buf, s, minlen)))
    return n;
  else
    return lendiff;
}

static void
set_bit(unsigned char *tab, int n)
{
  tab[n >> 3] |= 1 << (n & 7);
}

static int
get_bit(unsigned char *tab, int n)
{
  return !!(tab[n >> 3] & (1 << (n & 7)));
}

static void
debug_callback(const char *message, void *context)
{
  (void)context;

  if (debug_suppress)
    return;

  fprintf(stderr, "%s\n", message);
}

static void
failf(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  ma_verrorf(fmt, ap);
  va_end(ap);
  fail();
}

static void
fail(void)
{
  longjmp(*failpop(), 1);
}

static jmp_buf *
failpush(void)
{
  if (faillevel >= MAX_FAILLEVELS)
    fatal("out of fail levels");

  return &failenv[faillevel++];
}

static jmp_buf *
failpop(void)
{
  if (faillevel <= 0)
    fatal("invalid fail level");

  faillevel--;
  return &failenv[faillevel];
}

static void
fatal(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  ma_verrorf(fmt, ap);
  va_end(ap);
  exit(1);
}
