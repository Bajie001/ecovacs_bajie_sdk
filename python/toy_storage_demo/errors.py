from typing import Optional


class ToyStorageError(Exception):
    """Base error for toy storage workflow."""


class ValidationError(ToyStorageError):
    """Raised when workflow inputs or intermediate data are invalid."""


class RobotMissionError(ToyStorageError):
    """Raised when a robot mission fails."""

    def __init__(self, message: str, *, code: Optional[int] = None, msg: Optional[str] = None):
        super().__init__(message)
        self.code = code
        self.msg = msg

    @classmethod
    def from_error_info(cls, step: str, error_info) -> "RobotMissionError":
        return cls(
            f"{step} failed: code={error_info.code}, msg={error_info.msg}",
            code=error_info.code,
            msg=error_info.msg,
        )


class HttpServiceError(ToyStorageError):
    """Raised when HTTP perception service fails."""
