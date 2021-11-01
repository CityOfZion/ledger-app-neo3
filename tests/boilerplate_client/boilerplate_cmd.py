import struct
from typing import Tuple

from ledgercomm import Transport

from boilerplate_client.boilerplate_cmd_builder import BoilerplateCommandBuilder, InsType
from boilerplate_client.button import Button
from boilerplate_client.exception import DeviceException
from boilerplate_client.transaction import Transaction


class BoilerplateCommand:
    def __init__(self,
                 transport: Transport,
                 debug: bool = False) -> None:
        self.transport = transport
        self.builder = BoilerplateCommandBuilder(debug=debug)
        self.debug = debug

    def get_app_and_version(self) -> Tuple[str, str]:
        sw, response = self.transport.exchange_raw(
            self.builder.get_app_and_version()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=0x01)

        # response = format_id (1) ||
        #            app_name_len (1) ||
        #            app_name (var) ||
        #            version_len (1) ||
        #            version (var) ||
        offset: int = 0

        format_id: int = response[offset]
        offset += 1
        app_name_len: int = response[offset]
        offset += 1
        app_name: str = response[offset:offset + app_name_len].decode("ascii")
        offset += app_name_len
        version_len: int = response[offset]
        offset += 1
        version: str = response[offset:offset + version_len].decode("ascii")
        offset += version_len

        return app_name, version

    def get_version(self) -> Tuple[int, int, int]:
        sw, response = self.transport.exchange_raw(
            self.builder.get_version()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_VERSION)

        # response = MAJOR (1) || MINOR (1) || PATCH (1)
        assert len(response) == 3

        major, minor, patch = struct.unpack(
            "BBB",
            response
        )  # type: int, int, int

        return major, minor, patch

    def get_app_name(self) -> str:
        sw, response = self.transport.exchange_raw(
            self.builder.get_app_name()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_APP_NAME)

        return response.decode("ascii")

    def get_public_key(self, bip44_path: str, display: bool = False) -> bytes:
        sw, response = self.transport.exchange_raw(
            self.builder.get_public_key(bip44_path=bip44_path)
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_PUBLIC_KEY)

        assert len(response) == 65 # 04 + 64 bytes of uncompressed key

        return response

    def sign_tx(self, bip44_path: str, transaction: Transaction, network_magic: int, button: Button) -> Tuple[int, bytes]:
        sw: int
        response: bytes = b""

        for is_last, chunk in self.builder.sign_tx(bip44_path=bip44_path,
                                                   transaction=transaction,
                                                   network_magic=network_magic):
            self.transport.send_raw(chunk)

            if is_last:
                # Review Transaction
                button.right_click()
                # Destination address
                button.right_click()
                button.right_click()
                button.right_click()
                # Token Amount
                button.right_click()
                # Target network
                button.right_click()
                # System fee
                button.right_click()
                # Network fee
                button.right_click()
                # Total fees
                button.right_click()
                # Valid until
                button.right_click()
                # Signer 1 of 1
                button.right_click()
                # Account 1/3, 2/3, 3/3
                button.right_click()
                button.right_click()
                button.right_click()
                # Scope
                button.right_click()
                # Approve
                button.both_click()

            sw, response = self.transport.recv()  # type: int, bytes

            if sw != 0x9000:
                raise DeviceException(error_code=sw, ins=InsType.INS_SIGN_TX)

        return response

    def sign_vote_tx(self, bip44_path: str, transaction: Transaction, network_magic: int, button: Button) -> Tuple[int, bytes]:
        sw: int
        response: bytes = b""

        for is_last, chunk in self.builder.sign_tx(bip44_path=bip44_path,
                                                   transaction=transaction,
                                                   network_magic=network_magic):
            self.transport.send_raw(chunk)

            if is_last:
                # Review Transaction
                button.right_click()
                # Vote to public key
                button.right_click()
                button.right_click()
                button.right_click()
                button.right_click()
                # Target network
                button.right_click()
                # System fee
                button.right_click()
                # Network fee
                button.right_click()
                # Total fees
                button.right_click()
                # Valid until
                button.right_click()
                # Signer 1 of 1
                button.right_click()
                # Account 1/3, 2/3, 3/3
                button.right_click()
                button.right_click()
                button.right_click()
                # Scope
                button.right_click()
                # Approve
                button.both_click()

            sw, response = self.transport.recv()  # type: int, bytes

            if sw != 0x9000:
                raise DeviceException(error_code=sw, ins=InsType.INS_SIGN_TX)

        return response