import struct
from ledgerblue.commTCP import getDongle
from ledgerblue.comm import getDongle as usb_getDongle

SIGN_TX = 0x02        # sign transaction with BIP44 path and return signature
P2_MORE = 0x80
P2_LAST = 0x00

tx_unsigned_raw = bytes.fromhex("007b000000c8010000000000001503000000000000010000000254a64cac1b1073e662933ef3e30b007cd98d67d7300311c4d1f4fba619f2628870d36e3a9773e874705baaaad1f4fba619f2628870d36e3a9773e874705bbbbbd1f4fba619f2628870d36e3a9773e874705b01026241e7e26b38bb7154b8ad49458b97fb1c4797443dc921c5ca5774f511a2bbfc54a64cac1b1073e662933ef3e30b007cd98daaaa200102158c4a4810fa2a6a12f7d33d835680429e1a68ae61161c5b3fbc98c7f1f177650101020102")

def apdu(ins, p1, p2, cdata: bytes, cla = 0x80):
    return struct.pack("BBBBB",
                       cla,
                       ins,
                       p1,
                       p2,
                       len(cdata)) + cdata

def main():
    conn = getDongle('127.0.0.1', 9999, debug=True)
    # conn = usb_getDongle(debug=True)
    bip44 = bytes.fromhex("8000002C80000378800000000000000000000000")
    network_magic = struct.pack("I", 5195086)
    conn.exchange(apdu(SIGN_TX, p1=0, p2=P2_MORE, cdata=bip44))  # send BIP44 path
    conn.exchange(apdu(SIGN_TX, p1=1, p2=P2_MORE, cdata=network_magic))
    conn.exchange(apdu(SIGN_TX, p1=2, p2=P2_LAST, cdata=tx_unsigned_raw))
    conn.close()

if __name__ == "__main__":
    main()