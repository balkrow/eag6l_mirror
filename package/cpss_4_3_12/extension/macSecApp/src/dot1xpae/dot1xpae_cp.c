/**

   IEEE 802.1X-2010 CP state machine.

   File: dot1xpae_cp.c

   @description
   The step function dot1x_cp_step_one() of the state machine is
   a mechanical transcription of the Figure 12-2 of 802.1X-2010. Each
   arrow of the figure corresponds to a group of conditions in an `if'
   expression, each group consisting of an input state condition and
   potentially some input variable conditions. Each box of the figure
   corresponds to a block of statements guarded by an `if' expression.

   Differences from Figure 12-2:

   - The transition from INIT to CHANGE is triggered by portEnabled
     becoming TRUE, instead of being unconditional.

   - Setting ltx to TRUE is moved from TRANSMIT state to TRANSMITTING
     where otx is set to FALSE. Otherwise transmission would be
     enabled simultaneously for both (old and latest) SAs before
     usingTransmitSA became TRUE.

   - Added setting of oan and lan to zero in INIT.

   - Moved assignments oki=lki,otx=ltx,orx=ltx from RETIRE to the
     beginning of RECEIVE state.

   - Added assignment oan=lan to RECEIVE after the oki=lki assignment
     (moved from RETIRE).

   - Removed assignments lki=0 and ltx=lrx=FALSE from RETIRE.

   - Added assignment orx=FALSE to RETIRE.

   - Removed condition !controlledPortEnabled from the trigger of
     transition from RECEIVING to TRANSMIT.

   - Changed trigger of transition from READY to TRANSMIT from
     "serverTransmitting || !controlledPortEnabled" to
     "serverTransmitting || (transmitWhen == 0)".

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

 */

#include "dot1xpae_cp.h"
#include "dot1xpae_mka.h"
#include "dot1xpae_util.h"
#include "macseclmi.h"
#include <stddef.h>
#include <string.h>

/*
 * State machine convenience macros.
 */
#define CHGD_CIPHER(intf) \
  ((intf)->current_cipher_suite != (intf)->cipher_suite || \
   (intf)->confidentiality_offset != (intf)->cipher_offset)

#define CHGD_CONNECT(intf) \
  ((intf)->connect != DOT1X_CONNECT_SECURE || \
   (intf)->chgd_server || \
   CHGD_CIPHER(intf))

/*
 * Zero key identifier.
 */
static const Dot1xKeyIdentifier zero_ki;

/*
 * Empty authorization data.
 */
static const MacsecAuthData empty_auth_data;

/*
 * Run the CP state machine of a real or virtual port.
 */
static void
step_port(Dot1xPort *port);

/*
 * Update the CP timers of a real or virtual port.
 */
static void
tick_port(Dot1xPort *port);

/*
 * Set CP variables.
 */
static void
set_state(Dot1xPort *port, int value);
static void
set_controlled_port_enabled(Dot1xPort *port, unsigned value);
static void
set_port_valid(Dot1xPort *port, unsigned value);
static void
if_secure(Dot1xPort *port, unsigned value, const MacsecAuthData *auth_data);
static void
set_protect_frames(Dot1xPort *port, unsigned value);
static void
set_replay_protect(Dot1xPort *port, unsigned value);
static void
set_validate_frames(Dot1xPort *port, MacsecValidateFrames value);
static void
set_oki(Dot1xPort *port, const Dot1xKeyIdentifier *value);
static void
set_oan(Dot1xPort *port, unsigned value);
static void
set_orx(Dot1xPort *port, unsigned value);
static void
set_otx(Dot1xPort *port, unsigned value);
static void
set_lki(Dot1xPort *port, const Dot1xKeyIdentifier *value);
static void
set_lan(Dot1xPort *port, unsigned value);
static void
set_lrx(Dot1xPort *port, unsigned value);
static void
set_ltx(Dot1xPort *port, unsigned value);
static void
set_chgd_server(Dot1xPort *port, unsigned value);
static void
set_new_sak(Dot1xPort *port, unsigned value);
static void
set_new_info(Dot1xPort *port, unsigned value);
static void
set_current_cipher_suite(Dot1xPort *port, MacsecCipherSuite value);
static void
set_confidentiality_offset(Dot1xPort *port, MacsecConfidentialityOffset value);
static void
set_transmit_when(Dot1xPort *port, unsigned value);
static void
set_retire_when(Dot1xPort *port, unsigned value);
static void
create_sas(Dot1xPort *port);
static void
enable_receive_sas(Dot1xPort *port);
static void
enable_transmit_sa(Dot1xPort *port);
static void
delete_sas(Dot1xPort *port, const Dot1xKeyIdentifier *ki);

