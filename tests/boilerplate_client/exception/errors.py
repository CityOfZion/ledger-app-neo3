class UnknownDeviceError(Exception):
    pass


class DenyError(Exception):
    pass


class WrongP1P2Error(Exception):
    pass


class WrongDataLengthError(Exception):
    pass


class InsNotSupportedError(Exception):
    pass


class ClaNotSupportedError(Exception):
    pass


class WrongResponseLengthError(Exception):
    pass


class DisplayAddressFailError(Exception):
    pass


class DisplayAmountFailError(Exception):
    pass


class WrongTxLengthError(Exception):
    pass


class TxParsingFailError(Exception):
    pass


class TxHashFail(Exception):
    pass


class BadStateError(Exception):
    pass


class SignatureFailError(Exception):
    pass


class TxRejectSignError(Exception):
    pass


class BIP44BadPurposeError(Exception):
    pass


class BIP44BadCoinTypeError(Exception):
    pass


class BIP44BadAccountNotHardenedError(Exception):
    pass


class BIP44BadAccountError(Exception):
    pass


class BIP44BadBadChangeError(Exception):
    pass


class BIP44BadAddressError(Exception):
    pass


class MagicParsingError(Exception):
    pass


class DisplaySystemFeeFailError(Exception):
    pass


class DisplayNetworkFeeFailError(Exception):
    pass


class DisplayTotalFeeFailError(Exception):
    pass


class DisplayTransferAmountError(Exception):
    pass


class ConvertToAddressFailError(Exception):
    pass
