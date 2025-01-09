#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_ether.h>
#include <unistd.h>
#include <gtOs/gtEnvDep.h>
#include <string.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/events/cpssGenEventRequests.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include "syslog.h"
#include "eag6l.h"
#include "esmc_types.h"


/* 목적지 MAC 주소 (Ethernet Multicast) */
#if 0
uint8_t ESMC_DA[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02};
#endif
uint8_t eag_src[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};

extern int8_t gLocalQL;

extern GT_STATUS gtAppDemoOamSdmaTxPacketSend
(
 IN GT_U8    vid,
 IN GT_U8    *packetDataPtr,
 IN GT_U32   packetDataLength,
 IN GT_U32   dstPortNum
 );

static const unsigned char g_dst_addr[ETH_ALEN] = ESMC_PDU_IEEE_SLOW_PROTO_MCAST_ADDR;
static const unsigned char g_itu_oui[ESMC_PDU_ITU_OUI_LEN] = ESMC_PDU_ITU_OUI;
static const unsigned char g_itu_subtype[ESMC_PDU_ITU_SUBTYPE_LEN] = ESMC_PDU_ITU_SUBTYPE;
static const unsigned char g_ql_tlv_length[ESMC_PDU_QL_TLV_LENGTH_LEN] = ESMC_PDU_QL_TLV_LENGTH;
static const unsigned char g_ext_ql_tlv_length[ESMC_PDU_EXT_QL_TLV_LENGTH_LEN] = ESMC_PDU_EXT_QL_TLV_LENGTH;

static int esmc_ql_to_ssm_and_e_ssm_map(T_esmc_network_option net_opt, T_esmc_ql ql, unsigned char *ssm_code, unsigned char *e_ssm_code)
{
  switch(net_opt) {
    case E_esmc_network_option_1:
      switch(ql) {
        case E_esmc_ql_net_opt_1_ePRTC:
          *ssm_code = 0x02;
          *e_ssm_code = 0x21;
          break;
        case E_esmc_ql_net_opt_1_PRTC:
          *ssm_code = 0x02;
          *e_ssm_code = 0x20;
          break;
        case E_esmc_ql_net_opt_1_ePRC:
          *ssm_code = 0x02;
          *e_ssm_code = 0x23;
          break;
        case E_esmc_ql_net_opt_1_PRC:
          *ssm_code = 0x02;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_SSUA:
          *ssm_code = 0x04;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_SSUB:
          *ssm_code = 0x08;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_eSEC:
          *ssm_code = 0x0B;
          *e_ssm_code = 0x22;
          break;
        case E_esmc_ql_net_opt_1_SEC:
          *ssm_code = 0x0B;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_DNU:
          *ssm_code = 0x0F;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV0:
          *ssm_code = 0x00;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV1:
          *ssm_code = 0x01;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV3:
          *ssm_code = 0x03;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV5:
          *ssm_code = 0x05;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV6:
          *ssm_code = 0x06;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV7:
          *ssm_code = 0x07;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV9:
          *ssm_code = 0x09;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV10:
          *ssm_code = 0x0A;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV12:
          *ssm_code = 0x0C;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV13:
          *ssm_code = 0x0D;
          *e_ssm_code = 0xFF;
          break;
        case E_esmc_ql_net_opt_1_INV14:
          *ssm_code = 0x0E;
          *e_ssm_code = 0xFF;
          break;
        default:
          return -1;
      }
      break;

    case E_esmc_network_option_2:
      switch(ql) {
      case E_esmc_ql_net_opt_2_ePRTC:
        *ssm_code = 0x01;
        *e_ssm_code = 0x21;
        break;
      case E_esmc_ql_net_opt_2_PRTC:
        *ssm_code = 0x01;
        *e_ssm_code = 0x20;
        break;
      case E_esmc_ql_net_opt_2_ePRC:
        *ssm_code = 0x01;
        *e_ssm_code = 0x23;
        break;
      case E_esmc_ql_net_opt_2_PRS:
        *ssm_code = 0x01;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_STU:
        *ssm_code = 0x00;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_ST2:
        *ssm_code = 0x07;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_TNC:
        *ssm_code = 0x4;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_ST3E:
        *ssm_code = 0x0D;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_eEEC:
        *ssm_code = 0x0A;
        *e_ssm_code = 0x22;
        break;
      case E_esmc_ql_net_opt_2_ST3:
        *ssm_code = 0x0A;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_SMC:
        *ssm_code = 0x0C;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_PROV:
        *ssm_code = 0x0E;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_DUS:
        *ssm_code = 0x0F;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_INV2:
        *ssm_code = 0x02;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_INV3:
        *ssm_code = 0x03;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_INV5:
        *ssm_code = 0x05;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_INV6:
        *ssm_code = 0x06;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_INV8:
        *ssm_code = 0x08;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_INV9:
        *ssm_code = 0x09;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_2_INV11:
        *ssm_code = 0x0B;
        *e_ssm_code = 0xFF;
        break;
      default:
        return -1;
      }
      break;
  
    case E_esmc_network_option_3:
      switch(ql) {
      case E_esmc_ql_net_opt_3_UNK:
        *ssm_code = 0x00;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_SEC:
        *ssm_code = 0x0B;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV1:
        *ssm_code = 0x01;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV2:
        *ssm_code = 0x02;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV3:
        *ssm_code = 0x03;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV4:
        *ssm_code = 0x04;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV5:
        *ssm_code = 0x05;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV6:
        *ssm_code = 0x06;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV7:
        *ssm_code = 0x07;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV8:
        *ssm_code = 0x08;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV9:
        *ssm_code = 0x09;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV10:
        *ssm_code = 0x0A;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV12:
        *ssm_code = 0x0C;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV13:
        *ssm_code = 0x0D;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV14:
        *ssm_code = 0x0E;
        *e_ssm_code = 0xFF;
        break;
      case E_esmc_ql_net_opt_3_INV15:
        *ssm_code = 0x0F;
        *e_ssm_code = 0xFF;
        break;
      default:
        return -1;
      }
      break;

    default:
      return -1;
  }

  return 0;
}

