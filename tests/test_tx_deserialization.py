from typing import Union, cast, List, Tuple
import re
import enum
import struct
import logging

from neo3.network.payloads import WitnessScope, Transaction, Signer, HighPriorityAttribute, OracleResponse
from neo3.core import types, serialization

from pathlib import Path
from boilerplate_client.utils import bip44_path_from_string
from boilerplate_client.boilerplate_cmd_builder import InsType, BoilerplateCommandBuilder

CLA = BoilerplateCommandBuilder.CLA
bip44_path: str = "m/44'/888'/0'/0/0"
network_magic = 123  # actual value doesn't matter


class ParserStatus(enum.IntEnum):
    PARSING_OK = 1
    INVALID_LENGTH_ERROR = -1
    VERSION_PARSING_ERROR = -2
    VERSION_VALUE_ERROR = -3
    NONCE_PARSING_ERROR = -4
    SYSTEM_FEE_PARSING_ERROR = -5
    SYSTEM_FEE_VALUE_ERROR = -6
    NETWORK_FEE_PARSING_ERROR = -7
    NETWORK_FEE_VALUE_ERROR = -8
    VALID_UNTIL_BLOCK_PARSING_ERROR = -9
    SIGNER_LENGTH_PARSING_ERROR = -10
    SIGNER_LENGTH_VALUE_ERROR = -11
    SIGNER_ACCOUNT_PARSING_ERROR = -12
    SIGNER_ACCOUNT_DUPLICATE_ERROR = -13
    SIGNER_SCOPE_PARSING_ERROR = -14
    SIGNER_SCOPE_VALUE_ERROR_GLOBAL_FLAG = -15
    SIGNER_ALLOWED_CONTRACTS_LENGTH_PARSING_ERROR = -16
    SIGNER_ALLOWED_CONTRACTS_LENGTH_VALUE_ERROR = -17
    SIGNER_ALLOWED_CONTRACT_PARSING_ERROR = -18
    SIGNER_ALLOWED_GROUPS_LENGTH_PARSING_ERROR = -19
    SIGNER_ALLOWED_GROUPS_LENGTH_VALUE_ERROR = -20
    SIGNER_ALLOWED_GROUPS_PARSING_ERROR = -21
    ATTRIBUTES_LENGTH_PARSING_ERROR = -22
    ATTRIBUTES_LENGTH_VALUE_ERROR = -23
    ATTRIBUTES_UNSUPPORTED_TYPE = -24
    ATTRIBUTES_DUPLICATE_TYPE = -25
    SCRIPT_LENGTH_PARSING_ERROR = -26
    SCRIPT_LENGTH_VALUE_ERROR = -27
    SIGNER_SCOPE_GROUPS_NOT_ALLOWED_ERROR = -28
    SIGNER_SCOPE_CONTRACTS_NOT_ALLOWED_ERROR = -29


PARSER_RE = re.compile("\s+(?P<name>.*) = (?P<value>-?\d{1,2})")


def parse_parser_codes(path: Path) -> List[Tuple[str, int]]:
    if not path.is_file():
        raise FileNotFoundError(f"Can't find file: '{path}'")

    with open(str(path.absolute()), 'r') as f:
        lines = f.readlines()

    include = False

    results = []
    for line in lines:
        if "PARSING_OK" in line:
            include = True
        if "parser_status_e" in line:
            include = False

        if include:
            m = PARSER_RE.match(line)
            if m:
                results.append((m.group(1), int(m.group(2))))

    return results


def test_parser_codes(types_h_path):
    expected_parser_codes: List[Tuple[str, int]] = parse_parser_codes(types_h_path)
    for name, value in expected_parser_codes:
        assert name in ParserStatus.__members__
        assert value == ParserStatus.__members__[name].value, f"value mismatch for {name}"


def serialize(cla: int, ins: Union[int, enum.IntEnum], p1: int = 0, p2: int = 0, cdata: bytes = b"") -> bytes:
    ins = cast(int, ins.value) if isinstance(ins, enum.IntEnum) else cast(int, ins)
    header: bytes = struct.pack("BBBBB", cla, ins, p1, p2, len(cdata))
    return header + cdata


def send_bip44_and_magic(cmd):
    bip44_paths: List[bytes] = bip44_path_from_string(bip44_path)
    cdata: bytes = b"".join([*bip44_paths])

    cmd.transport.exchange_raw(serialize(cla=CLA,
                                         ins=InsType.INS_SIGN_TX,
                                         p1=0x00,
                                         p2=0x80,
                                         cdata=cdata))

    magic = struct.pack("I", network_magic)
    cmd.transport.exchange_raw(serialize(cla=CLA,
                                         ins=InsType.INS_SIGN_TX,
                                         p1=0x01,
                                         p2=0x80,
                                         cdata=magic))


