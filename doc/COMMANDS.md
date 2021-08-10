# NEO3 commands

## Overview

| Command name | INS | Description |
| --- | --- | --- |
| `GET_VERSION` | 0x00 | Get application version as `MAJOR`, `MINOR`, `PATCH` |
| `GET_APP_NAME` | 0x01 | Get ASCII encoded application name |
| `SIGN_TX` | 0x02 | Sign transaction given a BIP44 path, network magic and raw transaction |
| `GET_PUBLIC_KEY` | 0x04 | Get public key given BIP44 path |


## GET_VERSION

### Command

| CLA | INS | P1 | P2 | Lc | CData |
| --- | --- | --- | --- | --- | --- |
| 0x80 | 0x00 | 0x00 | 0x00 | 0x00 | - |

### Response

| Response length (bytes) | SW | RData |
| --- | --- | --- |
| 3 | 0x9000 | `MAJOR (1)` \|\| `MINOR (1)` \|\| `PATCH (1)` |

## GET_APP_NAME

### Command

| CLA | INS | P1 | P2 | Lc | CData |
| --- | --- | --- | --- | --- | --- |
| 0x80 | 0x01 | 0x00 | 0x00 | 0x00 | - |

### Response

| Response length (bytes) | SW | RData |
| --- | --- | --- |
| var | 0x9000 | `APPNAME (var)` |

## SIGN_TX

### Command

| CLA | INS | P1 | P2 | Lc | CData |
| --- | --- | --- | --- | --- | --- |
| 0x80 | 0x02 | 0x00 (chunk index) | 0x00 | 1 + 4n | `len(bip44_path) (1)` \|\|<br> `bip44_path{1} (4)` \|\|<br>`...` \|\|<br>`bip44_path{n} (4)` |
| 0x80 | 0x02 | 0x01 (chunk index) | 0x00 | 1 + 4 | `len(network_magic) (1)` \|\|<br> `network_magic (4)` |
| 0x80 | 0x02 | 0x02-0x03 (chunk index) | 0x00 (more) <br> 0x80 (last) | 1 + 4n | `len(tx_data) (1)` \|\|<br> `tx_data{1}` \|\|<br>`...` \|\|<br>`tx_data{n}` |

### Response

| Response length (bytes) | SW | RData |
| --- | --- | --- |
| var | 0x9000 | `ASN1.DER encoded signature (max 72 bytes)`|


## GET_PUBLIC_KEY

### Command

| CLA | INS | P1 | P2 | Lc | CData |
| --- | --- | --- | --- | --- | --- |
| 0x80 | 0x04 | 0x00 (no display) <br> 0x01 (display) | 0x00 | 1 + 4n | `len(bip44_path) (1)` \|\|<br> `bip44_path{1} (4)` \|\|<br>`...` \|\|<br>`bip44_path{n} (4)` |

### Response

| Response length (bytes) | SW | RData |
| --- | --- | --- |
| var | 0x9000 | `uncompressed public_key (65 bytes) starting with 0x04` |

## Status Words

TODO: update with final list!

| SW | SW name | Description |
| --- | --- | --- |
| 0x6985 | `SW_DENY` | Rejected by user |
| 0x6A86 | `SW_WRONG_P1P2` | Either `P1` or `P2` is incorrect |
| 0x6A87 | `SW_WRONG_DATA_LENGTH` | `Lc` or minimum APDU lenght is incorrect |
| 0x6D00 | `SW_INS_NOT_SUPPORTED` | No command exists with `INS` |
| 0x6E00 | `SW_CLA_NOT_SUPPORTED` | Bad `CLA` used for this application |
| 0xB000 | `SW_WRONG_RESPONSE_LENGTH` | Wrong response lenght (buffer size problem) |
| 0xB001 | `SW_WRONG_TX_LENGTH` | Max transaction length exceeded |
| 0xB002 | `SW_TX_PARSING_FAIL` | Failed to parse raw transaction |
| 0xB003 | `SW_TX_USER_CONFIRMATION_FAIL` | User rejected TX signing |
| 0xB004 | `SW_BAD_STATE` | Incorrect sign tx state. E.g. wrong order of data sending |
| 0xB005 | `SW_SIGN_FAIL` | Failed to create signature of data |
| 0xB100 | `SW_BIP44_BAD_PURPOSE` | Invalid BIP44 purpose field |
| 0xB101 | `SW_BIP44_BAD_COIN_TYPE` | BIP44 coin type does not match NEO |
| 0xB102 | `SW_BIP44_ACCOUNT_NOT_HARDENED` | BIP44 account is not hardened |
| 0xB103 | `SW_BIP44_BAD_ACCOUNT` | BIP44 account value exceeds 10 |
| 0xB104 | `SW_BIP44_BAD_CHANGE` | BIP44 change field is not 'internal' or 'external' |
| 0xB105 | `SW_BIP44_BAD_ADDRESS` | BIP44 address value exceeds 5000 |
| 0xB106 | `SW_MAGIC_PARSING_FAIL` | Failed to parse NEO network magic |
| 0xB107 | `SW_DISPLAY_SYSTEM_FEE_FAIL` | Status word for failing to parse the system fee into a format that can be displayed on the device |
| 0xB108 | `SW_DISPLAY_NETWORK_FEE_FAIL` | Status word for failing to parse the network fee into a format that can be displayed on the device |
| 0x9000 | `OK` | Success |
