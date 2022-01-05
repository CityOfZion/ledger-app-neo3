import struct
from hashlib import sha256

from ecdsa.curves import NIST256p
from ecdsa.keys import VerifyingKey
from ecdsa.util import sigdecode_der

from neo3.network import node, payloads
from neo3.core import types, serialization
from neo3 import contracts, wallet, vm


def test_sign_tx(cmd, button):
    """
    In order to debug this test locally while being able to see what's happening in the app
    run Speculos as follows:

    ./speculos.py --sdk 2.0 --ontop --button-port 42000 ../app-neo3/bin/app.elf

    Then in PyCharm add "--headless" as Additional Argument for the testcase.
    This will make sure it selects TCPButton instead of a fake button that does nothing.
    """
    bip44_path: str = "m/44'/888'/0'/0/0"

    pub_key = cmd.get_public_key(
        bip44_path=bip44_path,
        display=False
    )  # type: bytes

    pk: VerifyingKey = VerifyingKey.from_string(
        pub_key,
        curve=NIST256p,
        hashfunc=sha256
    )

    signer = payloads.Signer(account=types.UInt160.from_string("d7678dd97c000be3f33e9362e673101bac4ca654"),
                             scope=payloads.WitnessScope.CUSTOM_CONTRACTS)

    for i in range(1, 17):
        signer.allowed_contracts.append(types.UInt160(20 * i.to_bytes(1, 'little')))

    witness = payloads.Witness(invocation_script=b'', verification_script=b'\x55')
    magic = 860833102

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
                              witnesses=[witness])

    der_sig = cmd.sign_tx(bip44_path=bip44_path,
                          transaction=tx,
                          network_magic=magic,
                          button=button)

    with serialization.BinaryWriter() as writer:
        tx.serialize_unsigned(writer)
        tx_data: bytes = writer.to_array()

    assert pk.verify(signature=der_sig,
                     data=struct.pack("I", magic) + sha256(tx_data).digest(),
                     hashfunc=sha256,
                     sigdecode=sigdecode_der) is True


def test_sign_vote_script_tx(cmd, button):
    bip44_path: str = "m/44'/888'/0'/0/0"

    pub_key = cmd.get_public_key(
        bip44_path=bip44_path,
        display=False
    )  # type: bytes

    pk: VerifyingKey = VerifyingKey.from_string(
        pub_key,
        curve=NIST256p,
        hashfunc=sha256
    )

    signer = payloads.Signer(account=types.UInt160.from_string("d7678dd97c000be3f33e9362e673101bac4ca654"),
                             scope=payloads.WitnessScope.CALLED_BY_ENTRY)
    witness = payloads.Witness(invocation_script=b'', verification_script=b'\x55')
    magic = 860833102

    # build a NEO vote script
    from_account = wallet.Account.address_to_script_hash("NSiVJYZej4XsxG5CUpdwn7VRQk8iiiDMPM").to_array()
    vote_to = bytes.fromhex("03b209fd4f53a7170ea4444e0cb0a6bb6a53c2bd016926989cf85f9b0fba17a70c")

    sb = vm.ScriptBuilder()
    sb.emit_dynamic_call_with_args(contracts.NeoToken().hash, "vote", [from_account, vote_to])

    tx = payloads.Transaction(version=0,
                              nonce=123,
                              system_fee=456,
                              network_fee=789,
                              valid_until_block=1,
                              attributes=[],
                              signers=[signer],
                              script=sb.to_array(),
                              witnesses=[witness])

    der_sig = cmd.sign_vote_tx(bip44_path=bip44_path,
                               transaction=tx,
                               network_magic=magic,
                               button=button)

    with serialization.BinaryWriter() as writer:
        tx.serialize_unsigned(writer)
        tx_data: bytes = writer.to_array()

    assert pk.verify(signature=der_sig,
                     data=struct.pack("I", magic) + sha256(tx_data).digest(),
                     hashfunc=sha256,
                     sigdecode=sigdecode_der) is True