/*
 * Update SecY control variables protectFrames, replayProtect,
 * validateFrames, currentCiphersuite and confidentialityOffset.
 */
static void
update_secy_ctrl(Dot1xPort *port);

static void
set_suspend_on_request(Dot1xPort *port, unsigned value);

static void
set_suspend_for(Dot1xPort *port, unsigned value);

/*
 * Public functions.
 */

int
dot1x_step_cp(Dot1xIntf *intf)
{
  dot1x_foreach_port(intf, step_port);

  if (intf->cp_changed)
    {
      intf->cp_changed = 0;
      return 1;
    }

  return 0;
}

void
dot1x_tick_cp(Dot1xIntf *intf)
{
  dot1x_foreach_port(intf, tick_port);
}

/*
 * Static functions.
 */

static void
step_port(Dot1xPort *port)
{
  Dot1xIntf *intf = port->intf;

  if (port->cp_state == DOT1X_CP_BEGIN ||

      (port->cp_state != DOT1X_CP_INIT &&
       !intf->port_enabled))
    {
      set_state(port, DOT1X_CP_INIT);
      set_controlled_port_enabled(port, 0);
      set_protect_frames(port, intf->ctrl.macsec_protect);
      set_replay_protect(port, intf->ctrl.macsec_replay_protect);
      set_validate_frames(port, intf->ctrl.macsec_validate);
      set_port_valid(port, 0);
      set_oki(port, &zero_ki);
      set_oan(port, 0);
      set_orx(port, 0);
      set_otx(port, 0);
      set_lki(port, &zero_ki);
      set_lan(port, 0);
      set_lrx(port, 0);
      set_ltx(port, 0);
      set_chgd_server(port, 0);

      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);

      if (port->resuming_suspend)
        {
          MacsecSecyCtrl ctrl;

          macsec_get_secy_ctrl(port->intf->macsec_intf, port->portid, &ctrl);

          port->cipher_suite = ctrl.current_cipher_suite;
          port->cipher_offset = ctrl.confidentiality_offset;

          set_protect_frames(port, ctrl.protect_frames);
          set_replay_protect(port, ctrl.replay_protect);
          set_validate_frames(port, ctrl.validate_frames);
          set_current_cipher_suite(port, port->cipher_suite);
          set_confidentiality_offset(port, port->cipher_offset);

          port->secy_updated = 1;

          dot1x_set_suspended_while(port, intf->ctrl.suspended_while);
          dot1x_suspension_start(
                  port,
                  NULL,
                  intf->ctrl.suspended_while,
                  "local resumption");
        }
      else
        {
          dot1x_set_suspended_while(port, 0);
          port->has_sa = 0;
        }

      port->can_suspend = 0;

      intf->cp_changed = 1;
    }
  else
  if (port->cp_state == DOT1X_CP_INIT &&
      intf->port_enabled &&
      port->suspended_while > 0)
    {
      set_state(port, DOT1X_CP_RETIRE);
      set_lan(port, port->transmit_sa_an);
      set_lrx(port, 1);
      set_ltx(port, 1);

      port->can_suspend = 0;

      intf->cp_changed = 1;
    }
  else
  if ((port->cp_state == DOT1X_CP_INIT &&
       intf->port_enabled) ||

      (port->cp_state == DOT1X_CP_ALLOWED &&
       port->connect != DOT1X_CONNECT_UNAUTHENTICATED) ||

      (port->cp_state == DOT1X_CP_AUTHENTICATED &&
       port->connect != DOT1X_CONNECT_AUTHENTICATED) ||

      (port->cp_state == DOT1X_CP_SECURED &&
       CHGD_CONNECT(port)) ||

      (port->cp_state == DOT1X_CP_RETIRE &&
       CHGD_CONNECT(port) &&
       port->suspended_while == 0))
    {
      set_state(port, DOT1X_CP_CHANGE);
      update_secy_ctrl(port);
      set_port_valid(port, 0);
      set_controlled_port_enabled(port, 0);
      delete_sas(port, NULL);
      set_oki(port, &zero_ki);
      set_oan(port, 0);
      set_orx(port, 0);
      set_otx(port, 0);
      set_lki(port, &zero_ki);
      set_lan(port, 0);
      set_lrx(port, 0);
      set_ltx(port, 0);
      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);
      intf->cp_changed = 1;
    }
  else
  if (port->cp_state == DOT1X_CP_CHANGE &&
      port->connect == DOT1X_CONNECT_UNAUTHENTICATED)
    {
      set_state(port, DOT1X_CP_ALLOWED);
      set_protect_frames(port, 0);
      set_replay_protect(port, 0);
      set_validate_frames(port, MACSEC_VALIDATE_FRAMES_CHECK);
      update_secy_ctrl(port);
      set_port_valid(port, 0);
      if_secure(port, port->port_valid, NULL);
      set_controlled_port_enabled(port, 1);
      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);
      intf->cp_changed = 1;
    }
  else
  if (port->cp_state == DOT1X_CP_CHANGE &&
      port->connect == DOT1X_CONNECT_AUTHENTICATED)
    {
      set_state(port, DOT1X_CP_AUTHENTICATED);
      set_protect_frames(port, 0);
      set_replay_protect(port, 0);
      set_validate_frames(port, MACSEC_VALIDATE_FRAMES_CHECK);
      update_secy_ctrl(port);
      set_port_valid(port, 0);
      if_secure(port, port->port_valid, port->authorization_data);
      set_controlled_port_enabled(port, 1);
      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);
      intf->cp_changed = 1;
    }
  else
  if (port->cp_state == DOT1X_CP_CHANGE &&
      port->connect == DOT1X_CONNECT_SECURE)
    {
      set_state(port, DOT1X_CP_SECURED);
      set_chgd_server(port, 0);
      set_current_cipher_suite(port, port->cipher_suite);
      set_confidentiality_offset(port, port->cipher_offset);
      set_protect_frames(port, intf->ctrl.macsec_protect);
      set_replay_protect(port, intf->ctrl.macsec_replay_protect);
      set_validate_frames(port, intf->ctrl.macsec_validate);
      update_secy_ctrl(port);
      set_port_valid(port, 1);
      if_secure(port, port->port_valid, port->authorization_data);
      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);
      intf->cp_changed = 1;
    }
  else
  if ((port->cp_state == DOT1X_CP_SECURED &&
       port->new_sak &&
       !CHGD_CONNECT(port)) ||

      (port->cp_state == DOT1X_CP_ABANDON &&
       port->new_sak &&
       !CHGD_CONNECT(port)) ||

      (port->cp_state == DOT1X_CP_RETIRE &&
       port->new_sak &&
       !CHGD_CONNECT(port)))
    {
      set_state(port, DOT1X_CP_RECEIVE);
      update_secy_ctrl(port);
      set_oki(port, &port->lki);
      set_oan(port, port->lan);
      set_orx(port, port->lrx);
      set_otx(port, port->ltx);
      set_lki(port, &port->distributed_ki);
      set_lan(port, port->distributed_an);
      set_lrx(port, 0);
      set_ltx(port, 0);
      create_sas(port);
      enable_receive_sas(port);
      set_new_sak(port, 0);
      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);
      intf->cp_changed = 1;
    }
  else
  if (port->cp_state == DOT1X_CP_RECEIVE &&
      port->using_receive_sas)
    {
      set_state(port, DOT1X_CP_RECEIVING);
      update_secy_ctrl(port);
      set_transmit_when(port, intf->ctrl.transmit_delay);
      set_lrx(port, 1);
      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);
      intf->cp_changed = 1;
    }
  else
  if (port->cp_state == DOT1X_CP_RECEIVING &&
      !port->elected_self)
    {
      set_state(port, DOT1X_CP_READY);
      update_secy_ctrl(port);
      set_new_info(port, 1);
      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);
      intf->cp_changed = 1;
    }
  else
  if ((port->cp_state == DOT1X_CP_RECEIVING &&
       port->elected_self &&
       (port->all_receiving ||
        port->transmit_when_ticks == 0)) ||

      (port->cp_state == DOT1X_CP_READY &&
       (port->server_transmitting ||
        port->transmit_when_ticks == 0)))
    {
      set_state(port, DOT1X_CP_TRANSMIT);
      update_secy_ctrl(port);
      set_controlled_port_enabled(port, 1);
      enable_transmit_sa(port);
      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);
      intf->cp_changed = 1;
    }
  else
  if ((port->cp_state == DOT1X_CP_RECEIVING &&
       (port->new_sak ||
        CHGD_CONNECT(port))) ||

      (port->cp_state == DOT1X_CP_READY &&
       (port->new_sak ||
        CHGD_CONNECT(port))))
    {
      set_state(port, DOT1X_CP_ABANDON);
      update_secy_ctrl(port);
      set_lrx(port, 0);
      delete_sas(port, &port->lki);
      set_lki(port, &zero_ki);
      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);
      intf->cp_changed = 1;
    }
  else
  if (port->cp_state == DOT1X_CP_TRANSMIT &&
      port->using_transmit_sa)
    {
      set_state(port, DOT1X_CP_TRANSMITTING);
      update_secy_ctrl(port);
      set_retire_when(port, port->orx ? intf->ctrl.retire_delay : 0);
      set_otx(port, 0);
      set_ltx(port, 1);
      set_new_info(port, 1);
      set_suspend_on_request(port, 0);
      set_suspend_for(port, 0);
      intf->cp_changed = 1;
    }
  else
  if ((port->cp_state == DOT1X_CP_ABANDON &&
       CHGD_CONNECT(port)) ||

      (port->cp_state == DOT1X_CP_TRANSMITTING &&
       (port->retire_when_ticks == 0 ||
        CHGD_CONNECT(port))))
    {
      set_state(port, DOT1X_CP_RETIRE);
      update_secy_ctrl(port);
      set_orx(port, 0);
      delete_sas(port, &port->oki);
      set_suspend_on_request(port, intf->ctrl.suspend_on_request);
      set_suspend_for(port, intf->ctrl.suspend_for);
      intf->cp_changed = 1;
    }
  else
  if (port->cp_state == DOT1X_CP_INIT)
    {
        set_protect_frames(port, intf->ctrl.macsec_protect);
        set_replay_protect(port, intf->ctrl.macsec_replay_protect);
        set_validate_frames(port, intf->ctrl.macsec_validate);
        update_secy_ctrl(port);
        set_suspend_on_request(port, 0);
        set_suspend_for(port, 0);
    }
  else
  if (port->cp_state == DOT1X_CP_RETIRE)
    {
      /* handle management control changes in the stable states */
      set_protect_frames(port, intf->ctrl.macsec_protect);
      set_replay_protect(port, intf->ctrl.macsec_replay_protect);
      set_validate_frames(port, intf->ctrl.macsec_validate);
      update_secy_ctrl(port);
      set_suspend_on_request(port, intf->ctrl.suspend_on_request);
      set_suspend_for(port, intf->ctrl.suspend_for);
    }
}

