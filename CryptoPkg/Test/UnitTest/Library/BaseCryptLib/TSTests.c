/** @file
  Sample Implementation for RFC3161 Time Stamping Verification.

Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "TestBaseCryptLib.h"

//
// Sample Authenticode Data with RFC3161 time stamping signature.
// The data retrieved from one signed sample UEFI image, which is generated by MSFT's signtool
// utility in conjunction with RFC3161 timestamping, as the following command:
//   signtool sign /ac <xxx.cer> / f <xxx.pfx> /p <pass> /fd <digestAlg>
//     /tr http://timestamp.comodoca.com/rfc3161 sample.efi
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 AuthenticodeWithTS[] = {
  0x30, 0x82, 0x0c, 0x00, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0,
  0x82, 0x0b, 0xf1, 0x30, 0x82, 0x0b, 0xed, 0x02, 0x01, 0x01, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x09,
  0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x30, 0x78, 0x06, 0x0a, 0x2b,
  0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x01, 0x04, 0xa0, 0x6a, 0x30, 0x68, 0x30, 0x33, 0x06,
  0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x01, 0x0f, 0x30, 0x25, 0x03, 0x01, 0x00,
  0xa0, 0x20, 0xa2, 0x1e, 0x80, 0x1c, 0x00, 0x3c, 0x00, 0x3c, 0x00, 0x3c, 0x00, 0x4f, 0x00, 0x62,
  0x00, 0x73, 0x00, 0x6f, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x74, 0x00, 0x65, 0x00, 0x3e, 0x00, 0x3e,
  0x00, 0x3e, 0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02,
  0x01, 0x05, 0x00, 0x04, 0x20, 0x1e, 0x9e, 0x74, 0x31, 0xe1, 0x3e, 0x51, 0x46, 0xab, 0xce, 0x10,
  0x0d, 0x7c, 0x38, 0x66, 0x34, 0xd4, 0xdd, 0x04, 0xa5, 0xe7, 0x75, 0x40, 0xdd, 0x99, 0x73, 0xf3,
  0x2a, 0x54, 0x3e, 0xa8, 0x18, 0xa0, 0x82, 0x01, 0xee, 0x30, 0x82, 0x01, 0xea, 0x30, 0x82, 0x01,
  0x57, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x10, 0x2c, 0x65, 0xcf, 0xcf, 0xdd, 0x61, 0x7b, 0xa4,
  0x41, 0xad, 0x26, 0x1b, 0x63, 0xce, 0x91, 0x0f, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02,
  0x1d, 0x05, 0x00, 0x30, 0x13, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x08,
  0x54, 0x65, 0x73, 0x74, 0x52, 0x6f, 0x6f, 0x74, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x34, 0x30, 0x37,
  0x32, 0x38, 0x30, 0x37, 0x33, 0x38, 0x35, 0x39, 0x5a, 0x17, 0x0d, 0x33, 0x39, 0x31, 0x32, 0x33,
  0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x12, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03,
  0x55, 0x04, 0x03, 0x13, 0x07, 0x54, 0x65, 0x73, 0x74, 0x53, 0x75, 0x62, 0x30, 0x81, 0x9f, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81,
  0x8d, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0x94, 0xa6, 0x02, 0x15, 0x87, 0xd6, 0xbf,
  0x71, 0xe8, 0xc6, 0x68, 0xf6, 0x9f, 0x66, 0x09, 0x6c, 0xe7, 0x39, 0x52, 0xf4, 0x4e, 0xaf, 0xf5,
  0xe0, 0xba, 0x0f, 0xfd, 0xe6, 0x77, 0xa9, 0x71, 0x5b, 0x5c, 0x92, 0x50, 0x1d, 0xfd, 0x9b, 0x6e,
  0x52, 0x92, 0x9e, 0x3a, 0x75, 0x86, 0x41, 0x2a, 0x41, 0x30, 0x1b, 0x67, 0x66, 0x91, 0xde, 0x71,
  0x84, 0xe0, 0x90, 0xc3, 0x50, 0x36, 0x78, 0xb5, 0xa0, 0x1e, 0x72, 0xde, 0xe7, 0x66, 0x42, 0x4f,
  0x59, 0x5e, 0x3d, 0xf3, 0x85, 0x82, 0x0b, 0xa8, 0x26, 0x2d, 0xd9, 0xe3, 0x14, 0xda, 0x9d, 0x2e,
  0x3f, 0x53, 0x4d, 0x8d, 0x10, 0xbf, 0xa4, 0x7c, 0xe5, 0xaf, 0x3a, 0xa6, 0xaf, 0x49, 0x64, 0xb0,
  0x60, 0x17, 0x87, 0x71, 0x77, 0x59, 0x52, 0xe5, 0x5a, 0xed, 0x96, 0x7d, 0x7e, 0x5d, 0xc1, 0xef,
  0x6b, 0xfb, 0x80, 0xc5, 0x2b, 0x10, 0xfe, 0xe7, 0xd3, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x48,
  0x30, 0x46, 0x30, 0x44, 0x06, 0x03, 0x55, 0x1d, 0x01, 0x04, 0x3d, 0x30, 0x3b, 0x80, 0x10, 0x19,
  0x8d, 0x48, 0xa1, 0xb9, 0xf3, 0x5e, 0x3c, 0x13, 0xb4, 0x08, 0xb6, 0xd9, 0xf3, 0x4f, 0x0a, 0xa1,
  0x15, 0x30, 0x13, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x08, 0x54, 0x65,
  0x73, 0x74, 0x52, 0x6f, 0x6f, 0x74, 0x82, 0x10, 0x27, 0xcb, 0x16, 0x33, 0x8b, 0xed, 0x4d, 0xa8,
  0x47, 0xf0, 0x86, 0x47, 0x10, 0xef, 0x15, 0xd9, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02,
  0x1d, 0x05, 0x00, 0x03, 0x81, 0x81, 0x00, 0x51, 0x94, 0xed, 0x7a, 0x5c, 0x0b, 0x34, 0x16, 0x9c,
  0xf4, 0x5f, 0x88, 0x16, 0xa8, 0x4b, 0x13, 0xfc, 0xa4, 0x0a, 0xc7, 0xd9, 0x20, 0xb1, 0x93, 0xc5,
  0x81, 0x4f, 0x35, 0x3a, 0x89, 0x10, 0x04, 0xc4, 0xcc, 0x10, 0x34, 0xc3, 0x15, 0x57, 0x06, 0x97,
  0xee, 0x06, 0x2f, 0xf3, 0x24, 0xa1, 0xe6, 0x3a, 0x89, 0x4d, 0xb4, 0x7b, 0x12, 0x87, 0x90, 0x8c,
  0xfc, 0x5b, 0xb0, 0xf0, 0xdd, 0xaa, 0x3a, 0x24, 0x6d, 0x55, 0x47, 0x8a, 0xf2, 0x61, 0x08, 0x7a,
  0x59, 0x5f, 0x6e, 0x7b, 0xcb, 0x34, 0xbe, 0xb6, 0x5d, 0xcb, 0x60, 0xae, 0xc4, 0xda, 0x62, 0xbb,
  0x7f, 0x17, 0x1e, 0x73, 0xd1, 0x4e, 0x9f, 0x6e, 0xd3, 0xc8, 0x35, 0x58, 0x30, 0xd2, 0x89, 0xe5,
  0x22, 0x5e, 0x86, 0xac, 0x7a, 0x56, 0xd6, 0x70, 0xdb, 0x54, 0x10, 0x6c, 0xd3, 0xd5, 0x38, 0xfb,
  0x69, 0xcb, 0x4f, 0x36, 0x83, 0xc2, 0xe8, 0x31, 0x82, 0x09, 0x69, 0x30, 0x82, 0x09, 0x65, 0x02,
  0x01, 0x01, 0x30, 0x27, 0x30, 0x13, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13,
  0x08, 0x54, 0x65, 0x73, 0x74, 0x52, 0x6f, 0x6f, 0x74, 0x02, 0x10, 0x2c, 0x65, 0xcf, 0xcf, 0xdd,
  0x61, 0x7b, 0xa4, 0x41, 0xad, 0x26, 0x1b, 0x63, 0xce, 0x91, 0x0f, 0x30, 0x0d, 0x06, 0x09, 0x60,
  0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0xa0, 0x5e, 0x30, 0x10, 0x06, 0x0a,
  0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x01, 0x0c, 0x31, 0x02, 0x30, 0x00, 0x30, 0x19,
  0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x03, 0x31, 0x0c, 0x06, 0x0a, 0x2b,
  0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x01, 0x04, 0x30, 0x2f, 0x06, 0x09, 0x2a, 0x86, 0x48,
  0x86, 0xf7, 0x0d, 0x01, 0x09, 0x04, 0x31, 0x22, 0x04, 0x20, 0x97, 0x6e, 0x29, 0x47, 0xc4, 0x03,
  0x68, 0x70, 0x1c, 0x99, 0x2c, 0x61, 0xb0, 0xbc, 0xde, 0x77, 0xe1, 0xa1, 0xeb, 0x4c, 0x1c, 0xac,
  0x4c, 0x64, 0xf6, 0x43, 0x96, 0x94, 0x0b, 0xc0, 0xbb, 0x03, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
  0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x81, 0x80, 0x85, 0x93, 0xad, 0x93,
  0x92, 0x9e, 0xa4, 0x94, 0x30, 0x02, 0xe1, 0xc8, 0xcd, 0x37, 0xb2, 0xe1, 0xcb, 0xb2, 0x0f, 0x1c,
  0x67, 0xd1, 0xc9, 0xeb, 0x4d, 0x68, 0x85, 0x97, 0x5a, 0xa6, 0x0c, 0x03, 0xc7, 0x86, 0xae, 0xb3,
  0x35, 0xb4, 0x1d, 0x0e, 0x95, 0x5f, 0xed, 0x37, 0x13, 0x6b, 0x1e, 0x94, 0x80, 0xf1, 0xac, 0x55,
  0x73, 0xd1, 0x31, 0xf9, 0xad, 0x13, 0x7b, 0x26, 0xbf, 0xe7, 0x55, 0x7b, 0xb2, 0xf9, 0x21, 0x42,
  0x23, 0x64, 0xe6, 0x45, 0x03, 0x67, 0xcb, 0x42, 0xd3, 0x71, 0x3f, 0xd5, 0x29, 0x17, 0x4b, 0x49,
  0x45, 0x0e, 0x8b, 0xba, 0x1f, 0x15, 0x5a, 0x7f, 0x7b, 0x5e, 0x9b, 0x22, 0x46, 0xa7, 0x9c, 0x0d,
  0x25, 0x9c, 0x76, 0x25, 0x02, 0xc8, 0x15, 0x00, 0x51, 0xe6, 0x73, 0x39, 0xac, 0x8d, 0x41, 0x7b,
  0xc8, 0x42, 0xc9, 0xdb, 0x1b, 0x16, 0x13, 0xf6, 0x44, 0x32, 0xef, 0x17, 0xa1, 0x82, 0x08, 0x34,
  0x30, 0x82, 0x08, 0x30, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x03, 0x03, 0x01,
  0x31, 0x82, 0x08, 0x20,
  0x30, 0x82, 0x08, 0x1c, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0,
  0x82, 0x08, 0x0d, 0x30, 0x82, 0x08, 0x09, 0x02, 0x01, 0x03, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x05,
  0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x30, 0x81, 0xf6, 0x06, 0x0b, 0x2a, 0x86, 0x48, 0x86,
  0xf7, 0x0d, 0x01, 0x09, 0x10, 0x01, 0x04, 0xa0, 0x81, 0xe6, 0x04, 0x81, 0xe3, 0x30, 0x81, 0xe0,
  0x02, 0x01, 0x01, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0xb2, 0x31, 0x02, 0x01, 0x01, 0x30,
  0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14, 0xcd, 0x06,
  0xf0, 0xbd, 0x8b, 0xcd, 0x5c, 0x2e, 0x5a, 0x7c, 0x42, 0x56, 0x2c, 0x20, 0x4a, 0x15, 0xcb, 0x1d,
  0x8b, 0x0e, 0x02, 0x15, 0x00, 0xb6, 0xff, 0x47, 0x05, 0xb6, 0x2d, 0x15, 0xac, 0x3f, 0x5d, 0xd9,
  0xcf, 0x9d, 0x54, 0x35, 0x56, 0x7c, 0xc1, 0x6e, 0x8b, 0x18, 0x0f, 0x32, 0x30, 0x31, 0x34, 0x30,
  0x37, 0x32, 0x38, 0x30, 0x38, 0x35, 0x30, 0x30, 0x33, 0x5a, 0xa0, 0x81, 0x83, 0xa4, 0x81, 0x80,
  0x30, 0x7e, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x47, 0x42, 0x31,
  0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13, 0x12, 0x47, 0x72, 0x65, 0x61, 0x74, 0x65,
  0x72, 0x20, 0x4d, 0x61, 0x6e, 0x63, 0x68, 0x65, 0x73, 0x74, 0x65, 0x72, 0x31, 0x10, 0x30, 0x0e,
  0x06, 0x03, 0x55, 0x04, 0x07, 0x13, 0x07, 0x53, 0x61, 0x6c, 0x66, 0x6f, 0x72, 0x64, 0x31, 0x1a,
  0x30, 0x18, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x11, 0x43, 0x4f, 0x4d, 0x4f, 0x44, 0x4f, 0x20,
  0x43, 0x41, 0x20, 0x4c, 0x69, 0x6d, 0x69, 0x74, 0x65, 0x64, 0x31, 0x24, 0x30, 0x22, 0x06, 0x03,
  0x55, 0x04, 0x03, 0x13, 0x1b, 0x43, 0x4f, 0x4d, 0x4f, 0x44, 0x4f, 0x20, 0x54, 0x69, 0x6d, 0x65,
  0x20, 0x53, 0x74, 0x61, 0x6d, 0x70, 0x69, 0x6e, 0x67, 0x20, 0x53, 0x69, 0x67, 0x6e, 0x65, 0x72,
  0xa0, 0x82, 0x04, 0x97, 0x30, 0x82, 0x04, 0x93, 0x30, 0x82, 0x03, 0x7b, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x10, 0x47, 0x8a, 0x8e, 0xfb, 0x59, 0xe1, 0xd8, 0x3f, 0x0c, 0xe1, 0x42, 0xd2, 0xa2,
  0x87, 0x07, 0xbe, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05,
  0x05, 0x00, 0x30, 0x81, 0x95, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02,
  0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13, 0x02, 0x55, 0x54, 0x31,
  0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04, 0x07, 0x13, 0x0e, 0x53, 0x61, 0x6c, 0x74, 0x20, 0x4c,
  0x61, 0x6b, 0x65, 0x20, 0x43, 0x69, 0x74, 0x79, 0x31, 0x1e, 0x30, 0x1c, 0x06, 0x03, 0x55, 0x04,
  0x0a, 0x13, 0x15, 0x54, 0x68, 0x65, 0x20, 0x55, 0x53, 0x45, 0x52, 0x54, 0x52, 0x55, 0x53, 0x54,
  0x20, 0x4e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x31, 0x21, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x04,
  0x0b, 0x13, 0x18, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x75, 0x73,
  0x65, 0x72, 0x74, 0x72, 0x75, 0x73, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x31, 0x1d, 0x30, 0x1b, 0x06,
  0x03, 0x55, 0x04, 0x03, 0x13, 0x14, 0x55, 0x54, 0x4e, 0x2d, 0x55, 0x53, 0x45, 0x52, 0x46, 0x69,
  0x72, 0x73, 0x74, 0x2d, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x30,
  0x30, 0x35, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x31, 0x35, 0x30,
  0x35, 0x31, 0x30, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x7e, 0x31, 0x0b, 0x30, 0x09,
  0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x47, 0x42, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55,
  0x04, 0x08, 0x13, 0x12, 0x47, 0x72, 0x65, 0x61, 0x74, 0x65, 0x72, 0x20, 0x4d, 0x61, 0x6e, 0x63,
  0x68, 0x65, 0x73, 0x74, 0x65, 0x72, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x07, 0x13,
  0x07, 0x53, 0x61, 0x6c, 0x66, 0x6f, 0x72, 0x64, 0x31, 0x1a, 0x30, 0x18, 0x06, 0x03, 0x55, 0x04,
  0x0a, 0x13, 0x11, 0x43, 0x4f, 0x4d, 0x4f, 0x44, 0x4f, 0x20, 0x43, 0x41, 0x20, 0x4c, 0x69, 0x6d,
  0x69, 0x74, 0x65, 0x64, 0x31, 0x24, 0x30, 0x22, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x1b, 0x43,
  0x4f, 0x4d, 0x4f, 0x44, 0x4f, 0x20, 0x54, 0x69, 0x6d, 0x65, 0x20, 0x53, 0x74, 0x61, 0x6d, 0x70,
  0x69, 0x6e, 0x67, 0x20, 0x53, 0x69, 0x67, 0x6e, 0x65, 0x72, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d,
  0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01,
  0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xbc, 0x35, 0xa0, 0x36, 0x70,
  0x22, 0x81, 0x11, 0xc3, 0xb2, 0x83, 0xb9, 0xd3, 0x28, 0xc6, 0x36, 0xcd, 0x25, 0x6b, 0xa9, 0x7b,
  0xb2, 0x1c, 0xf6, 0x9b, 0x51, 0x9c, 0xef, 0x35, 0xf4, 0xed, 0x08, 0x8e, 0x5e, 0x38, 0x08, 0xf8,
  0x77, 0x3c, 0x0a, 0x42, 0xe0, 0xf3, 0x70, 0xdc, 0xa3, 0xd7, 0xca, 0xf5, 0x4c, 0x0b, 0xcf, 0xff,
  0x22, 0x9c, 0x0a, 0x7e, 0x68, 0xd6, 0x09, 0xa2, 0x2a, 0x84, 0x7b, 0xa6, 0x9d, 0xb4, 0xa9, 0xc1,
  0x33, 0xe2, 0xef, 0x1f, 0x17, 0x48, 0xca, 0x3a, 0xcd, 0x46, 0xe6, 0xc5, 0xaa, 0x77, 0xbd, 0xe3,
  0x77, 0x9a, 0xfa, 0x47, 0x53, 0x40, 0x28, 0x59, 0x43, 0x93, 0xf1, 0xa4, 0x81, 0xea, 0xef, 0x80,
  0xb5, 0x4f, 0xa7, 0x08, 0xce, 0xba, 0x6e, 0xbc, 0xca, 0x76, 0x0c, 0x97, 0x64, 0x59, 0x86, 0x24,
  0xbb, 0x3d, 0x82, 0x90, 0xa8, 0x55, 0xb1, 0x92, 0xd3, 0xa0, 0xa7, 0x05, 0xac, 0x9f, 0x53, 0x25,
  0x08, 0x10, 0x47, 0x99, 0xcd, 0x98, 0xde, 0x68, 0xe5, 0xb4, 0x50, 0x78, 0xa3, 0xaf, 0x01, 0xcc,
  0x59, 0x43, 0x58, 0xe4, 0x76, 0x6e, 0x7e, 0xac, 0xc7, 0xe2, 0x9e, 0x1f, 0x4f, 0xb0, 0x47, 0x2d,
  0xc8, 0x0c, 0xa3, 0x49, 0x27, 0x80, 0x75, 0x8c, 0xbb, 0x06, 0x91, 0x65, 0x0f, 0x90, 0x9b, 0xf4,
  0xba, 0xd1, 0x81, 0xc8, 0x5c, 0x6a, 0xec, 0x14, 0xe9, 0x25, 0x09, 0xbf, 0x23, 0x16, 0xf4, 0x95,
  0x46, 0x40, 0x40, 0x21, 0xbb, 0x83, 0x96, 0xfd, 0x86, 0x1f, 0x7a, 0xc8, 0x0d, 0x10, 0x8e, 0xa2,
  0xf8, 0x19, 0x07, 0x58, 0x7f, 0x9f, 0xbd, 0x37, 0x02, 0x60, 0xf2, 0xa4, 0xe9, 0x9d, 0x44, 0x3f,
  0x30, 0x05, 0xe4, 0xa7, 0x70, 0x99, 0x51, 0x9a, 0xe8, 0x17, 0xf1, 0x55, 0xca, 0xb2, 0x61, 0x89,
  0x65, 0x46, 0xa7, 0x6a, 0xf2, 0x58, 0x46, 0x7e, 0xaa, 0xa0, 0x07, 0x02, 0x03, 0x01, 0x00, 0x01,
  0xa3, 0x81, 0xf4, 0x30, 0x81, 0xf1, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30,
  0x16, 0x80, 0x14, 0xda, 0xed, 0x64, 0x74, 0x14, 0x9c, 0x14, 0x3c, 0xab, 0xdd, 0x99, 0xa9, 0xbd,
  0x5b, 0x28, 0x4d, 0x8b, 0x3c, 0xc9, 0xd8, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16,
  0x04, 0x14, 0x2e, 0x2d, 0xb0, 0x0a, 0x44, 0x4a, 0xd3, 0x87, 0xc0, 0x02, 0x07, 0xce, 0x97, 0x7d,
  0x50, 0x62, 0x20, 0xfd, 0x0f, 0x83, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff,
  0x04, 0x04, 0x03, 0x02, 0x06, 0xc0, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff,
  0x04, 0x02, 0x30, 0x00, 0x30, 0x16, 0x06, 0x03, 0x55, 0x1d, 0x25, 0x01, 0x01, 0xff, 0x04, 0x0c,
  0x30, 0x0a, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x08, 0x30, 0x42, 0x06, 0x03,
  0x55, 0x1d, 0x1f, 0x04, 0x3b, 0x30, 0x39, 0x30, 0x37, 0xa0, 0x35, 0xa0, 0x33, 0x86, 0x31, 0x68,
  0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x63, 0x72, 0x6c, 0x2e, 0x75, 0x73, 0x65, 0x72, 0x74, 0x72,
  0x75, 0x73, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x55, 0x54, 0x4e, 0x2d, 0x55, 0x53, 0x45, 0x52,
  0x46, 0x69, 0x72, 0x73, 0x74, 0x2d, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x2e, 0x63, 0x72, 0x6c,
  0x30, 0x35, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x01, 0x04, 0x29, 0x30, 0x27,
  0x30, 0x25, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x01, 0x86, 0x19, 0x68, 0x74,
  0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x6f, 0x63, 0x73, 0x70, 0x2e, 0x75, 0x73, 0x65, 0x72, 0x74, 0x72,
  0x75, 0x73, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
  0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0xc8, 0xfb, 0x63, 0xf8, 0x0b,
  0x75, 0x75, 0x2c, 0x3a, 0xf1, 0xf2, 0x13, 0xa7, 0x2d, 0xb6, 0xa3, 0x1a, 0x9c, 0xad, 0x01, 0x07,
  0xd3, 0x34, 0x8e, 0x77, 0xe0, 0xc2, 0x6e, 0xae, 0x02, 0x5d, 0x48, 0x4f, 0xa4, 0xd2, 0x21, 0xb6,
  0x36, 0xfd, 0x2a, 0x35, 0x43, 0x7c, 0x6b, 0xdf, 0x80, 0x87, 0x0b, 0x15, 0xf0, 0x76, 0x32, 0x00,
  0xb4, 0xce, 0xb5, 0x67, 0xa4, 0x2f, 0x2f, 0x20, 0x1b, 0x9c, 0x54, 0x9e, 0x83, 0x3f, 0x1f, 0x5f,
  0x14, 0x95, 0x62, 0x82, 0x0f, 0x22, 0x41, 0x22, 0x1f, 0x70, 0xb3, 0xf3, 0xf7, 0x42, 0xde, 0x6c,
  0x51, 0xcd, 0x4b, 0xf8, 0x21, 0xac, 0x9b, 0x3b, 0x8c, 0xb1, 0xe5, 0xe6, 0x28, 0x8f, 0xce, 0x2a,
  0x8a, 0xf9, 0xaa, 0x52, 0x4d, 0x8c, 0x5b, 0x77, 0xba, 0x4d, 0x5a, 0x58, 0xdb, 0xbb, 0x6a, 0x04,
  0xcc, 0x52, 0x1e, 0x9d, 0xe2, 0x28, 0x37, 0x0e, 0xbb, 0xe7, 0x0e, 0x91, 0xc7, 0xf8, 0xdb, 0xf1,
  0x81, 0x98, 0xeb, 0xcd, 0x37, 0xb3, 0x0e, 0xab, 0x65, 0xd3, 0x62, 0xec, 0x3a, 0xa5, 0x76, 0xeb,
  0x13, 0xa8, 0x35, 0x93, 0xc9, 0x2e, 0x0a, 0x01, 0xec, 0xc0, 0xe8, 0xcc, 0x3d, 0x7e, 0xb6, 0xeb,
  0xe2, 0xc1, 0xec, 0xd3, 0x14, 0x92, 0x82, 0x66, 0x87, 0x50, 0xdc, 0xfd, 0x50, 0x97, 0xac, 0xb3,
  0x4a, 0x76, 0x73, 0x06, 0xc4, 0x86, 0x11, 0x3a, 0xb3, 0x5f, 0x43, 0x04, 0x52, 0x6f, 0xea, 0xb3,
  0xd0, 0x74, 0x36, 0x4c, 0xca, 0xf1, 0x1b, 0x79, 0x84, 0x37, 0x70, 0x63, 0xad, 0x74, 0xb9, 0xaa,
  0x0e, 0xf3, 0x98, 0xb0, 0x86, 0x08, 0xeb, 0xdb, 0xe0, 0x1f, 0x8c, 0x10, 0xf2, 0x39, 0x64, 0x9b,
  0xae, 0x4f, 0x0a, 0x2c, 0x92, 0x8a, 0x4f, 0x18, 0xb5, 0x91, 0xe5, 0x8d, 0x1a, 0x93, 0x5f, 0x1f,
  0xae, 0xf1, 0xa6, 0xf0, 0x2e, 0x97, 0xd0, 0xd2, 0xf6, 0x2b, 0x3c, 0x31, 0x82, 0x02, 0x61, 0x30,
  0x82, 0x02, 0x5d, 0x02, 0x01, 0x01, 0x30, 0x81, 0xaa, 0x30, 0x81, 0x95, 0x31, 0x0b, 0x30, 0x09,
  0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x08, 0x13, 0x02, 0x55, 0x54, 0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04, 0x07, 0x13,
  0x0e, 0x53, 0x61, 0x6c, 0x74, 0x20, 0x4c, 0x61, 0x6b, 0x65, 0x20, 0x43, 0x69, 0x74, 0x79, 0x31,
  0x1e, 0x30, 0x1c, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x15, 0x54, 0x68, 0x65, 0x20, 0x55, 0x53,
  0x45, 0x52, 0x54, 0x52, 0x55, 0x53, 0x54, 0x20, 0x4e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x31,
  0x21, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x18, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f,
  0x2f, 0x77, 0x77, 0x77, 0x2e, 0x75, 0x73, 0x65, 0x72, 0x74, 0x72, 0x75, 0x73, 0x74, 0x2e, 0x63,
  0x6f, 0x6d, 0x31, 0x1d, 0x30, 0x1b, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x14, 0x55, 0x54, 0x4e,
  0x2d, 0x55, 0x53, 0x45, 0x52, 0x46, 0x69, 0x72, 0x73, 0x74, 0x2d, 0x4f, 0x62, 0x6a, 0x65, 0x63,
  0x74, 0x02, 0x10, 0x47, 0x8a, 0x8e, 0xfb, 0x59, 0xe1, 0xd8, 0x3f, 0x0c, 0xe1, 0x42, 0xd2, 0xa2,
  0x87, 0x07, 0xbe, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0xa0, 0x81,
  0x8c, 0x30, 0x1a, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x03, 0x31, 0x0d,
  0x06, 0x0b, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x10, 0x01, 0x04, 0x30, 0x1c, 0x06,
  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x05, 0x31, 0x0f, 0x17, 0x0d, 0x31, 0x34,
  0x30, 0x37, 0x32, 0x38, 0x30, 0x38, 0x35, 0x30, 0x30, 0x33, 0x5a, 0x30, 0x23, 0x06, 0x09, 0x2a,
  0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x04, 0x31, 0x16, 0x04, 0x14, 0x7a, 0xad, 0x35, 0xdc,
  0x5b, 0xd6, 0x00, 0xd7, 0x44, 0xac, 0x80, 0x8f, 0x4f, 0xb6, 0xb4, 0x03, 0x62, 0x34, 0x53, 0xdc,
  0x30, 0x2b, 0x06, 0x0b, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x10, 0x02, 0x0c, 0x31,
  0x1c, 0x30, 0x1a, 0x30, 0x18, 0x30, 0x16, 0x04, 0x14, 0x3d, 0xbb, 0x6d, 0xb5, 0x08, 0x5c, 0x6d,
  0xd5, 0xa1, 0xca, 0x7f, 0x9c, 0xf8, 0x4e, 0xcb, 0x1a, 0x39, 0x10, 0xca, 0xc8, 0x30, 0x0d, 0x06,
  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x82, 0x01, 0x00,
  0x73, 0x64, 0xb9, 0xa3, 0x54, 0x6f, 0x50, 0x97, 0x01, 0xa7, 0xf6, 0x0d, 0xb8, 0xce, 0x4b, 0xaa,
  0x43, 0xa2, 0x8f, 0xa3, 0xea, 0x93, 0xf2, 0xa3, 0xd0, 0x46, 0xde, 0xdd, 0x45, 0xe5, 0x94, 0x5a,
  0x45, 0xc2, 0x13, 0x1b, 0x90, 0x9b, 0xcf, 0x73, 0xcd, 0x28, 0x70, 0xf0, 0xf4, 0x54, 0xb5, 0x2d,
  0x31, 0xf9, 0xf3, 0x2d, 0x38, 0x78, 0xfe, 0x68, 0xea, 0x3c, 0xc0, 0xbe, 0x0b, 0x5a, 0x91, 0x49,
  0x63, 0xeb, 0x26, 0x32, 0x5b, 0x86, 0xcf, 0xe5, 0x8a, 0xa5, 0x9d, 0xe6, 0x4b, 0x57, 0x91, 0x8f,
  0x3c, 0xdc, 0xa6, 0x53, 0xd8, 0xdb, 0x8a, 0xfd, 0x3e, 0x7e, 0x19, 0x6f, 0x27, 0x72, 0x95, 0xc2,
  0x79, 0x73, 0xdf, 0xfb, 0x08, 0x5c, 0x5b, 0xc8, 0xb7, 0x94, 0x75, 0x88, 0x7a, 0x9a, 0x85, 0x9f,
  0x1b, 0xa3, 0x98, 0x30, 0x91, 0xee, 0xc0, 0x52, 0xd2, 0x75, 0x9c, 0xcb, 0x45, 0x0d, 0x94, 0x43,
  0x67, 0x7a, 0x49, 0x1c, 0xb1, 0x89, 0x9d, 0x6e, 0xfa, 0x87, 0xd2, 0x4d, 0x6e, 0x74, 0x90, 0xf5,
  0x80, 0x8c, 0x92, 0xda, 0xd9, 0xa1, 0x48, 0x20, 0x31, 0x02, 0x79, 0xde, 0xe3, 0xbd, 0x09, 0x04,
  0xa8, 0xd4, 0x99, 0xd7, 0x3b, 0xea, 0xf8, 0xdf, 0xb3, 0xb9, 0xd7, 0xa3, 0x36, 0xa1, 0xdb, 0xd3,
  0xec, 0x65, 0x8c, 0xb8, 0x8f, 0xfb, 0xd6, 0xef, 0x9c, 0x32, 0x3e, 0xab, 0x20, 0x74, 0xb9, 0x65,
  0x4c, 0xc6, 0x15, 0x2f, 0x31, 0x2a, 0x34, 0x3e, 0x84, 0x09, 0xb4, 0x75, 0xbc, 0xbe, 0xaf, 0xb3,
  0x9e, 0x85, 0xf1, 0xbb, 0x99, 0x1a, 0x07, 0xbd, 0x20, 0xa6, 0xed, 0xcf, 0xd1, 0xa6, 0x9a, 0x22,
  0xb2, 0x6d, 0x75, 0xf4, 0x23, 0x58, 0x13, 0x78, 0x73, 0x1a, 0xb2, 0x84, 0xde, 0xad, 0xe8, 0x6d,
  0xe6, 0xe7, 0x5c, 0xb6, 0xe6, 0x5b, 0x10, 0x37, 0x1f, 0xe3, 0x6e, 0xbd, 0x83, 0xd7, 0x51, 0xb1,
  0x00, 0x00, 0x00, 0x00, 0x0a
  };

//
// The Comodo Time Stamping Signer Certificate Used for the verification of TimeStamp signature.
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 TSTrustedCert[] = {
  0x30, 0x82, 0x04, 0x93, 0x30, 0x82, 0x03, 0x7b, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x10, 0x47,
  0x8a, 0x8e, 0xfb, 0x59, 0xe1, 0xd8, 0x3f, 0x0c, 0xe1, 0x42, 0xd2, 0xa2, 0x87, 0x07, 0xbe, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x30, 0x81,
  0x95, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b,
  0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13, 0x02, 0x55, 0x54, 0x31, 0x17, 0x30, 0x15, 0x06,
  0x03, 0x55, 0x04, 0x07, 0x13, 0x0e, 0x53, 0x61, 0x6c, 0x74, 0x20, 0x4c, 0x61, 0x6b, 0x65, 0x20,
  0x43, 0x69, 0x74, 0x79, 0x31, 0x1e, 0x30, 0x1c, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x15, 0x54,
  0x68, 0x65, 0x20, 0x55, 0x53, 0x45, 0x52, 0x54, 0x52, 0x55, 0x53, 0x54, 0x20, 0x4e, 0x65, 0x74,
  0x77, 0x6f, 0x72, 0x6b, 0x31, 0x21, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x18, 0x68,
  0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x75, 0x73, 0x65, 0x72, 0x74, 0x72,
  0x75, 0x73, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x31, 0x1d, 0x30, 0x1b, 0x06, 0x03, 0x55, 0x04, 0x03,
  0x13, 0x14, 0x55, 0x54, 0x4e, 0x2d, 0x55, 0x53, 0x45, 0x52, 0x46, 0x69, 0x72, 0x73, 0x74, 0x2d,
  0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x30, 0x30, 0x35, 0x31, 0x30,
  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x31, 0x35, 0x30, 0x35, 0x31, 0x30, 0x32,
  0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x7e, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
  0x06, 0x13, 0x02, 0x47, 0x42, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13, 0x12,
  0x47, 0x72, 0x65, 0x61, 0x74, 0x65, 0x72, 0x20, 0x4d, 0x61, 0x6e, 0x63, 0x68, 0x65, 0x73, 0x74,
  0x65, 0x72, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x07, 0x13, 0x07, 0x53, 0x61, 0x6c,
  0x66, 0x6f, 0x72, 0x64, 0x31, 0x1a, 0x30, 0x18, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x11, 0x43,
  0x4f, 0x4d, 0x4f, 0x44, 0x4f, 0x20, 0x43, 0x41, 0x20, 0x4c, 0x69, 0x6d, 0x69, 0x74, 0x65, 0x64,
  0x31, 0x24, 0x30, 0x22, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x1b, 0x43, 0x4f, 0x4d, 0x4f, 0x44,
  0x4f, 0x20, 0x54, 0x69, 0x6d, 0x65, 0x20, 0x53, 0x74, 0x61, 0x6d, 0x70, 0x69, 0x6e, 0x67, 0x20,
  0x53, 0x69, 0x67, 0x6e, 0x65, 0x72, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
  0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82,
  0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xbc, 0x35, 0xa0, 0x36, 0x70, 0x22, 0x81, 0x11, 0xc3,
  0xb2, 0x83, 0xb9, 0xd3, 0x28, 0xc6, 0x36, 0xcd, 0x25, 0x6b, 0xa9, 0x7b, 0xb2, 0x1c, 0xf6, 0x9b,
  0x51, 0x9c, 0xef, 0x35, 0xf4, 0xed, 0x08, 0x8e, 0x5e, 0x38, 0x08, 0xf8, 0x77, 0x3c, 0x0a, 0x42,
  0xe0, 0xf3, 0x70, 0xdc, 0xa3, 0xd7, 0xca, 0xf5, 0x4c, 0x0b, 0xcf, 0xff, 0x22, 0x9c, 0x0a, 0x7e,
  0x68, 0xd6, 0x09, 0xa2, 0x2a, 0x84, 0x7b, 0xa6, 0x9d, 0xb4, 0xa9, 0xc1, 0x33, 0xe2, 0xef, 0x1f,
  0x17, 0x48, 0xca, 0x3a, 0xcd, 0x46, 0xe6, 0xc5, 0xaa, 0x77, 0xbd, 0xe3, 0x77, 0x9a, 0xfa, 0x47,
  0x53, 0x40, 0x28, 0x59, 0x43, 0x93, 0xf1, 0xa4, 0x81, 0xea, 0xef, 0x80, 0xb5, 0x4f, 0xa7, 0x08,
  0xce, 0xba, 0x6e, 0xbc, 0xca, 0x76, 0x0c, 0x97, 0x64, 0x59, 0x86, 0x24, 0xbb, 0x3d, 0x82, 0x90,
  0xa8, 0x55, 0xb1, 0x92, 0xd3, 0xa0, 0xa7, 0x05, 0xac, 0x9f, 0x53, 0x25, 0x08, 0x10, 0x47, 0x99,
  0xcd, 0x98, 0xde, 0x68, 0xe5, 0xb4, 0x50, 0x78, 0xa3, 0xaf, 0x01, 0xcc, 0x59, 0x43, 0x58, 0xe4,
  0x76, 0x6e, 0x7e, 0xac, 0xc7, 0xe2, 0x9e, 0x1f, 0x4f, 0xb0, 0x47, 0x2d, 0xc8, 0x0c, 0xa3, 0x49,
  0x27, 0x80, 0x75, 0x8c, 0xbb, 0x06, 0x91, 0x65, 0x0f, 0x90, 0x9b, 0xf4, 0xba, 0xd1, 0x81, 0xc8,
  0x5c, 0x6a, 0xec, 0x14, 0xe9, 0x25, 0x09, 0xbf, 0x23, 0x16, 0xf4, 0x95, 0x46, 0x40, 0x40, 0x21,
  0xbb, 0x83, 0x96, 0xfd, 0x86, 0x1f, 0x7a, 0xc8, 0x0d, 0x10, 0x8e, 0xa2, 0xf8, 0x19, 0x07, 0x58,
  0x7f, 0x9f, 0xbd, 0x37, 0x02, 0x60, 0xf2, 0xa4, 0xe9, 0x9d, 0x44, 0x3f, 0x30, 0x05, 0xe4, 0xa7,
  0x70, 0x99, 0x51, 0x9a, 0xe8, 0x17, 0xf1, 0x55, 0xca, 0xb2, 0x61, 0x89, 0x65, 0x46, 0xa7, 0x6a,
  0xf2, 0x58, 0x46, 0x7e, 0xaa, 0xa0, 0x07, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x81, 0xf4, 0x30,
  0x81, 0xf1, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xda,
  0xed, 0x64, 0x74, 0x14, 0x9c, 0x14, 0x3c, 0xab, 0xdd, 0x99, 0xa9, 0xbd, 0x5b, 0x28, 0x4d, 0x8b,
  0x3c, 0xc9, 0xd8, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x2e, 0x2d,
  0xb0, 0x0a, 0x44, 0x4a, 0xd3, 0x87, 0xc0, 0x02, 0x07, 0xce, 0x97, 0x7d, 0x50, 0x62, 0x20, 0xfd,
  0x0f, 0x83, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02,
  0x06, 0xc0, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x02, 0x30, 0x00,
  0x30, 0x16, 0x06, 0x03, 0x55, 0x1d, 0x25, 0x01, 0x01, 0xff, 0x04, 0x0c, 0x30, 0x0a, 0x06, 0x08,
  0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x08, 0x30, 0x42, 0x06, 0x03, 0x55, 0x1d, 0x1f, 0x04,
  0x3b, 0x30, 0x39, 0x30, 0x37, 0xa0, 0x35, 0xa0, 0x33, 0x86, 0x31, 0x68, 0x74, 0x74, 0x70, 0x3a,
  0x2f, 0x2f, 0x63, 0x72, 0x6c, 0x2e, 0x75, 0x73, 0x65, 0x72, 0x74, 0x72, 0x75, 0x73, 0x74, 0x2e,
  0x63, 0x6f, 0x6d, 0x2f, 0x55, 0x54, 0x4e, 0x2d, 0x55, 0x53, 0x45, 0x52, 0x46, 0x69, 0x72, 0x73,
  0x74, 0x2d, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x2e, 0x63, 0x72, 0x6c, 0x30, 0x35, 0x06, 0x08,
  0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x01, 0x04, 0x29, 0x30, 0x27, 0x30, 0x25, 0x06, 0x08,
  0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x01, 0x86, 0x19, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f,
  0x2f, 0x6f, 0x63, 0x73, 0x70, 0x2e, 0x75, 0x73, 0x65, 0x72, 0x74, 0x72, 0x75, 0x73, 0x74, 0x2e,
  0x63, 0x6f, 0x6d, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05,
  0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0xc8, 0xfb, 0x63, 0xf8, 0x0b, 0x75, 0x75, 0x2c, 0x3a,
  0xf1, 0xf2, 0x13, 0xa7, 0x2d, 0xb6, 0xa3, 0x1a, 0x9c, 0xad, 0x01, 0x07, 0xd3, 0x34, 0x8e, 0x77,
  0xe0, 0xc2, 0x6e, 0xae, 0x02, 0x5d, 0x48, 0x4f, 0xa4, 0xd2, 0x21, 0xb6, 0x36, 0xfd, 0x2a, 0x35,
  0x43, 0x7c, 0x6b, 0xdf, 0x80, 0x87, 0x0b, 0x15, 0xf0, 0x76, 0x32, 0x00, 0xb4, 0xce, 0xb5, 0x67,
  0xa4, 0x2f, 0x2f, 0x20, 0x1b, 0x9c, 0x54, 0x9e, 0x83, 0x3f, 0x1f, 0x5f, 0x14, 0x95, 0x62, 0x82,
  0x0f, 0x22, 0x41, 0x22, 0x1f, 0x70, 0xb3, 0xf3, 0xf7, 0x42, 0xde, 0x6c, 0x51, 0xcd, 0x4b, 0xf8,
  0x21, 0xac, 0x9b, 0x3b, 0x8c, 0xb1, 0xe5, 0xe6, 0x28, 0x8f, 0xce, 0x2a, 0x8a, 0xf9, 0xaa, 0x52,
  0x4d, 0x8c, 0x5b, 0x77, 0xba, 0x4d, 0x5a, 0x58, 0xdb, 0xbb, 0x6a, 0x04, 0xcc, 0x52, 0x1e, 0x9d,
  0xe2, 0x28, 0x37, 0x0e, 0xbb, 0xe7, 0x0e, 0x91, 0xc7, 0xf8, 0xdb, 0xf1, 0x81, 0x98, 0xeb, 0xcd,
  0x37, 0xb3, 0x0e, 0xab, 0x65, 0xd3, 0x62, 0xec, 0x3a, 0xa5, 0x76, 0xeb, 0x13, 0xa8, 0x35, 0x93,
  0xc9, 0x2e, 0x0a, 0x01, 0xec, 0xc0, 0xe8, 0xcc, 0x3d, 0x7e, 0xb6, 0xeb, 0xe2, 0xc1, 0xec, 0xd3,
  0x14, 0x92, 0x82, 0x66, 0x87, 0x50, 0xdc, 0xfd, 0x50, 0x97, 0xac, 0xb3, 0x4a, 0x76, 0x73, 0x06,
  0xc4, 0x86, 0x11, 0x3a, 0xb3, 0x5f, 0x43, 0x04, 0x52, 0x6f, 0xea, 0xb3, 0xd0, 0x74, 0x36, 0x4c,
  0xca, 0xf1, 0x1b, 0x79, 0x84, 0x37, 0x70, 0x63, 0xad, 0x74, 0xb9, 0xaa, 0x0e, 0xf3, 0x98, 0xb0,
  0x86, 0x08, 0xeb, 0xdb, 0xe0, 0x1f, 0x8c, 0x10, 0xf2, 0x39, 0x64, 0x9b, 0xae, 0x4f, 0x0a, 0x2c,
  0x92, 0x8a, 0x4f, 0x18, 0xb5, 0x91, 0xe5, 0x8d, 0x1a, 0x93, 0x5f, 0x1f, 0xae, 0xf1, 0xa6, 0xf0,
  0x2e, 0x97, 0xd0, 0xd2, 0xf6, 0x2b, 0x3c, 0x0a
  };

UNIT_TEST_STATUS
EFIAPI
TestVerifyImageTimestampVerify (
  IN UNIT_TEST_CONTEXT           Context
  )
{
  BOOLEAN   Status;
  EFI_TIME  SigningTime;

  Status = FALSE;

  //
  // Verify RFC3161 Timestamp CounterSignature.
  //
  Status = ImageTimestampVerify (
             AuthenticodeWithTS,
             sizeof (AuthenticodeWithTS),
             TSTrustedCert,
             sizeof (TSTrustedCert),
             &SigningTime
             );
  UT_ASSERT_TRUE (Status);

  UT_ASSERT_EQUAL (SigningTime.Year, 2014);
  UT_ASSERT_EQUAL (SigningTime.Month, 7);
  UT_ASSERT_EQUAL (SigningTime.Day, 28);
  UT_ASSERT_EQUAL (SigningTime.Hour, 8);
  UT_ASSERT_EQUAL (SigningTime.Minute, 50);
  UT_ASSERT_EQUAL (SigningTime.Second, 3);

  return Status;
}

TEST_DESC mImageTimestampTest[] = {
    //
    // -----Description--------------------------------------Class----------------------------Function-----------------Pre---Post--Context
    //
    {"TestVerifyImageTimestampVerify()",  "CryptoPkg.BaseCryptLib.ImageTimestamp",   TestVerifyImageTimestampVerify, NULL, NULL, NULL},
};

UINTN mImageTimestampTestNum = ARRAY_SIZE(mImageTimestampTest);