void generate_clock_id(const unsigned char mac_addr[ETH_ALEN], unsigned char clock_id[MAX_CLK_ID_LEN])
{
	clock_id[0] = mac_addr[0];
	clock_id[1] = mac_addr[1];
	clock_id[2] = mac_addr[2];
	clock_id[3] = 0xFF;
	clock_id[4] = 0xFE;
	clock_id[5] = mac_addr[3];
	clock_id[6] = mac_addr[4];
	clock_id[7] = mac_addr[5];
}

/* ESMC 패킷 생성 함수 */
void generate_esmc_packet(T_esmc_network_option net_opt, T_esmc_pdu *pkt, uint8_t *src_mac, T_esmc_ql ql) {
  int off = ESMC_PDU_ETH_HDR_LEN;
  uint8_t ssm_code, e_ssm_code;
  T_port_ext_ql_tlv_data ext_ql_tlv_data;

  generate_clock_id(src_mac, ext_ql_tlv_data.originator_clock_id);
  ext_ql_tlv_data.num_cascaded_eEEC = 1;
  ext_ql_tlv_data.num_cascaded_EEC = 0;
  ext_ql_tlv_data.partial_chain = 0;

  /* ethernet header 설정 */
  memcpy(pkt->eth_hdr.h_dest, g_dst_addr, ETH_ALEN);
  memcpy(pkt->eth_hdr.h_source, src_mac, ETH_ALEN);
  pkt->eth_hdr.h_proto = htons(ETH_P_SLOW);

  pkt->slow_proto_subtype = ESMC_PDU_SLOW_PROTO_SUBTYPE;
  off += ESMC_PDU_SLOW_PROTO_SUBTYPE_LEN;

  memcpy(pkt->itu_oui, g_itu_oui, sizeof(g_itu_oui));
  off += ESMC_PDU_ITU_OUI_LEN;

  memcpy(pkt->itu_subtype, g_itu_subtype, sizeof(g_itu_subtype));
  off += ESMC_PDU_ITU_SUBTYPE_LEN;

  pkt->version_event_flag_reserved = (1 << ESMC_PDU_EVENT_FLAG_LSB) | (ESMC_PDU_VER << ESMC_PDU_VER_LSB);
  off += ESMC_PDU_VER_EVENT_LEN;

  off += ESMC_PDU_RSVD_LEN;

  esmc_ql_to_ssm_and_e_ssm_map(net_opt, ql, &ssm_code, &e_ssm_code);

  pkt->ql_tlv.type = ESMC_PDU_QL_TLV_TYPE;
  memcpy(pkt->ql_tlv.length, g_ql_tlv_length, ESMC_PDU_QL_TLV_LENGTH_LEN);
  pkt->ql_tlv.ssm_code = ssm_code;
  off += ESMC_PDU_QL_TLV_LEN;

  pkt->ext_ql_tlv.type = ESMC_PDU_EXT_QL_TLV_TYPE;
  memcpy(pkt->ext_ql_tlv.length, g_ext_ql_tlv_length, ESMC_PDU_EXT_QL_TLV_LENGTH_LEN);
  pkt->ext_ql_tlv.esmc_e_ssm_code = e_ssm_code;

  memcpy(pkt->ext_ql_tlv.syncE_clock_id, ext_ql_tlv_data.originator_clock_id, ESMC_PDU_EXT_QL_TLV_SYNCE_CLOCK_ID_LEN);

  pkt->ext_ql_tlv.flag = ((ext_ql_tlv_data.partial_chain << ESMC_PDU_EXT_QL_TLV_PARTIAL_CHAIN_FLAG_LSB) | (ext_ql_tlv_data.mixed_EEC_eEEC << ESMC_PDU_EXT_QL_TLV_MIXED_EEC_EEEC_FLAG_LSB));
  pkt->ext_ql_tlv.num_cascaded_eEEC = ext_ql_tlv_data.num_cascaded_eEEC;
  pkt->ext_ql_tlv.num_cascaded_EEC = ext_ql_tlv_data.num_cascaded_EEC;

  memset(pkt->ext_ql_tlv.reserved, 0, ESMC_PDU_EXT_QL_TLV_RSVD_LEN);
  off += ESMC_PDU_PADDING_LEN;
}

GT_STATUS esmc_packet_transmission(GT_U8 port, GT_U8 ssm_code)
{
	GT_STATUS	status = GT_OK;
	GT_U8		packetBuffs[128] = {0, }; /* Buffers of packet */ 
	GT_U8          *buffList[1];
	T_esmc_network_option net_opt = 0;

	if(port == 0xff)
		return status;

	if(gLocalQL)
		net_opt = gLocalQL;
	else
		net_opt = E_esmc_network_option_1;

	buffList[0] = calloc(128, sizeof(GT_U8));

	generate_esmc_packet(net_opt, (T_esmc_pdu  *)packetBuffs, eag_src, ssm_code); 
	memcpy(buffList[0], packetBuffs, 128);
	status = gtAppDemoOamSdmaTxPacketSend(0, packetBuffs, 128, port);
#ifdef DEBUG
	syslog(LOG_NOTICE, "port %d cpssDxChNetIfSdmaSyncTxPacketSend QL %x rc %d", port, ssm_code, status);
#endif
	free(buffList[0]);
	return status;
}