void
tick_port(Dot1xPort *port)
{
  if (port->transmit_when_ticks && --port->transmit_when_ticks == 0)
    dot1x_debug_cp(port, "transmitWhen = 0");

  if (port->retire_when_ticks && --port->retire_when_ticks == 0)
    dot1x_debug_cp(port, "retireWhen = 0");
}

static void
set_state(Dot1xPort *port, int value)
{
  port->cp_state = value;

  dot1x_debug_cp(port, "CP state = %s", dot1x_cp_state(port->cp_state));
}

static void
set_controlled_port_enabled(Dot1xPort *port, unsigned value)
{
  /* skip virtual port that has no MACsec-level virtual port */
  if (port != &port->intf->rport && port->peer_count <= 0)
    return;

  if (port->controlled_port_enabled == value)
    return;

  port->controlled_port_enabled = value;
  dot1x_debug_cp_secy(
    port, "controlledPortEnabled = %s",
    dot1x_bool(port->controlled_port_enabled));

  macsec_set_controlled_port_enabled(
    port->intf->macsec_intf, port->portid, port->controlled_port_enabled);
}

static void
set_port_valid(Dot1xPort *port, unsigned value)
{
  if (port->port_valid == value)
    return;

  port->port_valid = value;
  dot1x_debug_cp_client(port, "portValid = %s", dot1x_bool(port->port_valid));
}

