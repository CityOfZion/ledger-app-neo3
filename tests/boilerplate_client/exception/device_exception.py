import enum
from typing import Dict, Any, Union

from .errors import *


class DeviceException(Exception):  # pylint: disable=too-few-public-methods
    exc: Dict[int, Any] = {

        0x6985: DenyError,
        0x6A86: WrongP1P2Error,
        0x6A87: WrongDataLengthError,
        0x6D00: InsNotSupportedError,
        0x6E00: ClaNotSupportedError,
        0xB000: WrongResponseLengthError,
        0xB001: WrongTxLengthError,
        0xB002: TxParsingFailError,
        0xB003: TxRejectSignError,
        0xB004: BadStateError,
        0xB005: SignatureFailError,
        0xB100: BIP44BadPurposeError,
        0xB101: BIP44BadCoinTypeError,
        0xB102: BIP44BadAccountNotHardenedError,
        0xB103: BIP44BadAccountError,
        0xB104: BIP44BadBadChangeError,
        0xB105: BIP44BadAddressError,
        0xB106: MagicParsingError,
        0xB107: DisplaySystemFeeFailError,
        0xB108: DisplayNetworkFeeFailError,
        0xB109: DisplayTotalFeeFailError,
        0xB10A: DisplayTransferAmountError,
        0xB200: ConvertToAddressFailError
    }

    def __new__(cls,
                error_code: int,
                ins: Union[int, enum.IntEnum, None] = None,
                message: str = ""
                ) -> Any:
        error_message: str = (f"Error in {ins!r} command"
                              if ins else "Error in command")

        if error_code in DeviceException.exc:
            return DeviceException.exc[error_code](hex(error_code),
                                                   error_message,
                                                   message)

        return UnknownDeviceError(hex(error_code), error_message, message)
