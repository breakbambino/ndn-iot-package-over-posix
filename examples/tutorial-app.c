/*
 * Copyright (C) 2019
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v3.0. See the file LICENSE in the top level
 * directory for more details.
 *
 * See AUTHORS.md for complete list of NDN IOT PKG authors and contributors.
 */
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ndn-lite.h>
#include "ndn-lite/encode/name.h"
#include "ndn-lite/encode/data.h"
#include "ndn-lite/encode/interest.h"
#include "ndn-lite/app-support/service-discovery.h"
#include "ndn-lite/app-support/access-control.h"
#include "ndn-lite/app-support/security-bootstrapping.h"
#include "ndn-lite/app-support/ndn-sig-verifier.h"
#include "ndn-lite/app-support/pub-sub.h"
#include "ndn-lite/encode/key-storage.h"
#include "ndn-lite/encode/ndn-rule-storage.h"

// DEVICE manufacture-created private key
uint8_t secp256r1_prv_key_bytes[32] = {0};

// HERE TO SET pre-shared public key
uint8_t secp256r1_pub_key_bytes[64] = {0};

//HERE TO SET pre-shared secrets
uint8_t hmac_key_bytes[16] = {0};

// Device identifer
char device_identifier[30];
size_t device_len;

// Face Declare
// ndn_udp_face_t *face;
ndn_unix_face_t *face;
// Buf used in this program
uint8_t buf[4096];
// Wether the program is running or not
bool running;
// A global var to keep the brightness
uint8_t light_brightness = 0;

static ndn_trust_schema_rule_t same_room;
static ndn_trust_schema_rule_t controller_only;

int
load_bootstrapping_info()
{
  FILE * fp;
  char buf[255];
  char* buf_ptr;
  fp = fopen("../devices/tutorial_shared_info-398.txt", "r");
  if (fp == NULL) exit(1);
  size_t i = 0;
  for (size_t lineindex = 0; lineindex < 4; lineindex++) {
    memset(buf, 0, sizeof(buf));
    buf_ptr = buf;
    fgets(buf, sizeof(buf), fp);
    if (lineindex == 0) {
      for (i = 0; i < 32; i++) {
        sscanf(buf_ptr, "%2hhx", &secp256r1_prv_key_bytes[i]);
        buf_ptr += 2;
      }
    }
    else if (lineindex == 1) {
      buf[strlen(buf) - 1] = '\0';
      strcpy(device_identifier, buf);
    }
    else if (lineindex == 2) {
      for (i = 0; i < 64; i++) {
        sscanf(buf_ptr, "%2hhx", &secp256r1_pub_key_bytes[i]);
        buf_ptr += 2;
      }
    }
    else {
      for (i = 0; i < 16; i++) {
        sscanf(buf_ptr, "%2hhx", &hmac_key_bytes[i]);
        buf_ptr += 2;
      }
    }
  }
  fclose(fp);

  // prv key
  printf("Pre-installed ECC Private Key:");
  for (int i = 0; i < 32; i++) {
    printf("%02X", secp256r1_prv_key_bytes[i]);
  }
  printf("\nPre-installed Device Identifier: ");
  // device id
  printf("%s\nPre-installed ECC Pub Key: ", device_identifier);
  // pub key
  for (int i = 0; i < 64; i++) {
    printf("%02X", secp256r1_pub_key_bytes[i]);
  }
  printf("\nPre-installed Shared Secret: ");
  // hmac key
  for (int i = 0; i < 16; i++) {
    printf("%02X", hmac_key_bytes[i]);
  }
  printf("\n");
  return 0;
}

void
on_light_command(const ps_event_context_t* context, const ps_event_t* event, void* userdata)
{
  printf("RECEIVED NEW COMMAND\n");
  printf("Command id: %.*s\n", event->data_id_len, event->data_id);
  printf("Command payload: %.*s\n", event->payload_len, event->payload);
  printf("Scope: %s\n", context->scope);

  int new_val;
  // Execute the function
  if (event->payload) {
    // new_val = *real_payload;
    char content_str[128] = {0};
    memcpy(content_str, event->payload, event->payload_len);
    content_str[event->payload_len] = '\0';
    new_val = atoi(content_str);
  }
  else {
    new_val = 0xFF;
  }
  if (new_val != 0xFF) {
    if ((new_val > 0) != (light_brightness > 0)) {
      if (new_val > 0) {
        printf("Switch on the light.\n");
      }
      else {
        printf("Turn off the light.\n");
      }
    }
    if (new_val < 10) {
      light_brightness = new_val;
      if (light_brightness > 0) {
        printf("Successfully set the brightness = %u\n", light_brightness);
        ps_event_t data_content = {
          .data_id = "a",
          .data_id_len = strlen("a"),
          .payload = &light_brightness,
          .payload_len = 1
        };
        ps_publish_content(NDN_SD_LED, &data_content);
      }
    }
    else {
      light_brightness = 10;
      printf("Exceeding range. Set the brightness = %u\n", light_brightness);
    }
  }
  else {
    printf("Query the brightness = %u\n", light_brightness);
  }
}