static void
if_secure(Dot1xPort *port, unsigned value, const MacsecAuthData *auth_data)
{
  /* skip virtual port that has no MACsec-level virtual port */
  if (port != &port->intf->rport && port->peer_count <= 0)
    return;

  if (!auth_data)
    {
      dot1x_debug_cp_client(port, "ifSecure(%s)", dot1x_bool(value));
      macsec_set_authorization(
        port->intf->macsec_intf, port->portid, &empty_auth_data);
    }
  else
    {
      dot1x_debug_cp_client(port, "ifSecure(%s, authData)", dot1x_bool(value));
      macsec_set_authorization(
        port->intf->macsec_intf, port->portid, auth_data);
    }
}

static void
set_protect_frames(Dot1xPort *port, unsigned value)
{
  if (port->protect_frames == value)
    return;

  port->protect_frames = value;
  dot1x_debug_cp_secy(
    port, "protectFrames = %s", dot1x_bool(port->protect_frames));

  /* mark SecY controls out-of-date */
  port->secy_updated = 0;
}

static void
set_replay_protect(Dot1xPort *port, unsigned value)
{
  if (port->replay_protect == value)
    return;

  port->replay_protect = value;
  dot1x_debug_cp_secy(
    port, "replayProtect = %s", dot1x_bool(port->replay_protect));

  /* mark SecY controls out-of-date */
  port->secy_updated = 0;
}

