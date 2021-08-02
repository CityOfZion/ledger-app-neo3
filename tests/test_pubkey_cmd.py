from neo3crypto import ECCCurve, ECPoint

def test_get_public_key(cmd):
    pub_key = cmd.get_public_key(
        bip44_path="m/44'/888'/0'/0/0",
        display=False
    )  # type: bytes, bytes

    assert len(pub_key) == 65
    assert ECPoint(pub_key, ECCCurve.SECP256R1, validate=True)

    pub_key2 = cmd.get_public_key(
        bip44_path="m/44'/888'/1'/0/0",
        display=False
    )  # type: bytes, bytes

    assert len(pub_key2) == 65
    assert ECPoint(pub_key2, ECCCurve.SECP256R1, validate=True)