void periodic_publish(size_t param_size, uint8_t* param_value) {
  static ndn_time_ms_t last;
  // ps_event_t event = {
  //   .data_id = "a",
  //   .data_id_len = strlen("a"),
  //   .payload = "hello",
  //   .payload_len = strlen("hello")
  // };



/**
 * TLV_Type_Policy
 * TLV_Length_Policy
 *      TLV_Type_DataRule
 *      TLV_Length_DataRule
 *      TLV_Value_DataRule
 * 
 *      TLV_Type_KeyRule
 *      TLV_Length_KeyRule
 *      TLV_Value_KeyRule
 * 
 */

  uint8_t buffer[200];
  ndn_encoder_t encoder;


  // encode datarule
  encoder_init(&encoder, buffer, sizeof(buffer));
  encoder_append_type(&encoder, TLV_POLICY_DATARULE);
  const char* data_string = "(<>)(<>)<DATA>(<>)(<>)<>*";
  encoder_append_length(&encoder, strlen(data_string) + 1);
  encoder_append_raw_buffer_value(&encoder, data_string, strlen(data_string) + 1);

  // encode keyrule
  encoder_append_type(&encoder, TLV_POLICY_KEYRULE);
  const char* key_string = "\\0\\1\\2\\3<KEY><>";
  encoder_append_length(&encoder, strlen(key_string) + 1);
  encoder_append_raw_buffer_value(&encoder, key_string, strlen(key_string) + 1);


  // ps_event_t event = {
  //   .data_id = "a",
  //   .data_id_len = strlen("a"),
  //   .payload = "dfcgvhbjk",
  //   .payload_len = strlen("dfcgvhbjk")
  // };

  int ret_val = -1;
  uint32_t probe_1, probe_2;;
  ndn_decoder_t decoder;
  ndn_trust_schema_rule_t rule;
  decoder_init(&decoder, buffer, sizeof(buffer));
  ret_val = decoder_get_type(&decoder, &probe_1);
  if (ret_val != NDN_SUCCESS || probe_1 != TLV_POLICY_DATARULE) {
    printf("policy datarule type not correct, probe_1 = %d\n", probe_1);
  }

  ret_val = decoder_get_length(&decoder, &probe_1);
  if (ret_val != NDN_SUCCESS) {
    printf("policy datarule length not correct\n");  
  }

  uint8_t datarule[40], keyrule[40];
  ret_val = decoder_get_raw_buffer_value(&decoder, datarule, probe_1);


  ret_val = decoder_get_type(&decoder, &probe_2);
  ret_val = decoder_get_length(&decoder, &probe_2);
  ret_val = decoder_get_raw_buffer_value(&decoder, keyrule, probe_2);


  ret_val = ndn_trust_schema_rule_from_strings(&rule, (char*)datarule, probe_1, (char*)keyrule, probe_2);
  if (ret_val != NDN_SUCCESS) {
    printf("constuct, error code is %d\n", ret_val); 
  }

  // update or create the "default" rule
  ndn_trust_schema_rule_t* query = ndn_rule_storage_get_rule("default");
  if (query) {
      ret_val = ndn_trust_schema_pattern_from_string(&query->data_pattern, (char*)datarule, probe_1);
      ret_val = ndn_trust_schema_pattern_from_string(&query->data_pattern, (char*)keyrule, probe_2);
      if (ret_val != 0) {
         printf("pattern, construct, error code is %d\n", ret_val); 
        //return ret_val;
      }
  }

  else {
    ret_val = ndn_rule_storage_add_rule("default", &rule);
    if (ret_val != 0) {
      printf("add, error code is %d\n", ret_val); 
      //return ret_val;
    }
  }

  ps_event_t event = {
    .data_id = "a",
    .data_id_len = strlen("a"),
    .payload = buffer,
    .payload_len = encoder.offset
  };



  if (ndn_time_now_ms() - last >= 400000) {
    ps_publish_content(NDN_SD_LED, &event);
    last = ndn_time_now_ms();
  }
  ndn_msgqueue_post(NULL, periodic_publish, 0, NULL);
}

void
after_bootstrapping()
{
 // ps_subscribe_to_command(NDN_SD_LED, "", on_light_command, NULL);
  uint64_t interval = 4000;
  periodic_publish(sizeof(interval), &interval);
}

void SignalHandler(int signum){
  running = false;
}

int
main(int argc, char *argv[])
{
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  signal(SIGQUIT, SignalHandler);

  // PARSE COMMAND LINE PARAMETERS
  int ret = NDN_SUCCESS;
  if ((ret = load_bootstrapping_info()) != 0) {
    return ret;
  }
  ndn_lite_startup();

  // CREAT A MULTICAST FACE
  face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR, true);
  // face = ndn_udp_unicast_face_construct(INADDR_ANY, htons((uint16_t) 2000), inet_addr("224.0.23.170"), htons((uint16_t) 56363));
  // in_port_t multicast_port = htons((uint16_t) 56363);
  // in_addr_t multicast_ip = inet_addr("224.0.23.170");
  // face = ndn_udp_multicast_face_construct(INADDR_ANY, multicast_ip, multicast_port);

  // LOAD SERVICES PROVIDED BY SELF DEVICE
  uint8_t capability[1];
  capability[0] = NDN_SD_LED;

  // SET UP SERVICE DISCOVERY
  sd_add_or_update_self_service(NDN_SD_LED, true, 1); // state code 1 means normal
  ndn_ac_register_encryption_key_request(NDN_SD_LED);
  //ndn_ac_register_access_request(NDN_SD_LED);

  // START BOOTSTRAPPING
  ndn_bootstrapping_info_t booststrapping_info = {
    .pre_installed_prv_key_bytes = secp256r1_prv_key_bytes,
    .pre_installed_pub_key_bytes = secp256r1_pub_key_bytes,
    .pre_shared_hmac_key_bytes = hmac_key_bytes,
  };
  ndn_device_info_t device_info = {
    .device_identifier = device_identifier,
    .service_list = capability,
    .service_list_size = sizeof(capability),
  };
  ndn_security_bootstrapping(&face->intf, &booststrapping_info, &device_info, after_bootstrapping);

  // START MAIN LOOP
  running = true;
  while(running) {
    ndn_forwarder_process();
    usleep(100);
  }

  // DESTROY FACE
  ndn_face_destroy(&face->intf);
  return 0;
}