static void
set_validate_frames(Dot1xPort *port, MacsecValidateFrames value)
{
  if (port->validate_frames == value)
    return;

  port->validate_frames = value;
  dot1x_debug_cp_secy(
    port, "validateFrames = %s", dot1x_validate_frames(port->validate_frames));

  /* mark SecY controls out-of-date */
  port->secy_updated = 0;
}

static void
set_oki(Dot1xPort *port, const Dot1xKeyIdentifier *value)
{
  Dot1xFormatBuffer fb;

  if (!memcmp(&port->oki, value, sizeof port->oki))
    return;

  memcpy(&port->oki, value, sizeof port->oki);
  dot1x_debug_mka_cp(
    port, "oki = %s", dot1x_format_key_identifier(&fb, &port->oki));
}

static void
set_oan(Dot1xPort *port, unsigned value)
{
  if (port->oan == value)
    return;

  port->oan = value;
  dot1x_debug_mka_cp(port, "oan = %u", port->oan);
}

static void
set_orx(Dot1xPort *port, unsigned value)
{
  if (port->orx == value)
    return;

  port->orx = value;
  dot1x_debug_mka_cp(port, "orx = %s", dot1x_bool(port->orx));
}

static void
set_otx(Dot1xPort *port, unsigned value)
{
  if (port->otx == value)
    return;

  port->otx = value;
  dot1x_debug_mka_cp(port, "otx = %s", dot1x_bool(port->otx));
}

static void
set_lki(Dot1xPort *port, const Dot1xKeyIdentifier *value)
{
  Dot1xFormatBuffer fb;

  if (!memcmp(&port->lki, value, sizeof port->lki))
    return;

  memcpy(&port->lki, value, sizeof port->lki);
  dot1x_debug_mka_cp(
    port, "lki = %s", dot1x_format_key_identifier(&fb, &port->lki));
}

static void
set_lan(Dot1xPort *port, unsigned value)
{
  if (port->lan == value)
    return;

  port->lan = value;
  dot1x_debug_mka_cp(port, "lan = %u", port->lan);
}

static void
set_lrx(Dot1xPort *port, unsigned value)
{
  if (port->lrx == value)
    return;

  port->lrx = value;
  dot1x_debug_mka_cp(port, "lrx = %s", dot1x_bool(port->lrx));
}

static void
set_ltx(Dot1xPort *port, unsigned value)
{
  if (port->ltx == value)
    return;

  port->ltx = value;
  dot1x_debug_mka_cp(port, "ltx = %s", dot1x_bool(port->ltx));
}

static void
set_chgd_server(Dot1xPort *port, unsigned value)
{
  if (port->chgd_server == value)
    return;

  port->chgd_server = value;
  dot1x_debug_mka_cp(port, "chgdServer = %s", dot1x_bool(port->chgd_server));
}