def send_raw_tx_data(cmd, data: bytes):
    """
    This function makes the assumption the tx will always fit in 1 APDU
    and that an error code is returned due to deserialization failure
    """
    sw, response = cmd.transport.exchange_raw(serialize(cla=CLA,
                                                        ins=InsType.INS_SIGN_TX,
                                                        p1=0x02,  # seq number
                                                        p2=0x00,  # last apdu
                                                        cdata=data))
    assert sw == 0xB002  # SW_TX_PARSING_FAIL
    return sw, int.from_bytes(response, 'little', signed=True)


def test_invalid_version_value(cmd):
    send_bip44_and_magic(cmd)
    version = struct.pack("B", 1)  # version should be 0
    sw, error = send_raw_tx_data(cmd, version)
    assert error == ParserStatus.VERSION_VALUE_ERROR


def test_invalid_nonce_parsing(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00'  # a valid nonce would be 4 bytes
    sw, error = send_raw_tx_data(cmd, version + nonce)
    assert error == ParserStatus.NONCE_PARSING_ERROR


def test_system_fee_parsing(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = b'\x00'  # a valid system_fee would be 4 bytes
    sw, error = send_raw_tx_data(cmd, version + nonce + system_fee)
    assert error == ParserStatus.SYSTEM_FEE_PARSING_ERROR


def test_system_fee_value(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", -1)  # negative value not allowed. Yes NEO uses int64 not uint64 *shrug*
    sw, error = send_raw_tx_data(cmd, version + nonce + system_fee)
    assert error == ParserStatus.SYSTEM_FEE_VALUE_ERROR


def test_network_fee_parsing(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = b'\x00'  # a valid system_fee would be 4 bytes
    sw, error = send_raw_tx_data(cmd, version + nonce + system_fee + network_fee)
    assert error == ParserStatus.NETWORK_FEE_PARSING_ERROR


def test_network_fee_value(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", -1)  # negative value not allowed. Yes NEO uses int64 not uint64 *shrug*
    sw, error = send_raw_tx_data(cmd, version + nonce + system_fee + network_fee)
    assert error == ParserStatus.NETWORK_FEE_VALUE_ERROR


def test_valid_until_block(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00'  # a valid 'valid_until_block' would be 4 bytes
    sw, error = send_raw_tx_data(cmd, version + nonce + system_fee + network_fee + valid_until_block)
    assert error == ParserStatus.VALID_UNTIL_BLOCK_PARSING_ERROR


def test_signers_length(cmd):
    # by not adding a 'varint' to the data to indicate the signers length, we should fail to parse
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00' * 4
    sw, error = send_raw_tx_data(cmd, version + nonce + system_fee + network_fee + valid_until_block)
    assert error == ParserStatus.SIGNER_LENGTH_PARSING_ERROR


def test_signers_length2(cmd):
    # test signer length too large (3 vs max 2 allowed)
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00' * 4
    signer_length = b'\x03'  # max allowed is 2
    sw, error = send_raw_tx_data(cmd, version + nonce + system_fee + network_fee + valid_until_block + signer_length)
    assert error == ParserStatus.SIGNER_LENGTH_VALUE_ERROR


def test_signers_account(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00' * 4
    signer_length = b'\x01'
    # by providing no data for account it should fail to parse
    sw, error = send_raw_tx_data(cmd, version + nonce + system_fee + network_fee + valid_until_block + signer_length)
    assert error == ParserStatus.SIGNER_ACCOUNT_PARSING_ERROR


def test_signers_scope(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00' * 4
    signer_length = b'\x01'
    account = b'\x00' * 20  # UInt160
    # by providing no scope data it should fail to parse
    data = version + nonce + system_fee + network_fee + valid_until_block + signer_length + account
    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.SIGNER_SCOPE_PARSING_ERROR


def test_signers_scope_global(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00' * 4
    signer_length = b'\x01'
    account = b'\x00' * 20  # UInt160
    # a global scope is not allowed to be combined with other scopes
    scope = WitnessScope.GLOBAL | WitnessScope.CALLED_BY_ENTRY
    scope = scope.to_bytes(1, 'little')

    data = version + nonce + system_fee + network_fee + valid_until_block + signer_length + account + scope
    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.SIGNER_SCOPE_VALUE_ERROR_GLOBAL_FLAG


def test_signers_scope_contracts(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00' * 4
    signer_length = b'\x01'
    account = b'\x00' * 20  # UInt160

    scope = WitnessScope.CUSTOM_CONTRACTS
    scope = scope.to_bytes(1, 'little')

    contracts_count = b'\x11'

    data = version + nonce + system_fee + network_fee + valid_until_block + signer_length + account + scope + contracts_count
    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.SIGNER_ALLOWED_CONTRACTS_LENGTH_VALUE_ERROR


def test_signers_scope_contracts_no_data(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00' * 4
    signer_length = b'\x01'
    account = b'\x00' * 20  # UInt160

    scope = WitnessScope.CUSTOM_CONTRACTS
    scope = scope.to_bytes(1, 'little')

    contracts_count = b'\x01'
    # by not providing any actual contract data we should fail
    data = version + nonce + system_fee + network_fee + valid_until_block + signer_length + account + scope + contracts_count
    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.SIGNER_ALLOWED_CONTRACT_PARSING_ERROR


def test_signers_scope_groups(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00' * 4
    signer_length = b'\x01'
    account = b'\x00' * 20  # UInt160

    scope = WitnessScope.CUSTOM_GROUPS
    scope = scope.to_bytes(1, 'little')
    groups_count = b'\x03'

    data = version + nonce + system_fee + network_fee + valid_until_block + signer_length + account + scope + groups_count
    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.SIGNER_ALLOWED_GROUPS_LENGTH_VALUE_ERROR


def test_signers_scope_groups_no_data(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00' * 4
    signer_length = b'\x01'
    account = b'\x00' * 20  # UInt160

    scope = WitnessScope.CUSTOM_GROUPS
    scope = scope.to_bytes(1, 'little')
    groups_count = b'\x01'

    data = version + nonce + system_fee + network_fee + valid_until_block + signer_length + account + scope + groups_count
    # by not providing any actual group data we should fail
    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.SIGNER_ALLOWED_CONTRACT_PARSING_ERROR


def test_attributes(cmd):
    send_bip44_and_magic(cmd)
    version = b'\x00'
    nonce = b'\x00' * 4
    system_fee = struct.pack(">q", 0)
    network_fee = struct.pack(">q", 0)
    valid_until_block = b'\x00' * 4
    signer_length = b'\x01'
    account = b'\x00' * 20  # UInt160
    scope = WitnessScope.CALLED_BY_ENTRY
    scope = scope.to_bytes(1, 'little')

    # by not providing any attributes data it will fail the read a varint
    data = version + nonce + system_fee + network_fee + valid_until_block + signer_length + account + scope
    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.ATTRIBUTES_LENGTH_PARSING_ERROR


def test_attributes_value(cmd):
    send_bip44_and_magic(cmd)
    signer = Signer(account=types.UInt160.from_string("d7678dd97c000be3f33e9362e673101bac4ca654"),
                    scope=WitnessScope.CALLED_BY_ENTRY)
    # exceed max attributes count (2)
    attributes = [HighPriorityAttribute(), HighPriorityAttribute(), HighPriorityAttribute()]
    tx = Transaction(version=0, nonce=0, system_fee=0, network_fee=0, valid_until_block=1, signers=[signer],
                     attributes=attributes)

    with serialization.BinaryWriter() as br:
        tx.serialize_unsigned(br)
        data = br.to_array()

    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.ATTRIBUTES_LENGTH_VALUE_ERROR


def test_attributes_unsupported(cmd):
    send_bip44_and_magic(cmd)
    signer = Signer(account=types.UInt160.from_string("d7678dd97c000be3f33e9362e673101bac4ca654"),
                    scope=WitnessScope.CALLED_BY_ENTRY)
    # unsupported attribute
    attributes = [OracleResponse._serializable_init()]
    tx = Transaction(version=0, nonce=0, system_fee=0, network_fee=0, valid_until_block=1, signers=[signer],
                     attributes=attributes)

    with serialization.BinaryWriter() as br:
        tx.serialize_unsigned(br)
        data = br.to_array()

    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.ATTRIBUTES_UNSUPPORTED_TYPE


def test_attributes_duplicates(cmd):
    send_bip44_and_magic(cmd)
    signer = Signer(account=types.UInt160.from_string("d7678dd97c000be3f33e9362e673101bac4ca654"),
                    scope=WitnessScope.CALLED_BY_ENTRY)
    # duplicate attribute
    attributes = [HighPriorityAttribute(), HighPriorityAttribute()]
    tx = Transaction(version=0, nonce=0, system_fee=0, network_fee=0, valid_until_block=1, signers=[signer],
                     attributes=attributes)

    with serialization.BinaryWriter() as br:
        tx.serialize_unsigned(br)
        data = br.to_array()

    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.ATTRIBUTES_DUPLICATE_TYPE


def test_script(cmd):
    send_bip44_and_magic(cmd)
    signer = Signer(account=types.UInt160.from_string("d7678dd97c000be3f33e9362e673101bac4ca654"),
                    scope=WitnessScope.CALLED_BY_ENTRY)
    # no script is BAD
    tx = Transaction(version=0, nonce=0, system_fee=0, network_fee=0, valid_until_block=1, signers=[signer])

    with serialization.BinaryWriter() as br:
        tx.serialize_unsigned(br)
        data = br.to_array()

    sw, error = send_raw_tx_data(cmd, data)
    assert error == ParserStatus.SCRIPT_LENGTH_VALUE_ERROR
