/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/printk.h>
#include <zephyr/kernel.h>
#include <sys/byteorder.h>
#include <stdlib.h>
#include <zephyr.h>
#include <random/rand32.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/gatt_dm.h>

#include "lenkki.h"
#include "GNSS_compute.h"
#include "blowfish.h"

#define HS_ENCRYPTION

static struct k_work start_advertising_worker;

static struct bt_le_ext_adv *adv;

static struct S_hs_adv
{
	uint8_t hs_UUID[16];
	uint16_t uid_l;
	uint8_t lat_h;
	uint8_t lat_lon;
	uint8_t lon_h;
	uint8_t flags;
	uint8_t cnt;
	uint8_t dir_speed;
	uint8_t bark_lm;
	uint8_t bark;
	uint16_t uid_h;
	uint8_t spare;
} hs_adv = {.hs_UUID = {0x95, 0x84, 0x58, 0x55, 0xE9, 0x37, 0x37, 0xE2, 0xCC, 0xAB, 0x91, 0x68, 0x36, 0xCE, 0x17, 0x96}, .uid_l = 0x4182,
  .lat_h = 0x50, .lat_lon = 0, .lon_h = 0x50, .flags = 0xfc, .cnt = 1, .dir_speed = 0, .uid_h = 0, .bark_lm = 0, .bark = 0, .spare = 0x5A}; 

static struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_SVC_DATA128, &hs_adv, 29),
};

static int create_advertising_coded(void)
{
	int err;
	struct bt_le_adv_param param =
		BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_EXT_ADV |
				     BT_LE_ADV_OPT_CODED,
				    //  BT_GAP_ADV_SLOW_INT_MIN,
				    //  BT_GAP_ADV_SLOW_INT_MAX,
					0x0190,
					0x01E0,
				    NULL);

	err = bt_le_ext_adv_create(&param, NULL, &adv);
	if (err) {
		printk("Failed to create advertiser set (%d)\n", err);
		return err;
	}

	printk("Created adv: %p\n", adv);

	err = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Failed to set advertising data (%d)\n", err);
		return err;
	}

	return 0;
}

static void start_advertising_coded(struct k_work *item)
{
	int err;

	err = bt_le_ext_adv_start(adv, NULL);
	if (err) {
		printk("Failed to start advertising set (%d)\n", err);
		return;
	}

	printk("Advertiser %p set started\n", adv);
}

static void bt_ready(void)
{
	int err = 0;

	printk("Bluetooth initialized\n");

	k_work_init(&start_advertising_worker, start_advertising_coded);

	err = create_advertising_coded();
	if (err) {
		printk("Advertising failed to create (err %d)\n", err);
		return;
	}

	k_work_submit(&start_advertising_worker);
}


void main(void)
{
	static int err;
	static uint8_t i=0;
	static int reittipiste=0;
  	static int reittipisteindex=0;
  	static float lat, lon;
	static int32_t temp_lon, temp_lat;
	static uint8_t tama, dirspeed;
	static uint32_t L = 1, R = 2;
	static BLOWFISH_CTX ctx;
	static uint32_t random_uid = 0x01020304;
	// static uint8_t key[] = "TESTKEY";
	
	// Initialize the Bluetooth Subsystem
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	bt_ready();

	// Generate random UID
	random_uid = sys_rand32_get();
	hs_adv.uid_h = (uint16_t)(random_uid >> 16);
	hs_adv.uid_l = (uint16_t)(random_uid & 0x0000ffff);
	printk("%lX %X %X : ", (long unsigned int)random_uid, hs_adv.uid_h, hs_adv.uid_l);

	k_sleep(K_MSEC(600));	// Just to wait for BLE ad info to print
	// Blowfish encryption/decryption test
   	Blowfish_Init(&ctx, blowfish_key, 7);
 	Blowfish_Encrypt(&ctx, &L, &R);
 	printk("%lX %lX\n", (long unsigned int)L, (long unsigned int)R);
#ifdef TEST_ENCRYPTION
  	if (L == 0xA0A51EF2L && R == 0x35AE0591L)
	{
		printk("Test encryption OK.\n");
  	} else {
		printk("Test encryption failed.\n");
	}
	Blowfish_Decrypt(&ctx, &L, &R);
  	if (L == 1 && R == 2)
	{
  		printk("Test decryption OK.\n");
	} else {
		printk("Test decryption failed.\n");
	}
#endif

	// *** Main loop	***
	do {

	 	k_sleep(K_MSEC(1000));

	    lat = reitti[reittipiste][reittipisteindex*4];
	    lon = reitti[reittipiste][reittipisteindex*4+1];
		temp_lon = GNSS_Compute_lon(lon);
		temp_lat = GNSS_Compute_lat(lat);
		// printk( "lon: %f,   %x,   %d\n", lon, temp_lon, temp_lon);
		// printk( "lat: %f,   %x,   %d\n", lat, temp_lat, temp_lat);
		tama =	(uint8_t)(temp_lat>>7);
		hs_adv.lat_h = tama;
		// printk( "  %x", tama);
		tama = (uint8_t)(temp_lon>>7);
		// printk( "  %x", tama);
		hs_adv.lon_h = tama;
		tama = (((uint8_t)(temp_lon>>3))&0x0F) | ((((uint8_t)(temp_lat>>3))&0x0F)<<4);
		// printk( "  %x\n", tama);
		hs_adv.lat_lon = tama;
		// prepare packet for encryption

		hs_adv.flags = 0xfc;
		hs_adv.cnt = i++;

		dirspeed = (uint8_t)reitti[reittipiste][reittipisteindex*4+3];
		dirspeed = dirspeed << 5;
		dirspeed = dirspeed + (uint8_t)reitti[reittipiste][reittipisteindex*4+2];
		hs_adv.dir_speed = dirspeed;
		// printk("%0X %0X : %0X\n", reittipiste, reittipisteindex, dirspeed);

		hs_adv.bark_lm = 2;
		hs_adv.bark = 0x31;
		hs_adv.spare = 0x5A;

#ifdef HS_ENCRYPTION
		// Rearrenge data for encryption	ToDo! Make function that can use pointer directly for encryption
		L = (hs_adv.flags<<24) + (hs_adv.cnt<<16) + (hs_adv.dir_speed<<8) + hs_adv.bark_lm;
		R = (hs_adv.bark<<24) + (hs_adv.uid_h<<8) + hs_adv.spare;
	 	// printk("%0lX %0lX\n", (long unsigned int)L, (long unsigned int)R);
		Blowfish_Encrypt(&ctx, &L, &R);
	 	// printk("%0lX %0lX\n", (long unsigned int)L, (long unsigned int)R);
		// Rearrange  data from the encryption
		hs_adv.flags = (uint8_t)(L>>24);
		hs_adv.cnt = (uint8_t)(L>>16);
		hs_adv.dir_speed = (uint8_t)(L>>8); 
		hs_adv.bark_lm = (uint8_t)L;
		hs_adv.bark = (uint8_t)(R>>24);
		hs_adv.uid_h = (uint16_t)(R>>8);
		hs_adv.spare = (uint8_t)R;
	 	// printk("%02X%02X%02X%02X ", hs_adv.flags, hs_adv.cnt, hs_adv.dir_speed, hs_adv.bark_lm);
	 	// printk("%02X%04X%02X rec\n", hs_adv.bark, hs_adv.uid_h, hs_adv.spare);
#endif

		bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);

	    if (reittipisteindex++ > 2)
		{
			reittipiste++;
			reittipisteindex = 0;
			if (reittipiste > 254) reittipiste = 0;
		}

	} while (1);	
}