static void
set_new_sak(Dot1xPort *port, unsigned value)
{
  if (port->new_sak == value)
    return;

  port->new_sak = value;
  dot1x_debug_mka_cp(port, "newSAK = %s", dot1x_bool(port->new_sak));
}

static void
set_new_info(Dot1xPort *port, unsigned value)
{
  if (port->new_info == value)
    return;

  port->new_info = value;
  dot1x_debug_mka_cp(port, "newInfo = %s", dot1x_bool(port->new_info));
}

static void
set_current_cipher_suite(Dot1xPort *port, MacsecCipherSuite value)
{
  if (port->current_cipher_suite == value)
    return;

  port->current_cipher_suite = value;
  dot1x_debug_cp_secy(
    port, "currentCipherSuite = %s",
    dot1x_cipher_suite(port->current_cipher_suite));

  /* mark SecY controls out-of-date */
  port->secy_updated = 0;
}

static void
set_confidentiality_offset(Dot1xPort *port, MacsecConfidentialityOffset value)
{
  if (port->confidentiality_offset == value)
    return;

  port->confidentiality_offset = value;
  dot1x_debug_cp_secy(
    port, "confidentialityOffset = %s",
    dot1x_confidentiality_offset(port->confidentiality_offset));

  /* mark SecY controls out-of-date */
  port->secy_updated = 0;
}

static void
set_transmit_when(Dot1xPort *port, unsigned value)

{
  if (port->transmit_when_ticks == value * DOT1X_TICKS_PER_SECOND)
    return;

  port->transmit_when_ticks = value * DOT1X_TICKS_PER_SECOND;

  dot1x_debug_cp(
    port, "transmitWhen = %u",
    port->transmit_when_ticks / DOT1X_TICKS_PER_SECOND);
}

static void
set_retire_when(Dot1xPort *port, unsigned value)

{
  if (port->retire_when_ticks == value * DOT1X_TICKS_PER_SECOND)
    return;

  port->retire_when_ticks = value * DOT1X_TICKS_PER_SECOND;

  dot1x_debug_cp(
    port, "retireWhen = %u",
    port->retire_when_ticks / DOT1X_TICKS_PER_SECOND);
}

static void
create_sas(Dot1xPort *port)
{
  dot1x_debug_mka_cp(port, "createSAs()");

  dot1x_create_sas(port);
}

static void
enable_receive_sas(Dot1xPort *port)
{
  dot1x_debug_mka_cp(port, "enableReceiveSAs()");

  dot1x_enable_receive_sas(port);
}

static void
enable_transmit_sa(Dot1xPort *port)
{
  dot1x_debug_mka_cp(port, "enableTransmitSA()");

  dot1x_enable_transmit_sa(port);
}

static void
delete_sas(Dot1xPort *port, const Dot1xKeyIdentifier *value)
{
  (void)value;

  dot1x_debug_mka_cp(port, "deleteSAs()");

  dot1x_delete_sas(port);
}

static void
update_secy_ctrl(Dot1xPort *port)
{
  MacsecSecyCtrl ctrl;

  if (port->secy_updated)
    return;

  /* skip virtual port that has no MACsec-level virtual port */
  if (port != &port->intf->rport && port->peer_count <= 0)
    return;

  macsec_get_secy_ctrl(port->intf->macsec_intf, port->portid, &ctrl);
  ctrl.validate_frames = port->validate_frames;
  ctrl.replay_protect = port->replay_protect;
  ctrl.protect_frames = port->protect_frames;
  ctrl.current_cipher_suite = port->current_cipher_suite;
  ctrl.confidentiality_offset = port->confidentiality_offset;
  macsec_set_secy_ctrl(port->intf->macsec_intf, port->portid, &ctrl);

  port->secy_updated = 1;
}

static void
set_suspend_for(Dot1xPort *port, unsigned value)
{
  if (port->suspend_for == value)
    return;

  port->suspend_for = value;
  dot1x_debug_mka_cp(
    port, "suspendFor = %u", port->suspend_for);
  port->intf->cp_changed = 1;
}

static void
set_suspend_on_request(Dot1xPort *port, unsigned value)
{
  if (port->suspend_on_request == value)
    return;

  port->suspend_on_request = value;
  dot1x_debug_mka_cp(
    port, "suspendOnRequest = %s", dot1x_bool(port->suspend_on_request));
  port->intf->cp_changed = 1;
}
