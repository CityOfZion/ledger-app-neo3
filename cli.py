"""
Basic script show casing all commands

Requirements:
    pip install neo-mamba==0.9.2 ledgerblue
"""

import struct
import enum
from ledgerblue.commTCP import getDongle
from ledgerblue.comm import getDongle as usb_getDongle
from neo3.core import cryptography, types, serialization
from neo3.network import payloads
from neo3 import wallet, contracts, vm


class INS(enum.IntEnum):
    GET_APP_NAME = 0x0  # name of the application
    GET_VERSION = 0x01  # version of the application
    SIGN_TX = 0x02  # sign transaction with BIP44 path and return signature
    GET_PUBLIC_KEY = 0x04  # public key of corresponding BIP44 path and return uncompressed public key

P2_MORE = 0x80  # specific for SIGN_TX instruction
P2_LAST = 0x00  # specific for SIGN_TX instruction

BIP44 = bytes.fromhex("8000002C"
                      "80000378"
                      "80000001"
                      "00000000"
                      "00000000")
NETWORK_MAGIC = struct.pack("I", 860833102)


def apdu(ins: int, p1: int, p2: int, cdata: bytes = None, cla: int = 0x80):
    """ Helper for sending a basic APDU """
    if cdata is None:
        cdata = b''
    return struct.pack("BBBBB",
                       cla,
                       ins,
                       p1,
                       p2,
                       len(cdata)) + cdata


def get_app_name(conn) -> str:
    result = conn.exchange(apdu(INS.GET_APP_NAME, p1=0, p2=0))
    return result.decode()


def get_app_version(conn) -> str:
    result = conn.exchange(apdu(INS.GET_VERSION, p1=0, p2=0))
    return "%d.%d.%d" % struct.unpack("BBB", result)


def get_public_key(conn, showOnDevice: bool = True) -> cryptography.ECPoint:
    result = conn.exchange(apdu(INS.GET_PUBLIC_KEY, p1=0, p2=int(showOnDevice), cdata=BIP44))
    return cryptography.ECPoint(bytes(result), cryptography.ECCCurve.SECP256R1, validate=True)


def sign_tx(conn, tx_unsigned_data: bytes) -> str:
    conn.exchange(apdu(INS.SIGN_TX, p1=0, p2=P2_MORE, cdata=BIP44))  # send BIP44 path
    conn.exchange(apdu(INS.SIGN_TX, p1=1, p2=P2_MORE, cdata=NETWORK_MAGIC))
    result = conn.exchange(apdu(INS.SIGN_TX, p1=2, p2=P2_LAST, cdata=tx_unsigned_data))
    return result.hex()


def main():
    # conn = getDongle('127.0.0.1', 9999, debug=True)  # Use this when testing via the Speculos emulator
    conn = usb_getDongle(debug=True)  # Use this when testing on physical device

    print(f"App name: {get_app_name(conn)}")
    print(f"App version: {get_app_version(conn)}")
    print(f"Public key (compressed): {get_public_key(conn)}")

    # Create a TX to be signed
    signer = payloads.Signer(account=types.UInt160.from_string("d7678dd97c000be3f33e9362e673101bac4ca654"),
                             scope=payloads.WitnessScope.CALLED_BY_ENTRY)
    # build a NEO transfer script
    from_account = wallet.Account.address_to_script_hash("NSiVJYZej4XsxG5CUpdwn7VRQk8iiiDMPM").to_array()
    to_account = wallet.Account.address_to_script_hash("NU5unwNcWLqPM21cNCRP1LPuhxsTpYvNTf").to_array()
    amount = 11 * contracts.NeoToken().factor
    data = None
    sb = vm.ScriptBuilder()
    sb.emit_dynamic_call_with_args(contracts.NeoToken().hash, "transfer", [from_account, to_account, amount, data])

    tx = payloads.Transaction(version=0,
                              nonce=123,
                              system_fee=456,
                              network_fee=789,
                              valid_until_block=1,
                              attributes=[],
                              signers=[signer],
                              script=sb.to_array(),
                              witnesses=[])

    with serialization.BinaryWriter() as br:
        tx.serialize_unsigned(br)
        tx_unsigned_raw = br.to_array()

    # Requires confirmation by the user
    print(f"Signature: {sign_tx(conn, tx_unsigned_raw)}")

    invalid_tx = payloads.Transaction(version=0,
                              nonce=123,
                              system_fee=456,
                              network_fee=789,
                              valid_until_block=1,
                              attributes=[],
                              signers=[signer],
                              script=b'\x01',
                              witnesses=[])

    with serialization.BinaryWriter() as br:
        invalid_tx.serialize_unsigned(br)
        invalid_tx_unsigned_raw = br.to_array()

    # sign TX with a script that is not a NEO or GAS transfer will fail.
    sign_tx(conn, invalid_tx_unsigned_raw)

    conn.close()


if __name__ == "__main__":
    main()
